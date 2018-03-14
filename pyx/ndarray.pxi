import numpy as np
cimport numpy as np

cdef extern from "../src/util/ndarray.hpp" namespace "ss::iter":
    cdef cppclass NDArrayFiller:
        NDArrayFiller()
        void add_field(SlotPointer,np.dtype, size_t) except +
        inline void fill_row(np.ndarray, size_t)


ARRAY_GROWTH_SIZE = 32768

cdef np_dtype_from_dtype(ScalarType dtype, slot_info, index):
    if dtype == scalar_type.ByteSlice:
        if slot_info is None:
            raise ValueError("Need to provide length for slot {}".format(index))
        return (str(index), np.string_, slot_info+1)
    elif dtype == scalar_type.Int64:
        return (str(index), np.int64)
    # elif dtype == scalar_type.Utf8:
    #     if slot_info is None:
    #         raise ValueError("Need to provide length for slot {}".format(index))
    #     return (str(index), 'U{}'.format(slot_info+1))
    raise TypeError("Cannot convert {} to numpy type".format(c_dtype_to_dtype(dtype).name))


cdef np_dtype_from_iter(AnyIter input_iter, tuple slot_info):
    np_dtypes = []
    cdef size_t slot_index = 0;
    cdef Slice[SlotPointer] slots = input_iter.get().get_slots()
    cdef SlotPointer slot
    for slot_index in range(slots.len):
        slot = slots[slot_index]
        this_slot_info = None if slot_index >= len(slot_info) else slot_info[slot_index]
        np_dtypes.append(np_dtype_from_dtype(slot.type, this_slot_info, slot_index))
    if len(np_dtypes) == 1:
        return np.dtype(np_dtypes[0][1])
    return np.dtype(np_dtypes)


cdef setup_filler(NDArrayFiller *filler, AnyIter input_iter, np.ndarray array):
    cdef size_t slot_index = 0;
    cdef Slice[SlotPointer] slots = input_iter.get().get_slots()
    cdef SlotPointer slot
    cdef size_t offset
    cdef np.dtype dtype
    if slots.len == 1:
        slot = slots[0]
        dtype = array.dtype
        filler.add_field(slot, dtype, 0)
    else:
        for slot_index in range(slots.len):
            slot = slots[slot_index]
            dtype, offset = array.dtype.fields[str(slot_index)]
            filler.add_field(slot, dtype, offset)


cdef fill_array(np.ndarray array, NDArrayFiller *filler, list chain):
    cdef Chain c_chain = iters_to_c_chain(chain)
    cdef size_t cur_length = len(array)
    cdef size_t cur_index = 0
    while True:
        if cur_index >= cur_length:
            cur_length += ARRAY_GROWTH_SIZE
            array.resize(cur_length, refcheck=False)
        try:
            do_next(c_chain)
        except StopIteration:
            break
        filler.fill_row(array, cur_index)
        cur_index += 1 
    array.resize(cur_index, refcheck=False)



cdef np.ndarray ndarray_from_tube(Tube tube, tuple slot_info):
    cdef Chains chains = Chains(tube)
    made_chains, made_iters = chains.make_chains_iters()

    cdef IterWrapper output_iter = made_iters[tube]
    root_chain = made_chains[None, tube]

    cdef NDArrayFiller *nd_filler = new NDArrayFiller()

    cdef Slice[SlotPointer] slots = output_iter.iter.get().get_slots()
    np_dtype = np_dtype_from_iter(output_iter.iter, slot_info)
    cdef np.ndarray array = np.ndarray((ARRAY_GROWTH_SIZE, ), dtype=np_dtype)
    cdef NDArrayFiller * filler = new NDArrayFiller()
    try:
        setup_filler(filler, output_iter.iter, array)
        fill_array(array, filler, root_chain)
        return array
    finally:
        del filler
    