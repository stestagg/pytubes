#pragma once

#include "../iter.hpp"

namespace ss{ namespace iter{

    /*<-
        Iter:
            ChainIter: ["vector[Chain]", "vector[AnyIter]"]
        Tube:
            ChainTubes:
                props: [list parents]
                dtype: return self.parents[0].dtype
                chains: tuple((p, ) for p in self.parents)
                custom_iter: |
                    unique_inputs = set()
                    for p in self.parent:
                        unique_inputs.add(p.dtype)
                    # This should work...
                    # if len(set(p.dtype for p in self.parent)) > 1:
                    #     raise ValueError("Chain requires all inputs to have the same dtype")
                    if len(unique_inputs) > 1:
                        raise ValueError("Chain requires all inputs to have the same dtype")
                    cdef ChainIter *iter = new ChainIter(self._make_chains(chains), self._make_iters(iters))
                methods: |
                    @property
                    def _inputs(self):
                        return tuple(self.parents)

                    cdef vector[AnyIter] _make_iters(self, list args):
                        cdef IterWrapper arg
                        cdef vector[AnyIter] its
                        for arg in args:
                            its.push_back(arg.iter)
                        return its

                    cdef vector[Chain] _make_chains(self, list args):
                        cdef list arg
                        cdef Chain chain
                        cdef vector[Chain] chains
                        for arg in args:
                            chain = iters_to_c_chain(arg)
                            chains.push_back(chain)
                        return chains

        ->*/

    bool slots_are_same(AnyIter &a, AnyIter &b) {
        auto a_slots = a->get_slots();
        auto b_slots = b->get_slots();
        if (a_slots.len != b_slots.len) { return false; }
        size_t b_index = 0;
        for (auto &slot: a_slots) {
            if (slot.type != b_slots[b_index].type) { return false; }
            ++ b_index;
        }
        return true;
    }

    std::vector<Slice<SlotPointer>> iters_to_slot_pointers(AnyIter reference, std::vector<AnyIter> &inputs) {
        std::vector<Slice<SlotPointer>> out;
        for (auto &input : inputs) {
            throw_if(ValueError,
                !slots_are_same(reference, input),
                "All chain iters must have the same dtype"
            );
            out.push_back(input->get_slots());
        }
        return out;
    }

    std::vector<std::unique_ptr<StoredSlot>> make_slots(AnyIter input) {
        std::vector<std::unique_ptr<StoredSlot>> out;
        auto in_slots = input->get_slots();
        for (auto &in_slot : in_slots) {
            out.push_back(make_stored_slot(in_slot.type));
        }
        return out;
    }

    std::vector<SlotPointer> make_slot_pointers(std::vector<std::unique_ptr<StoredSlot>> &slots){
        std::vector<SlotPointer> out;
        for (auto &slot : slots) {
            out.push_back(slot->ptr);
        }
        return out;
    }

    class ChainIter : public Iter {
        std::vector<Chain> chains;
        std::vector<AnyIter> inputs;

        std::vector<std::unique_ptr<StoredSlot>> slots;
        std::vector<SlotPointer> slot_pointers;

        std::vector<Chain>::iterator cur_chain;
        std::vector<AnyIter>::iterator cur_input;
        Slice<SlotPointer> input_slots;

    public:
        ChainIter(std::vector<Chain> chains, std::vector<AnyIter> inputs)
            : chains(chains),
              inputs(inputs)
            {
                throw_if(ValueError, !inputs.size(), "Chain cannot be created from empty inputs")
                slots = make_slots(this->inputs[0]);
                slot_pointers = make_slot_pointers(this->slots);
                cur_chain = this->chains.begin();
                cur_input = this->inputs.begin();
                input_slots = (*this->cur_input)->get_slots();
            }

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(slot_pointers);
        }

        void next(){
            try{
                do_next(*cur_chain);
            } catch (const StopIterationExc &e) {
                ++cur_chain;
                if(cur_chain == chains.end()) { throw StopIteration; }
                ++cur_input;
                throw_if(RuntimeError, cur_input == inputs.end(),
                    "Chain ran out of input iterators, but still has input chains left.  "
                    "This should not be possible, and indicates a bug in the implementation");
                input_slots = (*cur_input)->get_slots();
                return next();
            }
            size_t index = 0;
            for (auto &slot : input_slots) {
                slots[index]->update(slot.ptr);
                ++index;
            }
        }


    };

}}