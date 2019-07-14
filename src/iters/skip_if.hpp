#pragma once

#include "../iter.hpp"

namespace ss{ namespace iter{

    class SkipIfIter : public Iter {
        /*<-
        Iter:
            SkipIfIter: [Chain, AnyIter, AnyIter]
        Tube:
            SkipIf:
                props: [Tube parent, {type: Tube, name: conditional, dtypes: [Bool]}]
                dtype: return self.parent.dtype
                chains: ((self.parent, self.conditional), )
                iter: [SkipIfIter, ["iters_to_c_chain(chains[0])", parent.iter, conditional.iter]]
                methods: |
                    cpdef _describe_self(self):
                        return f"SkipIf({self.conditional._repr(stop=set(self.parent))})"
        ->*/
        Chain chain;
        Slice<SlotPointer> parent;
        const bool *conditional;

    public:
        SkipIfIter(Chain chain, AnyIter parent, AnyIter conditional)
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
            } while (*conditional);
        }


    };

}}