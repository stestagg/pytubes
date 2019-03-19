import tubes

def test_split():
    tube = tubes.Each(['a\tb\tc\t', 'd\tef']).to(bytes).split('\t').to(str)
    assert list(tube) == ['a', 'b', 'c', 'd', 'ef']


def test_split_no_terminators():
    tube = tubes.Each(['abc', 'def', 'ghi', 'jkl']).to(bytes).split('\t').to(str)
    assert list(tube) == ['abcdefghijkl']


def test_split_across_boundaries():
    tube = tubes.Each(['ab|c', 'd|ef|', 'gh|ij']).to(bytes).split('|').to(str)
    assert list(tube) == ['ab', 'cd', 'ef', 'gh', 'ij']


def test_split_on_first_char():
    tube = tubes.Each(['|ab|c', '|d|ef|', '|g']).to(bytes).split('|').to(str)
    assert list(tube) == ['', 'ab', 'c', 'd', 'ef', '', 'g']


def test_empty_splits():
    tube = tubes.Each(['|||', '|||', '|']).to(bytes).split('|').to(str)
    assert list(tube) == [''] * 8

def test_skipping_empty_splits():
    tube = tubes.Each(['||a|', '|||b', '|']).to(bytes).split('|', skip_empty=True).to(str)
    assert list(tube) == ['a', 'b']


def test_split_across_three():
    tube = tubes.Each(['a|bc', 'def', 'g|h']).to(bytes).split('|').to(str)
    assert list(tube) == ['a', 'bcdefg', 'h']