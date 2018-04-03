#pragma once

#include <unordered_map>

#include "../util/default.hpp"
#include "../iter.hpp"

namespace ss{ namespace iter{

    template<class T> bool is_undefined(T *val){ return false; }
    template<> bool is_undefined(const JsonUtf8 *val) {
        return val->type == json::Type::Unset;
    }
    template<class T> bool is_undefined(const Slice<T> *val) {
        return val->is(Slice<T>::Null());
    }

    template<class T>
    class SlotGetIter : public Iter {
        /*<-
        Fn:
            - "Iter *slot_get_iter_from_dtype(AnyIter, size_t, PyObj&) except +"
        Iter:
            SlotGetIter: [AnyIter, size_t, PyObj]
        Tube:
            SlotGet:
                props: [Tube parent, size_t index, object default_val]
                dtype: return (self.parent.dtype[self.index], )
                init_check: |
                    if index > <size_t>len(parent.dtype):
                        raise IndexError(f"Cannot get slot {index} from parent with {len(parent.dtype)} slots")
                custom_iter: |
                    cdef PyObj default_ob = PyObj(<PyObject*>self.default_val)
                    cdef Iter *iter = slot_get_iter_from_dtype(parent.iter, self.index, default_ob)
        ->*/

        const T *parent;
        T value;
        SlotPointer slot;
        DefaultValue<T> default_val;

    public:
        SlotGetIter(AnyIter parent, size_t index, PyObj &default_val)
            : parent(parent->get_slots()[index]),
              slot(&this->value),
              default_val(default_val)
            {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next(){
            if (is_undefined(parent)) {
                value = default_val.default_or("Field not found");
            } else {
                value = *parent;
            }
        }

    };

    template<class T, class Enable>
    struct slot_get_iter_from_dtype_op{
        inline Iter *operator()(AnyIter parent, size_t index, PyObj &default_val) {
            return new SlotGetIter<T>(parent, index, default_val);
        }
    };


    Iter *slot_get_iter_from_dtype(AnyIter parent, size_t index, PyObj &default_val) {
        return dispatch_type<slot_get_iter_from_dtype_op>(parent->get_slots()[index].type, parent, index, default_val);
    }
}}