#pragma once

#include "../iter.hpp"

#include <zlib.h>

namespace ss::iter{

#define OUT_BUFFER_SIZE (8 * 1024 * 1024)

class ZlibDecodeIter : public Iter {
    /*<-
    Iter: 
        ZlibDecodeIter: [Chain, AnyIter, bint]
    Tube:
        Gunzip:
            props: [Tube parent, bint stream]
            dtype: return (ByteSlice, )
            chains: ((self.parent,), )
            iter: [ZlibDecodeIter, ["iters_to_c_chain(chains[0])", parent.iter, self.stream]]
    ->*/

    const ByteSlice *source_data;
    Chain chain;
    bool is_stream;
    
    ByteSlice encoded_slice;
    ByteSlice decoded_slice;

    SlotPointer slot;

    z_stream stream;
    bool stream_initted = false;
    uint8_t output_buffer[OUT_BUFFER_SIZE];

public:

    ZlibDecodeIter(Chain chain, AnyIter parent, bool is_stream) // <- Iter
        : source_data(parent->get_slots()[0]),
          chain(chain),
          is_stream(is_stream),
          slot(&decoded_slice)
        {
            init_decoder();
        }
    
    ~ZlibDecodeIter() { inflateEnd(&stream); }

    Slice<SlotPointer> get_slots(){
        return Slice<SlotPointer>(&slot, 1);
    }

    void handle_inflate_result(int result) {
        std::string msg;
        switch (result) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
                throw_py<ValueError>("Invalid or unsupported gzip format");
            case Z_MEM_ERROR:
                throw MemoryError();
        }
    }

    void init_decoder() {
        if(stream_initted) {
            inflateEnd(&stream);
        }
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.next_in = Z_NULL;
        stream.data_type = 0;
        stream.avail_in = 0;
        throw_if(RuntimeError, 
           inflateInit2(&stream, 15 | 32) != Z_OK,
           "Failed to initialize zlib"
        );
        stream_initted = true;
    }

    void next() {
        if (encoded_slice.is_empty()) {
            do_next(chain);
            encoded_slice = *source_data;
            // If we're not in streaming mode, a new slice from parent means
            // a new gzip, so reset the decoder...
            // TODO: why not auto-reset if the first read of a new slice fails?
            if (!is_stream) { init_decoder(); }
            stream.avail_in = encoded_slice.len;
            stream.next_in = (Bytef *)encoded_slice.start;
        }
        stream.avail_out = OUT_BUFFER_SIZE;
        stream.next_out = (Bytef *)output_buffer;
        auto result = inflate(&stream, Z_SYNC_FLUSH);
        handle_inflate_result(result);
        auto decoded = OUT_BUFFER_SIZE - stream.avail_out;
        throw_if(ValueError, 
            stream.avail_out > 0 && stream.avail_in > 0,
            "Trailing data in gzip stream"
        );
        if (stream.avail_in == 0) {
            encoded_slice = ByteSlice::Null();
        }
        decoded_slice = ByteSlice(&output_buffer[0], decoded);
        return;
    }
};

}
