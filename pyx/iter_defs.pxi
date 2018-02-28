
from libc.stdint cimport int64_t, uint8_t, uint16_t, uint32_t
from libcpp.string cimport string
from libcpp cimport bool as bool_t
from libcpp.vector cimport vector


# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/get.hpp" namespace "ss::iter":
    
    Iter *slot_get_iter_from_dtype(AnyIter, size_t, PyObj&) except +
    

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/get.hpp" namespace "ss::iter":
    
    Iter *name_lookup_from_dtype(AnyIter, vector[string] &) except +
    

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/compare.hpp" namespace "ss::iter":
    
    Iter *compare_iter_from_cmp_dtype(AnyIter, int, PyObj&) except +
    

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/split.hpp" namespace "ss::iter":
    
    Iter *split_iter_from_dtype(Chain, AnyIter, PyObj&) except +
    

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/convert.hpp" namespace "ss::iter":
    
    void check_can_convert(ScalarType, ScalarType, string) except +
    



# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/each.hpp" namespace "ss::iter":
    cdef cppclass EachIter:
        EachIter(PyObject *) except +
        PyObj iter


# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/zip.hpp" namespace "ss::iter":
    cdef cppclass ZipIter:
        ZipIter(Chain, vector[AnyIter]) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/get.hpp" namespace "ss::iter":
    cdef cppclass SlotGetIter:
        SlotGetIter(AnyIter, size_t, PyObj) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/get.hpp" namespace "ss::iter":
    cdef cppclass NameLookupIter[T]:
        NameLookupIter(AnyIter, vector[string]) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/multi.hpp" namespace "ss::iter":
    cdef cppclass MultiIter:
        MultiIter(vector[AnyIter]) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/json.hpp" namespace "ss::iter":
    cdef cppclass JsonParseIter:
        JsonParseIter(AnyIter) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/topy.hpp" namespace "ss::iter":
    cdef cppclass ToPyIter:
        ToPyIter(AnyIter) except +
        const PyObj get(size_t index)


# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/skip_unless.hpp" namespace "ss::iter":
    cdef cppclass SkipUnlessIter:
        SkipUnlessIter(Chain, AnyIter, AnyIter) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/fileread.hpp" namespace "ss::iter":
    cdef cppclass ReadFileIter:
        ReadFileIter(Chain, AnyIter) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/chain.hpp" namespace "ss::iter":
    cdef cppclass ChainIter:
        ChainIter(vector[Chain], vector[AnyIter]) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/skip.hpp" namespace "ss::iter":
    cdef cppclass SkipIter:
        SkipIter(Chain, AnyIter, size_t) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/filemap.hpp" namespace "ss::iter":
    cdef cppclass FileMapIter:
        FileMapIter(AnyIter) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/gunzip.hpp" namespace "ss::iter":
    cdef cppclass ZlibDecodeIter:
        ZlibDecodeIter(Chain, AnyIter, bint) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/count.hpp" namespace "ss::iter":
    cdef cppclass CountIter:
        CountIter(size_t) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/compare.hpp" namespace "ss::iter":
    cdef cppclass CompareIter[T, Cmp]:
        CompareIter(AnyIter, T) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/first.hpp" namespace "ss::iter":
    cdef cppclass FirstIter:
        FirstIter(AnyIter, size_t) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/split.hpp" namespace "ss::iter":
    cdef cppclass SplitIter[T]:
        SplitIter(AnyIter, T sep) except +
        

# AUTO GENERATED FROM tools/defn.tmpl
cdef extern from "/Users/stephenstagg/Dropbox/src/tubes/cpp2/src/iters/convert.hpp" namespace "ss::iter":
    cdef cppclass ConvertIter:
        ConvertIter(AnyIter, vector[ScalarType], string) except +
        





