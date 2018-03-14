from cpython.ref cimport PyObject

cdef extern from "../src/iter.hpp" namespace "ss":
    cdef cppclass Slice[T]:
        size_t len;
        T &operator[](size_t);


cdef extern from "../src/iter.hpp" namespace "ss::iter":

    cdef cppclass SlotPointer:
        scalar_type.ScalarType type

    cdef cppclass Iter:
        Slice[SlotPointer] get_slots()

    cdef cppclass AnyIter:
        Iter *get()

    cdef struct Chain:
        void push_back(AnyIter &)

    cdef AnyIter to_any[T](T *source)
    void convert_stop_iteration()
    cdef void do_next(const Chain &iters) except +convert_stop_iteration

    ScalarType field_dtype_from_dtype(ScalarType)


cdef extern from "../src/util/pyobj.hpp" namespace "ss":
    
    cdef cppclass PyObj:
        PyObj()
        PyObj(PyObject*)

        PyObject *obj
        PyObj acquire()
        void incref()
        PyObject *give()


cdef class IterWrapper:
    cdef AnyIter iter

cdef inline wrap(AnyIter it):
    cdef IterWrapper wrapper = IterWrapper()
    wrapper.iter = it
    return wrapper
