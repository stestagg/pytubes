import pytest
import tubes

def test_single_index_get_on_json_value():
    tube = tubes.Each(["[]", "[1, 2, 3]"]).json().get(1, 'null')
    assert list(tube) == [None, 2]


def test_single_index_get_on_json_value():
    tube = tubes.Each(["[1,2,3]", "[8,9,10]", '["a", "b", "c"]']).json().multi(lambda x: (
        x.get(0),
        x.get(2),
        x.get(1),
        ))
    assert list(tube) == [(1, 3, 2), (8, 10, 9), ('a', 'c', 'b')]


def test_escaped_multi_index_get_on_json():
    tube = tubes.Each([
        r'["\t","\b","\u1234"]',
        r'["\"","","a"]',
        r'["x", "y\ta\bb\n", "z"]'
    ]).json().multi(lambda x: (
        x.get(0),
        x.get(1),
        x.get(2),
    )).to(str, str, str)
    assert list(tube) == [('\t', '\b', '\u1234'), ('"', '', 'a'), ('x', 'y\tx\bb\n', 'z')]