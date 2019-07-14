import pytest
import tubes


def test_strlen_on_bytes():
    tube = tubes.Each(['', 'b', 'ba', 'ban', 'bana']).to(bytes).len()
    assert list(tube) == [0, 1, 2, 3, 4]


def test_strlen_on_simple_utf8():
    tube = tubes.Each(['', 'b', 'ba', 'ban', 'bana']).to(str).len()
    assert list(tube) == [0, 1, 2, 3, 4]


def test_strlen_on_complex_strs():
    vals = ['🧙', '😀', '🐳🌱', 'banana:🍌']
    base = tubes.Each(vals)
    assert list(base.to(str).len()) == [1, 1, 2, 8]
    assert list(base.to(bytes).len()) == [4, 4, 8, 11]


