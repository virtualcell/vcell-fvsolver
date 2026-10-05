"""The Python API the binding exposes (pyvcell imports `solve` and `version`).

The extension is built against the Python stable ABI (nanobind STABLE_ABI, abi3 for Python >= 3.12), so these
checks run unchanged on every CPython the one wheel installs into.
"""
import re

import pytest

import pyvcell_fvsolver as fv
from pyvcell_fvsolver import _core


def test_exports():
    assert set(fv.__all__) == {"__doc__", "__version__", "version", "solve"}
    assert fv.solve is _core.solve
    assert fv.version is _core.version


def test_version():
    assert isinstance(fv.__version__, str)
    assert fv.__version__ == "dev" or re.fullmatch(r"\d+\.\d+\.\d+.*", fv.__version__)
    assert isinstance(fv.version(), str)
    assert fv.version()


def test_docstrings():
    assert "VCell FiniteVolume solver" in fv.__doc__
    assert "fvinput" in fv.solve.__doc__


def test_missing_input_raises_with_the_cpp_message(tmp_path):
    missing = tmp_path / "missing.fvinput"
    with pytest.raises(RuntimeError, match=r"^Could not open input file: .*missing\.fvinput$"):
        fv.solve(str(missing), str(tmp_path / "missing.vcg"), str(tmp_path / "out"))


def test_missing_vcg_raises_with_the_cpp_message(tmp_path):
    fvinput = tmp_path / "empty.fvinput"
    fvinput.write_text("")
    with pytest.raises(RuntimeError, match=r"^Could not open vcg file: .*missing\.vcg$"):
        fv.solve(str(fvinput), str(tmp_path / "missing.vcg"), str(tmp_path / "out"))


def test_keyword_arguments(tmp_path):
    missing = tmp_path / "missing.fvinput"
    with pytest.raises(RuntimeError, match="Could not open input file"):
        fv.solve(fvInputFilename=str(missing), vcgInputFilename=str(tmp_path / "missing.vcg"),
                 outputDir=str(tmp_path / "out"))


def test_wrong_argument_types_raise_type_error():
    with pytest.raises(TypeError):
        fv.solve(1, 2, 3)
