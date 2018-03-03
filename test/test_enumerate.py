import tubes

def test_enumerate():
    c = tubes.Count(2).enumerate().first(4)
    assert list(c) == [(0, 2), (1, 3), (2, 4), (3, 5)]

def test_enumerate_strings():
    c = tubes.Each(['a', 'b', 'c']).enumerate(1)
    assert list(c) == [(1, 'a'), (2, 'b'), (3, 'c')]


