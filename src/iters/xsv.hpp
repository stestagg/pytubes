#pragma once

#include "../util/xsv.hpp"
#include "../util/stream_reader.hpp"

namespace ss{ namespace iter{

    struct CsvRowFinder {
        enum FinderState{
            BEGIN,
            MID_FIELD,
            START_QUOTE,
        };

        FinderState state = BEGIN;
        uint8_t sep;

        CsvRowFinder(uint8_t sep) : sep(sep) {}

        inline const bytes *find_next(ByteSlice slice) {
            const bytes *match;
            while(true) {
                if (slice.len == 0) { return slice.end(); }
                switch(state) {
                    case BEGIN:
                    case MID_FIELD:
                        match = slice.find_first_of<'"', '\n'>();
                        // New line with no " means we found a full row
                        if (match == slice.end()) {
                            // We hit the end of current chunk, look at last char
                            // to see if we ended on a start of field char
                            state = *(match-1) == sep ? BEGIN : MID_FIELD;
                            return match;
                        }
                        if (*match == '\n') { return match; }
                        // If it's a quote, this only matters if start of field:
                        if(state == BEGIN && match == slice.start){
                            state = START_QUOTE;
                            slice = slice.slice_from(1);
                        } else {
                            // We've just checked if this is the start of the string
                            // so now look-behind to see if this is the start of a field
                            // or not (can only be start of field  if previous is a sep
                            // char
                            if (match == slice.start) {
                                // MID_FIELD, but found a quote, so no changes
                            } else if (*(match - 1) == sep) {
                                state = START_QUOTE;
                            } else {
                                state = MID_FIELD;
                            }
                            slice = slice.slice_from_ptr(match+1);
                        }
                        break;
                    case START_QUOTE:
                        match = slice.find_first_of<'"'>();
                        if (match == slice.end()) { return match; }
                        state = BEGIN;
                        slice = slice.slice_from_ptr(match+1);
                        break;
                }
            }
            return match;
        }

        inline ByteSlice get_remaining(const ByteSlice &slice, const bytes* const match) const{
            return slice.slice_from_ptr(match  + 1);
        }
    };

    template<class RowType, bool Split>
    struct split_impl{
        inline const ByteSlice *get_ptr();
        inline void next_row();
    };

    template<class T> struct split_impl<T, false> {
        Chain chain;
        const ByteSlice *source;

        split_impl(Chain &chain, const ByteSlice *source, bytes sep):
            chain(chain),
            source(source)
        {}

        inline const ByteSlice *get_ptr(){
            return source;
        }

        inline void next_row() {
            do_next(chain);
        }
    };

    struct splitting_split_impl {
        StreamReader<bytes> reader;
        ByteSlice current;

        splitting_split_impl(Chain &chain, const ByteSlice *source):
            reader(chain, source)
        {}

        inline const ByteSlice *get_ptr() {
            return &current;
        }
    };

    template<> struct split_impl<TsvRow, true> : splitting_split_impl {
        split_impl(Chain &chain, const ByteSlice *source, bytes sep):
            splitting_split_impl(chain, source)
        {}

        inline void next_row(){
            current = reader.read_until_char('\n');
        }
    };

    template<> struct split_impl<CsvRow, true> : splitting_split_impl {
        CsvRowFinder finder;

        split_impl(Chain &chain, const ByteSlice *source, bytes sep):
            splitting_split_impl(chain, source),
            finder(sep)
        {}

        inline void next_row(){
            current = reader.read_until(finder);
        }
    };


    template<class T, bool Split>
    class XsvIter : public Iter {

        /*<-
        Fn:
            - cdef cppclass c_CsvRow "ss::CsvRow"
            - cdef cppclass c_TsvRow "ss::TsvRow"
            - cdef Iter *make_xsv_iter(Chain, AnyIter, char, bool_t, char, bool_t, bool_t)
        Iter:
            XsvIter:
                init: [Chain, AnyIter, uint8_t, bool_t]
                template: [T, U]
        Tube:
            Xsv:
                props: [
                    {type: Tube, name: parent, dtypes: [ByteSlice]},
                    str variant,
                    str sep,
                    bool_t headers=True,
                    bool_t row_split=True,
                    bool_t skip_empty_rows=True,
                ]
                unnamed_props: [variant]
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
                    iter = make_xsv_iter(
                        iters_to_c_chain(chains[0]),
                        parent.iter,
                        sep,
                        self.headers,
                        ord(self.variant[0]),
                        self.row_split,
                        self.skip_empty_rows,
                    )
    ->*/
        split_impl<T, Split> row_impl;
        const ByteSlice *source;
        XsvHeader<T> header_row;

        bool read_headers;
        bool skip_empty_rows;

        T current_row;
        SlotPointer slot;

    public:
        XsvIter(Chain chain, AnyIter parent, uint8_t sep, bool headers, bool skip_empty_rows)
            : row_impl(chain, parent->get_slots()[0], sep),
              source(row_impl.get_ptr()),
              header_row(sep),
              read_headers(headers),
              skip_empty_rows(skip_empty_rows),
              slot(&current_row)
            {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next() {
            do{
                row_impl.next_row();
            } while (skip_empty_rows && source->len == 0);
            if (read_headers && !header_row.have_headers) {
                auto row = T(*source, NULL);
                header_row.read(row);
                row_impl.next_row();
            }
            current_row = T(*source, &header_row);
        }

    };

    template<class T>
    Iter *make_xsv_iter_inner(Chain chain, AnyIter parent, uint8_t sep, bool headers, bool row_split, bool skip_empty) {
        if(row_split) {
            return (Iter*)new XsvIter<T, true >(chain, parent, sep, headers, skip_empty);
        } else {
            return (Iter*)new XsvIter<T, false>(chain, parent, sep, headers, skip_empty);
        }
    }

    Iter *make_xsv_iter(Chain chain, AnyIter parent, uint8_t sep, bool headers, char xsv_type, bool row_split, bool skip_empty) {
        switch(xsv_type) {
            case 'c': return make_xsv_iter_inner<CsvRow>(chain, parent, sep, headers, row_split, skip_empty);
            case 't': return make_xsv_iter_inner<TsvRow>(chain, parent, sep, headers, row_split, skip_empty);
            default: throw_py<ValueError>("Unrecognized XSV row type: ", xsv_type);
        };
    }

}}