
cimport cython

cimport scalar_type
from scalar_type cimport ScalarType
from libcpp.vector cimport vector
from libcpp.map cimport map as map_t
from libcpp.string cimport string
from cpython.object cimport Py_LT, Py_LE, Py_EQ, Py_NE, Py_GT, Py_GE

__doc__ = """
PyTubes
-------

PyTubes is a library for efficient data loading.  Implemented as a cython/c++
extension module, it can extract data from large datasets much faster than
by using traditional python methods.

"""


include "version.pxi"
include "iter.pxi"


cdef extern from '../src/iter.hpp' namespace 'ss::iter':
    void init_pytubes_internal()


cdef class DType:
    cdef scalar_type.ScalarType type
    cdef readonly str name

    def __repr__(self):
        return f"DType[{self.name}]"

C_DTYPE_TO_Dtype = {}


cdef map_t[scalar_type.ScalarType, string] _SCALAR_TYPE_TO_NAME

cdef _make_dtype(scalar_type.ScalarType ty, str name):
    cdef DType dtype = DType()
    dtype.type = ty
    dtype.name = name
    C_DTYPE_TO_Dtype[<int>ty] = dtype
    return dtype


cdef c_dtype_to_dtype(scalar_type.ScalarType ty):
    return C_DTYPE_TO_Dtype[<int>ty]


Null = _make_dtype(scalar_type.Null, "Null")
Bool = _make_dtype(scalar_type.Bool, "Bool")
Int64 = _make_dtype(scalar_type.Int64, "Int64")
Float = _make_dtype(scalar_type.Float, "float")
ByteSlice = _make_dtype(scalar_type.ByteSlice, "bytes")
Utf8 = _make_dtype(scalar_type.Utf8, "str")
Object = _make_dtype(scalar_type.Object, "object")
JsonUtf8 = _make_dtype(scalar_type.JsonUtf8, "Json")
TsvRow = _make_dtype(scalar_type.Tsv, "Tsv")
CsvRow = _make_dtype(scalar_type.Csv, "Csv")


cdef class _UNDEFINED:
    def __repr__(self):
        return "UNDEFINED"
UNDEFINED = _UNDEFINED()
cdef public PyObject *UNDEFINED_OBJ = <PyObject*>UNDEFINED


DTYPE_MAP = {
    None: Null,
    bool: Bool,
    int: Int64,
    float: Float,
    bytes: ByteSlice,
    str: Utf8, # Not ideal, but..
    object: Object
}

include "pyiter.pxi"
include "ndarray.pxi"
include "arrow.pxi"


