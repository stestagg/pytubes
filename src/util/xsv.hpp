#pragma once

#include <cstdio>
#include <vector>

#include "error.hpp"
#include "slice.hpp"
#include "array.hpp"
#include "../bytes.hpp"
#include "skiplist.hpp"

namespace ss {

template<class RowType> struct XsvHeader;
template<class ValueIter> struct XsvRow;


struct TsvValueIter {
    ByteSlice row;
    ByteSlice cur;

    uint8_t sep;

    TsvValueIter(ByteSlice row, uint8_t sep) : row(row), sep(sep) {}

    inline bool next(ByteString &_) {
        if (!row.len) {
            cur = row;
            return false;
        }
        auto match = row.find_first(sep);
        cur = row.slice_to_ptr(match);
        if(match == row.end()) {
            return false;
        } else {
            row = row.slice_from_ptr(match + 1);
            return true;
        }
    }

    inline bool skip_next() {
        if (!row.len) { return false; }
        auto match = row.find_first(sep);
        if (match == row.end()) {
            return false;
        }
        row = row.slice_from_ptr(match + 1);
        return true;
    }
};

struct CsvValueIter {
    ByteSlice row;
    ByteSlice cur;

    uint8_t sep;

    CsvValueIter(ByteSlice row, uint8_t sep) :
        row(row),
        sep(sep) {}

    inline bool buffered_read_next(ByteString &buffer) {
        while (true) {
            auto next = row.find_first('"');
            throw_if(ValueError, next == row.end(), "Unterminated CSV value: ", row);
            if (next + 1 == row.end()) {
                buffer += row.slice_to_ptr(next);
                cur = ByteSlice(buffer);
                return false;
            } else if (*(next+1) == sep) {
                buffer += row.slice_to_ptr(next);
                cur = ByteSlice(buffer);
                row = row.slice_from_ptr(next + 2);
                return true;
            } else if(*(next + 1) == '"') {
                buffer += row.slice_to_ptr(next + 1);
                row = row.slice_from_ptr(next + 2);
            }else {
                throw_py<ValueError>(
                    "Invalid quote in quoted CSV value. ",
                    "Expected '\"', got '",
                    *(next+1),
                    "'"
                );
            }
        }
    }

    inline bool next(ByteString &buffer) {
        if(row.len == 0) {
            cur = row;
            return false;
        }
        if (row[0] != '"') {
            auto match = row.find_first(sep);
            cur = row.slice_to_ptr(match);
            if(match == row.end()) {
                return false;
            } else {
                row = row.slice_from_ptr(match + 1);
                return true;
            }
        } else {
            // This is a quoted value, remove the leading "
            row = row.slice_from(1);
            // Find the next "
            auto next = row.find_first('"');
            // If we hit the end of row with no closing ", that's bad
            throw_if(ValueError, next == row.end(), "Unterminated CSV value:", row);
            // If the quote is the last char in the row...
            if (next + 1 == row.end()){
                cur = row.slice_to_ptr(next);
                return false;
            // or is a ','
            } else if (*(next+1) == sep) {
                cur = row.slice_to_ptr(next);
                row = row.slice_from_ptr(next + 2);
                return true;
            } else if(*(next+1) == '"') {
                buffer = row.slice_to_ptr(next+1);
                row = row.slice_from_ptr(next+2);
                return buffered_read_next(buffer);
            } else {
                // We've got a " that isn't followed by a , or EOL
                // until we change how whitespace is handled, this must
                // be another quote character:
                throw_py<ValueError>(
                    "Invalid quote in quoted CSV value. ",
                    "Expected '\"', got '",
                    *(next+1),
                    "'"
                );
            }
        }
        return true;
    }

