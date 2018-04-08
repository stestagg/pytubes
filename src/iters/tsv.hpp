#pragma once

#include "../util/xsv.hpp"

namespace ss{ namespace iter{

    class TsvIter : public Iter {

        /*<-
        Iter:
            TsvIter: [Chain, AnyIter, uint8_t, bool_t]
        Tube:
            Tsv:
                props: [
                    {type: Tube, name: parent, dtypes: [ByteSlice]},
                    bool_t headers,
                    {type: str, name: sep, default: "'\\t'"},
                ]
                dtype: return (TsvRow,)
                chains: ((self.parent,), )
                init_check: |
                    if len(sep) > 1:
                        raise ValueError("TSV separator must be a single character")
                iter: [TsvIter, ["iters_to_c_chain(chains[0])", parent.iter, ord(self.sep), self.headers]]
    ->*/
        const ByteSlice *source;
        const Chain chain;
        TsvHeader header_row;

        bool read_headers;

        TsvRow current_row;
        SlotPointer slot;

    public:
        TsvIter(Chain chain, AnyIter parent, uint8_t sep, bool headers)
            : source(parent->get_slots()[0]),
              chain(chain),
              header_row(sep),
              read_headers(headers),
              slot(&current_row)
            {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next() {
            do_next(chain);
            if (read_headers && !header_row.have_headers) {
                auto row = TsvRow(*source, NULL);
                header_row.read(row);
                do_next(chain);
            }
            current_row = TsvRow(*source, &header_row);
        }

    };

}}