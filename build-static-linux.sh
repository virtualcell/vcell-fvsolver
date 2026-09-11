#!/usr/bin/env bash
#
# Build fully static FiniteVolume_x64 and smoldyn_x64 for Linux.
#
# The result has no dynamic section at all -- no glibc, no libstdc++, no
# libgfortran, no HDF5/libzip/zlib -- so the same binary runs on any Linux
# distribution with a 3.2.0-or-newer kernel, regardless of its glibc vintage.
#
# Requires: gcc/g++/gfortran, cmake >= 3.22, conan 2, git submodules checked out.
# Conan supplies hdf5, libzip and zlib (built from source as static archives).
#
#   ./build-static-linux.sh              # configure + build + verify
#   ./build-static-linux.sh --test       # also run ctest (needs h5py + numpy)
#   ./build-static-linux.sh --clean      # wipe the build tree first
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/build-static}"
RUN_TESTS=false
CLEAN=false

for arg in "$@"; do
	case "$arg" in
		--test)  RUN_TESTS=true ;;
		--clean) CLEAN=true ;;
		--help|-h) sed -n '2,20p' "${BASH_SOURCE[0]}" | sed 's/^# \?//'; exit 0 ;;
		*) echo "Unknown option: $arg (try --help)" >&2; exit 1 ;;
	esac
done

for tool in gcc g++ gfortran cmake conan git; do
	command -v "$tool" >/dev/null || { echo "ERROR: $tool not found on PATH" >&2; exit 1; }
done

# libzippp, vcell-messaging and vcell-expressionparser are submodules the
# build cannot proceed without.
for sub in libzippp vcell-messaging vcell-expressionparser; do
	if [ ! -f "${REPO_ROOT}/${sub}/CMakeLists.txt" ]; then
		echo "Submodule ${sub} is not checked out; running git submodule update --init --recursive"
		git -C "${REPO_ROOT}" submodule update --init --recursive
		break
	fi
done

case "$(uname -m)" in
	x86_64)          PROFILE=Linux-AMD64-static_profile.txt ;;
	aarch64|arm64)   PROFILE=Linux-ARM64-static_profile.txt ;;
	*) echo "ERROR: no static profile for $(uname -m)" >&2; exit 1 ;;
esac

# Tag the Conan dependencies with the GCC that will actually perform the final
# link. Mixing archives built against a different libstdc++ ABI into a static
# link is the one thing that turns a portable binary into a broken one.
GCC_MAJOR="$(gcc -dumpversion | cut -d. -f1)"

$CLEAN && rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

echo "==> conan install (profile ${PROFILE}, gcc ${GCC_MAJOR})"
conan install "${REPO_ROOT}" \
	--output-folder="${BUILD_DIR}" \
	-pr:a="${REPO_ROOT}/conan-profiles/${PROFILE}" \
	-s:a compiler.version="${GCC_MAJOR}" \
	--build=missing

cd "${BUILD_DIR}"
# Conan's generated env scripts reference variables they have not set yet, so
# they are not -u clean.
set +u
# shellcheck disable=SC1091
source ./conanbuild.sh
set -u

echo "==> cmake configure"
# LIBZIPPP_CMAKE_CONFIG_MODE is required whenever libzip comes from Conan:
# libzippp's bundled FindLIBZIP.cmake is pkg-config based and finds nothing in a
# Conan tree. (Do NOT pass it on macOS -- see CLAUDE.md.)
cmake -B . -S "${REPO_ROOT}" -G Ninja \
	-DCMAKE_TOOLCHAIN_FILE="${BUILD_DIR}/conan_toolchain.cmake" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=gcc \
	-DCMAKE_CXX_COMPILER=g++ \
	-DCMAKE_Fortran_COMPILER=gfortran \
	-DLIBZIPPP_CMAKE_CONFIG_MODE=ON \
	-DOPTION_STATICALLY_LINK=ON \
	-DOPTION_TARGET_PYTHON_BINDING=OFF \
	-DOPTION_TARGET_MESSAGING=OFF \
	-DOPTION_TARGET_SMOLDYN_SOLVER=ON \
	-DOPTION_TARGET_FV_SOLVER=ON \
	-DOPTION_TARGET_DOCS=OFF \
	-DOPTION_TARGET_TESTS=ON

echo "==> build"
cmake --build . --config Release

echo "==> verifying the executables really are static"
status=0
for exe in bin/FiniteVolume_x64 bin/smoldyn_x64; do
	if readelf -l "$exe" | grep -q INTERP; then
		echo "FAIL: $exe still has a program interpreter (not static)"
		readelf -d "$exe" | sed -n '1,20p'
		status=1
	else
		printf '  ok  %-24s %s\n' "$exe" "$(file -b "$exe" | cut -d, -f1-4)"
	fi
done
[ "$status" -eq 0 ] || exit "$status"

if $RUN_TESTS; then
	echo "==> ctest"
	# The smoke test shells out to whichever python3 is on PATH and needs
	# h5py + numpy there; the repo's .venv is the usual place for them.
	if [ -x "${REPO_ROOT}/.venv/bin/python3" ]; then
		PATH="${REPO_ROOT}/.venv/bin:${PATH}" ctest --output-on-failure
	else
		ctest --output-on-failure
	fi
fi

echo
echo "Static executables are in ${BUILD_DIR}/bin"
