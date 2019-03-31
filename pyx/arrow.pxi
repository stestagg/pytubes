
import pyarrow as pa
from cython.operator cimport dereference as deref
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr
cimport pyarrow.lib as pa


cdef extern from "../src/util/pa_table.hpp" namespace "ss::iter":
    shared_ptr[pa.CTable] fill_table(AnyIter iter, vector[string] fields, Chain &chain) except +


cpdef pa_from_tube(Tube tube, fields):
    cdef Chains chains = Chains(tube)

    made_chains, made_iters = chains.make_chains_iters()
    cdef IterWrapper output_iter = made_iters[tube]
    cdef Chain root_chain = iters_to_c_chain(made_chains[None, tube])

    cdef Slice[SlotPointer] slots = output_iter.iter.get().get_slots()

    field_bytes = [f.encode('utf-8') for f in fields]

    cdef shared_ptr[pa.CTable] c_table = fill_table(
        output_iter.iter,
        field_bytes,
        root_chain
    )
    return pa.pyarrow_wrap_table(c_table)
