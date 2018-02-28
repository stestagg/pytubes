#pragma once

#include "../iter.hpp"

namespace ss::iter {

    static std::vector<SlotPointer> slots_from_parents(std::vector<AnyIter> &parents) {
        std::vector<SlotPointer> out;
        for (auto it : parents) {
            for (auto slot : it->get_slots()) {
                out.push_back(slot);
            }
        }
        return out;
    }

    /*<-
        Iter: 
            ZipIter: [Chain, "vector[AnyIter]"]
        Tube:
            Zip:
                props: [list inputs]
                dtype: return tuple(dty for t in self.inputs for dty in t.dtype)
                chains: (tuple(self.inputs), )
                custom_iter: |
                    cdef ZipIter *iter = new ZipIter(iters_to_c_chain(chains[0]), self._make_iters(iters))
                methods: |
                    @property
                    def _inputs(self):
                        return tuple(self.inputs)

                    cdef vector[AnyIter] _make_iters(self, list args):
                        cdef IterWrapper arg
                        cdef vector[AnyIter] its
                        for arg in args:
                            its.push_back(arg.iter)
                        return its

                    cpdef _describe_self(self):
                        cdef Tube i
                        input_reprs = [i._repr(stop=set(self.inputs)) for i in self.inputs]
                        return f"Zip({input_reprs})"
        ->*/
    
    class ZipIter : public Iter {
        const std::vector<SlotPointer> slots;
        const Chain chain;

    public:
        ZipIter(Chain chain, std::vector<AnyIter> parents) 
            : slots(slots_from_parents(parents)), 
              chain(chain)
            {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(slots.data(), slots.size());
        }

        void next(){
            do_next(chain);
        }


    };

}