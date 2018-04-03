import numpy as np
cimport numpy as np
np.import_array()

cdef extern from "../src/util/ndarray.hpp" namespace "ss::iter":
    cdef cppclass NDArrayFiller:
        NDArrayFiller()
        void add_field(SlotPointer,np.dtype, size_t) except +
        inline void fill_row(np.ndarray, size_t)
    void fill_ndarray(np.PyArrayObject *array, NDArrayFiller *filler, Chain &chain, size_t growth_factor)


cdef np_dtype_from_dtype(ScalarType dtype, slot_info, index):
    if dtype == scalar_type.ByteSlice:
        if slot_info is None:
            raise ValueError("Need to provide length for slot {}".format(index))
        return (np.string_, slot_info+1)
    elif dtype == scalar_type.Int64:
        return np.int64
    elif dtype == scalar_type.Float:
        return np.double
    elif dtype == scalar_type.Bool:
        return np.bool_
    elif dtype == scalar_type.Object:
        return np.object_
    raise TypeError("Cannot convert {} to numpy type".format(c_dtype_to_dtype(dtype).name))


cdef np_dtypes_from_iter(AnyIter input_iter, tuple slot_info):
    np_dtypes = []
    cdef size_t slot_index = 0;
    cdef Slice[SlotPointer] slots = input_iter.get().get_slots()
    cdef SlotPointer slot
    for slot_index in range(slots.len):
        slot = slots[slot_index]
        this_slot_info = None if slot_index >= len(slot_info) else slot_info[slot_index]
        np_dtype = np_dtype_from_dtype(slot.type, this_slot_info, slot_index)
        np_dtypes.append(np_dtype)
    return np_dtypes


cdef setup_field_filler(NDArrayFiller *filler, AnyIter input_iter, np.ndarray array):
    cdef size_t slot_index = 0;
    cdef Slice[SlotPointer] slots = input_iter.get().get_slots()
    cdef SlotPointer slot
    cdef size_t offset
    cdef np.dtype dtype
    for slot_index in range(slots.len):
        slot = slots[slot_index]
        dtype, offset = array.dtype.fields[str(slot_index)]
        filler.add_field(slot, dtype, offset)


cdef setup_dimension_filler(NDArrayFiller *filler, AnyIter input_iter, np.ndarray array):
    cdef size_t slot_index = 0;
    cdef Slice[SlotPointer] slots = input_iter.get().get_slots()
    cdef SlotPointer slot
    cdef size_t offset = 0
    cdef size_t value_stride = array.strides[1]
    cdef np.dtype dtype
    for slot_index in range(slots.len):
        slot = slots[slot_index]
        filler.add_field(slot, array.dtype, offset)
        offset += value_stride


cdef np.ndarray ndarray_from_tube(Tube tube, tuple slot_info, size_t estimated_rows=32768, fields=None):
    if estimated_rows < 2:
        estimated_rows = 2
    cdef Chains chains = Chains(tube)
    made_chains, made_iters = chains.make_chains_iters()

    cdef IterWrapper output_iter = made_iters[tube]
    cdef Chain root_chain = iters_to_c_chain(made_chains[None, tube])

    cdef NDArrayFiller *nd_filler = new NDArrayFiller()

    cdef Slice[SlotPointer] slots = output_iter.iter.get().get_slots()
    np_dtypes = np_dtypes_from_iter(output_iter.iter, slot_info)

    cdef NDArrayFiller * filler = new NDArrayFiller()
    cdef size_t initial_rowcount = estimated_rows + 1

    cdef np.ndarray array
    if len(set(np_dtypes)) == 1 and not fields:
        # All 'columns' are identical types, so create a standard n-dimentional array
        array = np.ndarray((initial_rowcount, len(np_dtypes)), dtype=np_dtypes[0])
        setup_dimension_filler(filler, output_iter.iter, array)
    else:
        if fields == False:
            raise ValueError("Cannot create non-field based ndarray with differing dtypes")
        # 'Column' types differ, use fields
        field_dtype = np.dtype([(str(i), d) for i, d in enumerate(np_dtypes)])
        array = np.ndarray((initial_rowcount, ), dtype=field_dtype)
        setup_field_filler(filler, output_iter.iter, array)

    try:
        fill_ndarray(<np.PyArrayObject*>array, filler, root_chain, estimated_rows//2)
    finally:
        del filler

    return array.squeeze()
    