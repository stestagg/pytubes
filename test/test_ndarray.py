import pytest
import numpy as np

import tubes


def test_fill_ndarray():
    nd = tubes.Each(["abc", "def", "ghi"]).to(bytes).ndarray(2)
    assert [tuple(x) for x in nd] == [(b'ab',), (b'de',), (b'gh',)]


def test_fill_ndarray():
    nd = tubes.Count().first(100).to(int).ndarray()
    assert list(nd) == list(range(100))


if __name__ == '__main__':
    test_fill_ndarray()