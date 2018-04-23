import tubes

def test_tsv_to_py():
    tube = tubes.Each(['a\tb\tc', 'd\tef']).to(tubes.TsvRow)
    assert list(tube) == [[b'a', b'b', b'c'], [b'd', b'ef']]

def test_empty_tsv():
    tube = tubes.Each(['']).to(tubes.CsvRow)
    assert list(tube) == [[b'']]

def test_tsv_one_row():
    tube = tubes.Each(['a\tb\tc']).to(tubes.TsvRow).get(1)
    assert list(tube) == [b'b']


def test_tsv_two_rows():
    tube = tubes.Each(['a\tb\tc', 'd\te\tf']).to(tubes.TsvRow).get(0)
    assert list(tube) == [b'a', b'd']


def test_tsv_uneven_rows():
    tube = tubes.Each(['a', 'b\tc', 'd\te\t', 'f\tg\th']).to(tubes.TsvRow).get(2, 'xx')
    assert list(tube) == [b'xx', b'xx', b'', b'h']

def test_tsv_uneven_rows_get_many():
    tube = tubes.Each(['a', 'b\tc', 'd\te\t', 'f\tg\th']).to(tubes.TsvRow).multi(lambda x: (
        x.get(0),
        x.get(1, 'xx'),
        x.get(2, 'xx'),
    )).to(str, str, str)
    assert list(tube) == [
        ('a', 'xx', 'xx'),
        ('b', 'c', 'xx'),
        ('d', 'e', ''),
        ('f', 'g', 'h'),
    ]

def test_tsv_multi():
    tube = tubes.Each(['a\tb\tc', 'd\te\tf']).to(tubes.TsvRow).multi(lambda x:(
        x.get(0),
        x.get(1),
        x.get(2),
        x.get(3, 'xx')
    )).to(str, str, str, str)
    assert list(tube) == [('a', 'b', 'c', 'xx'), ('d', 'e', 'f', 'xx')]


def test_tsv_headers_get_single():
    tube = tubes.Each(['a\tb\tc', 'd\te\tf']).tsv(headers=True, split=False).get('a').to(str)
    assert list(tube) == ['d']

def test_tsv_headers_one_row():
    tube = tubes.Each(['a\tb\tc', 'd\te\tf']).tsv(headers=True, split=False).multi(lambda x:(
        x.get('a'),
        x.get(1),
        x.get(2),
        x.get('c')
    )).to(str, str, str, str)
    assert list(tube) == [('d', 'e', 'f', 'f')]


def test_reading_tsv_headers_different_orders():
    tsv_1 = """a\tb\tc
1\t2\t3
4\t5\t6
"""
    tsv_2 = """c\ta\tb
9\t7\t8
12\t10\t11
"""
    tube = tubes.Each([tsv_1, tsv_2]).to(bytes).split().tsv(headers=True, split=False).chunk(1).multi(lambda x:(
        x.get('a'),
        x.get('b'),
        x.get('c')
    )).to(int, int, int)
    assert list(tube) == [(1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12)]


def test_tsv_non_tab_separator():
    tube = tubes.Each(['a|b|c\n', 'd|e\tf|g']).to(bytes).tsv(headers=False, sep='|')
    assert list(tube) == [[b'a', b'b', b'c'], [b'd', b'e\tf', b'g']]


if __name__ == '__main__':
    test_tsv_non_tab_separator()