cdef class Tube:

    """
    The base class for all tubes.  Pipelines are typically built by calling
    the methods defines on this class.
    """

    cdef object _name_lookup
    cdef int _name_lookup_inited
    cdef object _index_lookup
    cdef int _index_lookup_inited

    cdef IterWrapper _make_iter(self, args):
        raise NotImplementedError("_make_iter")

    @property
    def dtype(self):
        raise NotImplementedError("dtype")

    @property
    def _inputs(self):
        raise NotImplementedError("_inputs")

    cpdef _describe_self(self):
        raise NotImplementedError("_describe_self")

    def __iter__(self):
        if not isinstance(self, ToPy):
            return iter(self.to_py())
        cdef Chains chains = Chains(self)
        made_chains, made_iters = chains.make_chains_iters()
        if len(self.dtype) == 1:
            return TubeSingleIter(made_iters[self], made_chains[None, self])
        return TubeMultiIter(made_iters[self], made_chains[None, self], len(self.dtype))

    def ndarray(self, *slot_info, estimated_rows=32768, fields=None):
        """
        Create a new numpy ``ndarray`` with appropriate numpy dtype, and
        fill it with the results of the tube.

        :param int estimated_rows: A hint to pytubes as to how many rows are
                                   expected in the completed array.  This affects
                                   the way in which the ndarray is resized during
                                   iteration.
        :param fields: If ``True``, the created numpy array is 1-dimentional, using
                       structured types.  Fields names are string slot numbers.
                       If ``False``, all slots must have an identical type, produces
                       an n-dimentional array, with each slot in a different dimension.
        :param slot_info: Provide metadata about each slot to help create the ndarray
                           Currently required by bytes types to set the column size.
                           The n-th slot_info value is used to affect the numpy
                           dtype of the n-th slot of the input.

        >>> Each(['abcd', 'efgh']).to(bytes).ndarray(2)
        array([b'ab', b'ef'], dtype='|S3')
        >>> Each(['abcd', 'efgh']).to(bytes).ndarray(4)
        array([b'abcd', b'efgh'], dtype='|S5')

        """
        return ndarray_from_tube(self, slot_info, estimated_rows, fields=fields)

    def to_pyarrow(self, fields):
        """
        Create a new pyarrow ``Array`` or ``Table``, and fill it with the
        results of the tube.

        :param fields: The names of the Table columns

        >>> TODO
        """
        return pa_from_tube(self, fields)

    cdef _repr(self, stop=None):
        cdef Tube tube_input
        if stop is None:
            stop = set()
        inputs = self._inputs
        if inputs and not (set(inputs) & stop):
            tube_input = inputs[0]
            return f"{tube_input._repr(stop)}.{self._describe_self()}"
        return self._describe_self()

    def __repr__(self):
        return self._repr()

    cdef _set_name_lookup(self, NameLookup lookup):
        assert self._name_lookup_inited == 0
        self._name_lookup = lookup
        self._name_lookup_inited = 1

    cdef NameLookup name_lookup(self):
        if not self._name_lookup_inited:
            self._set_name_lookup(NameLookup(self, []))
        return self._name_lookup

    cdef _set_index_lookup(self, IndexLookup lookup):
        assert self._index_lookup_inited == 0
        self._index_lookup = lookup
        self._index_lookup_inited = 1

    cdef IndexLookup index_lookup(self):
        if not self._index_lookup_inited:
            self._set_index_lookup(IndexLookup(self, []))
        return self._index_lookup

    @property
    def one(self):
        """
        Return just the first value from the tube, useful as a debugging aid
        """
        return next(iter(self))

    def first(self, size_t num):
        """
        Compatibility: tube.first(5)
        Create a tube that yields no more than the first `num` items from its parent.

        >>>  list(Each([1,2,3,4,5]).first(1))
        [1]
        >>>  list(Each([1,2,3,4,5]).first(10))
        [1, 2, 3, 4, 5]
        """
        return First(self, num)

    def skip(self, size_t num):
        """
        Compatibility: tube.skip(5)
        Create a tube that skips the first `num` items, yielding any furthe times.

        >>>  list(Each([1,2,3,4,5]).skip(1))
        [2, 3, 4, 5]
        >>>  list(Each([1,2,3,4,5]).skip(10))
        []
        """
        return Skip(self, num)

    def to_py(self):
        """
        Compatibility: tube.to_py()
        Convert a tube of any type to python objects.

        This is not typically needed, as calling ``__iter__`` on a tube implicitly
        converts values to python
        """
        return ToPy(self)

    def map_files(self):
        """
        Compatible Dtypes: ``bytes``, ``Utf8``

        For each filename from the input, open the file and mmap it to a
        byte slice.
        """
        if self.dtype[0] != ByteSlice:
            return FileMap(self.to(ByteSlice, codec="fs"))
        return FileMap(self)

    def read_files(self):
        """
        Compatible Dtypes: ``bytes``, ``Utf8``

        For each filename from the input, open the file and read successive
        chunks of data from it.  Each new file will start at  the beginning of
        a slice, but a single file may result in multiple slices.

        Care must be taken when using this with iterators that have file-level
        context (for example gunzip()) as file boundaries are not communicated.

        `chunk(1)` may be used to process files in isolation before compbining the
        iterators.

        >>> list(Each(['file1.txt']).read_files().split())
        ['file 1, line 1', 'file 1, line 2']
        >>> list(Each(['file1.txt', 'file2.txt']).read_files().split().skip(1).chunk(1))
        ['file 1 line 2', 'file 1 line 3', 'file 2 line 2', ...]
        """
        if self.dtype[0] != ByteSlice:
            return ReadFile(self.to(ByteSlice, codec="fs"))
        return ReadFile(self)

    def read_fileobj(self, size=8_388_608):
        """
        Compatible Dtypes: ``object``

        Each item in the input must be a binary file obj.

        Returns an iterator that reads the contents of each file obj in `size` 
        sized chunks and returns it.
        """
        return ReadFileObj(self, size)

    def gunzip(self, stream=False):
        """
        Compatibility: tube.gunzip()
        zlib/gzip decompress the input bytes, returning a view of up to 16
        megabytes of decompressed data.

        By default, this assumes that each input slice is an entire stream
        (i.e. from map_files).  Setting `stream=True` treats all input values
        as part of a single gzip stream.

        >>> list(Each(["file.gz", "file2.gz"]).map_files().gunzip())
        ["file1 contents", "file2 contents"]
        >>> list(Each(["file.gz", "file2.gz"]).read_files().gunzip(stream=True).chunk(1))
        ["file1 contents", "file2 contents"]
        """
        return Gunzip(self, stream)

    def split(self, sep="\n", trim='', skip_empty=False):
        """
        Compatibility: tube.split()
        Split the input view  on the character `sep`.  This behaves similarly
        to the python :func:`str.split()` function.

        This iterator will typically produce multiple values for each input
        value, as each split produces a new output value.

        >>> list(Each(['a.b.c', 'd.e']).split("."))
        ['a', 'b', 'c', 'd', 'e']
        >>> list(Each(['ab\\ncd', 'ef\\ngh']).split())
        ['ab', 'cd', 'ef', 'gh']
        """
        return Split(self, sep, trim, skip_empty)

    def json(self):
        """
        Compatibility: tube.json()
        Interpret the input as JSON documents.
        The JSON parser is *not* validating, and will accept invalid JSON.

        >>> list(Each(['1', '{}', '[]', 'null']).to(tubes.Utf8).json())
        [1, {}, [], None]
        >>> list(Each(['neil']).to(tubes.Utf8).json())
        [None]
        """
        if self.dtype[0] in {ByteSlice, Object}:
            return JsonParse(self.to(str, codec="utf-8"))
        return JsonParse(self)

    def tsv(self, headers=True, sep='\t', split=True, skip_empty_rows=True):
        """
        Compatibility: tube.tsv()
        Interpret the input values as rows of a TSV file.

        :param bool headers: [default: ``True``] If ``True``, will read the first
            input value as a tab-separated list of field names,
            allowing subsequent access to values by name, as well as by index.
        :param str sep: [default: '\t'] A single-character string that is
            used as the field separator when reading rows.
        :param bool split: [default: ``True``] If ``True``, split the input bytes
            on newlines, to identify rows.  If ``False``, each input value is assumed
            to be a separate row.
        :param bool skip_empty_rows: Skip over blank rows in the in input (note
            whitespace causes a row to be considered non-blank)

        >>> list(Each(['sample.tsv']).read_files().tsv())
        [(b'abc', b'def'), (b'ghi', b'jkl')]
        >>> list(Each(['a\\tb', 'c\\td']).tsv())
        [(b'a', b'b'), (b'c', b'd')]
        >>> list(Each(['a\\tb', 'c\\td']).tsv(headers=True).get('a'))
        [b'c']
        >>> list(Each(['a|b', 'c|d']).tsv(headers=False, sep='|').get(1).to(str))
        ['b', 'd']
        """
        if self.dtype[0] in {Utf8, Object}:
            return Xsv(self.to(bytes, codec="utf-8"), "tsv", sep, headers, split, skip_empty_rows)
        return Xsv(self, "tsv", sep, headers, split, skip_empty_rows)

    def csv(self, headers=True, sep=',', split=True, skip_empty_rows=True):
        """
        Compatibility: tube.csv()
        Interpret the input values as rows of a CSV file.
        Each input to csv() is treated as a separate row in the file.

        :param bool headers: [default: `True`] If true, will read the first
            input value as a tab-separated list of field names,
            allowing subsequent access to values by name, as well as by index.
        :param str sep: [default: ','] A single-character string that is
            used as the field separator when reading rows.
        :param bool split: [default: True] If `True`, split the input bytes
            on newlines, to identify rows.  If `False`, each input value is assumed
            to be a separate row.
        :param bool skip_empty_rows: Skip over blank rows in the in input (note
            whitespace causes a row to be considered non-blank)

        >>> list(Each(['sample.csv']).read_files().csv())
        [(b'abc', b'def'), (b'ghi', b'jkl')]
        >>> list(Each(['a,b', 'c,d']).csv())
        [(b'a', b'b'), (b'c', b'd')]
        >>> list(Each(['a,b', 'c,d']).csv(headers=True).get('a'))
        [b'c']
        >>> list(Each(['a,b', 'c,d']).csv(headers=False).get(1).to(str))
        ['b', 'd']
        """
        if self.dtype[0] in {Utf8, Object}:
            return Xsv(self.to(bytes, codec="utf-8"), "csv", sep, headers, split, skip_empty_rows)
        return Xsv(self, "csv", sep, headers, split, skip_empty_rows)

    def to(self, *types, codec="utf-8"):
        """
        Convert the input to the specified dtype.

        Supported conversions:
        XCompatibility: from_tube.to(*to_tube.dtype)
        """
        resolved = []
        for slot_type in types:
            if not isinstance(slot_type, DType):
                slot_type = DTYPE_MAP[slot_type]
            resolved.append(slot_type)
        return Convert(self, resolved, codec=codec.encode('ascii'))

    def enum(self, codec="utf-8"):
        """
        Compatibility: list(tube.enum())

        Convert the input to a python object, storing and returning duplicate
        values where possible to reduce allocation overhead
        """
        return Enum(self, codec=codec.encode('ascii'))

    def group_id(self):
        """
        Compatibility: list(tube.group_id())

        Assuming the input is sorted, for each unique value in the input, return a
        unique integer ID for that value. If the input isn't sorted, return a value
        that increments every time the input changes.

        >>> list(Each(['a', 'a', 'b', 'b', 'c', 'd']).group_id())
        [0, 0, 1, 1, 2, 3]
        """
        return GroupId(self)

    def multi(self, *makers):
        """
        Compatibility: tube.multi(lambda x: (x, ))

        Perform multiple operations on a tube, and return a tube with `n` slots

        ``makers`` should be a callable that takes the input tube, and returns a
        tuple of tubes derived from the input.

        >>> list(Count().multi(lambda x: (x.lt(1), x.equals(1), x.gt(1))).first(3))
        [(True, False, False), (False, True, False), (False, False, True)]

        """
        tubes = []
        for maker in makers:
            result = maker(self)
            if isinstance(result, Tube):
                tubes.append(result)
            else:
                tubes.extend(result)
        return Multi(self, tubes)

    def get(self, object key, default=UNDEFINED, codec='utf-8'):
        """
        Compatibility: tube.get("field")

        Efficiently read the field ``key`` from the input object and
        return it's value.

        If field is missing and a ``default`` is provided, return ``default``,
        otherwise raise.

        ``default`` must be a valid value for the input type.
        For example, if the input is a Json dtype, default must be a string/bytes
        value that is valid JSON (e.g. ``'null'``)

        >>> list(Each(['{"a": 1}', '{"a": 2}']).json().get("a"))
        [1, 2]
        >>> list(Each(['{"a": 1}', '{"b": 2}']).json().get("a"))
        Traceback (most recent call last):
        ...
        KeyError: Field not found
        >>> list(Each(['{"a": 1}', '{"b": 2}']).json().get("a", "null"))
        [1, None]
        """
        if isinstance(key, str):
            return self.name_lookup().lookup_name_str(key, default, codec)
        if isinstance(key, bytes):
            return self.name_lookup().lookup_name(key, default)
        return self.index_lookup().lookup_index(key, default)
        
    def slot(self, size_t num, default=UNDEFINED):
        """
        Compatibility: tube.slot(0)

        Return slot number ``num`` from the parent iter.  This is only useful
        when dealing with multi-slot tubes.

        If the value in the slot can be considered undefined (for example json missing value)
        then ``default`` is returned if provided, otherwise ``KeyError`` is raised.
        """
        return SlotGet(self, num, default)

    def skip_unless(self, conditional):
        """
        Compatibility: tube.skip_unless(lambda x: x.to(bool))

        ``conditional`` must either be a pre-made ``bool`` tube, or a callable that takes a single tube
        argument (the parent), and returns a ``bool`` tube.

        Iterates over ``conditional`` and the parent together, yielding values only
        where the result of conditional is True.

        Stops only when either the input __or__ conditional raise ``StopIteration``.
        This can be sligtly unexpected in, for example, this case:
        :code:`Count().skip_unless(lambda x: x.lt(3))`
        in which case, the result is :code:`[0, 1, 2]`, but iteration over the
        tube will never complete because ``skip_unless`` isn't clever enough to
        work out that the condition tube will never return another ``True``.
        In this case, an explicit :code:`.first(n)` will limit the run time.

        >>> list(Count().skip_unless(lambda x: x.gt(4)).first(2))
        [5, 6]
        >>> list(Count().skip_unless(Each([False, True, False]).to(bool)))
        [1]
        """
        if isinstance(conditional, Tube):
            condition_tube = conditional
        else:
            condition_tube = conditional(self)
        return SkipUnless(self, condition_tube)

    def skip_if(self, conditional):
        """
        Compatibility: tube.skip_if(lambda x: x.to(bool))

        ``conditional`` must either be a pre-made ``bool`` tube, or a callable that takes a single tube
        argument (the parent), and returns a ``bool`` tube.

        Iterates over conditional and the parent together, yielding values only
        where the result of conditional is False.

        Stops only when either the input __or__ conditional raise ``StopIteration``.
        This can be sligtly unexpected in, for example, this case:
        :code:`Count().skip_if(lambda x: x.gt(2))`
        in which case, the result is :code:`[0, 1, 2]`, but iteration over the
        tube will never complete because ``skip_if`` isn't clever enough to
        work out that the condition tube will never return another ``True``.
        In this case, an explicit :code:`.first(n)` will limit the run time.

        >>> list(Count().skip_if(lambda x: x.lt(5)).first(2))
        [5, 6]
        >>> list(Count().skip_if(Each([False, True, False]).to(bool)))
        [0, 2]
        """
        if isinstance(conditional, Tube):
            condition_tube = conditional
        else:
            condition_tube = conditional(self)
        return SkipIf(self, condition_tube)

    def enumerate(self, start=0):
        """
        Compatibility: tube.enumerate()
        Similar to the python builtin :func:``enumerate`` function, prefix
        the tube's dtype with an Int64 counter, starting from ``start``

        >>> list(Each(['a', 'b', 'c']).enumerate())
        [(0, 'a'), (1, 'b'), (2, 'c')]
        >>> list(Each(['a', 'b', 'c']).enumerate(10))
        [(10, 'a'), (11, 'b'), (12, 'c')]
        """
        return Zip([Count(start), self])

    def zip(self, other):
        """
        Combine two inputs into one, by joining their dtypes, and iterating both
        together.

        >>> first = Each(['a', 'b', 'c']).to(str)
        >>> second = Each([1, 2, 3]).to(int)
        >>> (first.dtype, second.dtype)
        ((DType[Utf8],), (DType[Int64],))
        >>> first.zip(second).dtype
        (DType[Utf8], DType[Int64])
        >>>  list(first.zip(second))
        [('a', 1), ('b', 2), ('c', 3)]

        """
        return Zip([self, other])

    def len(self):
        """
        Compatibility: list(tube.len())

        Return the length of the bytes/string value.
        For `bytes` types, this returns the byte length of the value, for `str` types, 
        the number of characters is returned.

        Note, technically, the length returned for `str` types is the count of unicode
        codepoints.  This is not always equal to the number of visual characters, especially
        when, for example, modifiers are used.  For example: 👋🏽 _may_ appear as a single 
        symbol, but is counted as two characters here.

        >>> list(Each(['', 'b', 'ba', 'ban']).to(str).len())
        [0, 1, 2, 3]
        >>> list(Each(['£', '👋🏽']).to(str).len()) 
        [1, 2]
        >>> list(Each(['£', '👋🏽']).to(bytes).len())
        [2, 8]
        """
        return StrLen(self)

    def is_blank(self):
        """
        Compatibility: list(tube.len())

        Returns true if the length of the input is 0, otherwise false.

        >>> list(Each(['', 'b', 'ba', 'ban']).to(str).is_blank())
        [True, False, False, False]
        """
        return self.len().equals(0)

    def equals(self, value):
        """
        Compatibility: list(tube.equals(value))

        Compare the values in parent against a static value, and output a ``bool``
        tube with the result of that comparison.

        >>> list(Each(['apple', 'banana', 'cloud']).to(str).equals('banana'))
        [False, True, False]
        >>> list(Each([False, 0, '']).equals(False))
        [True, True, False]
        """
        return Compare(self, '==', value)

    def gt(self, value):
        """
        Compatibility: list(tube.gt(value))

        Return a bool tube, that is True if the input is greater than value, otherwise False

        >>> list(Count().skip_unless(lambda x: x.gt(4)).first(2))
        [5, 6]
        """
        return Compare(self, '>', value)

    def lt(self, value):
        """
        Compatibility: list(tube.lt(value))

        Return a bool tube, that is True if the input is less than value, otherwise False

        >>> list(Count().skip_unless(lambda x: x.lt(4)).first(100))
        [0, 1, 2, 3]
        """
        return Compare(self, '<', value)

    def chunk(self, size_t num):
        """
        Compatibility: tube.chunk(2)

        Recursively look at the inputs to find an Each() tube.  If a single
        Each() tube with a list/tuple contents is found, split the value into
        ``num`` sized chunks, and chain them together.

        This is a bit of a hack to support some tricky use-cases (for example
        reading very large gzipped files requires treating every file as a
        different gzip stream, so calling .chunk(1) allows this to work)

        It's also an experiment to see how multi-threading may work in the future

        >>> list(Each(["file.gz", "file2.gz"]).read_files().gunzip(stream=True))
        Traceback (most recent call last):
        ...
        ValueError: Trailing data in gzip stream
        >>> list(Each(["file.gz", "file2.gz"]).read_files().gunzip(stream=True).chunk(1))
        ["file1 contents", "file2 contents"]
        """
        return Chunk(self, num)


def is_blank(tube):
    return tube.is_blank()


include "iter_defs.pxi"
include "chunk.pxi"
init_pytubes_internal()