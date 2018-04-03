#pragma once

#include <cstdio>
#include <vector>

#include "error.hpp"
#include "slice.hpp"
#include "array.hpp"
#include "../bytes.hpp"
#include "skiplist.hpp"

namespace ss {

struct TsvHeader;
struct TsvRow;

struct TsvValueIter {
    ByteSlice row;
    ByteSlice cur;

    TsvValueIter(ByteSlice row) : row(row) {
        cur = row.slice_to_ptr(row.find_first('\t'));
    }

    inline void operator++() {
        if (cur.end() == row.end()) {
            row = ByteSlice::Null();
            cur = ByteSlice::Null();
        } else {
            row = row.slice_from(cur.len+1);
            cur = row.slice_to_ptr(row.find_first('\t'));
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


struct TsvRow{
    using iterator = TsvValueIter;
    using const_iterator = const TsvValueIter;

    ByteSlice row;
    TsvHeader *header;

    TsvRow() {}
    TsvRow(ByteSlice row, TsvHeader *header) : row(row), header(header) {}

    inline iterator begin() const { return iterator(row); }
    inline iterator end() const { return iterator(ByteSlice::Null()); }

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

struct TsvHeader {
    Array<ByteSlice> fields;
    Array<ByteString> stored_fields;
    bool have_headers = false;

    void read(TsvRow &row) {
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
        throw_if(ValueError, out_fields.size != slots.size, "Tried to apply TSV header with incorrect values");
        throw_if(ValueError, !have_headers, "Tried to apply uninitialized TSV header");
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

}