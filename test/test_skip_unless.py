import tubes


def test_skip_unless():
    tube = tubes.Count().skip_unless(tubes.Each([False, True, False, True]).to(bool))
    assert list(tube) == [1, 3]

def test_skip_unless_lambda():
    tube = tubes.Count().skip_unless(lambda x: x.lt(3).first(5))
    assert list(tube) == [0, 1, 2]
