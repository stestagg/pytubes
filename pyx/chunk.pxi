
from functools import partial

def update_each_iter(object chunk, IterWrapper wrapped):
    cdef Iter *it = wrapped.iter.get()
    cdef EachIter *each = <EachIter*>it
    py_iter = iter(chunk)
    each.iter = PyObj(<PyObject *>py_iter)


@cython.final
cdef class Chunk(Tube):
    cdef public Tube parent
    cdef size_t chunk_size

    def __cinit__(self, Tube parent, size_t chunk_size=1):
        self.parent = parent
        self.chunk_size = chunk_size

    cdef IterWrapper _make_iter(self, args):
        cdef IterWrapper output_iter
        chains = Chains(self.parent)
        each_inputs = {i for i in chains.inputs if isinstance(i, Each)}
        if len(each_inputs) != 1:
            raise ValueError("Cannot chunk tubes without a single fixed-sized Each input")
        cdef Each each_input = each_inputs.pop()
        try:
            total_num = len(each_input._iter)
        except TypeError:
            raise ValueError("Found an Each input, but cannot get len() of it")
        iters = []
        cdef vector[Chain] c_chains
        stopped = False
        for i in range(0, total_num, self.chunk_size):
            chunk = each_input._iter[i:i+self.chunk_size]
            made_chains, made_iters = chains.make_chains_iters(modifiers={each_input: partial(update_each_iter, chunk)})
            output_iter = made_iters[self.parent]
            iters.append(output_iter)
            if not stopped:
                stopped = True
            c_chains.push_back(iters_to_c_chain(made_chains[None, self.parent]))
        cdef ChainIter *iter = new ChainIter(c_chains, self._make_iters(iters))
        return wrap(to_any(iter))

    cdef vector[AnyIter] _make_iters(self, list args):
        cdef IterWrapper arg
        cdef vector[AnyIter] its
        for arg in args:
            its.push_back(arg.iter)
        return its

    cpdef _describe_self(self):
        return f"Chunk({self.parent._repr()}, {self.chunk_size})"

    @property
    def dtype(self):
        return self.parent.dtype

    @property
    def _chains(self):
        return ()

    @property
    def _inputs(self):
        return ()

