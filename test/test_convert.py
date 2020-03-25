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


def test_to_str_codec():
    tube = tubes.Each([b'\xff']).to(str, codec='latin1')
    assert list(tube) == ['\xff']


def test_to_str_invalid_utf8():
    with pytest.raises(UnicodeDecodeError):
        tube = tubes.Each([b'\xff']).to(str)
        assert list(tube) == ['\xff']


def test_convert_from_bool():
    assert list(tubes.Each([True, False]).to(bool).to(int)) == [1, 0]
    assert list(tubes.Each([True, False]).to(bool).to(float)) == [1., 0.]
    assert list(tubes.Each([True, False]).to(bool).to(bytes)) == [b'True', b'False']
    assert list(tubes.Each([True, False]).to(bool).to(str)) == ['True', 'False']

@pytest.mark.parametrize("val, from_dtype, to_dtype, result", [
    ([None], tubes.Null, tubes.Null, [None]),
    (['1', '3.14', '-1', '1e100', 'inf'], str, float, [1, 3.14, -1, 1e100, float('inf')]),
    (['1', '3.14', '-1', '1e+100', 'inf'], bytes, float, [1, 3.14, -1, 1e100, float('inf')]),
    ([1, 3.14, -1, 1e100, float('inf')], float, bytes, [b'1.0', b'3.14', b'-1.0', b'1e+100', b'inf']),
    ([1, 3.14, -1, 1e100, float('inf'), 1/3], float, str, ['1.0', '3.14', '-1.0', '1e+100', 'inf', '0.3333333333333333']),
])
def test_convert_types(val, from_dtype, to_dtype, result):
    assert list(tubes.Each(val).to(from_dtype).to(to_dtype)) == result
