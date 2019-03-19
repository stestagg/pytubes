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
        - "Iter *split_iter_from_dtype(Chain, AnyIter, PyObj&, PyObj&, bool_t) except +"
    Iter:
        SplitIter:
            template: T
            init: [AnyIter, T sep, T trim, bool_t skip_empty]
    Tube:
        Split:
            props: [
                {type: Tube, name: parent, dtypes: [ByteSlice, Utf8]},
                object sep,
                object trim='',
                bool_t skip_empty=True,
            ]
            dtype: return self.parent.dtype
            chains: ((self.parent, ), )
            custom_iter: |
                cdef PyObj sep_ob = PyObj(<PyObject*>self.sep)
                cdef PyObj trim_ob = PyObj(<PyObject*>self.trim)
                cdef Iter *iter = split_iter_from_dtype(
                    iters_to_c_chain(chains[0]),
                    parent.iter,
                    sep_ob,
                    trim_ob,
                    self.skip_empty,
                )
    ->*/

    StreamReader<T> reader;
    AnyIter parent;

    Slice<T> current;
    SlotPointer slot;
    T sep;
    T trim;
    bool do_trim;
    bool skip_empty;

public:

    SplitIter(Chain chain, AnyIter parent, T sep, T trim, bool do_trim, bool skip_empty) :
        reader(chain, parent->get_slots()[0]),
        parent(parent),
        slot(&current),
        sep(sep),
        trim(trim),
        do_trim(do_trim),
        skip_empty(skip_empty)
    {}

    Slice<SlotPointer> get_slots(){
        return Slice<SlotPointer>(&slot, 1);
    }

    void next() {
        current = reader.read_until_char(sep);
        if(do_trim) {
            current = current.rstrip(trim);
        }
        if(skip_empty && current.len == 0) {
            return next();
        }
    }
};

template<class T, class Enable>
struct split_iter_op{
    inline Iter *operator()(Chain chain, AnyIter parent, PyObj &sep, PyObj &trim, bool skip_empty) {
        throw_py<ValueError>(
            "Split has not been implemented on iterators of type ",
            ScalarType_t<T>::type_name()
            );
        return NULL;
    }
};

template<> inline Iter *split_iter_op<ByteSlice, bool>::operator() (Chain chain, AnyIter parent, PyObj &sep, PyObj &trim, bool skip_empty) {
    Converter<PyObj, ByteSlice> converter(&sep, std::string("ascii"));
    converter.convert();
    ByteSlice converted = *converter.to;
    Converter<PyObj, ByteSlice> trim_converter(&trim, std::string("ascii"));
    trim_converter.convert();
    ByteSlice trim_val = *trim_converter.to;
    throw_if(ValueError, converted.len != 1, "Splitting is currently only supported on a single character, not '", converted, "'");
    throw_if(ValueError, trim_val.len > 1, "Trimming is currently only supported on a single character, not '", trim_val, "' (length ", trim_val.len, ")");
    return new SplitIter<uint8_t>(chain, parent, converted[0], trim_val.len ? trim_val[0] : 0, trim_val.len, skip_empty);
}

Iter *split_iter_from_dtype(Chain chain, AnyIter parent, PyObj &sep, PyObj &trim, bool skip_empty) {
    return dispatch_type<split_iter_op>(parent->get_slots()[0].type, chain, parent, sep, trim, skip_empty);
}

}}