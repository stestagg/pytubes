
try:
    import pyarrow
    HAVE_PYARROW=1
except ImportError:
    HAVE_PYARROW = 0

from cython.operator cimport dereference as deref
from libcpp.vector cimport vector
from libcpp.memory cimport unique_ptr
from libc.stdint cimport uint8_t, int64_t, intptr_t


cdef extern from * namespace "polyfill":
    """
    namespace polyfill {

    template <typename T>
    inline typename std::remove_reference<T>::type&& move(T& t) {
        return std::move(t);
    }

    template <typename T>
    inline typename std::remove_reference<T>::type&& move(T&& t) {
        return std::move(t);
    }

    }  // namespace polyfill
    """
    cdef T move[T](T)

cdef extern from "../src/util/pyarrow.hpp" namespace "ss::iter":
    vector[PyObj] fill_arrays(AnyIter iter, Chain &chain) except +


cdef extern from "../src/util/arrow/buffer.hpp" namespace "ss::arrow":
    cdef cppclass AnyBuffer:
        void *data()
        size_t size()


cdef class BridgedBuffer:
    cdef unique_ptr[AnyBuffer] arrow_buffer

    def size(self):
        return self.arrow_buffer.get().size()

    def get(self):
        cdef AnyBuffer *buf = self.arrow_buffer.get()
        cdef size_t num_items = buf.size()
        return pyarrow.foreign_buffer(
            <intptr_t>(buf.data()),
            num_items,
            self
        )

cdef public object pyarrow_make_buffer(unique_ptr[AnyBuffer] buf):
    cdef BridgedBuffer bridged = BridgedBuffer()
    bridged.arrow_buffer = move(buf)
    return bridged.get()

cdef public object pyarrow_make_array(const char *type_name, size_t array_len, list buffers):
    cdef bytes type_name_str = type_name
    pa_type = getattr(pyarrow, type_name_str.decode('utf8'))()
    return pyarrow.Array.from_buffers(pa_type, array_len, buffers)


cdef public object pyarrow_make_str_array(size_t array_len, offsets, data):
    return pyarrow.StringArray.from_buffers(array_len, offsets, data)


cpdef pa_from_tube(Tube tube, fields):
    cdef Chains chains = Chains(tube)

    made_chains, made_iters = chains.make_chains_iters()
    cdef IterWrapper output_iter = made_iters[tube]
    cdef Chain root_chain = iters_to_c_chain(made_chains[None, tube])

    cdef Slice[SlotPointer] slots = output_iter.iter.get().get_slots()

    arrays = fill_arrays(
        output_iter.iter,
        root_chain
    )
    cdef list columns = []
    cdef PyObj array
    for array in arrays:
        column = <object>array.obj
        columns.append(column)
    return pyarrow.Table.from_arrays(columns, fields)