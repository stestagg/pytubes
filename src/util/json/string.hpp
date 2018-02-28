#pragma once

namespace ss::json{
    class InvalidJson;
}

namespace ss::json::string {

    // template <class I, class O> 
    // inline void decode_into(std::basic_string<O> &out, Slice<I> in);

    template <class I, class O>
    inline uint16_t decode_u(std::basic_string<O> &out, Slice<I> in);

    template <class I, class O>
    inline Slice<I> decode_escape(std::basic_string<O> &out, Slice<I> in, Slice<I> &full);

    template <class I, class O>
    inline void write_char(std::basic_string<O> &out, I val);

    template<class I, class O>
    inline typename std::enable_if<std::is_same<I, O>::value, void>::type
    decode_into(std::basic_string<O> &out, Slice<I> in) {
        const I *first_bs = in.find_first('\\');
        std::copy(in.begin(), first_bs, std::back_inserter(out));
        if (first_bs < in.end()) {
            auto remain = decode_escape<I, O>(out, in.slice_from_ptr(first_bs+1), in);
            if (remain.len) {
                decode_into(out, remain);
            }
        }
    }

    template<class I, class O>
    inline typename std::enable_if<std::is_same<I, O>::value, Slice<I>>::type
    decode_str(Slice<I> in, std::basic_string<O> &buf) {
        const I *first_bs = in.find_first('\\');
        if (first_bs == in.end()) {
            return in;
        }
        std::copy(in.begin(), first_bs, std::back_inserter(buf));
        auto remain = decode_escape<I, O>(buf, in.slice_from_ptr(first_bs+1), in);
        if (remain.len) {
            decode_into(buf, remain);
        }
        return Slice<I>(buf);
    }

    // Lookup table to resolve escape values, unfortunately gcc is quite picky
    // about initializers in c++, so listing out the full table...
    // escape values:   "\/bfnrt
    //  " => 34  => ["]
    //  \ => 92  => [\]
    //  / => 47  => [/]
    //  b => 98  => [backspace - 8]
    //  f => 102 => [form feed - 12]
    //  n => 110 => [new line - 10]
    //  r => 114 => [return - 13]
    //  t => 116 => [tab - 9]
    static uint8_t escape_lookup[256] = {
        /*0  */ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*16 */ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*32 */ 0  , 0  , '"', 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  ,'/',
        /*48 */ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*64 */ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*80 */ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  ,'\\', 0  , 0  , 0 ,
        /*96 */ 0  , 0  ,'\b', 0  , 0  , 0  ,'\f', 0  , 0  , 0  , 0  , 0  , 0  , 0  ,'\n', 0 ,
        /*112*/ 0  , 0  ,'\r', 0  ,'\t', 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*128*/ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*144*/ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*160*/ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*176*/ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*192*/ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*208*/ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*224*/ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0 ,
        /*240*/ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0
    };

    template <class I, class O>
    inline Slice<I> decode_escape(std::basic_string<O> &out, Slice<I> in, Slice<I> &full) {
        throw_if(InvalidJson, in.len == 0  , "Invalid escape in string: '", full, "'");
        auto esc_val = *in.start;
        if (esc_val == 'u') {
            throw_if(InvalidJson, in.len < 5, "Invalid \\u escape in string: '", full, "'");
            uint16_t val = decode_u(out, Slice<I>(in.start+1, 4));
            // UTF-16 surrogate pairs look like this: \uD801\uDC37
            // and should be decoded to a uint32_t value of 150370
            // The slice starts at the first u (uD801\uDC37), so 
            // if the val is in the correct ranges, and slice is >= 11 chars, 
            // there should be a second escape seq.
            if (in.len > 10 && val >= 0xd800 && val < 0xdc00) {
                uint16_t lowval = decode_u(out, Slice<I>(in.start+7, 4));
                write_char<uint32_t>(out, ((val - 0xd800) * 0x400) + (lowval - 0xdc00) + 0x10000);
                return in.slice_from(11);
            }
            write_char<uint16_t>(out, val);
            return in.slice_from(5);
        }
        throw_if(InvalidJson, esc_val > 255, "Invalid escape in string: '", full, "'");
        auto result = escape_lookup[(uint8_t)esc_val];
        throw_if(InvalidJson, result == 0  , "Invalid escape in string: '", full, "'");
        out.push_back(result);
        return in.slice_from(1);
    }

    static uint8_t hex_mask[256] = {
        // 1 if the corresponding character is a valid hex char, 0 if not
        /*0  */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*16 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*32 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*48 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
        /*64 */ 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*96 */ 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*112*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*128*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*144*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*160*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*176*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*192*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*208*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*224*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*240*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static uint8_t hex_lookup[256] = {
        // For each char, the decimal value of that digit in a hex number
        /*0  */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*16 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*32 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*48 */ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
        /*64 */ 0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*96 */ 0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*112*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*128*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*144*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*160*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*176*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*192*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*208*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*224*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*240*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    inline bool is_valid(uint8_t a, uint8_t b, uint8_t c, uint8_t d){
        return hex_mask[a] & hex_mask[b] & hex_mask[c] & hex_mask[d];
    }
    inline uint16_t decode_hex(uint8_t a, uint8_t b, uint8_t c, uint8_t d){
        return (hex_lookup[a] << 12) + (hex_lookup[b] << 8) + (hex_lookup[c] << 4) + (hex_lookup[d]);
    }

    template <class I, class O>
    inline uint16_t decode_u(std::basic_string<O> &out, Slice<I> in){
        uint8_t a = *in.start;
        uint8_t b = *(in.start+1);
        uint8_t c = *(in.start+2);
        uint8_t d = *(in.start+3);
        throw_if(InvalidJson, !is_valid(a, b, c, d), "Invalid \\u escape sequence");
        return decode_hex(a, b, c, d);
    }

    template <> inline void write_char(std::basic_string<uint8_t> &out, uint32_t val){
        if (val <= 0x7f) { out.push_back(val & 0xff); }
        else if (val <= 0x7ff) {
            out.push_back((0xc0 | ((val >> 6) & 0xff)));
            out.push_back((0x80 | ((val & 0x3f))));
        } else if (val <= 0xffff) {
            out.push_back((0xe0 | ((val >> 12) & 0xff)));
            out.push_back((0x80 | ((val >> 6) & 0x3f)));
            out.push_back((0x80 | (val & 0x3f)));
        } else {
            out.push_back((0xf0 | ((val >> 18) & 0xff)));
            out.push_back((0x80 | ((val >> 12) & 0x3f)));
            out.push_back((0x80 | ((val >> 6) & 0x3f)));
            out.push_back((0x80 | (val & 0x3f)));
        }
    }

    template <> inline void write_char(std::basic_string<uint8_t> &out, uint16_t val){
        // utf16 > utf8 conversion
        if (val <= 0x7f) { out.push_back(val & 0xff); }
        else if (val <= 0x7ff) {
            out.push_back((0xc0 | ((val >> 6) & 0xff)));
            out.push_back((0x80 | ((val & 0x3f))));
        }else {
            out.push_back((0xe0 | ((val >> 12) & 0xff)));
            out.push_back((0x80 | ((val >> 6) & 0x3f)));
            out.push_back((0x80 | (val & 0x3f)));
        }
        
    }

}
