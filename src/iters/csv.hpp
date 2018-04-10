#pragma once

#include "../util/xsv.hpp"

namespace ss{ namespace iter{

    class CsvIter : public Iter {

        /*<-
        Iter:
            CsvIter: [Chain, AnyIter, bool_t]
        Tube:
            Csv:
                props: [{type: Tube, name: parent, dtypes: [ByteSlice]}, bool_t headers]
                dtype: return (CsvRow,)
                chains: ((self.parent,), )
                iter: [CsvIter, ["iters_to_c_chain(chains[0])", parent.iter, self.headers]]
    ->*/
        StreamReader<T> reader;
        const Chain chain;
        XsvHeader< header_row;
        XsvHeader *header_ptr;

        TsvRow current_row;
        SlotPointer slot;

    public:
        TsvIter(Chain chain, AnyIter parent, bool headers) :
            reader(chain, parent->get_slots()[0]),
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