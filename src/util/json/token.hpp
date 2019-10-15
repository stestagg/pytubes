#pragma once


namespace ss{ namespace json{

    using std::make_pair;

    template<class T>
    inline Slice<T> stripWhiteSpace(const Slice<T> &in) {
        /* Whitespace is:
          %x20  - Space
          %x09  - Tab
          %x0A  - Line feed
          %x0D  - CR
        */
        return in.template lstrip<0x20, 0x09, 0x0a, 0x0d>();
    }

    template<class T>
    inline Slice<T> stripWhiteSpaceEnd(const Slice<T> &in) {
        // As stripWhiteSpace, but remove trailing whitespace
        return in.template rstrip<0x20, 0x09, 0x0a, 0x0d>();
    }

    template<class T>
    const T *find_string_end(Slice<T> source) {
        while(true) {
            auto end = source.find_first('"');
            throw_if(InvalidJson, end == source.end(), "Unterminated string: '\"", source, "'");
            // Found a ", but was it prefixed by a '\'?
            if (*(end - 1) == '\\') {
                // It was prefixed by a '\' but this might be ok, if there are an even
                // number of \ chars before it
                auto slash_start = end - 1;
                size_t slash_count = 0;
                while (slash_start >= source.start) {
                    if (*slash_start != '\\') { break; }
                    slash_count += 1;
                    slash_start -= 1;
                }
                if (slash_count % 2 == 0) {
                    // String looks like this: \\" which terminates it correctly
                    // so return the slice
                    return end;
                }
                // we found a \", so false alarm, increment cur_pos and try again
                source = source.slice_from_ptr(end+1);
            } else {
                return end;
            }
        }
    }

    template<class T>
    inline  const T *find_object_end(Slice<T> source) {
        while(true) {
            // '}' can occur within strings, or to close a nested object
            // look for the next '"' or '{' to handle this situation.
            // find_first returns a pointer to beyond the end of slice if not found
            auto match = source.template find_first_of<'}', '"', '{'>();
            throw_if(InvalidJson, match == source.end(), "Unclosed object");
            const T *next_end = source.end();
            switch (*match) {
                case '}':
                    return match;
                case '{':
                    next_end = find_object_end<T>(source.slice_from_ptr(match+1));
                    break;
                case '"':
                    next_end = find_string_end<T>(source.slice_from_ptr(match+1));
                    break;
            }
            source = source.slice_from_ptr(next_end + 1);
        }
    }

    template<class T>
    inline const T *find_array_end(Slice<T> source) {
        while(true) {
            // ']' can occur within strings, or to close a nested array
            // look for the next '"' or '[' to handle this situation.
            // find_first returns a pointer to beyond the end of slice if not found
            auto match = source.template find_first_of<']', '"', '['>();
            throw_if(InvalidJson, match == source.end(), "Unclosed array");
            const T *next_end = source.end();
            switch (*match) {
                case ']':
                    return match;
                case '[':
                    next_end = find_array_end<T>(source.slice_from_ptr(match+1));
                    break;
                case '"':
                    next_end = find_string_end<T>(source.slice_from_ptr(match+1));
                    break;
            }
            source = source.slice_from_ptr(next_end + 1);
        }
    }

    // Lookup table to identify which character values could make up
    // part of a json number.
    static uint8_t num_chars[256] = {
        /*0  */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*16 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*32 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0,
        /*48 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
        /*64 */ 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*96 */ 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*112*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*128*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*144*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*160*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*176*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*192*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*208*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*224*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /*240*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    template<class T>
    const T *find_num_end(Slice<T> source) {
        // Num is the fallback type, so it may not actually be a number
        // at all, so if the first char isn't valid for a number, just raise
        throw_if(InvalidJson, !num_chars[(uint8_t)*source.start], "Invalid json: '", source, "'");
        for (const T *cur_char = source.begin(); cur_char != source.end(); ++cur_char) {
            if (num_chars[(uint8_t)*cur_char] == 0) {
                return cur_char;
            }
        }
        return source.end();
    }

    template<class T>
    Value<T> tokenize(Slice<T> source) {
        const T *end;
        source = stripWhiteSpace(source);
        if (source.len == 0) { return Value<T>(Type::Unset, source); }
        switch (source.start[0]) {
            case 'n':
                throw_if(InvalidJson, source.len < 4, "Expected null, found '", source, "'");
                return Value<T>(Type::Null, source.slice_to(4));
            case 't':
                throw_if(InvalidJson, source.len < 4,  "Expected true, found '", source, "'");
                return Value<T>(Type::Bool, source.slice_to(4));
            case 'f':
                throw_if(InvalidJson, source.len < 5, "Expected false, found '", source, "'");
                return Value<T>(Type::Bool, source.slice_to(5));
            case '"':
                source = source.slice_from(1);
                end = find_string_end<T>(source);
                return Value<T>(Type::String, source.slice_to_ptr(end));
            case '{':
                source = source.slice_from(1);
                end = find_object_end<T>(source);
                return Value<T>(Type::Object, source.slice_to_ptr(end));
            case '[':
                source = source.slice_from(1);
                end = find_array_end<T>(source);
                return Value<T>(Type::Array, source.slice_to_ptr(end));
            default:
                end = find_num_end<T>(source);
                return Value<T>(Type::Number, source.slice_to_ptr(end));
        }
    }

    template<class T>
    Value<T> tokenize_entire(Slice<T> source) {
        source = stripWhiteSpace(stripWhiteSpaceEnd(source));
        if (source.len == 0) { return Value<T>(Type::Unset, source); }
        switch (source.start[0]) {
            case 'n':
                throw_if(InvalidJson, source.len < 4, "Expected null, found '", source, "'");
                return Value<T>(Type::Null, source);
            case 't':
                throw_if(InvalidJson, source.len < 4,  "Expected true, found '", source, "'");
                return Value<T>(Type::Bool, source);
            case 'f':
                throw_if(InvalidJson, source.len < 5, "Expected false, found '", source, "'");
                return Value<T>(Type::Bool, source);
            case '"':
                throw_if(InvalidJson, source.len < 2, "Unterminated string found, '", source, "'");
                source = source.slice_from(1);
                return Value<T>(Type::String, source.slice_to(source.len - 1));
            case '{':
                throw_if(InvalidJson, source.len < 2, "Unterminated object found, '", source, "'");
                source = source.slice_from(1);
                return Value<T>(Type::Object, source.slice_to(source.len - 1));
            case '[':
                throw_if(InvalidJson, source.len < 2, "Unterminated array found, '", source, "'");
                source = source.slice_from(1);
                return Value<T>(Type::Array, source.slice_to(source.len - 1));
            default:
                return Value<T>(Type::Number, source);
        }
    }

}}