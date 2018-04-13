import tubes

def test_csv_to_py():
    tube = tubes.Each(['a,b,c', 'd,ef']).to(tubes.CsvRow)
    assert list(tube) == [[b'a', b'b', b'c'], [b'd', b'ef']]

def test_empty_csv():
    tube = tubes.Each(['']).to(tubes.CsvRow)
    assert list(tube) == [[b'']]

def test_csv_one_row():
    tube = tubes.Each(['a,b,c']).to(tubes.CsvRow).get(1)
    assert list(tube) == [b'b']

def test_csv_two_rows():
    tube = tubes.Each(['a,b,c', 'd,e,f']).to(tubes.CsvRow).get(0)
    assert list(tube) == [b'a', b'd']

def test_csv_escaping():
    tube = tubes.Each(['a"x","b",""', '"d","e,f",g']).to(tubes.CsvRow).multi(lambda x: (
        x.get(0),
        x.get(1),
        x.get(2),
    ))
    assert list(tube) == [(b'a"x"', b'b', b''), (b'd', b'e,f', b'g')]

def test_csv_quote_escaping():
    tube = tubes.Each(['"a""b","""", """"""', '"c""""d",e""f']).to(tubes.CsvRow).multi(lambda x: (
        x.get(0, 'x'),
        x.get(1, 'x'),
        x.get(2, 'x'),
    ))
    print(">>>", list(tube))
    assert list(tube) == [(b'a"b"', b'"', b'""'), (b'c""d', b'e"f')]

def test_csv_uneven_rows():
    tube = tubes.Each(['a', 'b,c', 'd,e,', 'f,g,h']).to(tubes.CsvRow).get(2, 'xx')
    assert list(tube) == [b'xx', b'xx', b'', b'h']

def test_csv_uneven_rows_get_many():
    tube = tubes.Each(['a', 'b,c', 'd,e,', 'f,g,h']).to(tubes.CsvRow).multi(lambda x: (
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

def test_csv_multi():
    tube = tubes.Each(['a,b,c', 'd,e,f']).to(tubes.CsvRow).multi(lambda x:(
        x.get(0),
        x.get(1),
        x.get(2),
        x.get(3, 'xx')
    )).to(str, str, str, str)
    assert list(tube) == [('a', 'b', 'c', 'xx'), ('d', 'e', 'f', 'xx')]


def test_csv_headers_one_row():
    tube = tubes.Each(['a,b,c', 'd,e,f']).csv(headers=True).multi(lambda x:(
        x.get('a'),
        x.get(1),
        x.get(2),
        x.get('c')
    )).to(str, str, str, str)
    assert list(tube) == [('d', 'e', 'f', 'f')]


def test_reading_csv_headers_different_orders():
    tsv_1 = """a,b,c
1,2,3
4,5,6
"""
    tsv_2 = """c,a,b
9,7,8
12,10,11
"""
    tube = tubes.Each([tsv_1, tsv_2]).to(bytes).split().csv(headers=True).chunk(1).multi(lambda x:(
        x.get('a'),
        x.get('b'),
        x.get('c')
    )).to(int, int, int)
    assert list(tube) == [(1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12)]


def test_csv_non_comma_separator():
    tube = tubes.Each(['a|b|c', 'd|e,f|g']).to(bytes).csv(headers=False, sep='|')
    assert list(tube) == [[b'a', b'b', b'c'], [b'd', b'e,f', b'g']]



if __name__ == '__main__':
    test_csv_quote_escaping()