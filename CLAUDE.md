# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

`vcell-fvsolver` is the Virtual Cell finite-volume reaction–diffusion–advection PDE solver. It is a C++17/C/Fortran codebase that ships in two forms from a single CMake build:

- **Standalone executables** `FiniteVolume_x64` and `smoldyn_x64` (consumed by the VCell modeling app).
- **Python wheel `pyvcell_fvsolver`** (built via scikit-build-core / pybind11), which exposes `version()` and `solve(fvInputFilename, vcgInputFilename, outputDir)` from `src/main.cpp`. Inputs are VCell `.fvinput` + `.vcg` files; outputs are `.log`, `.zip`, `.mesh`, `.meshmetrics`, `.hdf5`.

Top-level `CMakeLists.txt` has a hard switch between these two modes via `OPTION_TARGET_PYTHON_BINDING`. When that option is ON it forces several other options (messaging, docs, libtiff, iconv) off and pulls the project name/version from the scikit-build environment — pure CMake invocations must pass `-DOPTION_TARGET_PYTHON_BINDING=OFF` to build the executables.

## Build & test

Both build modes need a Fortran compiler (flang ≥ 21 in CI), a C/C++ compiler (clang), CMake ≥ 3.22, and Conan 2 to provide hdf5, libaec, libzip, zlib (see `conanfile.txt`). On macOS, libzip/libaec come from Homebrew rather than Conan and CMake calls `brew --prefix` directly.

The submodules (`vcell-messaging`, `vcell-expressionparser`, `extern/pybind11`, `libzippp`) must be initialized: clone with `--recurse-submodules` or run `git submodule update --init --recursive`.

### Native executables (Linux/macOS)

```bash
mkdir build && cd build
conan install .. --output-folder . --build=missing       # Linux/Windows; macOS skips this
source conanbuild.sh                                     # Linux/Windows only
cmake -B . -S .. -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \         # omit on macOS
  -DCMAKE_BUILD_TYPE=Release \
  -DLIBZIPPP_CMAKE_CONFIG_MODE=ON \                      # omit on macOS, see gotcha below
  -DOPTION_TARGET_PYTHON_BINDING=OFF \
  -DOPTION_TARGET_MESSAGING=OFF \
  -DOPTION_TARGET_SMOLDYN_SOLVER=ON \
  -DOPTION_TARGET_FV_SOLVER=ON \
  -DOPTION_TARGET_TESTS=ON
cmake --build . --config Release
ctest -VV                                                # runs gtest suite + smoke test
./bin/FiniteVolume_x64                                   # built executable
```

macOS prerequisites: `brew install hdf5 libzip libaec flang`, plus an up-to-date Command Line Tools install whose SDKs include the SDK matching your macOS major version (Homebrew flang's driver looks for `/Library/Developer/CommandLineTools/SDKs/MacOSX<N>.sdk` and the linker fails with `ld: library 'System' not found` if it's missing — `sudo rm -rf /Library/Developer/CommandLineTools && xcode-select --install` to refresh).

`Dockerfile` is the canonical reference for an Ubuntu 24.04 build (LLVM 21 + Conan 2.26 + Ninja). `.github/workflows/cd.yml` is the canonical reference for macOS / Windows / Linux CI builds and for which CMake flags are passed in each environment.

### Portable static Linux executables

`./build-static-linux.sh` produces `FiniteVolume_x64` and `smoldyn_x64` in
`build-static/bin` with **no dynamic section at all** — no glibc, no libstdc++,
no libgfortran, no HDF5/libzip/zlib. `ldd` reports "not a dynamic executable"
and the ELF asks only for a 3.2.0 kernel, so the same binary runs on any Linux
distribution regardless of its glibc vintage. `--test` also runs `ctest`,
`--clean` wipes the build tree first.

Three things about this path differ from the CI build and are deliberate:

- **GCC/gfortran, not clang/flang.** `-static` has to satisfy the Fortran, C++
  and C runtimes from static archives in one link. GCC is the only toolchain
  that ships all three (`libgfortran.a`, `libstdc++.a`, `libc.a`) on a stock
  distribution; a clang/libc++/flang static link additionally needs `libc++.a`,
  `libc++abi.a`, `libunwind.a` and flang's runtime archives, which most
  distributions do not package. `conan-profiles/Linux-{AMD64,ARM64}-static_profile.txt`
  carry the profile; the script overrides `compiler.version` with the GCC
  actually on `PATH` so the Conan-built dependencies match the final link.
