#pragma once

#include "../util/mmap.hpp"
#include "../iter.hpp"

namespace ss{ namespace iter{

class FileMapIter : public Iter {
    /*<-
        Iter: 
            FileMapIter: [AnyIter]
        Tube:
            FileMap:
                props: [{type: Tube, name: parent, dtypes: [ByteSlice]}]
                dtype: return (ByteSlice, )
                iter: [FileMapIter, [parent.iter]]
    ->*/

    const ByteSlice *filename_slice;
    ByteSlice cur_slice;
    mmap::Mmap cur_map;

    SlotPointer slot;

public:
    FileMapIter(AnyIter parent) // <- Iter
        : filename_slice(parent->get_slots()[0]), cur_map(), slot(&cur_slice)
        {} 

    Slice<SlotPointer> get_slots(){
        return Slice<SlotPointer>(&slot, 1);
    }

    void next() {
        std::basic_string<uint8_t> filename_str(*filename_slice);
        const char *filename = (const char *)filename_str.c_str();
        cur_map = mmap::Mmap(filename);
        cur_slice = cur_map.slice();
    }

};

}}