#include <iostream>


namespace ss{

    template<class T>
    inline int64_t slice_to_int(T &src) {
        auto stripped = src.template strip<' ', '\t'>();
        throw_if(ValueError, stripped.len == 0, "Tried to parse empty value as integer");
        const uint8_t *end = stripped.end();
        const uint8_t *cur = stripped.begin();
        int64_t val = 0;

        bool neg = false;
        if (*cur == '-') {
            neg = true;
            ++cur;
        }
        while (cur != end) {
            auto cur_val = *cur;
            throw_if(ValueError, cur_val < '0' || cur_val > '9', "Invalid integer: '", stripped, "'");
            val = val * 10 + (cur_val - '0');
            ++cur;
        }
        return neg ? -val : val;
    }

}