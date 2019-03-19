
import pyarrow as pa
from cython.operator cimport dereference as deref
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr
cimport pyarrow.lib as pa

cdef extern from "../src/util/pa_table.hpp" namespace "ss::iter":
    shared_ptr[pa.CTable] fill_table(AnyIter iter, vector[string] fields, Chain &chain) except +


# cdef shared_ptr[pa.CDataType] pa_dtype_from_dtype(ScalarType dtype):
#     if dtype == scalar_type.ByteSlice:
#         return pa.GetPrimitiveType(pa._Type_BINARY)
#     elif dtype == scalar_type.Int64:
#         return pa.GetPrimitiveType(pa._Type_INT64)
#     elif dtype == scalar_type.Float:
#         return pa.GetPrimitiveType(pa._Type_DOUBLE)
#     elif dtype == scalar_type.Bool:
#         return pa.GetPrimitiveType(pa._Type_BOOL)
#     raise TypeError("Cannot convert {} to pyarrow type".format(c_dtype_to_dtype(dtype).name))


# cdef shared_ptr[pa.CSchema] pa_schema_from_iter(AnyIter input_iter, tuple fields):
#     cdef size_t slot_index = 0
#     cdef shared_ptr[pa.CKeyValueMetadata] no_meta
#     cdef str field_name
#     cdef vector[shared_ptr[pa.CField]] pa_fields
#     cdef shared_ptr[pa.CDataType] pa_type
#     cdef shared_ptr[pa.CField] pa_field
#     cdef Slice[SlotPointer] slots = input_iter.get().get_slots()

#     pa_fields.resize(slots.len)

#     for slot_index in range(slots.len):
#         slot = slots[slot_index]
#         field_name = fields[slot_index]
#         pa_type = pa_dtype_from_dtype(slot.type)
#         pa_fields[slot_index].reset(new pa.CField(field_name, pa_type, True))

#     return shared_ptr[pa.CSchema](new pa.CSchema(pa_fields, no_meta))


# cdef shared_ptr[pa.CTable] _pa_from_tube(AnyIter input_iter, tuple fields):
#     cdef shared_ptr[pa.CSchema] schema = pa_schema_from_iter(input_iter, fields)

#     return shared_ptr[pa.CTable]()


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