    inline bool skip_next() {
        // This is very similar to next() except it never copies data in the
        // case of embedded escaped quote chars
        if(row.len == 0) { return false; }
        if (row[0] != '"') {
            auto match = row.find_first(sep);
            if (match == row.end()) {
                return false;
            }
            row = row.slice_from_ptr(match + 1);
            return true;
        } else {
            row = row.slice_from(1);
            while (true) {
                auto next = row.find_first('"');
                throw_if(ValueError, next == row.end(), "Unterminated CSV value", row);
                if (next + 1 == row.end()) {
                    return false;
                } else if (*(next+1) == sep) {
                    row = row.slice_from_ptr(next + 2);
                    return true;
                } else {
                    throw_if(ValueError, *(next+1) != '"', "Invalid quote in quoted CSV value");
                    row = row.slice_from_ptr(next+2);
                }
            }
        }
    }

};

inline void fill_remaining(SkipList<ByteSlice>::iterator cur, SkipList<ByteSlice>::const_iterator end) {
    for(; cur != end; ++cur) {
        *(cur->destination) = ByteSlice::Null();
    }
}

template<class ValueIter>
struct XsvRow{
    using Cls = XsvRow<ValueIter>;
    using IsXsv = std::true_type;
    using Header = XsvHeader<Cls>;

    static const bytes default_separator();

    ByteSlice row;
    Header *header;

    static inline const char* variant_name();

    XsvRow() {}
    XsvRow(ByteSlice row, Header *header) : row(row), header(header) {}

    inline void populate_slots(SkipList<ByteSlice> &skips, Array<ByteString> &buffers) const {
        auto iter = this->iter();
        auto cur_buffer = buffers.begin();
        for (auto skip = skips.begin(); skip != skips.end(); ++skip) {
            size_t to_skip = skip->skip;
            while(to_skip--){
                if (!iter.skip_next()) { return fill_remaining(skip, skips.end()); }
            }
            bool should_continue = iter.next(*cur_buffer);
            *(skip->destination) = iter.cur;
            if(!should_continue) { ++skip; return fill_remaining(skip, skips.end()); }
            ++cur_buffer;
        }
        
    }

    inline ValueIter iter() const {
        return ValueIter(this->row, header ? header->sep : default_separator());
    }

};

template<class RowType>
struct XsvHeader {
    Array<ByteSlice> fields;
    Array<ByteString> stored_fields;
    bool have_headers = false;
    uint8_t sep;

    XsvHeader(uint8_t sep) : sep(sep) {}

    void read(RowType &row) {
        std::vector<ByteString> field_vec;
        throw_if(ValueError, have_headers, "Trying to read header row, but already have headers");
        auto iter = row.iter();
        ByteString buffer;
        while (true) {
            bool should_continue = iter.next(buffer);
            field_vec.emplace_back(iter.cur);
            if (!should_continue) { break; }
        }
        stored_fields = field_vec;
        fields = Array<ByteSlice>(stored_fields.size);
        std::transform(
            stored_fields.begin(),
            stored_fields.end(),
            fields.begin(),
            [](ByteString &x){ return ByteSlice(x); }
        );
        have_headers = true;
    }

    SkipList<ByteSlice> make_skip_list(const Array<ByteSlice> &out_fields, const Array<ByteSlice> &slots) {
        SkipList<ByteSlice> skips;
        throw_if(ValueError, out_fields.size != slots.size, "Tried to apply header with incorrect values");
        throw_if(ValueError, !have_headers, "Tried to apply uninitialized header");
        // This is N*M, but /probably/ doesn't matter, I'm happy to be wrong about this
        size_t last_header_index = 0;
        bool first = true;
        for (size_t header_index = 0; header_index < stored_fields.size; ++header_index){
            for (size_t slot_index = 0; slot_index < slots.size; ++slot_index){
                auto &out_field = out_fields[slot_index];
                if (out_field == fields[header_index]) {
                    throw_if(ValueError, !first && header_index == last_header_index,
                        "Tried to read field ", header_index, " multiple times"
                    );
                    size_t skip = header_index - last_header_index;
                    if (first) {
                        first = false;
                    } else {
                        skip -= 1;
                    }
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

template<> inline const char *TsvRow::variant_name() { return "TSV"; }
template<> inline const char *CsvRow::variant_name() { return "CSV"; }
template<> inline const bytes TsvRow::default_separator() { return '\t'; };
template<> inline const bytes CsvRow::default_separator() { return ','; };

}