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