- **`OPTION_STATICALLY_LINK=ON`** (top-level `CMakeLists.txt`) adds `-static
  -no-pie` and forces `CMAKE_POSITION_INDEPENDENT_CODE` off — CMP0083 would
  otherwise add `-pie` alongside `-static`. It is rejected on Apple and with
  `OPTION_TARGET_PYTHON_BINDING` (a Python extension must be a PIC `.so`).
- The linker emits `warning: Using 'dlopen'/'getaddrinfo'/'gethostbyname' in
  statically linked applications ...` for HDF5's plugin loader and OpenSSL's
  BIO. Both are unreachable here: the solver writes only built-in (deflate)
  filters and, with messaging off, never opens a socket. Turning
  `OPTION_TARGET_MESSAGING` on *does* make the static glibc's NSS a real
  problem, and the CMake warns about it at configure time.

### Dependencies come from `conanfile.py`, not `conanfile.txt`

The recipe is a `conanfile.py` (same shape as `vcell-ode` / `vcell-chombo`)
because `libaec` has to be a **macOS-only** requirement, which a `conanfile.txt`
cannot express. Nothing outside the `if (APPLE)` branch of the top-level
`CMakeLists.txt` references libaec, and its only upstream
(`gitlab.dkrz.de`) rate-limits hard enough to fail Linux builds outright.

It deliberately does **not** implement `layout()`: `Dockerfile` and
`.github/workflows/cd.yml` both run `conan install . --output-folder build`,
which puts `conan_toolchain.cmake` and `conanbuild.sh` directly in `build/`.
A `cmake_layout()` would move them to `build/generators/` and break both.

### Python wheel / bindings

`src/main.cpp` is the pybind11 module (`_core`), re-exported by
`src/pyvcell_fvsolver/__init__.py` as `version()` and `solve()`. It ships to
PyPI as `pyvcell_fvsolver`.

**The bare commands below only work if CMake can find HDF5/libzip/zlib.** Nothing
in the Python build runs Conan, so on a machine without those as system packages
you must hand it a toolchain — the top-level `CMakeLists.txt` picks
`CMAKE_TOOLCHAIN_FILE` up from the environment:

```bash
# one-time: produce the Conan toolchain (any build dir will do)
./build-static-linux.sh

# install into the repo's .venv, with pytest, and run the binding tests
CMAKE_TOOLCHAIN_FILE="$PWD/build-static/conan_toolchain.cmake" \
  .venv/bin/pip install '.[test]' \
    -C cmake.define.LIBZIPPP_CMAKE_CONFIG_MODE=ON \
    -C cmake.define.OPTION_TARGET_PYTHON_BINDING=ON \
    -C cmake.define.OPTION_TARGET_TESTS=OFF \
    -C cmake.define.CMAKE_C_COMPILER=gcc \
    -C cmake.define.CMAKE_CXX_COMPILER=g++ \
    -C cmake.define.CMAKE_Fortran_COMPILER=gfortran
.venv/bin/pytest tests/ -v
```

```bash
python -m build --wheel -o ./wheelhouse                            # produce wheel (same -C flags)
nox -s dev                                                         # writes a .venv with editable install + compile_commands.json
nox -s tests                                                       # pip install .[test] && pytest
pytest tests/test_basic.py::test_version_function                  # single test
```

`tests/test_basic.py` covers `version()` and drives a real `solve()` against the
same fixtures as `VCellTest.ExcerciseFV`, writing into pytest's `tmp_path`. The
solve case skips (rather than fails) when the source tree's
`VCell/tests/testFiles` is not reachable, since cibuildwheel runs the suite
against an installed wheel.

`pyproject.toml` enables `filterwarnings = ["error"]`, so any Python warning fails tests.

Two things that do not carry over from the native build:

- **The extension cannot be statically linked** — a CPython extension has to be a
  shared object. HDF5, libzip, zlib and Smoldyn are still linked in statically,
  but `libgfortran`, `libstdc++`, `libgcc_s` and libc stay dynamic. Wheel
  portability is therefore a separate problem, solved by building in a manylinux
  image and running `auditwheel repair` (see `cd.yml`), not by
  `OPTION_STATICALLY_LINK` — which the CMake rejects outright in this mode.
- **CI builds wheels with clang/flang** (`[tool.cibuildwheel.linux.environment]`),
  not the gcc/gfortran the static build uses. Both work, but they leave different
  runtime dependencies for auditwheel to bundle.

