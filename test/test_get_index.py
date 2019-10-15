import pytest
import tubes


def test_single_index_get_on_pyobj():
    tube = tubes.Each([[1,2], [], [8, 9]]).get(0, None)
    assert list(tube) == [1, None, 8]

def test_multi_index_get_on_pyobj():
    tube = tubes.Each([[1], [1, 1], [1, 2, 1], [1, 3, 3, 1], None]).multi(lambda x: tuple(x.get(i, 'X') for i in range(5)))
    assert list(tube) == [
        (1, 'X', 'X', 'X', 'X'),
        (1, 1, 'X', 'X', 'X'),
        (1, 2, 1, 'X', 'X'),
        (1, 3, 3, 1, 'X'),
        (None, 'X', 'X', 'X', 'X'),
    ]

def test_multi_index_pyobj_out_of_order():
    order = [4, 2, 0, 3, 0, 1]
    tube = tubes.Each([[1], [1, 1], [1, 2, 1], [1, 3, 3, 1], None]).multi(lambda x: tuple(x.get(i, 'X') for i in order))
    assert list(tube) == [
        ('X', 'X', 1, 'X', 1, 'X'),
        ('X', 'X', 1, 'X', 1, 1),
        ('X', 1, 1, 'X', 1, 2),
        ('X', 3, 1, 1, 1, 3),
        ('X', 'X', None, 'X', None, 'X'),
    ]

def test_multi_index_pyobj_weird_types():
    thing = object()
    tube = tubes.Each([
        [1], 
        'a', 
        4, 
        thing, 
        True
    ]).multi(lambda x: tuple(x.get(i, None) for i in range(2)))
    assert list(tube) == [
        (1, None),
        ('a', None),
        (4, None),
        (thing, None),
        (True, None),
    ]
    



def test_single_index_get_on_json_value():
    tube = tubes.Each(["[]", "[1, 2, 3]"]).json().get(1, 'null')
    assert list(tube) == [None, 2]


def test_multi_index_get_on_json_value():
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
    assert list(tube) == [('\t', '\b', '\u1234'), ('"', '', 'a'), ('x', 'y\ta\bb\n', 'z')]