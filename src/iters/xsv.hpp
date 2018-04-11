#pragma once

#include "../util/xsv.hpp"

namespace ss{ namespace iter{

    template<class T>
    class XsvIter : public Iter {

        /*<-
        Fn:
            - cdef cppclass c_CsvRow "ss::CsvRow"
            - cdef cppclass c_TsvRow "ss::TsvRow"
        Iter:
            XsvIter:
                init: [Chain, AnyIter, uint8_t, bool_t]
                template: T
        Tube:
            Xsv:
                props: [
                    {type: Tube, name: parent, dtypes: [ByteSlice]},
                    bool_t headers,
                    str sep,
                    str variant
                ]
                dtype: "return ({'csv': CsvRow, 'tsv': TsvRow}[self.variant],)"
                chains: ((self.parent,), )
                init_check: |
                    if len(sep) > 1:
                        raise ValueError("Separator must be a single character")
                    if variant not in {"tsv", "csv"}:
                        raise ValueError("Variant must be one of tsv, csv")
                custom_iter: |
                    sep = ord(self.sep)
                    cdef Iter *iter
                    if self.variant == "csv":
                        iter = <Iter*>new XsvIter[c_CsvRow](iters_to_c_chain(chains[0]), parent.iter, sep, self.headers)
                    elif self.variant == "tsv":
                        iter = <Iter*>new XsvIter[c_TsvRow](iters_to_c_chain(chains[0]), parent.iter, sep, self.headers)
                    else:
                        raise ValueError("Variant must be one of tsv, csv")
    ->*/
        const ByteSlice *source;
        const Chain chain;
        XsvHeader<T> header_row;

        bool read_headers;

        T current_row;
        SlotPointer slot;

    public:
        XsvIter(Chain chain, AnyIter parent, uint8_t sep, bool headers)
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
                auto row = T(*source, NULL);
                header_row.read(row);
                do_next(chain);
            }
            current_row = T(*source, &header_row);
        }

    };

}}