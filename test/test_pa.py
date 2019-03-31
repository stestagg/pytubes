import pyarrow as pa
import tubes

if hasattr(tubes.Tube, 'pa_table'):
    def test_enumerate():
        tube = tubes.Count(2).enumerate().first(4)
        table = tube.pa_table(('a', 'b'))
        assert isinstance(table, pa.Table)
        assert str(table.columns[0].type) == 'int64'
        assert str(table.columns[1].type) == 'int64'
        assert table.to_pandas().to_dict() == {
            'a': {0: 0, 1: 1, 2: 2, 3: 3},
            'b': {0: 2, 1: 3, 2: 4, 3: 5}
        }


    def test_str():
        tube = tubes.Each(['a', 'b', 'c', 'd', 'e']).to(str).enumerate()
        table = tube.pa_table(('index', 'val'))
        assert isinstance(table, pa.Table)
        assert str(table.columns[0].type) == 'int64'
        assert str(table.columns[1].type) == 'string'
        assert table.to_pandas().to_dict() == {
            'index': {0: 0, 1: 1, 2: 2, 3: 3, 4: 4},
            'val': {0: 'a', 1: 'b', 2: 'c', 3: 'd', 4: 'e'}
        }

    def test_longer_table():
        tube = tubes.Count(5).multi(lambda x:(
            tubes.Count(),
            x,
            x.to(object).to(str),
            x.gt(1000)
        )).first(500000)
        table = tube.pa_table(('index', 'src', 'strval', 'is_big'))
        assert isinstance(table, pa.Table)
        assert [str(c.type) for c in table.columns] == ['int64', 'int64', 'string', 'bool']
        assert table.shape == (500000, 4)
        assert dict(table.to_pandas().iloc[-1]) == {
            'index': 499999,
            'src': 500004,
            'strval': '500004',
            'is_big': True
        }
