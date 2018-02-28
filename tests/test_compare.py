import pytest

import tubes

def test_py_object_equal():
    assert list(tubes.Each([1, 'b', None]).equals(1))    == [True, False, False]
    assert list(tubes.Each([1, 'b', None]).equals('b'))  == [False, True, False]
    assert list(tubes.Each([1, 'b', None]).equals(None)) == [False, False, True]


def test_py_object_gt():
    with pytest.raises(TypeError):
        list(tubes.Each([1, 'b', None]).gt(1))
    assert list(tubes.Each([1, 2, 3]).gt(1))  == [False, True, True]
    assert list(tubes.Each(['a', 'b', 'c']).gt("apple"))  == [False, True, True]


@pytest.mark.parametrize("src, ty, value, dest", [
    ([True, False], bool, False, [False, True]),
    ([True, 1, 2, 10], int, 1, [True, True, False, False]),
    ([True, 1, 2, 10], int, 2, [False, False, True, False]),
    ([True, 1, 2, 10], int, 10, [False, False, False, True]),
    ([1, 1.5, 3.1415], float, 1, [True, False, False]),
    ([1, 1.5, 3.1415], float, 1.5, [False, True, False]),
    ([1, 1.5, 3.1415], float, 3.1415, [False, False, True]),
    (['baa', 'bab', 'ba'], bytes, 'baa', [True, False, False]),
    (['baa', 'bab', 'ba'], bytes, 'bab', [False, True, False]),
    (['baa', 'bab', 'ba'], bytes, 'ba', [False, False, True]),
    ([None], tubes.Null, None, [True]),
])
def test_equal(src, ty, value, dest):
    assert list(tubes.Each(src).to(ty).equals(value)) == dest


@pytest.mark.parametrize("src, ty, value, dest", [
    ([True, False], bool, False, [True, False]),
    ([True, 0, 2, 10], int, 0, [True, False, True, True]),
    ([True, 0, 2, 10], int, 1, [False, False, True, True]),
    ([True, 0, 2.5, 10], float, 1, [False, False, True, True]),
    ([True, 0, 2.5, 10], float, 2.3, [False, False, True, True]),
    ([True, 0, 2.5, 10], float, 2.6, [False, False, False, True]),
])
def test_gt(src, ty, value, dest):
    assert list(tubes.Each(src).to(ty).gt(value)) == dest


@pytest.mark.parametrize("src, ty, value, dest", [
    ([True, False], bool, True, [False, True]),
    ([True, 0, 2, 10], int, 0, [False, False, False, False]),
    ([True, 0, 2, 10], int, 1, [False, True, False, False]),
    ([True, 0, 2.5, 10], float, 1, [False, True, False, False]),
    ([True, 0, 2.5, 10], float, 2.3, [True, True, False, False]),
    ([True, 0, 2.5, 10], float, 2.6, [True, True, True, False]),
])
def test_lt(src, ty, value, dest):
    assert list(tubes.Each(src).to(ty).lt(value)) == dest