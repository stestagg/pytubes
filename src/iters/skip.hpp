#pragma once

#include "../iter.hpp"

namespace ss::iter {

    /*<-
        Iter: 
            SkipIter: [Chain, AnyIter, size_t]
        Tube:
            Skip:
                props: [Tube parent, size_t num]
                dtype: return self.parent.dtype
                chains: ((self.parent,), )
                iter: [SkipIter, ["iters_to_c_chain(chains[0])", parent.iter, self.num]]
        ->*/
    
    class SkipIter : public Iter {
        Slice<SlotPointer> slots;
        Chain chain;
        size_t left;

    public:
        SkipIter(Chain chain, AnyIter parent, size_t left) 
            : slots(parent->get_slots()), 
              chain(chain), 
              left(left) 
            {}

        Slice<SlotPointer> get_slots(){
            return slots;
        }

        void next(){            
            while (left > 0){
                do_next(chain);
                left -= 1;
            }
            do_next(chain);
        }


    };

}