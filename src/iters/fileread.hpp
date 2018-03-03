#pragma once

#include "../util/mmap.hpp"
#include "../iter.hpp"

namespace ss{ namespace iter{

#define BUFFER_SIZE (8 * 1024 * 1024)

class ReadFileIter : public Iter {
    /*<-
        Iter: 
            ReadFileIter: [Chain, AnyIter]
        Tube:
            ReadFile:
                props: [{type: Tube, name: parent, dtypes: [ByteSlice]}]
                dtype: return (ByteSlice, )
                chains: ((self.parent, ), )
                iter: [ReadFileIter, ["iters_to_c_chain(chains[0])", parent.iter]]
    ->*/

    const ByteSlice *filename_slice;
    Chain chain;
    uint8_t buffer[BUFFER_SIZE];
    ByteSlice buffer_slice;
    mmap::OpenFile cur_file;

    SlotPointer slot;

public:
    ReadFileIter(Chain chain, AnyIter parent) // <- Iter
        : filename_slice(parent->get_slots()[0]), 
          chain(chain),
          buffer_slice(buffer, BUFFER_SIZE),
          slot(&buffer_slice)
        {} 

    Slice<SlotPointer> get_slots(){
        return Slice<SlotPointer>(&slot, 1);
    }

    void next() {
        if (!cur_file.is_open()) {
            do_next(chain);
            std::basic_string<uint8_t> filename_str(*filename_slice);
            const char *filename = (const char *)filename_str.c_str();
            cur_file = mmap::OpenFile(filename);
        }
        size_t n_read = fread(buffer, 1, BUFFER_SIZE, cur_file.fd);
        buffer_slice.len = n_read;
        if (n_read < BUFFER_SIZE) {
            throw_if(IOError, ferror(cur_file.fd), "Error reading file");
            cur_file.close();
        }
    }

};

}}