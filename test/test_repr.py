import tubes
import pytest

@pytest.mark.parametrize(('tube', 'desc'), [
    (tubes.Each([]).chunk(2), "Chunk(Each([]), 2)"),
    (tubes.Each([]).enum(), "Each([]).Enum(b'utf-8')"),
    (tubes.Each([]).enumerate(10), "Count(10).Zip(Count(10), Each([]))"),
    (tubes.Each([]).equals(6), "Each([]).Compare(2, 6)"), # 2 is EQ
    (tubes.Each([]).first(2), "Each([]).First(2)"),
    (tubes.Each([]).get(0), "Each([]).IndexLookup([0]).SlotGet(0, UNDEFINED)"),
    (tubes.Each([]).get(0, default='x'), "Each([]).IndexLookup([0]).SlotGet(0, 'x')"),
    (tubes.Each([]).group_id(), "Each([]).GroupId()"),
    (tubes.Each([]).gt(9), "Each([]).Compare(4, 9)"),
    (tubes.Each([]).gunzip(), "Each([]).Gunzip(False)"),
    (tubes.Each([]).gunzip(stream=True), "Each([]).Gunzip(True)"),
    (tubes.Each([]).json(), "Each([]).Convert([DType[str]], b'utf-8').JsonParse()"),
    (tubes.Each([]).lt(5), "Each([]).Compare(0, 5)"),
    (tubes.Each([]).map_files(), "Each([]).Convert([DType[bytes]], b'fs').FileMap()"),
    (
        tubes.Each([]).multi(lambda x: (x.get(0), x.get(1))),
        "Each([]).Multi(Each([]).IndexLookup([0, 1]).SlotGet(0, UNDEFINED), Each([]).IndexLookup([0, 1]).SlotGet(1, UNDEFINED))"
    ),
    (tubes.Each([]).read_files(), "Each([]).Convert([DType[bytes]], b'fs').ReadFile()"),
    (tubes.Each([]).skip(3), "Each([]).Skip(3)"),
    (tubes.Each([]).skip_unless(lambda x: x.gt(4)), "Each([]).SkipUnless(Each([]).Compare(4, 4))"),
    (tubes.Each([]).slot(0), "Each([]).SlotGet(0, UNDEFINED)"),
    (tubes.Each([]).to(bytes).split('r'), "Each([]).Convert([DType[bytes]], b'utf-8').Split('r')"),
    (tubes.Each([]).to(float), "Each([]).Convert([DType[float]], b'utf-8')"),
    (tubes.Each([]).to_py(), "Each([]).ToPy()"),
    (tubes.Each([]).tsv(), "Each([]).Convert([DType[bytes]], b'utf-8').Xsv(True, '\\t', 'tsv')"),
    (tubes.Each([]).zip(tubes.Each([])), "Each([]).Zip(Each([]), Each([]))"),
])
def test_repr(tube, desc):
    assert repr(tube) == desc