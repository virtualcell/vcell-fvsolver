import pyvcell_fvsolver as fv


def test_version():
    assert fv.__version__ == "0.1.0"


def test_version_function():
    assert fv.version() is not None
