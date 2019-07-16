import pytest
import itertools

import tubes


def test_chunking():
    tube = tubes.Each([1,2,3,4]).first(1).chunk(2)
    assert list(tube) == [1, 3]
    tube = tubes.Each([1,2,3,4]).skip(1).chunk(2)
    assert list(tube) == [2, 4]

def test_multiple_chunks():
    with pytest.raises(ValueError):
        list(tubes.Each([1,2,3,4]).zip(tubes.Each([1,2,3,4])).chunk(2))

def test_chunk_with_generator():
    with pytest.raises(ValueError):
        list(tubes.Each(itertools.count()).chunk(2))


def test_chunk_empty_input():
	tube = tubes.Each([]).chunk(2)
	with pytest.raises(ValueError):
		list(tube)


if __name__ == '__main__':
    test_chunking()