from __future__ import annotations

from pathlib import Path

import pytest

import pyvcell_fvsolver as fv

# The same inputs VCellTest.ExcerciseFV drives the C++ solver with. They are
# referenced in place rather than copied: the .fvinput names its siblings
# relatively (BASE_FILE_NAME, VCG_FILE) and solve() writes only under outputDir,
# so nothing here mutates the source tree.
REPO_ROOT = Path(__file__).parent.parent
FV_INPUT_DIR = REPO_ROOT / "VCell/tests/testFiles/input/FVSolver"
FV_INPUT = FV_INPUT_DIR / "SimID_11538992_0_.fvinput"
VCG_INPUT = FV_INPUT_DIR / "SimID_11538992_0_.vcg"

# What VCell expects a finished run to leave behind.
EXPECTED_OUTPUTS = frozenset(
    {
        "SimID_11538992_0_.log",
        "SimID_11538992_0_.mesh",
        "SimID_11538992_0_.meshmetrics",
        "SimID_11538992_0_.hdf5",
        "SimID_11538992_0_00.zip",
    }
)


def test_version_function() -> None:
    assert fv.version() is not None


@pytest.mark.skipif(
    not (FV_INPUT.is_file() and VCG_INPUT.is_file()),
    reason=f"solver input fixtures not found under {FV_INPUT_DIR}",
)
def test_solve_runs_a_simulation(tmp_path: Path) -> None:
    """Drive a real PDE solve through the binding, not just the version string.

    This is the only Python-side coverage of solve(); without it the extension
    could export a symbol that segfaults the moment it is called and the suite
    would still pass.
    """
    output_dir = tmp_path / "out"

    # solve() creates outputDir as needed, so it deliberately does not exist yet.
    return_code = fv.solve(str(FV_INPUT), str(VCG_INPUT), str(output_dir))
    assert return_code == 0

    produced = {p.name for p in output_dir.iterdir()}
    assert produced >= EXPECTED_OUTPUTS, f"missing: {sorted(EXPECTED_OUTPUTS - produced)}"

    # Guard against a run that exits 0 having written empty stubs.
    for name in sorted(EXPECTED_OUTPUTS):
        assert (output_dir / name).stat().st_size > 0, f"{name} is empty"
