#pragma once

#include "../iter.hpp"

namespace ss{ namespace iter{
    
    class SkipUnlessIter : public Iter {
        /*<-
        Iter: 
            SkipUnlessIter: [Chain, AnyIter, AnyIter]
        Tube:
            SkipUnless:
                props: [Tube parent, {type: Tube, name: conditional, dtypes: [Bool]}]
                dtype: return self.parent.dtype
                chains: ((self.parent, self.conditional), )
                iter: [SkipUnlessIter, ["iters_to_c_chain(chains[0])", parent.iter, conditional.iter]]
                methods: |
                    cpdef _describe_self(self):
                        return f"SkipUnless({self.conditional._repr(self.parent)})"
        ->*/
        Chain chain;
        Slice<SlotPointer> parent;
        const bool *conditional;

    public:
        SkipUnlessIter(Chain chain, AnyIter parent, AnyIter conditional) 
            : chain(chain), 
              parent(parent->get_slots()), 
              conditional(conditional->get_slots()[0])
            {}

        Slice<SlotPointer> get_slots(){
            return parent;
        }

        void next(){
            do{
                do_next(chain);
            } while (!*conditional);
        }


    };

}}