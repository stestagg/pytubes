#pragma once

#include <string>

#include "../scalar.hpp"
#include "../util/error.hpp"
#include "../util/stream_reader.hpp"

#include "convert.hpp"

namespace ss{ namespace iter{

template<class T>
class SplitIter : public Iter {
    /*<-
    Fn:
        - "Iter *split_iter_from_dtype(Chain, AnyIter, PyObj&) except +"
    Iter:
        SplitIter:
            template: T
            init: [AnyIter, T sep]
    Tube:
        Split:
            props: [{type: Tube, name: parent, dtypes: [ByteSlice, Utf8]}, object sep]
            dtype: return self.parent.dtype
            chains: ((self.parent, ), )
            custom_iter: |
                cdef PyObj sep_ob = PyObj(<PyObject*>self.sep)
                cdef Iter *iter = split_iter_from_dtype(iters_to_c_chain(chains[0]), parent.iter, sep_ob)
    ->*/

    StreamReader<T> reader;
    AnyIter parent;

    Slice<T> current;
    SlotPointer slot;
    T sep;

public:

    SplitIter(Chain chain, AnyIter parent, T sep) :
        reader(chain, parent->get_slots()[0]),
        parent(parent),
        slot(&current),
        sep(sep)
    {}

    Slice<SlotPointer> get_slots(){
        return Slice<SlotPointer>(&slot, 1);
    }

    void next() {
        current = reader.read_until_char(sep);
    }
};

template<class T, class Enable>
struct split_iter_op{
    inline Iter *operator()(Chain chain, AnyIter parent, PyObject *sep) {
        throw_py<ValueError>(
            "Split has not been implemented on iterators of type ",
            ScalarType_t<T>::type_name()
            );
        return NULL;
    }
};

template<> inline Iter *split_iter_op<ByteSlice, bool>::operator() (Chain chain, AnyIter parent, PyObject *sep) {
    PyObj s(sep);
    Converter<PyObj, ByteSlice> converter(&s, std::string("ascii"));
    converter.convert();
    ByteSlice converted = *converter.to;
    throw_if(ValueError, converted.len != 1, "Splitting is currently only supported on a single character, not '", converted, "'");
    return new SplitIter<uint8_t>(chain, parent, converted[0]);
}

Iter *split_iter_from_dtype(Chain chain, AnyIter parent, PyObj &sep) {
    return dispatch_type<split_iter_op>(parent->get_slots()[0].type, chain, parent, sep.obj);
}

}}