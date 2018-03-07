import numpy as np
cimport numpy as np

ARRAY_GROWTH_SIZE = 16,384


cdef np_dtype_from_dtype(dtype, slot_info, index):
    if dtype is ByteSlice:
        if slot_info is None:
            raise ValueError("Need to provide length for slot {}".format(index))
        return (str(index), 'a{}'.format(slot_info+1))
    raise TypeError("Cannot convert {} to numpy type".format(dtype))


cdef np_dtype_from_tube(Tube tube, tuple slot_info):
    np_dtypes = []
    for index, dtype in enumerate(tube.dtype):
        this_slot_info = None if index >= len(slot_info) else slot_info[index]
        np_dtypes.append(np_dtype_from_dtype(dtype, this_slot_info, index))
    return np.dtype(np_dtypes)


cdef np.ndarray ndarray_from_tube(Tube tube, tuple slot_info):
    cdef Chains chains = Chains(tube)
    made_chains, made_iters = chains.make_chains_iters()
    cdef AnyIter output_iter = made_iters[tube]
    root_chain = made_chains[None, self]

    np_dtype = np_dtype_from_tube(tube, slot_info)
    cdef np.ndarray array = np.ndarray((ARRAY_GROWTH_SIZE, ), dtype=np_dtype)
    
    field_info = [array.dtype.fields[str(x)] for x in range(len(tube.dtype))]

    cdef NdFiller nd_filler = make_nd_array_filler(output_iter, field_info)

    cdef size_t cur_length = ARRAY_GROWTH_SIZE
    cdef size_t cur_index = 0
    while True:
        try:
            do_next(root_chain)
        except StopIteration:
            break
        if cur_index >= cur_length:
            PyArray_Resize(array, x, 0, np.NPY_KEEPORDER)
        cur_index += 1
