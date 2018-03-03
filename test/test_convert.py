import pytest
import tubes


@pytest.mark.parametrize("val, dtype, result", [
    ([None], tubes.Null, [None]),
    ([False, 0, '', b''], tubes.Bool, [False, False, False, False]),
    ([True, 1, 'a', b'a'], tubes.Bool, [True, True, True, True]),
    ([True, 2, 4.2, -2**60, '1234'], tubes.Int64, [1, 2, 4, -2**60, 1234]),
    ([True, 2, 3.2, -10e10, '1.234', 'inf'], tubes.Float, [1.0, 2.0, 3.2, -10e10, 1.234, float('inf')]),
    ([True, 2, 3.2, b'abc', 'abc'], tubes.ByteSlice, [b'True', b'2', b'3.2', b'abc', b'abc']),
    ([True, 2, 3.2, b'abc', 'abc'], tubes.Utf8, ['True', '2', '3.2', 'abc', 'abc']),
])
def test_convert_from_to_py(val, dtype, result):
    assert list(tubes.Each(val).to(dtype)) == result


def test_convert_from_bool():
    assert list(tubes.Each([True, False]).to(bool).to(int)) == [1, 0]
    assert list(tubes.Each([True, False]).to(bool).to(float)) == [1., 0.]
    assert list(tubes.Each([True, False]).to(bool).to(bytes)) == [b'True', b'False']
    assert list(tubes.Each([True, False]).to(bool).to(str)) == ['True', 'False']