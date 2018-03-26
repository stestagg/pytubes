#pragma once

#include "../iter.hpp"
#include "../util/valuestore.hpp"


namespace ss{ namespace iter{

    template<class T>
    class GroupIdIter : public Iter {
        /*<-
        Fn:
            - "Iter *group_id_from_dtype(AnyIter) except +"
        Iter: 
            GroupIdIter: 
                template: T
                init: [AnyIter]
        Tube:
            GroupId:
                props: [Tube parent]
                dtype: return self.parent.dtype
                iter: [GroupIdIter, [parent.iter]]
                custom_iter: |
                    cdef Iter *iter = group_id_from_dtype(parent.iter)
        ->*/
        const T *parent;
        ValueStorage<T> last_value_store;
        T * const last_value;
        int64_t group_id = 0;
        SlotPointer slot;
        bool started = false;

    public:
        GroupIdIter(AnyIter parent) :
            parent(parent->get_slots()[0]),
            last_value(last_value_store.get_pointer()),
            slot(&group_id) {}

        Slice<SlotPointer> get_slots() {
            return Slice<SlotPointer>(&slot, 1);
        }

        void next(){
            if (!started) {
                started = true;
                last_value_store.set(*parent);
            } else {
                if (*last_value != *parent) {
                    last_value_store.set(*parent);
                    ++group_id;
                }
            }
        }
    };

    template<class T, class Enable=bool> struct group_id_op {
        template<class Q=T>
        inline Iter *operator()(AnyIter parent) {
            throw_py<ValueError>(
            "group_id has not been implemented on iterators of type ",
            ScalarType_t<Q>::type_name()
            );
            return NULL;
        } 
    };

    template<class T> struct group_id_op<T, ignore_t<bool, decltype(&ValueStorage<T>::get_pointer)>> {
        inline Iter *operator()(AnyIter parent) {
            return new GroupIdIter<T>(parent);
        }
    };

    Iter *group_id_from_dtype(AnyIter parent) {
        return dispatch_type<group_id_op>(parent->get_slots()[0].type, parent);
    }

}}