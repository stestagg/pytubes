import pytest
import tubes

def test_group_id_increments_integer():
    tube = tubes.Each([3, 4, 4, 5]).to(int).group_id()
    assert list(tube) == [0, 1, 1, 2]

def test_group_id_increments_string():
    tube = tubes.Each(['3', '4', '4', '5']).to(int).group_id()
    assert list(tube) == [0, 1, 1, 2]

def test_group_id_increments_bool():
    tube = tubes.Each([False, True, False, True, True]).to(bool).group_id()
    assert list(tube) == [0, 1, 2, 3, 3]