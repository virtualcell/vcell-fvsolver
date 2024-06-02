from __future__ import annotations

import pyvcell_fvsolver as fv


def test_version():
    assert fv.__version__ == "0.0.1"


def test_add():
    assert fv.add(1, 2) == 3


def test_sub():
    assert fv.subtract(1, 2) == -1
