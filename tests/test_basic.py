import pyvcell_fvsolver as fv

def test_version():
    assert fv.__version__ == "0.0.1"

def test_version():
    assert fv.version() is not None
