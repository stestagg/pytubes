import pytest

import tubes
from io import BytesIO, StringIO

def test_reading_a_file():
    buf = BytesIO(b"Mary had a little lamb")
    tube = tubes.Each([buf]).read_fileobj()
    assert list(tube) == [b'Mary had a little lamb']

def test_reading_two_files():
    buf1 = BytesIO(b"Mary had")
    buf2 = BytesIO(b'a little lamb')
    tube = tubes.Each([buf1, buf2]).read_fileobj()
    assert list(tube) == [b'Mary had', b'a little lamb']

def test_reading_two_files_small_buffer():
    buf1 = BytesIO(b"Mary had")
    buf2 = BytesIO(b'a little lamb')
    tube = tubes.Each([buf1, buf2]).read_fileobj(size=2).to(str)
    assert list(tube) == ['Ma', 'ry', ' h', 'ad', 'a ', 'li', 'tt', 'le', ' l', 'am', 'b']

def test_reading_two_files_small_buffer_multiple_lines():
    buf1 = BytesIO(b"Mary had ")
    buf2 = BytesIO(b'a\nlittle lamb')
    tube = tubes.Each([buf1, buf2]).read_fileobj(size=2).split().to(str)
    assert list(tube) == ['Mary had a', 'little lamb']

def test_reading_nota_fileobj():
    buf1 = BytesIO(b"Mary had")
    buf2 = "string"
    tube = tubes.Each([buf1, buf2]).read_fileobj()
    with pytest.raises(ValueError) as exc:
        list(tube)
    assert exc.match(r'only accepts objects with a \.read\(\)')

def test_reading_unicode():
    buf1 = BytesIO(b"Mary had")
    buf2 = StringIO("string")
    tube = tubes.Each([buf1, buf2]).read_fileobj()
    with pytest.raises(ValueError) as exc:
        list(tube)
    assert exc.match('expects binary')
