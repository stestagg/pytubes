#pragma once

#include <Python.h>

#include "../util/pyobj.hpp"
#include "../iter.hpp"


namespace ss{ namespace iter{

class EachIter: public Iter {
    /*<-
        Iter: 
            EachIter: 
                init: [PyObject *]
                extra: >
                    PyObj iter
        Tube:
            Each:
                props: [object _iter]
                dtype: return (Object,)
                iter: [EachIter, [self._ob_to_iter().obj]]
                methods: >
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
                docstring: |
                    Iterate over the provided python object, as an input to a tube.
                    Takes one argument, which should either be a python iterator/generator,
                    or an iterable.

                    >>> list(Each([1, 2, 3]))
                    [1, 2, 3]
                    >>> list(Each(itertools.count()).first(5))
                    [0, 1, 2, 3, 4]
                    >>> list(Each(i*2 for i in range(5)))
                    [0, 2, 4, 6, 8]
    ->*/
public:
    PyObj iter;
    PyObj cur;
    SlotPointer slot;
    

    EachIter(PyObject *iter): iter(iter), slot(&this->cur) {
        throw_if(ValueError, !PyIter_Check(iter), "PyIter requires an iterable or iterator object");
    }

    Slice<SlotPointer> get_slots(){
        return Slice<SlotPointer>(&slot, 1);
    }

    void next(){
        cur = PyObj(PyIter_Next(iter.obj), true);
        if (!cur.obj) {
            throw StopIteration;
        }
    }
};

}}