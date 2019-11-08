
import pyarrow
from cython.operator cimport dereference as deref
from libcpp.vector cimport vector
from libcpp.memory cimport shared_ptr
from libc.stdint cimport uint8_t, int64_t


cdef extern from 'arrow/buffer.h' namespace 'arrow':
    cdef cppclass Buffer:
        int64_t size() const
        const uint8_t *data() const

cdef extern from 'arrow/array.h' namespace 'arrow':
    cdef cppclass ArrayData:
        int64_t length
        vector[shared_ptr[Buffer]] buffers

cdef extern from "../src/util/pyarrow.hpp" namespace "ss::iter":
    vector[PyObj] fill_arrays(AnyIter iter, vector[string] fields, Chain &chain) except +


cdef class BridgedBuffer:
    cdef shared_ptr[Buffer] arrow_buffer

    def get(self):
        cdef Buffer *buf = self.arrow_buffer.get()
        return pyarrow.foreign_buffer(
            <size_t>(buf.data()),
            buf.size(),
            self
        )


cdef inline bridge_buffer(shared_ptr[Buffer] buf):
    cdef BridgedBuffer shared = BridgedBuffer()
    shared.arrow_buffer = buf
    return shared


cdef public object pyarrow_make_simple_array(const char *type_name, shared_ptr[ArrayData] array_data_ptr):
    cdef bytes type_name_str = type_name
    cdef ArrayData *array_data = array_data_ptr.get()
    cdef Buffer *buf
    pa_type = getattr(pyarrow, type_name_str.decode('utf8'))()
    bridged_buffers = []
    for buf_ptr in array_data.buffers:
        bridged_buffers.append(bridge_buffer(buf_ptr).get())
    return pyarrow.Array.from_buffers(pa_type, array_data.length, bridged_buffers)


cdef public object pyarrow_make_str_array(shared_ptr[ArrayData] array_data_ptr):
    cdef ArrayData *array_data = array_data_ptr.get()
    values_array = bridge_buffer(array_data.buffers[1]).get()
    data_array = bridge_buffer(array_data.buffers[2]).get()
    return pyarrow.StringArray.from_buffers(array_data.length, values_array, data_array)


cpdef pa_from_tube(Tube tube, fields):
    cdef Chains chains = Chains(tube)

    made_chains, made_iters = chains.make_chains_iters()
    cdef IterWrapper output_iter = made_iters[tube]
    cdef Chain root_chain = iters_to_c_chain(made_chains[None, tube])

    cdef Slice[SlotPointer] slots = output_iter.iter.get().get_slots()

    field_bytes = [f.encode('utf-8') for f in fields]
    
    arrays = fill_arrays(
        output_iter.iter,
        field_bytes,
        root_chain
    )
    cdef list columns = [] 
    cdef PyObj array
    for array in arrays:
        column = <object>array.obj
        columns.append(column)
    return pyarrow.Table.from_arrays(columns, fields)