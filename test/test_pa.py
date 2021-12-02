import tubes

try:
    import pyarrow as pa
except ImportError:
    pass
else:
    def test_doubles():
        tube = tubes.Count(2).to(float).first(4)
        table = tube.to_pyarrow(('a', ))
        assert isinstance(table, pa.Table)
        assert str(table.columns[0].type) == 'double'
        assert table.to_pandas().to_dict() == {
            'a': {0: 2, 1: 3, 2: 4, 3: 5},
        }


    def test_int():
        tube = tubes.Count().to(int).first(20_000)
        table = tube.to_pyarrow(('a', ))
        assert isinstance(table, pa.Table)
        assert len(table) == 20_000
        assert list(table.to_pandas()['a']) == list(range(20_000))


    def test_none():
        tube = tubes.Each([None, None]).to(None)
        table = tube.to_pyarrow(('x', ))
        assert len(table) == 2
        assert list(table.to_pandas().x) == [None, None]


    def test_enumerate():
        tube = tubes.Count(2).enumerate().first(4)
        table = tube.to_pyarrow(('a', 'b'))
        assert isinstance(table, pa.Table)
        assert str(table.columns[0].type) == 'int64'
        assert str(table.columns[1].type) == 'int64'
        assert table.to_pandas().to_dict() == {
            'a': {0: 0, 1: 1, 2: 2, 3: 3},
            'b': {0: 2, 1: 3, 2: 4, 3: 5}
        }


    def test_str():
        tube = tubes.Each(['a', 'b', 'c', 'd', 'e']).to(str).enumerate()
        table = tube.to_pyarrow(('index', 'val'))
        assert isinstance(table, pa.Table)
        assert str(table.columns[0].type) == 'int64'
        assert str(table.columns[1].type) == 'string'
        assert table.to_pandas().to_dict() == {
            'index': {0: 0, 1: 1, 2: 2, 3: 3, 4: 4},
            'val': {0: 'a', 1: 'b', 2: 'c', 3: 'd', 4: 'e'}
        }


    def test_mixed_types():
        table = (tubes
            .Each(['apple', 'banana', 'apple'])
            .to(str).enumerate()
            .multi(lambda x: (x.slot(0), x.slot(0).to(float), x.slot(1)))
            .to_pyarrow(('index', 'index_double', 'val'))
        )
        assert isinstance(table, pa.Table)
        assert str(table.columns[0].type) == 'int64'
        assert str(table.columns[1].type) == 'double'
        assert str(table.columns[2].type) == 'string'
        assert table.to_pandas().to_dict() == {
            'index': {0: 0, 1: 1, 2: 2},
            'index_double': {0: 0., 1: 1., 2: 2.},
            'val': {0: 'apple', 1: 'banana', 2: 'apple'}
        }


    def test_longer_table():
        tube = tubes.Count(5).multi(lambda x:(
            tubes.Count(),
            x,
            x.to(object).to(str),
            x.gt(1000)
        )).first(500000)
        table = tube.to_pyarrow(('index', 'src', 'strval', 'is_big'))
        assert isinstance(table, pa.Table)
        assert [str(c.type) for c in table.columns] == ['int64', 'int64', 'string', 'bool']
        assert table.shape == (500000, 4)
        assert dict(table.to_pandas().iloc[-1]) == {
            'index': 499999,
            'src': 500004,
            'strval': '500004',
            'is_big': True
        }

    def test_csv_issue():
        table = (tubes
            .Each(['a,b,c\n1,2,'])
            .csv()
            .multi(lambda x: (
                x.get('a').to(str), 
                x.get('c').to(str)
            ))
            .to_pyarrow(('a', 'c'))
        )
        assert table.to_pydict() == {
            'a': ['1'],
            'c': ['']
        }