import pytest

import tubes

def test_get_invalid_slot():
    base = tubes.Each([1])
    with pytest.raises(IndexError):
        base.slot(2)


def test_get_json():
    base = tubes.Each(['{"a": "b", "b": 1}']).json()
    assert list(base.get("a")) == ["b"]
    assert list(base.get("b")) == [1]


def test_get_missing_field_no_default():
    base = tubes.Each(['{"a": "b", "b": 1}']).json()
    with pytest.raises(KeyError):
        list(base.get("c"))