@cython.final
cdef class Each(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    """
    Iterate over the provided python object, as an input to a tube.
    Takes one argument, which should either be a python iterator/generator,
    or an iterable.

    >>> list(Each([1, 2, 3]))
    [1, 2, 3]
    >>> list(Each(itertools.count()).first(5))
    [0, 1, 2, 3, 4]
    >>> list(Each(i*2 for i in range(5)))
    [0, 2, 4, 6, 8]

    """
    
    cdef public object _iter

    def __cinit__(self, object _iter):
        self._iter = _iter
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef EachIter *iter = new EachIter(self._ob_to_iter().obj)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return (Object,)

    @property
    def _chains(self):
        return ()

    
    @property
    def _inputs(self):
        return ()
    

    

    cdef PyObj _ob_to_iter(self):
        if hasattr(self._iter, "__next__"):
            return PyObj(<PyObject *>self._iter)
        cdef object real_it = iter(self._iter)
        return PyObj(<PyObject *>real_it)

    cpdef _describe_self(self):
        iter_desc = repr(self._iter)
        if len(iter_desc) > 20:
            iter_desc = iter_desc[:9] + "..." + iter_desc[-9:]
        return f"Each({iter_desc})"


@cython.final
cdef class Zip(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public list inputs

    def __cinit__(self, list inputs):
        self.inputs = inputs
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        
        cdef ZipIter *iter = new ZipIter(iters_to_c_chain(chains[0]), self._make_iters(iters))

        return wrap(to_any(iter))

    @property
    def dtype(self):
        return tuple(dty for t in self.inputs for dty in t.dtype)

    @property
    def _chains(self):
        return (tuple(self.inputs), )

    

    

    @property
    def _inputs(self):
        return tuple(self.inputs)

    cdef vector[AnyIter] _make_iters(self, list args):
        cdef IterWrapper arg
        cdef vector[AnyIter] its
        for arg in args:
            its.push_back(arg.iter)
        return its

    cpdef _describe_self(self):
        cdef Tube i
        input_reprs = [i._repr(stop=set(self.inputs)) for i in self.inputs]
        return f"Zip({input_reprs})"


@cython.final
cdef class SlotGet(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent
    cdef public size_t index
    cdef public object default_val

    def __cinit__(self, Tube parent, size_t index, object default_val):
        self.parent = parent
        self.index = index
        self.default_val = default_val
        
        if index > <size_t>len(parent.dtype):
            raise IndexError(f"Cannot get slot {index} from parent with {len(parent.dtype)} slots")


    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        
        cdef PyObj default_ob = PyObj(<PyObject*>self.default_val)
        cdef Iter *iter = slot_get_iter_from_dtype(parent.iter, self.index, default_ob)

        return wrap(to_any(iter))

    @property
    def dtype(self):
        return (self.parent.dtype[self.index], )

    @property
    def _chains(self):
        return ()

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"SlotGet({repr(self.index)}, {repr(self.default_val)})"
    

    

@cython.final
cdef class NameLookup(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent
    cdef public list items
    cdef public dict _name_lookups

    def __cinit__(self, Tube parent, list items, dict _name_lookups=None):
        self.parent = parent
        self.items = items
        self._name_lookups = _name_lookups
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        
        cdef vector[string] fields = [f.encode('utf-8') for f in self.items]
        cdef Iter *iter = name_lookup_from_dtype(parent.iter, fields)

        return wrap(to_any(iter))

    @property
    def dtype(self):
        return (self.parent.dtype[0], ) * len(self.items)

    @property
    def _chains(self):
        return ()

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"NameLookup({repr(self.items)})"
    

    cdef lookup_name(self, str name, default):
        try:
            index = self.items.index(name)
        except ValueError:
            self.items.append(name)
            index = len(self.items) - 1
        return self.get_slot_shared(index, default)

    cdef get_slot_shared(self, size_t index, default):
        cdef Tube tube = SlotGet(self, index, default)
        if self._name_lookups is None:
            self._name_lookups = {}
        if index not in self._name_lookups:
            shared_index_get = SlotGet(self, index, b"")
            self._name_lookups[index] = NameLookup(shared_index_get, [])
        tube._set_name_lookup(self._name_lookups[index])
        return tube
        


@cython.final
cdef class Multi(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent
    cdef public list inputs

    def __cinit__(self, Tube parent, list inputs):
        self.parent = parent
        self.inputs = inputs
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        
        cdef MultiIter *iter = new MultiIter(self._make_iters(args))

        return wrap(to_any(iter))

    @property
    def dtype(self):
        return tuple(d for t in self._inputs[1:] for d in t.dtype)

    @property
    def _chains(self):
        return ()

    

    

    @property
    def _inputs(self):
        return (self.parent, ) + tuple(self.inputs)

    cdef vector[AnyIter] _make_iters(self, list args):
        cdef IterWrapper arg
        cdef vector[AnyIter] its
        for arg in args[1:]:
            its.push_back(arg.iter)
        return its

    cpdef _describe_self(self):
        cdef Tube i
        input_reprs = [i._repr(stop=self.parent) for i in self.inputs]
        return f"Multi({input_reprs})"

            


@cython.final
cdef class JsonParse(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent

    def __cinit__(self, Tube parent):
        
        if parent.dtype[0] not in (Utf8,):
            raise ValueError(f"Cannot make a JsonParse Tube with 'parent' tube of type { parent.dtype[0] }")
        self.parent = parent
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        cdef JsonParseIter *iter = new JsonParseIter(parent.iter)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return (JsonUtf8,)

    @property
    def _chains(self):
        return ()

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"JsonParse()"
    

    

@cython.final
cdef class ToPy(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent

    def __cinit__(self, Tube parent):
        self.parent = parent
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        cdef ToPyIter *iter = new ToPyIter(parent.iter)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return (Object, ) * len(self.parent.dtype)

    @property
    def _chains(self):
        return ()

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"ToPy()"
    

    

@cython.final
cdef class SkipUnless(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent
    cdef public Tube conditional

    def __cinit__(self, Tube parent, Tube conditional):
        self.parent = parent
        
        if conditional.dtype[0] not in (Bool,):
            raise ValueError(f"Cannot make a SkipUnless Tube with 'conditional' tube of type { conditional.dtype[0] }")
        self.conditional = conditional
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        cdef IterWrapper conditional = iters[1]
        cdef SkipUnlessIter *iter = new SkipUnlessIter(iters_to_c_chain(chains[0]), parent.iter, conditional.iter)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return self.parent.dtype

    @property
    def _chains(self):
        return ((self.parent, self.conditional), )

    
    @property
    def _inputs(self):
        return (self.parent, self.conditional, )
    

    

    cpdef _describe_self(self):
        return f"SkipUnless({self.conditional._repr(self.parent)})"


@cython.final
cdef class ReadFile(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent

    def __cinit__(self, Tube parent):
        
        if parent.dtype[0] not in (ByteSlice,):
            raise ValueError(f"Cannot make a ReadFile Tube with 'parent' tube of type { parent.dtype[0] }")
        self.parent = parent
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        cdef ReadFileIter *iter = new ReadFileIter(iters_to_c_chain(chains[0]), parent.iter)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return (ByteSlice, )

    @property
    def _chains(self):
        return ((self.parent, ), )

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"ReadFile()"
    

    

@cython.final
cdef class ChainTubes(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public list parents

    def __cinit__(self, list parents):
        self.parents = parents
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        
        unique_inputs = set()
        for p in self.parent:
            unique_inputs.add(p.dtype)
        # This should work...
        # if len(set(p.dtype for p in self.parent)) > 1:
        #     raise ValueError("Chain requires all inputs to have the same dtype")
        if len(unique_inputs) > 1:
            raise ValueError("Chain requires all inputs to have the same dtype")
        cdef ChainIter *iter = new ChainIter(self._make_chains(chains), self._make_iters(iters))

        return wrap(to_any(iter))

    @property
    def dtype(self):
        return self.parents[0].dtype

    @property
    def _chains(self):
        return tuple((p, ) for p in self.parents)

    

    
    cpdef _describe_self(self):
        return f"ChainTubes({repr(self.parents)})"
    

    @property
    def _inputs(self):
        return tuple(self.parents)

    cdef vector[AnyIter] _make_iters(self, list args):
        cdef IterWrapper arg
        cdef vector[AnyIter] its
        for arg in args:
            its.push_back(arg.iter)
        return its

    cdef vector[Chain] _make_chains(self, list args):
        cdef list arg
        cdef Chain chain
        cdef vector[Chain] chains
        for arg in args:
            chain = iters_to_c_chain(arg)
            chains.push_back(chain)
        return chains


@cython.final
cdef class Skip(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent
    cdef public size_t num

    def __cinit__(self, Tube parent, size_t num):
        self.parent = parent
        self.num = num
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        cdef SkipIter *iter = new SkipIter(iters_to_c_chain(chains[0]), parent.iter, self.num)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return self.parent.dtype

    @property
    def _chains(self):
        return ((self.parent,), )

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"Skip({repr(self.num)})"
    

    

@cython.final
cdef class FileMap(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent

    def __cinit__(self, Tube parent):
        
        if parent.dtype[0] not in (ByteSlice,):
            raise ValueError(f"Cannot make a FileMap Tube with 'parent' tube of type { parent.dtype[0] }")
        self.parent = parent
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        cdef FileMapIter *iter = new FileMapIter(parent.iter)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return (ByteSlice, )

    @property
    def _chains(self):
        return ()

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"FileMap()"
    

    

@cython.final
cdef class Gunzip(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent
    cdef public bint stream

    def __cinit__(self, Tube parent, bint stream):
        self.parent = parent
        self.stream = stream
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        cdef ZlibDecodeIter *iter = new ZlibDecodeIter(iters_to_c_chain(chains[0]), parent.iter, self.stream)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return (ByteSlice, )

    @property
    def _chains(self):
        return ((self.parent,), )

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"Gunzip({repr(self.stream)})"
    

    

@cython.final
cdef class Count(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    """
    Iterator that behaves similarly to :func:`itertools.count`.

    Takes an optional numeric argument ``start`` that sets the 
    first number returned by Count()  [default:0]

    >>> list(Count().first(5))
    [0, 1, 2, 3, 4]
    >>> list(Count(10).first(5))
    [10, 11, 12, 13, 14]

    """
    
    cdef public size_t _start

    def __cinit__(self, size_t _start=0):
        self._start = _start
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef CountIter *iter = new CountIter(self._start)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return (Int64,)

    @property
    def _chains(self):
        return ()

    
    @property
    def _inputs(self):
        return ()
    

    
    cpdef _describe_self(self):
        return f"Count({repr(self._start)})"
    

    

@cython.final
cdef class Compare(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent
    cdef public int op
    cdef public object value

    def __cinit__(self, Tube parent, int op, object value):
        self.parent = parent
        self.op = op
        self.value = value
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        
        cdef PyObj value_ob = PyObj(<PyObject*>self.value)
        cdef Iter *iter = compare_iter_from_cmp_dtype(parent.iter, self.op, value_ob)

        return wrap(to_any(iter))

    @property
    def dtype(self):
        return (Bool, )

    @property
    def _chains(self):
        return ()

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"Compare({repr(self.op)}, {repr(self.value)})"
    

    

@cython.final
cdef class First(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent
    cdef public size_t num

    def __cinit__(self, Tube parent, size_t num):
        self.parent = parent
        self.num = num
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        cdef FirstIter *iter = new FirstIter(parent.iter, self.num)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return self.parent.dtype

    @property
    def _chains(self):
        return ()

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"First({repr(self.num)})"
    

    

@cython.final
cdef class Split(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent
    cdef public object sep

    def __cinit__(self, Tube parent, object sep):
        
        if parent.dtype[0] not in (ByteSlice, Utf8,):
            raise ValueError(f"Cannot make a Split Tube with 'parent' tube of type { parent.dtype[0] }")
        self.parent = parent
        self.sep = sep
        
        

    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        
        cdef PyObj sep_ob = PyObj(<PyObject*>self.sep)
        cdef Iter *iter = split_iter_from_dtype(iters_to_c_chain(chains[0]), parent.iter, sep_ob)

        return wrap(to_any(iter))

    @property
    def dtype(self):
        return self.parent.dtype

    @property
    def _chains(self):
        return ((self.parent, ), )

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"Split({repr(self.sep)})"
    

    

@cython.final
cdef class Convert(Tube):  # AUTO GENERATED FROM tools/defn2.tmpl
    
    cdef public Tube parent
    cdef public list to_types
    cdef public bytes codec

    def __cinit__(self, Tube parent, list to_types, bytes codec=b"utf-8"):
        self.parent = parent
        self.to_types = to_types
        self.codec = codec
        
        if len(parent.dtype) < len(self.dtype):
            raise ValueError("Convert iter cannot have more elements than parent")
        cdef DType from_dtype
        cdef DType to_dtype 
        for from_dtype, to_dtype in zip(parent.dtype, self.dtype):
            check_can_convert(from_dtype.type, to_dtype.type, codec)


    cdef IterWrapper _make_iter(self, args):
        
        expected_args = len(self._inputs) + len(self._chains)
        if len(args) != expected_args:
            raise ValueError(f"Expected {expected_args} inputs to _make_iter, got {len(args)}")
        
        chains = args[:len(self._chains)]
        iters = args[len(self._chains):]
        cdef IterWrapper parent = iters[0]
        cdef ConvertIter *iter = new ConvertIter(parent.iter, self.dtype_vec(), self.codec)
        
        return wrap(to_any(iter))

    @property
    def dtype(self):
        return tuple(self.to_types)

    @property
    def _chains(self):
        return ()

    
    @property
    def _inputs(self):
        return (self.parent, )
    

    
    cpdef _describe_self(self):
        return f"Convert({repr(self.to_types)}, {repr(self.codec)})"
    

    cdef vector[scalar_type.ScalarType] dtype_vec(self):
        cdef DType dtype
        cdef vector[scalar_type.ScalarType] types
        for dtype in self.to_types:
            types.push_back(dtype.type)
        return types

