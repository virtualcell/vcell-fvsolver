#!/usr/bin/env bash
#
# Pack the static solvers and the whole test suite into one tarball that can be
# copied to any Linux machine and run there.
#
# The bundle reproduces all 16 checks that `ctest` runs here:
#   1      libzippp's own suite            (bin/libzippp_static_test)
#   2      FiniteVolume smoke test         (smoke/smoke.py, needs h5py)
#   3-16   the gtest suite                 (bin/vcell_test)
#
# vcell_test used to be unshippable because CMake baked this machine's absolute
# paths into testResourceLocations.h. They are resolved at run time now, against
# $VCELL_TEST_RESOURCES, which run-tests.sh points at the bundled testFiles/.
#
#   ./make-test-bundle.sh                  # -> fvsolver-test-bundle-<arch>.tgz
#   ./make-test-bundle.sh /tmp/out.tgz     # explicit output path
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/build-static}"
OUT="${1:-${REPO_ROOT}/fvsolver-test-bundle-$(uname -m).tgz}"

for exe in FiniteVolume_x64 smoldyn_x64 vcell_test; do
	[ -x "${BUILD_DIR}/bin/${exe}" ] || {
		echo "ERROR: ${BUILD_DIR}/bin/${exe} not found -- run ./build-static-linux.sh first" >&2
		exit 1
	}
	if readelf -l "${BUILD_DIR}/bin/${exe}" | grep -q INTERP; then
		echo "ERROR: ${exe} is not statically linked; it will not run elsewhere" >&2
		exit 1
	fi
done

STAGE="$(mktemp -d)"
trap 'rm -rf "${STAGE}"' EXIT
PKG="${STAGE}/fvsolver-test-bundle"
mkdir -p "${PKG}/bin" "${PKG}/smoke" "${PKG}/testFiles"

cp "${BUILD_DIR}/bin/FiniteVolume_x64" \
   "${BUILD_DIR}/bin/smoldyn_x64" \
   "${BUILD_DIR}/bin/vcell_test" "${PKG}/bin/"
# libzippp builds its test under whichever name the static/shared switch picked.
cp "${BUILD_DIR}/bin/libzippp_static_test" "${PKG}/bin/libzippp_test" 2>/dev/null \
	|| cp "${BUILD_DIR}/bin/libzippp_test" "${PKG}/bin/libzippp_test" 2>/dev/null \
	|| echo "NOTE: libzippp test binary not found; bundle will skip check 1"

# Only tracked fixtures. A previous local run leaves generated .fvinput/.hdf5/
# .log files in these directories, and shipping those would have the test
# compare the target's output against output this machine produced.
git -C "${REPO_ROOT}" ls-files -z VCell/tests/smoke/ \
	| xargs -0 -I{} cp "${REPO_ROOT}/{}" "${PKG}/smoke/"

git -C "${REPO_ROOT}" ls-files -z VCell/tests/testFiles/input \
	| (cd "${REPO_ROOT}" && xargs -0 tar cf -) \
	| tar xf - -C "${PKG}/testFiles" --strip-components=3

cat > "${PKG}/run-tests.sh" <<'RUNNER'
#!/usr/bin/env bash
# Verify the bundled static solvers and run the full test suite on this machine.
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOGDIR="${HERE}/logs"; mkdir -p "${LOGDIR}"
fail=0
note() { printf '%-42s %s\n' "$1" "$2"; }

# Never let a step block: no stdin, and a wall-clock cap when `timeout` exists.
# </dev/null is load-bearing -- smoldyn_x64 with no arguments prompts "Enter
# name of configuration file:" and waits forever on an interactive terminal.
if command -v timeout >/dev/null 2>&1; then
	CAP() { timeout "${STEP_TIMEOUT:-900}" "$@"; }
else
	CAP() { "$@"; }
fi

echo "== host =="
uname -srm
(. /etc/os-release 2>/dev/null && echo "${PRETTY_NAME:-unknown distro}") || true
if command -v ldd >/dev/null 2>&1; then ldd --version 2>/dev/null | head -1; else echo "no ldd"; fi
command -v readelf >/dev/null 2>&1 || echo "NOTE: readelf not found (install binutils)"
echo

echo "== binaries are self-contained =="
for exe in FiniteVolume_x64 smoldyn_x64 vcell_test; do
	if ! command -v readelf >/dev/null 2>&1; then
		note "$exe" "skipped - no readelf"
	elif readelf -l "${HERE}/bin/${exe}" | grep -q INTERP; then
		note "$exe" "FAIL - wants a dynamic loader"; fail=1
	else
		note "$exe" "ok - no PT_INTERP"
	fi
done
echo

