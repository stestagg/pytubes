
cimport cython

cimport scalar_type
from scalar_type cimport ScalarType
from libcpp.vector cimport vector
from cpython.object cimport Py_LT, Py_LE, Py_EQ, Py_NE, Py_GT, Py_GE

__doc__ = """
Tubes
-------

Tubes is a library for efficient data loading.  Implemented as a cython/c++
extension module, it can extract data from large datasets much faster than
by using traditional python methods.

"""


include "iter.pxi"


cdef class DType:
    cdef scalar_type.ScalarType type
    cdef readonly str name

    def __repr__(self):
        return f"DType[{self.name}]"


cdef _make_dtype(scalar_type.ScalarType ty, str name):
    cdef DType dtype = DType()
    dtype.type = ty
    dtype.name = name
    return dtype


Null = _make_dtype(scalar_type.Null, "Null")
Bool = _make_dtype(scalar_type.Bool, "Bool")
Int64 = _make_dtype(scalar_type.Int64, "Int64")
Float = _make_dtype(scalar_type.Float, "float")
ByteSlice = _make_dtype(scalar_type.ByteSlice, "bytes")
Utf8 = _make_dtype(scalar_type.Utf8, "str")
Object = _make_dtype(scalar_type.Object, "object")
JsonUtf8 = _make_dtype(scalar_type.JsonUtf8, "Json")

cdef object UNDEFINED = object()
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

cdef class Tube:

    """
    The base class for all tubes.  Pipelines are typically built by calling
    the methods defines on this class.
    """

    cdef object _name_lookup
    cdef int _name_lookup_inited
    
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
        chains = Chains(self)
        made_chains, made_iters = chains.make_chains_iters()
        if len(self.dtype) == 1:
            return TubeSingleIter(made_iters[self], list(made_iters.values()), made_chains[None, self])
        return TubeMultiIter(made_iters[self], list(made_iters.values()), made_chains[None, self], len(self.dtype))

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
            self._name_lookup = NameLookup(self, [])
            self._name_lookup_inited = 1
        return self._name_lookup

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

    def split(self, sep="\n"):
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
        return Split(self, sep)

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

    def get(self, str key, default=UNDEFINED):
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
        return self.name_lookup().lookup_name(key, default)

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

        ``conditional`` must either be a callable that takes a single tube 
        argument (the parent), and returns a ``bool`` tube, or a  ``bool`` tube.

        Iterates over conditional and the parent together, yielding values only
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
        return Compare(self, Py_EQ, value)

    def gt(self, value):
        """
        Compatibility: list(tube.gt(value))

        Return a bool tube, that is True if the input is greater than value, otherwise False
        
        >>> list(Count().skip_unless(lambda x: x.gt(4)).first(2))
        [5, 6]
        """
        return Compare(self, Py_GT, value)

    def lt(self, value):
        """
        Compatibility: list(tube.lt(value))

        Return a bool tube, that is True if the input is less than value, otherwise False
        
        >>> list(Count().skip_unless(lambda x: x.lt(4)).first(100))
        [0, 1, 2, 3]
        """
        return Compare(self, Py_LT, value)

    def chunk(self, size_t num):
        """
        Compatibility: tube.chunk(2)

        Recursively look at the inputs to find an Each() tube.  If a single
        Each() tube with a list/tuple contents is found, split the value into
        ``num`` sized chunks, and chain them together.

        This is a bit of a hack to support some tricky use-cases (for example
        reading very large g-zipped files requires treating every file as a 
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


include "iter_defs.pxi"
include "chunk.pxi"