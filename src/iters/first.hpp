#pragma once

#include "../iter.hpp"

namespace ss{ namespace iter{

    class FirstIter : public Iter {
        /*<-
        Iter:
            FirstIter: [AnyIter, size_t]
        Tube:
            First:
                props: [Tube parent, size_t num]
                unnamed_props: [num]
                dtype: return self.parent.dtype
                iter: [FirstIter, [parent.iter, self.num]]
        ->*/

        AnyIter parent;
        size_t left;

    public:
        FirstIter(AnyIter parent, size_t left) : parent(parent), left(left) {}

        Slice<SlotPointer> get_slots(){
            return parent.get()->get_slots();
        }

        void next(){
            if (left == 0) {
                throw StopIteration;
            }
            left -= 1;
        }


    };

}}