FROM ubuntu:24.04

SHELL ["/bin/bash", "-c"]
RUN apt-get clean && apt-get update
RUN apt-get install -y git lsb-release wget software-properties-common gnupg apt-utils python3 python3-pip cmake
RUN python3 -m pip install h5py numpy --break-system-packages


# LLVM Install (need version 21+ for flang)
RUN wget https://apt.llvm.org/llvm.sh
RUN python3 <<'EOF'
content = open('llvm.sh').read()
old = '    if test $LLVM_VERSION -gt 14; then\n        PKG="$PKG libclang-rt-$LLVM_VERSION-dev libpolly-$LLVM_VERSION-dev"\n    fi\nfi'
new = '    if test $LLVM_VERSION -gt 14; then\n        PKG="$PKG libclang-rt-$LLVM_VERSION-dev libpolly-$LLVM_VERSION-dev"\n    fi\n    if test $LLVM_VERSION -ge 16; then\n        PKG="$PKG flang-$LLVM_VERSION"\n    fi\nfi'
open('llvm.sh', 'w').write(content.replace(old, new, 1))
EOF
RUN chmod +x llvm.sh
RUN ./llvm.sh 21 all
RUN /usr/lib/llvm-21/bin/clang --version
RUN ln -s /usr/lib/llvm-21/bin/clang /usr/bin/clang
RUN /usr/lib/llvm-21/bin/clang++ --version
RUN ln -s /usr/lib/llvm-21/bin/clang++ /usr/bin/clang++
RUN /usr/lib/llvm-21/bin/flang --version
RUN ln -s /usr/lib/llvm-21/bin/flang /usr/bin/flang
RUN ln -fs /usr/bin/clang /usr/bin/cc
RUN rm /usr/bin/gcc
RUN rm /usr/bin/g++
#RUN export PATH="/usr/lib/llvm-21/bin:$PATH"
#RUN echo "PATH=/usr/lib/llvm-21/bin:$PATH" >> /etc/bash.bashrc
#RUN source /etc/bash.bashrc

# Conan install
RUN export arch_string=$([[ $(uname -m) == "x86_64" ]] && echo "amd64" || echo "arm64") && wget https://github.com/conan-io/conan/releases/download/2.26.2/conan-2.26.2-${arch_string}.deb
RUN apt-get install -y ./conan-*.deb
RUN conan --version
RUN mkdir -p ~/.conan2/profiles/
RUN touch ~/.conan2/profiles/default # if we don't make a file first, cp thinks its a folder that doesn't exist
COPY . /vcellroot
RUN export arch_string=$([[ $(uname -m) == "x86_64" ]] && echo "AMD64" || echo "ARM64") && cp /vcellroot/conan-profiles/CI-CD/Linux-${arch_string}_profile.txt ~/.conan2/profiles/default

# Fix broken dependencies and reconfigure ca-certificates
RUN apt-get -y update && apt-get install -f && dpkg --configure -a && apt-get clean

RUN mkdir -p /vcellroot/build/bin
WORKDIR /vcellroot

RUN CC=clang CXX=clang++ FC=flang conan install . --output-folder build --build=missing
WORKDIR /vcellroot/build
RUN source conanbuild.sh
RUN cmake -B . -S .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLIBZIPPP_CMAKE_CONFIG_MODE=ON \
  -DOPTION_TARGET_PYTHON_BINDING=OFF \
  -DOPTION_TARGET_MESSAGING=OFF \
  -DOPTION_TARGET_SMOLDYN_SOLVER=ON \
  -DOPTION_TARGET_FV_SOLVER=ON \
  -DOPTION_TARGET_DOCS=OFF \
  -DOPTION_TARGET_TESTS=ON
RUN cmake --build . --config Release
RUN ctest -VV
RUN ./bin/FiniteVolume_x64 || true
RUN ./bin/smoldyn_x64 || true
WORKDIR /vcellroot/build/bin
ENV PATH="/vcellroot/build/bin:${PATH}"
