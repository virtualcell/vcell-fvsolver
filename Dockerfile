# syntax=docker/dockerfile:1.7
#
# vcell-fvsolver: FiniteVolume_x64 and smoldyn_x64, built once on a manylinux_2_28 base (glibc 2.28)
# and shipped two ways (see SOLVER-RELEASE.md):
#
#   --target archive   the Linux release archive root (linux64.tgz / linux64arm.tgz contents):
#                        docker buildx build --target archive --output type=local,dest=dist .
#   (default) runtime  the solver image, ghcr.io/virtualcell/vcell-fvsolver:<X.Y.Z> -- a slim distro
#                      base with that same archive on PATH and the standard VCell solver entrypoint:
#                        docker run --rm -v "$PWD:/simdata" vcell-fvsolver FiniteVolume_x64 /simdata/x.fvinput
#
# Built with VCell messaging ON: FiniteVolume_x64 accepts the trailing `-tid <n>` that VCell's
# SlurmProxy appends and posts its status to the broker named in the .fvinput JMS block.
#
# Multi-arch: TARGETARCH picks the matching manylinux image (build natively on each architecture;
# under QEMU this build takes hours).

ARG RUNTIME_BASE=ubuntu:24.04

FROM quay.io/pypa/manylinux_2_28_x86_64 AS manylinux-amd64
FROM quay.io/pypa/manylinux_2_28_aarch64 AS manylinux-arm64

# ---------------------------------------------------------------------------------------------
# build: gcc-toolset-14 (gcc, g++, gfortran), static conan dependencies, the test suite
# ---------------------------------------------------------------------------------------------
FROM manylinux-${TARGETARCH} AS build
ARG VERSION=0.0.0-dev
SHELL ["/bin/bash", "-euo", "pipefail", "-c"]

ENV PATH=/opt/python/cp312-cp312/bin:${PATH}
RUN python -m pip install --no-cache-dir conan==2.26.2 h5py numpy \
 && conan profile detect --force > /dev/null

COPY docker/conan /vcellroot/docker/conan
WORKDIR /vcellroot
RUN cp docker/conan/global.conf "$(conan config home)/global.conf" \
 && conan install docker/conan --output-folder build --build=missing \
      --profile:all docker/conan/linux-gcc.profile

COPY . /vcellroot
RUN source build/conanbuild.sh \
 && cmake -S . -B build -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE="$PWD/build/conan_toolchain.cmake" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_Fortran_COMPILER=gfortran \
      -DCMAKE_Fortran_FLAGS="-fallow-argument-mismatch" \
      -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc" \
      -DLIBZIPPP_CMAKE_CONFIG_MODE=ON \
      -DOPTION_TARGET_PYTHON_BINDING=OFF \
      -DOPTION_TARGET_MESSAGING=ON \
      -DOPTION_TARGET_SMOLDYN_SOLVER=ON \
      -DOPTION_TARGET_FV_SOLVER=ON \
      -DOPTION_TARGET_DOCS=OFF \
      -DOPTION_TARGET_TESTS=ON \
      -DVCELL_VERSION_STRING="${VERSION}" \
 && cmake --build build \
 && (cd build && ctest --output-on-failure)

RUN docker/package-linux.sh build /dist "${VERSION}"

# ---------------------------------------------------------------------------------------------
# archive: just the release archive root, for `--output type=local`
# ---------------------------------------------------------------------------------------------
FROM scratch AS archive
COPY --from=build /dist/ /

# ---------------------------------------------------------------------------------------------
# runtime: the solver image (and, via Apptainer, the SIF VCell's cluster runs)
# ---------------------------------------------------------------------------------------------
FROM ${RUNTIME_BASE} AS runtime
ARG VERSION=0.0.0-dev
LABEL org.opencontainers.image.source="https://github.com/virtualcell/vcell-fvsolver" \
      org.opencontainers.image.description="VCell FiniteVolume and Smoldyn solvers (messaging enabled)" \
      org.opencontainers.image.licenses="MIT" \
      org.opencontainers.image.version="${VERSION}"

COPY --from=build /dist/ /opt/vcell-fvsolver/
COPY docker/entrypoint.sh /usr/local/bin/vcell-solver-entrypoint
# Any uid, read-only root: the solvers write only into the directories their inputs name and
# $TMPDIR (FiniteVolume stages each .sim file there before zipping it).
ENV PATH=/opt/vcell-fvsolver:${PATH} \
    VCELL_SOLVER_DIR=/opt/vcell-fvsolver \
    HDF5_USE_FILE_LOCKING=FALSE

RUN chmod 0755 /usr/local/bin/vcell-solver-entrypoint \
 && vcell-solver-entrypoint --help

WORKDIR /tmp
ENTRYPOINT ["/usr/local/bin/vcell-solver-entrypoint"]
CMD ["--help"]
