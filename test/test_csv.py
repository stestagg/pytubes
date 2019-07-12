import pytest
import tubes

def test_csv_to_py():
    tube = tubes.Each(['a,b,c', 'd,ef']).to(tubes.CsvRow)
    assert list(tube) == [[b'a', b'b', b'c'], [b'd', b'ef']]

def test_empty_csv():
    tube = tubes.Each(['']).to(tubes.CsvRow)
    assert list(tube) == [[b'']]

def test_csv_two_values():
    tube = tubes.Each(['a,b']).to(tubes.CsvRow)
    assert list(tube) == [[b'a', b'b']]

def test_csv_trailing_comma():
    tube = tubes.Each(['a,']).to(tubes.CsvRow)
    assert list(tube) == [[b'a', b'']]

def test_csv_one_row():
    tube = tubes.Each(['a,b,c']).to(tubes.CsvRow).get(1, 'x')
    assert list(tube) == [b'b']

def test_csv_two_rows():
    tube = tubes.Each(['a,b,c', 'd,e,f']).to(tubes.CsvRow).get(0)
    assert list(tube) == [b'a', b'd']

def test_csv_escaping():
    tube = tubes.Each(['a"x","b",""', '"d","e,f",g']).to(tubes.CsvRow).multi(lambda x: (
        x.get(0, 'xx'),
        x.get(1, 'xx'),
        x.get(2, 'xx'),
    ))
    assert list(tube) == [(b'a"x"', b'b', b''), (b'd', b'e,f', b'g')]

def test_csv_quote_escaping():
    tube = tubes.Each(['"a""b","""",""""""', '"c""""d",e""f']).to(tubes.CsvRow).multi(lambda x: (
        x.get(0),
        x.get(1),
        x.get(2, 'x'),
    ))
    assert list(tube) == [(b'a"b', b'"', b'""'), (b'c""d', b'e""f', b'x')]

def test_csv_uneven_rows():
    tube = tubes.Each(['a', 'b,c', 'd,e,', 'f,g,h']).to(tubes.CsvRow).get(2, 'xx').to(str)
    assert list(tube) == ['xx', 'xx', '', 'h']

def test_csv_uneven_rows_named():
    file = "\n".join(['a,b,c', '1,2,3', '4,5,', '6,7', '8,', '9', ','])
    tube = tubes.Each([file]).csv().get('b', 'xx').to(str)
    assert list(tube) == ['2', '5', '7', '', 'xx', '']
    tube = tubes.Each([file]).csv().get('c', '99').to(str)
    assert list(tube) == ['3', '', '99', '99', '99', '99']

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
    tube = (tubes
        .Each(['a,b,c', 'd,e,f'])
        .csv(headers=True, split=False)
        .multi(lambda x:(
            x.get('a'),
            x.get(1),
            x.get(2),
            x.get('c')
        ))
        .to(str, str, str, str)
    )
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
    tube = tubes.Each([tsv_1, tsv_2]).to(bytes).csv(headers=True).chunk(1).multi(lambda x:(
        x.get('a'),
        x.get('b'),
        x.get('c')
    )).to(int, int, int)
    assert list(tube) == [(1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12)]


def test_csv_non_comma_separator():
    tube = tubes.Each(['a|b|c', 'd|e,f|g']).to(bytes).csv(headers=False, sep='|', split=False)
    assert list(tube) == [[b'a', b'b', b'c'], [b'd', b'e,f', b'g']]


def test_csv_line_splitting():
    tube = tubes.Each(["a,b\nc,d\ne", ",f"]).csv(headers=False)
    assert list(tube) == [[b'a', b'b'], [b'c', b'd'], [b'e', b'f']]


def test_csv_line_splitting_embedded_nl():
    tube = tubes.Each(['a,b\n"c\nd",e\nf', ',g']).csv(headers=False)
    assert list(tube) == [[b'a', b'b'], [b'c\nd', b'e'], [b'f', b'g']]


def test_csv_latin1():
    tube = tubes.Each([b'a,\xff\n\xfe,\xfa\nf']).csv(headers=False)
    assert list(tube) == [[b'a', b'\xff'], [b'\xfe', b'\xfa'], [b'f']]


def test_csv_latin1_get_by_name():
    tube = tubes.Each([b'a,\xff\n\xfe,\xdf\nf']).csv().get(b'\xff', '').to(str, codec='latin1')
    #import pdb; pdb.set_trace()
    assert list(tube) == ['ß', '']


if __name__ == '__main__':
    test_csv_one_row()