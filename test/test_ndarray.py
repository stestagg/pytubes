import pytest
import numpy as np
import string

import tubes


def test_fill_ndarray_chars():
    nd = tubes.Each(["abc", "def", "ghi"]).to(bytes).ndarray(2)
    assert nd.shape == (3, )
    assert list(nd) == [b'ab', b'de', b'gh']


def test_fill_ndarray_integers():
    nd = tubes.Count().first(100).to(int).ndarray(estimated_rows=2)
    assert nd.shape == (100, )
    assert list(nd) == list(range(100))


def test_fill_ndarray_many_chars():
    nd = tubes.Each([x * 10 for x in string.ascii_lowercase] * 10).to(bytes).ndarray(5, estimated_rows=2)
    assert nd.shape == (260, )
    assert list(nd) == list([x.encode('ascii') * 5 for x in string.ascii_lowercase] * 10)


def test_fill_ndarray_mixed_type():
    nd = (tubes.Each([x * 10 for x in string.ascii_lowercase])
        .to(bytes)
        .enumerate()
        .ndarray(None, 5)
    )
    assert nd.shape == (26, )
    assert dict(nd.dtype.fields) == {'0': (np.dtype('int64'), 0), '1': (np.dtype('S6'), 8)}
    expected = [(i, (x * 5).encode('ascii')) for i, x in enumerate(string.ascii_lowercase)]
    assert [tuple(x) for x in nd] == expected


def test_fill_ndarray_same_type():
    nd = tubes.Count(5).first(4).enumerate().ndarray()
    assert nd.shape == (4, 2)
    assert [tuple(x) for x in nd] == [(0, 5), (1, 6), (2, 7), (3, 8)]


def test_fill_ndarray_same_type_fields():
    nd = tubes.Count(5).first(4).enumerate().ndarray(fields=True)
    assert nd.shape == (4, )
    assert [tuple(x) for x in nd] == [(0, 5), (1, 6), (2, 7), (3, 8)]


def test_fill_ndarray_with_doubles():
    nd = tubes.Each([1., 2., 3.14, 4.5]).to(float).ndarray()
    assert list(nd) == [1.0, 2.0, 3.14, 4.5]


def test_fill_ndarray_with_bool():
    nd = tubes.Each([True, False, True]).to(bool).ndarray()
    assert nd.dtype == np.bool_
    assert list(nd) == [True, False, True]


def test_fill_ndarray_with_object():
    nd = tubes.Each([True, False, 'a', 12]).ndarray()
    assert nd.dtype == np.object_
    assert list(nd) == [True, False, 'a', 12]


if __name__ == '__main__':
    # test_fill_ndarray_chars()
    test_fill_ndarray_many_chars()