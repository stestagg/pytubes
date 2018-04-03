#pragma once

namespace ss{

    template<class T> struct Slice;

    using bytes = uint8_t;
    using ByteString = std::basic_string<uint8_t>;
    using ByteSlice = Slice<uint8_t>;

}