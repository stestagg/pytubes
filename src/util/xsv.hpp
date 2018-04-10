#pragma once

#include <cstdio>
#include <vector>

#include "error.hpp"
#include "slice.hpp"
#include "array.hpp"
#include "../bytes.hpp"
#include "skiplist.hpp"

namespace ss {

#define TSV_SEP '\t'

template<class RowType> struct XsvHeader;
template<class ValueIter> struct XsvRow;


struct TsvValueIter {
    ByteSlice row;
    ByteSlice cur;

    uint8_t sep;

    TsvValueIter(ByteSlice row, uint8_t sep) : row(row), sep(sep) {
        cur = row.slice_to_ptr(row.find_first(sep));
    }

    inline void operator++() {
        if (cur.end() == row.end()) {
            row = ByteSlice::Null();
            cur = ByteSlice::Null();
        } else {
            row = row.slice_from(cur.len+1);
            cur = row.slice_to_ptr(row.find_first(sep));
        }
    }

    inline ByteSlice operator*() const {
        return cur;
    }

    inline bool operator==(const TsvValueIter &other) const {
        return row.is(other.row);
    }
    inline bool operator!=(const TsvValueIter &other) const {
        return !row.is(other.row);
    }

};


struct CsvValueIter {
    ByteSlice row;
    std::basic_string<bytes> buffer;
    const bytes *cur_value_end;
    ByteSlice cur;

    CsvValueIter(ByteSlice row) : row(row) {}

    inline void buffered_read_next(ByteSlice remaining) {
        while (true) {
            auto next = remaining.find_first('"');
            if (next + 1 == remaining.end() || *(next+1) == ',') {
                buffer += remaining.slice_to_ptr(next);
                cur = ByteSlice(buffer);
                return;
            } else {
                throw_if(ValueError, *(next+1) != '"', "Invalid quote in quoted CSV value");
                buffer += remaining.slice_to_ptr(next+1);
                remaining = remaining.slice_from_ptr(next+2);
            }
        }
    }

    inline void read_next_val() {
        // a,b,c
        // a,"b",c
        // a,"b""b",c
        if (row[0] == '"') {
            // This is a quoted value, remove the leading "
            auto remaining = row.slice_from(1);
            // Find the next "
            auto next = remaining.find_first('"');
            // If we hit the end of row with no closing ", that's bad
            throw_if(ValueError, next == remaining.end(), "Untermintaed CSV value");
            // If the quote is the last char in the row, or is a ',' then that's simple:
            if (next + 1 == remaining.end() || *(next+1) == ',') {
                cur = remaining.slice_to_ptr(next);
                cur_value_end = next + 1;
                return;
            } else {
                // We've got a " that isn't followed by a , or EOL
                // until we change how whitespace is handled, this must
                // be another quote character:
                throw_if(ValueError, *(next+1) != '"', "Invalid quote in quoted CSV value");
                buffer = remaining.slice_to_ptr(next+1);
                return buffered_read_next(remaining.slice_from_ptr(next+2));
            }
        } else {
            cur_value_end = row.find_first(',');
            cur = row.slice_to_ptr(cur_value_end);
        }
    }

    inline void operator++() {
        if (cur_value_end == row.end()) {
            row = ByteSlice::Null();
        } else {
            row = row.slice_from_ptr(cur_value_end);
            read_next_val();
        }
    }

    inline ByteSlice operator*() const {
        return cur;
    }

    inline bool operator==(const TsvValueIter &other) const {
        return row.is(other.row);
    }
    inline bool operator!=(const TsvValueIter &other) const {
        return !row.is(other.row);
    }

};

template<class ValueIter>
struct XsvRow{
    using Cls = XsvRow<ValueIter>;
    using iterator = ValueIter;
    using const_iterator = const ValueIter;
    using IsXsv = std::true_type;
    using Header = XsvHeader<Cls>;

    ByteSlice row;
    Header *header;

    XsvRow() {}
    XsvRow(ByteSlice row, Header *header) : row(row), header(header) {}

    static const char* variant_name();

    inline iterator begin() const;
    inline iterator end() const { return iterator(ByteSlice::Null(), TSV_SEP); }

    inline void populate_slots(SkipList<ByteSlice> &skips) const {
        auto value = begin();
        for (auto &skip : skips) {
            size_t to_skip = skip.skip;
            while(to_skip--){
                ++value;
                if (value == end()) { return; }
            }
            *(skip.destination) = *value;
        }
    }

};

template<class RowType>
struct XsvHeader {
    Array<ByteSlice> fields;
    Array<ByteString> stored_fields;
    bool have_headers = false;
    uint8_t sep;

    TsvHeader(uint8_t sep=TSV_SEP) : sep(sep) {}

    void read(RowType &row) {
        std::vector<ByteString> field_vec;
        throw_if(ValueError, have_headers, "Trying to read header row, but already have headers");
        for (auto val : row) {
            field_vec.emplace_back(val);
        }
        stored_fields = field_vec;
        fields = Array<ByteSlice>(stored_fields.size);
        std::transform(stored_fields.begin(), stored_fields.end(), fields.begin(), [](ByteString &x){ return ByteSlice(x); });
        have_headers = true;
    }

    SkipList<ByteSlice> make_skip_list(const Array<ByteSlice> &out_fields, const Array<ByteSlice> &slots) {
        SkipList<ByteSlice> skips;
        throw_if(ValueError, out_fields.size != slots.size, "Tried to apply header with incorrect values");
        throw_if(ValueError, !have_headers, "Tried to apply uninitialized header");
        // This is N*M, but /probably/ doesn't matter, I'm happy to be wrong about this
        size_t last_header_index = 0;
        for (size_t header_index = 0; header_index < stored_fields.size; ++header_index){
            for (size_t slot_index = 0; slot_index < slots.size; ++slot_index){
                auto &out_field = out_fields[slot_index];
                if (out_field == fields[header_index]) {
                    size_t skip = header_index - last_header_index;
                    skips.emplace_back(skip, &slots[slot_index]);
                    last_header_index = header_index;
                    break;
                }
            }
        }
        return skips;
    }
};


using TsvRow = XsvRow<TsvValueIter>;
using CsvRow = XsvRow<CsvValueIter>;

inline TsvRow::iterator TsvRow::begin() const { return iterator(row, header ? header->sep : TSV_SEP); }

}