import itertools

import pytest

import tubes


def test_static_tube_takes_a_list():
    tube = tubes.Each([1, 2, 3])
    assert list(tube) == [1, 2, 3]


def test_static_tube_takes_an_iter():
    tube = tubes.Each(itertools.count(10)).first(3)
    assert list(tube) == [10, 11, 12]


def test_static_tube_with_strings():
    tube = tubes.Each(['a', 'b', 'c'])
    assert list(tube) == ['a', 'b', 'c']


def test_static_tube_with_strings():
    tube = tubes.Each(['a', 'b', 'c'])
    assert list(tube.to(str)) == ['a', 'b', 'c']
    assert list(tube.to(bytes)) == [b'a', b'b', b'c']


def test_static_tube_with_encoding():
    tube = tubes.Each(['£', '😃', ''])
    assert list(tube.to(str)) == ['£', '😃', '']
    assert list(tube.to(bytes)) == [b'\xc2\xa3', b'\xf0\x9f\x98\x83', b'']
    with pytest.raises(UnicodeEncodeError):
        list(tube.to(bytes, codec='ascii'))


