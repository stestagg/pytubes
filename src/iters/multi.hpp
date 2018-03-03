#pragma once

#include "../util/json/json.hpp"

namespace ss{ namespace iter{

    std::vector<SlotPointer> slots_from_children(std::vector<AnyIter> &children) {
        std::vector<SlotPointer> slots;
        for (AnyIter child : children) {
            for (SlotPointer slot : child->get_slots()){
                slots.push_back(slot);
            }
        }
        return slots;
    }

    class MultiIter : public Iter {

        /*<-
        Iter: 
            MultiIter: ["vector[AnyIter]"]
        Tube:
            Multi:
                props: [Tube parent, list inputs]
                dtype: return tuple(d for t in self._inputs[1:] for d in t.dtype)
                custom_iter: |
                    cdef MultiIter *iter = new MultiIter(self._make_iters(args))
                methods: |
                    @property
                    def _inputs(self):
                        return (self.parent, ) + tuple(self.inputs)

                    cdef vector[AnyIter] _make_iters(self, list args):
                        cdef IterWrapper arg
                        cdef vector[AnyIter] its
                        for arg in args[1:]:
                            its.push_back(arg.iter)
                        return its

                    cpdef _describe_self(self):
                        cdef Tube i
                        input_reprs = [i._repr(stop=self.parent) for i in self.inputs]
                        return f"Multi({input_reprs})"

                            
    ->*/
        const std::vector<SlotPointer> slots;

    public:
        MultiIter(std::vector<AnyIter> children)
            : slots(slots_from_children(children))
            {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(slots.data(), slots.size());
        }

        void next() {}
    };

}}