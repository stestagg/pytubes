#pragma once

#include "../convert.hpp"
#include "../iter.hpp"

namespace ss{ namespace iter{

    template<class T>
    class StrLenIter : public Iter {
        /*<-
        Fn:
            - "Iter *strlen_iter_from_dtype(AnyIter) except +"
        Iter:
            StrLenIter: {
                template: [T],
                init: [AnyIter]
            }
        Tube:
            StrLen:
                props: [Tube parent]
                dtype: return (Int64, )
                custom_iter: |
                    cdef Iter *iter = strlen_iter_from_dtype(parent.iter)
        ->*/

        const T *parent;
        int64_t value;
        SlotPointer slot;

    public:
        StrLenIter(AnyIter parent)
            : parent(parent->get_slots()[0]),
              value(0),
              slot(&value)
            {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next();

    };

    template<> void StrLenIter<ByteSlice>::next() {
        value = (int64_t)parent->len;
    }

    inline bool is_utf8_char_start(unsigned char val){
        return (val & 0xc0) != 0x80;
    }
    template<> void StrLenIter<Utf8>::next() {
        value = std::count_if(parent->begin(), parent->end(), is_utf8_char_start);
    }

    template<class T, class Enable>
    struct strlen_iter_op{
        inline Iter *operator()(AnyIter parent) {
            throw_py<ValueError>(
                "Cannot calculate strlen on type ",
                ScalarType_t<T>::type_name()
                );
            return NULL;
        }
    };

    template<> inline Iter *strlen_iter_op<ByteSlice, bool>::operator() (AnyIter parent) {
        return new StrLenIter<ByteSlice>(parent);
    }

    template<> inline Iter *strlen_iter_op<Utf8, bool>::operator() (AnyIter parent) {
        return new StrLenIter<Utf8>(parent);
    }

    Iter *strlen_iter_from_dtype(AnyIter parent) {
        return dispatch_type<strlen_iter_op>(parent->get_slots()[0].type, parent);
    }

}}