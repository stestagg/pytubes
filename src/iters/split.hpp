#pragma once

#include <string>

#include "../scalar.hpp"
#include "../util/error.hpp"

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

    const Slice<T> *source_data;
    Chain chain;
    Slice<T> current;
    SlotPointer slot;

    Slice<T> remaining_source;
    std::vector<typename std::remove_cv<T>::type> buffer;
    T sep;
    bool done = false;
    bool buffer_pending = false;

public:

    SplitIter(Chain chain, AnyIter parent, T sep):
        source_data(parent->get_slots()[0]),
        chain(chain),
        slot(&current),
        sep(sep)
    {}

    Slice<SlotPointer> get_slots(){
        return Slice<SlotPointer>(&slot, 1);
    }

    void next() {
        if (remaining_source.is_empty()) {
            // Nothing left in the upstream chunk to search for separators in
            if (done) {
                // Parent threw StopIteration already, so bubble it up
                throw StopIteration;
            }
            try{
                // Try toget the next chunk from upstream
                do_next(chain);
                remaining_source = *source_data;
            } catch (StopIterationExc e) {
                // Parent has no more to give, so set done state, but check if
                // there's anything left over that was unterminated..
                // just to be safe, nuke cur_slice
                remaining_source = Slice<T>::Null();
                done = true;
                if (buffer_pending) {
                    // Found some, return a view onto this
                    current = Slice<T>(buffer.data(), buffer.size());
                    return;
                }
                // Bubble up the stop iteration if nothing saved
                throw;
            }
            // Now we've got a chunk, but there may be some data left from the
            // last chunk, and these have to be joined, so special case that
            if (buffer_pending) {
                const T *match = remaining_source.find_first(sep);
                auto this_line = remaining_source.slice_to_ptr(match);
                buffer.insert(buffer.end(), this_line.begin(), this_line.end());
                // Adjust the rest of the slice to compensate, and return
                // the whole buffer
                remaining_source = remaining_source.slice_from_ptr(match + 1);
                // We've returned the buffer, so not pending any more
                buffer_pending = false;
                current = Slice<T>(buffer.data(), buffer.size());
                return;
            }
        }
        // This is the happy-path, find the first separator, and return it..
        const T *match = remaining_source.find_first(sep);
        if (match == remaining_source.end()) {
            // We reached the end of the upstream chunk without finding a
            // separator(line end).
            // set buffer to be the remaining data, for later, allowing us
            // to call next() on the parent iter because we've got a copy
            // of the trailer part here.
            buffer.resize(remaining_source.len);
            buffer_pending = true;
            std::copy(remaining_source.begin(), remaining_source.end(), buffer.begin());
            // remaining_source is no longer useful, so make it null so we know next
            // time that it's exhausted
            remaining_source = Slice<T>::Null();
            // tail recurse to grab a new chunk and continue processing.
            return next();
        }
        current = remaining_source.slice_to_ptr(match);
        remaining_source = remaining_source.slice_from_ptr(match + 1);
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