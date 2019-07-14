import tubes

def test_skip_if():
    tube = tubes.Count().first(5).skip_if(tubes.Each([False, True, False, True]).to(bool))
    assert list(tube) == [0, 2]


def test_skip_if_lambda():
    tube = tubes.Count().first(6).skip_if(lambda x: x.lt(3))
    assert list(tube) == [3, 4, 5]
