<p align="center" width="100%">
 <a href="https://vcell.org">
    <img width="10%" src="https://github.com/biosimulations/biosimulations/blob/dev/docs/src/assets/images/about/partners/vcell.svg">
 </a>
</p>

---
![CI](https://github.com/virtualcell/vcell-solvers/actions/workflows/cd.yml/badge.svg)

# vcell-fvsolver
Virtual Cell Finite Volume solver [virtualcell/vcell-fvsolver](https://github.com/virtualcell/vcell-fvsolver) 
is a reaction-diffusion-advection PDE solver for computational cell biology. 
This solver is used within the Virtual Cell modeling and simulation application [virtualcell/vcell](https://github.com/virtualcell/vcell) 
and as a component in the Virtual Cell Python API [virtualcell/pyvcell](https://github.com/virtualcell/pyvcell) (coming soon).

## The Virtual Cell Project
The Virtual Cell is a modeling and simulation framework for computational biology.  For details see http://vcell.org and http://github.com/virtualcell.

## Docker container
the vcell-fvsolver is available as a docker container at ghcr.io/virtualcell/vcell-fvsolver.

## Standalone executables
`FiniteVolume_x64` and `smoldyn_x64` can be built on Windows, MacOS, and Linux
(see `.github/workflows/cd.yml` for the canonical CI recipe). Prebuilt executables
are available in the release section of this repository. To build them yourself,
see [Building from source](#building-from-source) below.

## Python API - pyvcell_fvsolver
The Python API for the VCell Finite Volume solver is a low level wrapper which 
accepts VCell solver input files (.fvinput, .vcg) 
and generates the output files (.log, .zip, .mesh, .meshmetrics, .hdf5).  The 
.functions file is not used by the solver, but is helpful for interpreting the 
results in the context of the original model.

This package is intended to be used by the Virtual Cell Python API [virtualcell/pyvcell](https://github.com/virtualcell/pyvcell) (coming soon).


---

## Building from source

### Prerequisites

| | |
|---|---|
| C / C++ compiler | GCC 13+ or Clang 19+ (must support C++20) |
| Fortran compiler | gfortran, or flang 21+ |
| CMake | 3.22 or newer |
| [Conan](https://conan.io) | 2.x — supplies HDF5, libzip and zlib |
| Ninja | recommended generator |

C++20 is required: the `vcell-messaging` and `vcell-expressionparser` submodules
use `std::format`.

**Check out the submodules.** The build will not configure without them:

```bash
git clone --recurse-submodules https://github.com/virtualcell/vcell-fvsolver.git
# or, in an existing clone:
git submodule update --init --recursive
```

| Submodule | Purpose |
|---|---|
| `vcell-messaging` | job status / progress reporting, shared with the other VCell solvers |
| `vcell-expressionparser` | the math expression parser and stack-machine evaluator |
| `libzippp` | C++ wrapper over libzip, used to roll up `.sim` output |
| `extern/pybind11` | only needed for the Python bindings |

### Portable static Linux executables (recommended)

One command. The result has **no dynamic section at all** — no glibc, no
libstdc++, no libgfortran, no HDF5 — so the same binary runs on any Linux
distribution regardless of its glibc vintage:

```bash
./build-static-linux.sh              # configure, build, verify
./build-static-linux.sh --test       # ...and run the test suite
./build-static-linux.sh --clean      # wipe the build tree first
```

Binaries land in `build-static/bin/`. Verify with `ldd build-static/bin/FiniteVolume_x64`,
which should report `not a dynamic executable`.

This path uses GCC/gfortran rather than Clang/flang, deliberately: `-static` has
to resolve the Fortran, C++ and C runtimes from static archives in one link, and
GCC is the only toolchain that ships all three on a stock distribution.

### Conventional (dynamically linked) build

What CI does. Useful on MacOS and Windows, where the fully static link is not
available:

```bash
conan install . --output-folder build --build=missing
cd build
source conanbuild.sh                                     # Linux/Windows only
cmake -B . -S .. -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DLIBZIPPP_CMAKE_CONFIG_MODE=ON \
  -DOPTION_TARGET_PYTHON_BINDING=OFF \
  -DOPTION_TARGET_MESSAGING=OFF \
  -DOPTION_TARGET_SMOLDYN_SOLVER=ON \
  -DOPTION_TARGET_FV_SOLVER=ON \
  -DOPTION_TARGET_TESTS=ON
cmake --build . --config Release
```

Two things that catch people out:

- `-DOPTION_TARGET_PYTHON_BINDING=OFF` is **required** for a native build. The
  default is ON, which only works when invoked through `pip`/scikit-build-core;
  a bare `cmake ..` with the default silently configures with the wrong project
  name and no solver targets.
- `-DLIBZIPPP_CMAKE_CONFIG_MODE=ON` is needed whenever libzip comes from Conan,
  but **breaks local MacOS builds** where libzip comes from Homebrew. Omit it there.

MacOS skips Conan entirely: `brew install hdf5 libzip libaec flang`.

### Python wheel

The bindings are built by scikit-build-core, not by the CMake invocations above.
Nothing in the Python build path runs Conan, so unless HDF5 and libzip are
installed system-wide you must hand CMake a toolchain:

```bash
./build-static-linux.sh          # one-time, to produce the Conan toolchain
python3 -m venv .venv            # one-time, if you do not already have one

CMAKE_TOOLCHAIN_FILE="$PWD/build-static/conan_toolchain.cmake" \
  .venv/bin/pip install '.[test]' \
    -C cmake.define.LIBZIPPP_CMAKE_CONFIG_MODE=ON \
    -C cmake.define.OPTION_TARGET_PYTHON_BINDING=ON \
    -C cmake.define.OPTION_TARGET_TESTS=OFF \
    -C cmake.define.CMAKE_C_COMPILER=gcc \
    -C cmake.define.CMAKE_CXX_COMPILER=g++ \
    -C cmake.define.CMAKE_Fortran_COMPILER=gfortran
```

Any virtual environment works — `.venv` is just the one this repo already uses
for the smoke test's `h5py`. Whichever you pick, **install and test from the
same interpreter**; see [Python bindings](#python-bindings) below.

Substitute `python -m build --wheel -o ./wheelhouse` for `pip install` to produce
a redistributable wheel. Note that the extension module cannot be statically
linked — a CPython extension has to be a shared object — so wheel portability is
handled by building inside a manylinux image and running `auditwheel repair`,
as `cd.yml` does.

## Running the tests

### On the build machine

The smoke test shells out to whichever `python3` is first on `PATH` and needs
`h5py` and `numpy` there, so **activate an environment that has them before
running `ctest`** — once per shell, not once per command:

```bash
python3 -m venv .venv && .venv/bin/pip install h5py numpy   # one-time

source .venv/bin/activate                                   # once per shell
cd build-static                                             # or your build dir
ctest
```

16 tests, about a minute:

| # | Test | Covers |
|---|---|---|
| 1 | `libzippp_tests` | libzippp's own suite |
| 2 | `FiniteVolume_x64_smoke` | runs the solver on a real `.fvinput`, diffs the HDF5 output against a reference |
| 3 | `VCellTest.ExcerciseFV` | the FV solver end to end |
| 4 | `SmoldynTest.ExerciseSmoldyn` | the hybrid FV/Smoldyn coupling (the slow one, ~30s) |
| 5–16 | `CHashtable*`, `CLinkedList*`, `CVector*` | container unit tests |

If you skip the activate step, test 2 fails like this — a test-environment
problem, not a solver failure:

```
2/16 Test #2: FiniteVolume_x64_smoke ...***Failed
smoke.py needs h5py and numpy, and /usr/bin/python3 does not have them.
```

No rebuild is needed when you change Python environments: CMake registers the
test with the literal command `python3`, which is resolved fresh on every
`ctest` run.

Useful variations (all assume the environment is active):

```bash
ctest -VV                                      # full output
ctest --output-on-failure                      # quiet unless something breaks
ctest -R VCellTest                             # filter by name regex
./bin/vcell_test --gtest_filter=CVectorTest.*  # drive gtest directly
```

Note that the tests write into the **source** tree (`VCell/tests/smoke/` and
`VCell/tests/testFiles/output/`), not the build tree. Most of it is gitignored.
A second run therefore exercises a different code path from the first — the
restart-from-existing-`.log` path — so running twice is worthwhile.

### On a different machine

`make-test-bundle.sh` packs the static solvers, the gtest binary and all fixtures
into one tarball that reproduces all 16 checks anywhere, with no toolchain, no
source tree and no `ctest` required on the target:

```bash
./make-test-bundle.sh                        # -> fvsolver-test-bundle-<arch>.tgz
scp fvsolver-test-bundle-*.tgz target:
ssh target 'tar xzf fvsolver-test-bundle-*.tgz && ./fvsolver-test-bundle/run-tests.sh'
```

Exit status 0 means everything passed; per-test logs are left in
`fvsolver-test-bundle/logs/`. The bundle directory must be **writable** — the FV
and Smoldyn cases write their output underneath it. If the target has `h5py` and
`numpy`, the smoke test compares HDF5 structure against the reference; if not, it
still runs the solver end to end and checks the output was produced.

### Python bindings

These require the binding to be installed first — see
[Python wheel](#python-wheel) above — and **must run from the same interpreter
you installed it into**:

```bash
.venv/bin/pytest tests/ -v          # or: source .venv/bin/activate && pytest tests/ -v
```

A bare `pytest tests/` uses whatever is first on `PATH`, which is usually the
system Python and will not have the module:

```
E   ModuleNotFoundError: No module named 'pyvcell_fvsolver'
```

The interpreter pytest is actually using is the path at the end of its own
header line, which is the quickest way to spot the mismatch:

```
platform linux -- Python 3.12.2, pytest-9.0.2, pluggy-1.6.0 -- /usr/local/bin/python3
                                                              ^^^^^^^^^^^^^^^^^^^^^^
```

That has to be the interpreter you ran `pip install` with.

The suite covers `version()` and drives a real `solve()` through the binding.
The solve case skips rather than fails when the source tree's
`VCell/tests/testFiles` is not reachable, since the wheel test runs against an
installed package.
