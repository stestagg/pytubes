import tubes
import pytest

@pytest.mark.parametrize(('tube', 'desc'), [
    (tubes.Each([]).chunk(2), "Chunk(Each([]), chunk_size=2)"),
    (tubes.Each([]).enum(), "Each([]).Enum()"),
    (tubes.Each([1]).enum(), "Each([1]).Enum()"),
    (tubes.Each([]).enumerate(10), "Count(start=10).Zip(Count(start=10), Each([]))"),
    (tubes.Each([]).equals(6), "Each([]).Compare(op='==', value=6)"), # 2 is EQ
    (tubes.Each([]).first(2), "Each([]).First(2)"),
    (tubes.Each([]).get(0), "Each([]).IndexLookup([0]).SlotGet(0)"),
    (tubes.Each([]).get(0, default='x'), "Each([]).IndexLookup([0]).SlotGet(0, default_val='x')"),
    (tubes.Each([]).group_id(), "Each([]).GroupId()"),
    (tubes.Each([]).gt(9), "Each([]).Compare(op='>', value=9)"),
    (tubes.Each([]).gunzip(), "Each([]).Gunzip(stream=False)"),
    (tubes.Each([]).gunzip(stream=True), "Each([]).Gunzip(stream=True)"),
    (tubes.Each([]).json(), "Each([]).Convert(to_types=[DType[str]]).JsonParse()"),
    (tubes.Each([]).lt(5), "Each([]).Compare(op='<', value=5)"),
    (tubes.Each([]).map_files(), "Each([]).Convert(to_types=[DType[bytes]], codec=b'fs').FileMap()"),
    (
        tubes.Each([]).multi(lambda x: (x.get(0), x.get(1))),
        "Each([]).Multi(Each([]).IndexLookup([0, 1]).SlotGet(0), Each([]).IndexLookup([0, 1]).SlotGet(1))"
    ),
    (
        tubes.Each([]).multi(lambda x: [x.get(0), x.get(1)]),
        "Each([]).Multi(Each([]).IndexLookup([0, 1]).SlotGet(0), Each([]).IndexLookup([0, 1]).SlotGet(1))"
    ),
    (tubes.Each([]).read_files(), "Each([]).Convert(to_types=[DType[bytes]], codec=b'fs').ReadFile()"),
    (tubes.Each([]).skip(3), "Each([]).Skip(num=3)"),
    (tubes.Each([]).skip_unless(lambda x: x.gt(99)), "Each([]).SkipUnless(Each([]).Compare(op='>', value=99))"),
    (tubes.Each([]).slot(0), "Each([]).SlotGet(0)"),
    (tubes.Each([]).to(bytes).split('r'), "Each([]).Convert(to_types=[DType[bytes]]).Split(sep='r', skip_empty=False)"),
    (tubes.Each([]).to(float), "Each([]).Convert(to_types=[DType[float]])"),
    (tubes.Each([]).to_py(), "Each([]).ToPy()"),
    (tubes.Each([]).tsv(), "Each([]).Convert(to_types=[DType[bytes]]).Xsv('tsv', sep='\\t')"),
    (tubes.Each([]).zip(tubes.Each([])), "Each([]).Zip(Each([]), Each([]))"),
])
def test_repr(tube, desc):
    assert repr(tube) == desc