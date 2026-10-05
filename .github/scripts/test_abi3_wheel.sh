#!/usr/bin/env bash
# Check the wheel in ./wheelhouse is the one cp312-abi3 wheel for this platform, audit it against the
# stable ABI, then install that same file into Python 3.12 and 3.14 and run tests/ against it.
# Needs uv on PATH (astral-sh/setup-uv); uv fetches the interpreters.
set -euo pipefail

shopt -s nullglob
wheels=(wheelhouse/*.whl)
if [ "${#wheels[@]}" -ne 1 ]; then
  echo "::error::expected exactly one wheel in wheelhouse/, found ${#wheels[@]}: ${wheels[*]}"; exit 1
fi
wheel="${wheels[0]}"
case "$(basename "$wheel")" in
  *-cp312-abi3-*.whl) echo "wheel: $wheel" ;;
  *) echo "::error::$wheel is not tagged cp312-abi3"; exit 1 ;;
esac

uvx abi3audit --strict --report "$wheel"

for py in 3.12 3.14; do
  echo "------ Python $py ------"
  uv run --isolated --no-project --python "$py" --with "$wheel" --with pytest -- \
    python -c 'import sys, pyvcell_fvsolver as fv; print(sys.version); print(fv.__file__); print(fv.__version__, "|", fv.version())'
  uv run --isolated --no-project --python "$py" --with "$wheel" --with pytest -- \
    python -m pytest tests -p no:cacheprovider
done
