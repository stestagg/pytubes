import string

import numpy
import numpy.random as random
import pytest
import tubes


def rand_chars():
    n_bytes = random.randint(0,10)
    return random.bytes(n_bytes).decode("latin1")


def csv_escape(data):
    data = data.replace(b'"', b'""')
    return b'"' + data + b'"'


@pytest.fixture(params=[False, True])
def do_split(request):
    return request.param


def get_cols(max_cols):
    cols_to_read = list(range(max_cols))
    random.shuffle(cols_to_read)
    return cols_to_read[:random.randint(max_cols)]


@pytest.mark.parametrize("seed", [random.randint(2147483648) for i in range(60)])
def test_fuzz_csv(seed, do_split):
    random.seed(seed)
    n_rows = random.randint(30)
    cols_to_read = get_cols(32)
    csv_rows = []
    expected_rows = []
    for _ in range(n_rows):
        csv_row = []
        expected_row = (['xx'] * len(cols_to_read))
        for col_no in range(random.randint(30)):
            data = '\r'
            while data.endswith('\r'):
                data = rand_chars()
            if col_no in cols_to_read:
                expected_row[cols_to_read.index(col_no)] = data
            data = data.encode("utf8")
            if b'"' in data or b'\n' in data or b',' in data or random.choice([False, False, True]):
                data = csv_escape(data)
            csv_row.append(data)
        if len(csv_row) == 0:
            if 0 in cols_to_read:
                expected_row[cols_to_read.index(0)] = ''
        expected_rows.append(tuple(expected_row))
        csv_rows.append(b",".join(csv_row))
    if do_split:
        tube_input = [b'\n'.join(csv_rows)] if n_rows else []
        slot_tube = tubes.Each(tube_input).csv(headers=False, skip_empty_rows=False)
    else:
        slot_tube = tubes.Each(csv_rows).to(tubes.CsvRow)

    slot_tube = slot_tube.multi(lambda x: [x.get(c, 'xx').to(str) for c in cols_to_read])
    actual_rows = list(slot_tube)
    for row_num in range(len(expected_rows)):
        for col_num in range(len(cols_to_read)):
            expected = expected_rows[row_num][col_num]
            if len(cols_to_read) == 1:
                actual = actual_rows[row_num]
            else:
                actual = actual_rows[row_num][col_num]
            assert expected == actual
    assert len(expected_rows) == len(actual_rows)


def get_tsv(seed):
    random.seed(seed)
    n_rows = random.randint(30)
    cols_to_read = get_cols(32)
    tsv_rows = []
    expected_rows = []
    for _ in range(n_rows):
        tsv_row = []
        expected_row = (['xx'] * len(cols_to_read))
        for col_no in range(random.randint(30)):
            data = '\t'
            while '\t' in data or data.endswith('\r'):
                data = rand_chars()
            if col_no in cols_to_read:
                expected_row[cols_to_read.index(col_no)] = data
            data = data.encode("utf8")
            tsv_row.append(data)
        if len(tsv_row) == 0:
            if 0 in cols_to_read:
                expected_row[cols_to_read.index(0)] = ''
        expected_rows.append(tuple(expected_row))
        tsv_rows.append(b'\t'.join(tsv_row))
    return tsv_rows, expected_rows, cols_to_read


@pytest.mark.parametrize("seed", [random.randint(2147483648) for i in range(60)])
def test_fuzz_tsv(seed):
    tsv_rows, expected_rows, cols_to_read = get_tsv(seed)
    slot_tube = tubes.Each(tsv_rows).to(tubes.TsvRow).multi(lambda x: [x.get(c, 'xx').to(str) for c in cols_to_read])
    actual_rows = list(slot_tube)
    for row_num in range(len(expected_rows)):
        for col_num in range(len(cols_to_read)):
            expected = expected_rows[row_num][col_num]
            if len(cols_to_read) == 1:
                actual = actual_rows[row_num]
            else:
                actual = actual_rows[row_num][col_num]
            assert expected == actual
    assert len(expected_rows) == len(actual_rows)


def any_double(n):
    buffer = numpy.random.bytes(8 * n)
    return numpy.frombuffer(buffer, dtype=numpy.double)

def tiny_double(n):
    return numpy.random.uniform(-1e-10, 1e10, size=n)

def small_double(n):
    return numpy.random.uniform(-10_000, 10_000, size=n)

def large_double(n):
    return numpy.random.uniform(-1e100, 1e100, size=n)


@pytest.mark.parametrize("seed", [random.randint(2147483648) for i in range(10)])
@pytest.mark.parametrize("maker", [any_double, tiny_double, small_double, large_double])
def test_fuzz_random_double_to_str(seed, maker):
    numpy.random.seed(seed)
    array = maker(10240)
    actual = list(tubes.Each(array).to(float).to(str))
    expected = [str(x).replace('e-0', 'e-').replace('e+0', 'e+') for x in array]
    assert actual == expected


def make_str_col(length):
    return numpy.array([rand_chars() for _ in range(length)])
make_str_col.dtype = str


def make_float_col(length):
    return random.rand(length)
make_float_col.dtype = float


def make_int_col(length):
    return random.randint(-1e9, 1e9, size=(length, ))
make_int_col.dtype = int


if tubes.HAVE_PYARROW:
    @pytest.mark.parametrize("seed", [random.randint(2147483648) for i in range(10)])
    def test_fuzz_pa(seed):
        random.seed(seed)
        num_rows = random.randint(1000)
        num_cols = random.randint(100)
        types = [random.choice([make_str_col, make_int_col, make_float_col]) for _ in range(num_cols)]
        cols = [m(num_rows) for m in types]
        col_names = ['%i: %s' % (i, rand_chars()) for i in range(num_cols)]
        given = list(zip(*cols))
        given_pod = [[x.item() for x in r] for r in given]

        slot_tube = tubes.Each(given_pod).multi(lambda x: [x.get(i).to(t.dtype) for i, t in enumerate(types)])
        result = slot_tube.to_pyarrow(col_names)
        table = result.to_pandas()
        assert tuple(table.columns) == tuple(col_names)
        for col_name, col in zip(col_names, cols):
            result = table[col_name]
            assert all(result == col)


if __name__ == '__main__':
    for i in range(10000):
        test_fuzz_csv(i, True)
        if i % 500 == 0:
            print("Iteration: ", i)