`nox -s lint` cannot currently run: it invokes `pre-commit run --all-files` and
there is no `.pre-commit-config.yaml` in the repo.

### Running the suite on another machine

`./make-test-bundle.sh` packs the static solvers, the gtest binary and the
fixtures into one tarball that reproduces all 16 ctest checks anywhere:

```bash
./make-test-bundle.sh                       # -> fvsolver-test-bundle-<arch>.tgz (25M)
scp fvsolver-test-bundle-*.tgz target:
ssh target 'tar xzf fvsolver-test-bundle-*.tgz && ./fvsolver-test-bundle/run-tests.sh'
```

Exit 0 means everything passed; per-test logs land in `fvsolver-test-bundle/logs/`.

This works because **`testResourceLocations.h` resolves its paths at run time**
now. It used to emit `constexpr` absolute paths from the configuring source
tree, which made `vcell_test` unrunnable anywhere else. Only the root is baked
in, and `$VCELL_TEST_RESOURCES` overrides it — point that at a copy of
`VCell/tests/testFiles` and the suite runs against it. The root has to be
**writable**: the FV and Smoldyn cases write their output underneath it.

Two things the runner works around, both of which cost real debugging time:

- **`smoldyn_x64` with no arguments prompts on stdin** ("Enter name of
  configuration file:") and blocks forever on an interactive terminal. Every
  invocation gets `</dev/null` plus a `timeout` cap. A harness that pipes stdin
  gets EOF and never sees this; an ssh session hangs.
- **`readelf` may be absent** on a minimal target. The static-link check reports
  `skipped - no readelf` rather than silently passing on an empty grep.

The smoke test degrades gracefully: with `h5py` + `numpy` on the target it diffs
the HDF5 structure against the reference, without them it still runs the solver
end to end and checks the output file was produced.

### Running individual C++ tests

`OPTION_TARGET_TESTS=ON` builds `vcell_test` (googletest, fetched via `FetchContent` in `VCell/tests/CMakeLists.txt`) and registers it with CTest via `gtest_discover_tests`.

```bash
ctest -VV -R VCellTest                          # filter by test name regex
./VCell/tests/vcell_test --gtest_filter=VCellTest.ExcerciseFV
```

The smoke test (`VCell/tests/smoke/smoke.py`) runs `FiniteVolume_x64` against a templated `.fvinput.in` and diffs the resulting HDF5 structure against `*.hdf5.expected`. It needs `h5py` and `numpy` from whichever `python3` is on `PATH` when `ctest` runs. The repo has a `.venv/` for this; install the deps once and put it on PATH:

```bash
.venv/bin/python3 -m pip install h5py numpy
PATH="$(git rev-parse --show-toplevel)/.venv/bin:$PATH" ctest      # or: source .venv/bin/activate
```

`VCell/tests/CMakeLists.txt` registers the test with the literal command `python3 …/smoke.py`, so the test only resolves whichever `python3` is on `PATH` at ctest time — no rebuild is needed when you swap Python environments.

The `gtest` suite uses an auto-generated header `VCell/tests/testFiles/input/testResourceLocations.h` (configured from `.h.in`); regenerate it by re-running CMake configure if you add new test inputs.

## Architecture

### Module layout (build-order-meaningful)

The top-level CMake adds subdirectories in this order, which encodes the dependency graph:

1. **`vcell-messaging`** (submodule, target `vcellmessaging`), **`VCellZipUtils`**, **`libzippp`** — thin utility libs (curl transport is compiled out unless `OPTION_TARGET_MESSAGING=ON`, but `MessageEventManager`'s worker thread is always built).
2. **`vcell-expressionparser`** (submodule, target `vcellexpressionparser`) — JJTree-generated expression parser (see its `Parser.jjt`); produces an AST + `StackMachine` evaluator used everywhere by `*VarContextExpression` classes.
3. **`sundials`**, **`blas`** (skipped on Apple — uses the system Accelerate framework instead), **`PCGPack`**, **`qhull`** — vendored numerical libraries. PCG (preconditioned conjugate gradient) is wrapped through a Fortran shim (`VCell/src/pcgwrapper.f`) so the C++ sparse solver can call into it.
4. **`smoldyn-2.38`** — vendored Smoldyn particle simulator. Built with VCell-specific defines (`-DVCELL_HYBRID -DVCELL`).
5. **`bridgeVCellSmoldyn`** — adapters that let the FV solver and Smoldyn share a mesh/value source (`SimpleMesh`/`VCellMesh`, `SimpleValueProvider`/`VCellValueProvider`) and that emit Smoldyn output through the FV `DataGenerator`/HDF5 pipeline. Builds the `vcellsmoldynbridge` static lib and the `smoldyn_x64` executable.
6. **`VCell`** — the FV solver core. Builds the `vcell` static lib (everything else links against it) and the `FiniteVolume_x64` executable from `VCell/src/FiniteVolume.cpp`.

When the Python target is on, `src/main.cpp` is built into the `_core` pybind11 extension that links against `vcell` and is installed into `pyvcell_fvsolver/`.

### VCell solver core (`VCell/src` + `VCell/include/VCELL`)

Roughly 70 `.cpp` files; the conceptual layers are:

- **Model description**: `VCellModel` aggregates `Feature` (volume subdomains), `Membrane`, and `Structure`. Each carries `VarContext` subclasses (`VolumeVarContextExpression`, `MembraneVarContextExpression`, `*RegionVarContextExpression`, `MembraneRegionVarContextExpression`) that bind named expressions (initial conditions, diffusion, advection, reaction, boundary/jump) to symbol tables resolved at runtime by `vcell-expressionparser`.
- **Mesh & geometry**: `Mesh` / `CartesianMesh` hold `VolumeRegion`, `MembraneRegion`, and `MembraneElement` connectivity. `FieldData` and `RandomVariable` provide spatially varying inputs.
- **Variables**: `Variable` hierarchy → `VolumeVariable`, `MembraneVariable`, region variants, plus `*ParticleVariable` for hybrid Smoldyn coupling. `RegionSizeVariable` is computed, not solved.
- **Equation builders → solvers**: `EqnBuilder` subclasses (`SparseVolumeEqnBuilder`, `EllipticVolumeEqnBuilder`, `MembraneEqnBuilderDiffusion`, `*RegionEqnBuilder`, `EqnBuilderReactionForward`) populate `SparseMatrixPCG` matrices that the `PDESolver` (`SparseLinearSolver` via PCG, or `StructuredPDESolver` for ADI) drives. Stiff problems use `SundialsPdeScheduler` (CVODE); non-stiff use the operator-splitting `SplitScheduler`/`SerialScheduler`. ODE-only problems go through `ODESolver`. Algebraic constraints are handled by `FastSystem` / `FastSystemExpression` (Newton on a per-point reduced system, used for fast-equilibrium reactions).
- **Driver**: `SimTool` is the top-level orchestrator. `FVSolver` parses `.fvinput`/`.vcg`, builds the model, picks a scheduler, and drives stepping. `SimulationExpression` couples `Simulation` with the expression-based variable contexts. Per-timestep output flows through `PostProcessingBlock` → `PostProcessingHdf5Writer` and the `DataGenerator` family (`Gaussian`/`Projection`/`Roi`/`VariableStatistics`).
- **Entrypoints**: `SolverMain.h` exposes `version()` + `solve()` for both the CLI (`FiniteVolume.cpp`) and the Python module (`src/main.cpp`).

### Hybrid FV/Smoldyn coupling

`bridgeVCellSmoldyn/vcellhybrid.{h,cpp}` flips Smoldyn into a "hybrid" mode (`vcellhybrid::setHybrid()` is called at the top of `FiniteVolume.cpp` `main()` to put the linked Smoldyn library in the correct state before any FV setup runs). The bridge layer lets Smoldyn read concentrations from the FV solver via `VCellValueProvider` and write particle output through the FV HDF5 writer (`SmoldynHdf5Writer`).

### Static linking convention

`CMAKE_FIND_LIBRARY_SUFFIXES` is forced to `.a` and `BUILD_SHARED_LIBS=OFF` at the top of the top-level `CMakeLists.txt` — both the standalone executables and the Python extension bundle dependencies statically. CI then runs platform-specific scripts (`.github/scripts/install_name_tool_macos.sh`, `ldd`-walking on Linux, `lipo` for universal Mac binaries) to pull system libs into the artifact. If you add a dependency, plan on a static build.

## Shared submodules

`vcell-messaging` and `vcell-expressionparser` are git submodules shared with
`vcell-ode`, `vcell-chombo`, `vcell-stochastic` and `vcell-mbsolver`. They
replaced the in-tree `VCellMessaging/` and `ExpressionParser/` directories.

**A fix made here has to go upstream to `virtualcell/<name>` before this repo
can be cloned by anyone else** — bumping a gitlink to a commit that only exists
locally produces a checkout nobody can resolve.

Both require C++20 (`std::format`), which is why the top-level
`CMAKE_CXX_STANDARD` is 20 and every Conan profile sets `compiler.cppstd=20`,
even though the solver's own code is C++17-era.

### What the swap changed in this repo

- **`SimulationMessaging` is the 2.0 API.** `create()` and `start()` are gone —
  `getInstVar()` builds the singleton lazily and `MessageEventManager` owns the
  worker thread. `setWorkerEvent(new WorkerEvent(JOB_X, ...))` became
  `setWorkerEvent(JobEvent::JOB_X, ...)`, `delete SimulationMessaging::getInstVar()`
  became `cleanupInstanceVar()` (the destructor is protected now), and the JMS
  `create(broker, user, pwd, queue, topic, ...)` became
  `initialize_curl_messaging(alsoPrintToStdOut, broker, vcusername, simKey, jobIndex, taskID)`.
  The `.fvinput` `JMS_PARAM_BEGIN` block is unchanged and still parsed in full;
  the curl transport simply ignores the queue/topic/password fields.
- **`getInstVar()` never returns null.** The `if (getInstVar() == nullptr)`
  guards in `vcellExit` were dead code and are gone. Behaviour is preserved:
  in stdout mode `sendStdOutStatus()` writes a `JOB_FAILURE` message to `cerr`,
  which is what the old null branch did by hand.
- **Status messages are now asynchronous.** They are queued and drained by
  `MessageEventManager`'s thread, so lines like `initializing mesh` no longer
  appear at a fixed point in the solver's own stdout.
- **`using namespace std;` is gone from `VCELL/SimulationMessaging.h`** (as is
  its `<memory.h>`). Four files leaned on that leak and now declare what they
  use: `FVSolver.cpp`, `SimTool.cpp`, `FiniteVolume.cpp`, `vcellcmd.cpp`.
- **C++20 deleted `operator>>(istream&, char*)`.** `FVSolver::loadJMSInfo` used
  it against six `new char[256]` buffers; those are `std::string` now.

## Conventions / gotchas

- macOS skips Conan entirely and uses Homebrew for `libzip`, `libaec`, `hdf5`, `boost`, `flang`. Do not assume `conan_toolchain.cmake` exists in macOS builds. macOS also skips the bundled `blas` directory and uses the system Accelerate framework.
- **`-DLIBZIPPP_CMAKE_CONFIG_MODE=ON` breaks local macOS builds** even though CI passes it. The top-level `find_package(LIBZIP)` uses the bundled `libzippp/cmake/FindLIBZIP.cmake` (module mode) which defines `libzip::zip`; with that flag set, `libzippp/CMakeLists.txt` *additionally* calls `find_package(libzip CONFIG)`, which loads Homebrew's `libzip-config.cmake` and tries to import `libzip::zipcmp`/`zipmerge`/`ziptool`, hitting a "Some (but not all) targets in this export set were already defined" error. Newer Homebrew libzip exports more targets than older ones, so the flag works in some environments and breaks in others — **omit it on macOS**, keep it on Linux/Windows where Conan provides libzip.
- The default `OPTION_TARGET_PYTHON_BINDING=ON` only works when invoked through `pip` / `scikit-build-core`, because the `project(${SKBUILD_PROJECT_NAME} ...)` call needs scikit-build to inject those vars. A bare `cmake ..` with the default silently configures with project name `"VERSION"` and `OPTION_TARGET_FV_SOLVER=OFF` — always pass `-DOPTION_TARGET_PYTHON_BINDING=OFF` for native builds.
- `OPTION_VCELL` and `VCELL_HYBRID` defines are required for the Smoldyn build to behave as the hybrid FV/particle solver rather than vanilla Smoldyn.
- `FORTRAN_UNDERSCORE` is added globally; the Fortran ↔ C boundary in `pcgwrapper.f` depends on it.
- `vcell-expressionparser/Parser.jjt` is the source of the JavaCC/JJTree-generated parser files in that submodule. The generated `.cpp`/`.h` files are checked in — regenerate them deliberately rather than editing the generated files by hand, and do it upstream (see **Shared submodules** below).
- The `cmake-build-debug-without-binding-with-msg/` directory is a CLion build tree, not source — don't edit files inside it.
