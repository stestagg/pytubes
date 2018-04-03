#pragma once

#include "../util/tsv.hpp"

namespace ss{ namespace iter{

    class TsvIter : public Iter {

        /*<-
        Iter:
            TsvIter: [Chain, AnyIter, bool_t]
        Tube:
            Tsv:
                props: [{type: Tube, name: parent, dtypes: [ByteSlice]}, bool_t headers]
                dtype: return (TsvRow,)
                chains: ((self.parent,), )
                iter: [TsvIter, ["iters_to_c_chain(chains[0])", parent.iter, self.headers]]
    ->*/
        const ByteSlice *source;
        const Chain chain;
        TsvHeader header_row;
        TsvHeader *header_ptr;

        TsvRow current_row;
        SlotPointer slot;

    public:
        TsvIter(Chain chain, AnyIter parent, bool headers)
            : source(parent->get_slots()[0]),
              chain(chain),
              header_ptr(headers ? &header_row : NULL),
              slot(&current_row)
            {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next() {
            do_next(chain);
            if (header_ptr && !header_ptr->have_headers) {
                auto row = TsvRow(*source, NULL);
                header_ptr->read(row);
                do_next(chain);
            }
            current_row = TsvRow(*source, header_ptr);
        }

    };

}}