echo "== they start with an empty environment =="
for exe in FiniteVolume_x64 smoldyn_x64; do
	CAP env -i "${HERE}/bin/${exe}" </dev/null >/dev/null 2>&1
	rc=$?
	case "$rc" in
		124)            note "$exe" "FAIL - timed out (hung)"; fail=1 ;;
		126|127)        note "$exe" "FAIL - could not execute"; fail=1 ;;
		12[89]|13[0-9]) note "$exe" "FAIL - killed by signal $((rc - 128))"; fail=1 ;;
		*)              note "$exe" "ok - runs (exit $rc)" ;;
	esac
done
echo

echo "== test 1/16: libzippp =="
if [ -x "${HERE}/bin/libzippp_test" ]; then
	work="${LOGDIR}/libzippp-work"; rm -rf "$work"; mkdir -p "$work"
	if ( cd "$work" && CAP "${HERE}/bin/libzippp_test" </dev/null ) >"${LOGDIR}/libzippp.log" 2>&1; then
		note "libzippp_tests" "PASS"
	else
		note "libzippp_tests" "FAIL - see logs/libzippp.log"; fail=1
	fi
else
	note "libzippp_tests" "skipped - not bundled"
fi
echo

echo "== test 2/16: FiniteVolume smoke (HDF5 compared against reference) =="
if command -v python3 >/dev/null && python3 -c "import h5py, numpy" 2>/dev/null; then
	if ( cd "${HERE}/smoke" && CAP python3 smoke.py "${HERE}/bin/FiniteVolume_x64" </dev/null ) \
		>"${LOGDIR}/smoke.log" 2>&1; then
		note "FiniteVolume_x64_smoke" "PASS"
	else
		note "FiniteVolume_x64_smoke" "FAIL - see logs/smoke.log"; fail=1
	fi
else
	# Fallback when the target has no h5py: still a real end-to-end solver run,
	# just without the structural comparison.
	echo "   (python3 with h5py+numpy not found -- running solver without HDF5 diff)"
	prefix="${HERE}/smoke/SimID_1585623750_0_"
	sed "s|@BASE_FILE_NAME@|${prefix}|g" "${prefix}.fvinput.in" > "${prefix}.fvinput"
	if ( cd "${HERE}/smoke" && CAP "${HERE}/bin/FiniteVolume_x64" "${prefix}.fvinput" </dev/null ) \
		>"${LOGDIR}/smoke.log" 2>&1 && [ -s "${prefix}.hdf5" ]; then
		note "FiniteVolume_x64 (no HDF5 diff)" "PASS (weaker check)"
	else
		note "FiniteVolume_x64 (no HDF5 diff)" "FAIL - see logs/smoke.log"; fail=1
	fi
fi
echo

echo "== tests 3-16/16: gtest suite =="
# VCELL_TEST_RESOURCES redirects the compiled-in resource root at the bundled
# copy. It must be writable -- the FV and Smoldyn cases write output under it.
if [ ! -w "${HERE}/testFiles" ]; then
	note "vcell_test" "FAIL - ${HERE}/testFiles is not writable"; fail=1
else
	VCELL_TEST_RESOURCES="${HERE}/testFiles" \
		CAP "${HERE}/bin/vcell_test" </dev/null >"${LOGDIR}/vcell_test.log" 2>&1
	rc=$?
	summary="$(grep -E '^\[==========\] [0-9]+ tests? from .* ran' "${LOGDIR}/vcell_test.log" | tail -1)"
	passed="$(grep -cE '^\[       OK \]' "${LOGDIR}/vcell_test.log")"
	failed="$(grep -cE '^\[  FAILED  \] [A-Za-z]' "${LOGDIR}/vcell_test.log")"
	if [ "$rc" -eq 0 ]; then
		note "vcell_test" "PASS - ${passed} tests"
	elif [ "$rc" -eq 124 ]; then
		note "vcell_test" "FAIL - timed out (hung)"; fail=1
	elif [ "$rc" -gt 128 ]; then
		note "vcell_test" "FAIL - killed by signal $((rc - 128)) after ${passed} tests"; fail=1
	else
		note "vcell_test" "FAIL - ${passed} passed, see logs/vcell_test.log"; fail=1
		grep -E '^\[  FAILED  \] [A-Za-z]' "${LOGDIR}/vcell_test.log" | sed 's/^/     /'
	fi
	[ -n "$summary" ] && echo "     ${summary}"
fi

echo
if [ "$fail" -eq 0 ]; then
	echo "ALL CHECKS PASSED"
else
	echo "SOME CHECKS FAILED - logs are in ${LOGDIR}"
fi
exit "$fail"
RUNNER
chmod +x "${PKG}/run-tests.sh"

tar czf "${OUT}" -C "${STAGE}" fvsolver-test-bundle
echo "wrote ${OUT} ($(du -h "${OUT}" | cut -f1))"
echo
echo "On the target machine:"
echo "  tar xzf $(basename "${OUT}") && ./fvsolver-test-bundle/run-tests.sh"
