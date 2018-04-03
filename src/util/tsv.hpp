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

struct SliceValueReader {
    ByteSlice row;

    SliceValueReader(ByteSlice row) : row(row) {}

    inline bool operator==(const TsvValueIter &other) const {
        return row.is(other.row);
    }
    inline bool operator!=(const TsvValueIter &other) const {
        return !row.is(other.row);
    }
}

struct TsvValueReader : SliceValueReader {
    ByteSlice cur;

    uint8_t sep;

    TsvValueIter(ByteSlice row, uint8_t sep) : SliceValueReader(row), sep(sep) {
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

};

struct CsvValueReader : SliceValueReader {
    // RFC 4180
     ByteSlice cur;

    CsvValueIter(ByteSlice row) : SliceValueReader(row){}

    inline ByteSlice read_next(ByteSlice &src) {
        if (src.len == 0) { return src; }
        while (src[0] == '"') {
            auto next = src.find_first('"');
            if (next >= src.end() -1) {
                throw_if(ValueError, next == src.end(), "Unterminated quoted csv field");
                return src.slice_from(1).slice_to_ptr(next);
            }
            if (*(next+1) == ',') {
                return src.slice_from(1).slice_to_ptr(next);
            }
        }
     }

}

struct XsvRow{
    using iterator = TsvValueIter;
    using const_iterator = const TsvValueIter;

    ByteSlice row;
    XsvHeader *header;

    XsvRow() {}
    XsvRow(ByteSlice row, XsvHeader *header) : row(row), header(header) {}

    inline iterator begin() const;
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

struct XsvMeta {
    bool have_headers = false;
    uint8_t sep;

    Array<ByteSlice> fields;
    Array<ByteString> stored_fields;

    virtual ~XsvMeta() = default;

    void read(XsvRow &row) {
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
}

struct TsvMeta : XsvMeta {
    TsvHeader(uint8_t sep=TSV_SEP) : XsvMeta(sep) {}
};

inline TsvRow::iterator TsvRow::begin() const { return iterator(row, header ? header->sep : TSV_SEP); }

}