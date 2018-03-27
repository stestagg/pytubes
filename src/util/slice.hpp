#pragma once

#include <cstdint>
#include <cstring>

#include <ostream>
#include <string>
#include <algorithm>
#include <vector>

#include <cityhash.hpp>

#include "error.hpp"


namespace ss{

    namespace slice {
        static const uint8_t empty_array[0] = {};

        // Template based element-wise comparison function for int/char based slices.
        // Should allow the optimizer to do its best on small comparisons.
        template<size_t start, class T>
        static inline bool _static_startswith_impl(const T *val) { return true; }

        template<size_t start, class T, int A, int...More>
        static inline bool _static_startswith_impl(const T *val) {
            return (val[start] == A) && _static_startswith_impl<start+1, T, More...>(val);
        }

        template<class T>
        static inline bool is_one_of_impl(const T &val) { 
            return false; 
        }

        template<class T, T A, T...More>
        static inline bool is_one_of_impl(const T &val) { 
            return val == A || is_one_of_impl<T, More...>(val); 
        }

    }
        
    template <class T> struct Slice {

        /* This is basically a std::basic_string_view */
        using el_t = T;
        using iterator = const T*; 
        using const_iterator = const T*;

        const T *start;
        size_t len;

        static inline constexpr Slice<T> Null() {
            return Slice<T>((T*)slice::empty_array, 0);
        }

        Slice() : start((T*)slice::empty_array), len(0) {}
        Slice(const T *start, size_t len): start(start), len(len) {}
        Slice(const T *begin, const T *end, bool _): start(begin), len(end - begin) {}
        Slice(const Slice<T> &src) : start(src.start), len(src.len) {}

        // Special case char > uint8_t coercion
        template<class Q = T, typename std::enable_if<std::is_same<Q, uint8_t>::value, int>::type = 0>
        Slice(const char *start, size_t len): start((const uint8_t *)start), len(len) {}

        template<class Q = T, typename std::enable_if<std::is_same<Q, uint8_t>::value, int>::type = 0>
        Slice(const basic_string<char> &src): start((const uint8_t *)src.c_str()), len(src.length()) {}

        template<class Q = T, typename std::enable_if<std::is_arithmetic<Q>::value, int>::type = 0>
        Slice(const std::basic_string<T> &src) : start(src.c_str()), len(src.length()) {}
        
        Slice(const std::vector<T> &src) : start(src.data()), len(src.size()) {}

        template<class Q = T, typename std::enable_if<std::is_arithmetic<Q>::value, int>::type = 0>
        inline operator std::basic_string<Q>() const {
            return std::basic_string<T>(begin(), end());
        }

        inline const T* begin() const { return start; }
        inline const T* end() const { return &start[len]; }

        inline const T *cbegin() const { return start; }
        inline const T *cend() const { return &start[len]; }

        const T &operator[](size_t index) const {
            throw_if(IndexError, index >= len, "Tried to access item at index ",
                index, " beyond end of slice (", len, ") items");
            return *(start + index);
        }

        inline bool operator==(const Slice<T> &other) const {
            return len == other.len && std::equal(begin(), end(), other.begin());
        }

        inline bool operator!=(const Slice<T> &other) const {
            return !(*this == other);
        }

        inline bool is(const Slice<T> &other) const {
            return start == other.start && len == other.len;
        }

        bool is_empty() const { return len == 0; }
        bool is_null() const { return (void*)start == (void*)slice::empty_array; }

        template<int First, int...More>
        inline bool static_startswith() const {
            if (len < 1/*First*/ + sizeof...(More)) { return false /* too short */; }
            return slice::_static_startswith_impl<0, T, First, More...>(start);
        }

        inline const T *find_first(T value) const {
            return std::find(begin(), &start[len], value);
        }

        template<size_t... Chars>
        inline const T*find_first_of() const {
            T match[] = {Chars...};
            return std::find_first_of(begin(), end(), &match[0], &match[sizeof...(Chars)]);
        }

        template<typename std::conditional<std::is_arithmetic<T>::value, T, char>::type... Chars>
        inline Slice<T> lstrip() const {
            const T *cur = begin();
            while (cur < end()) {
                if (!slice::is_one_of_impl<T, Chars...>(*cur)){
                    break;
                }
                ++cur;
            }
            return slice_from_ptr(cur);
        }

        template<typename std::conditional<std::is_arithmetic<T>::value, T, char>::type... Chars>
        inline Slice<T> rstrip() const {
            const T *cur = end();
            while (cur >= start) {
                --cur;
                if (!slice::is_one_of_impl<T, Chars...>(*cur)){
                    break;
                }
            }
            return slice_to_ptr(cur+1);
        }

        template<typename std::conditional<std::is_arithmetic<T>::value, T, char>::type... Chars>
        inline Slice<T> strip() const {
            return rstrip<Chars...>().template lstrip<Chars...>();
        }

        inline Slice<T> slice_to_ptr(const T *end) const {
            size_t new_len = end - start;
            throw_if(IndexError,  new_len > len, "Tried to index after slice end");
            return Slice<T>(start, new_len);
        }

        inline Slice<T> slice_from_ptr(const T *new_start) const {
            size_t new_len = len - (new_start - start);
            throw_if(IndexError,  new_len > len , "Invalid slice");
            return Slice<T>(new_start, new_len);
        }

        inline Slice<T> slice_to(const size_t index) const {
            throw_if(IndexError,  index > len, "Tried to index after slice end");
            return Slice<T>(start, index);
        }

        inline Slice<T> slice_from(const size_t index) const {
            throw_if(IndexError,  index > len, "Tried to index after slice end");
            return Slice<T>(start + index, (len - index));
        }
    };

    using ByteSlice = Slice<uint8_t>;

    template<> inline const uint8_t *Slice<uint8_t>::find_first(const uint8_t value) const {
        auto match = (uint8_t *)memchr(start, value, len);
        return match ? match : end();
    }
    template<> inline const uint16_t *Slice<const uint16_t>::find_first(const uint16_t value) const {
        auto match = (uint16_t *)wmemchr((wchar_t*)start, (wchar_t)value, len);
        return match ? match : end();
    }

    template<class T, class U>
    inline 
    typename std::enable_if<
        std::is_same<
            typename std::make_unsigned<T>::type, 
            typename std::make_unsigned<U>::type
        >::value, 
        std::basic_string<T> 
    >::type 
    operator+(const std::basic_string<T> left, const ss::Slice<U> right) {
        return left + std::basic_string<T>((T*)right.start, right.len);
    }

    // iostream integration
    template<class T>
    inline std::ostream & operator<< (std::ostream &out, ss::Slice<T> const &t) {
        out << "Slice[" << t.len << "]";
        return out;
    }

    inline std::ostream & operator<< (std::ostream &out, ss::ByteSlice const &t) {
        return out << std::string((char *)t.start, t.len);
    }
    inline std::ostream & operator<< (std::ostream &out, ss::Slice<char> const &t) {
        return out << std::string(t.start, t.len);
    }

}


namespace std{
    template <class T>
    struct hash<ss::Slice<T>>{
        std::size_t operator()(const ss::Slice<T>& val) const {
            return CityHash<sizeof(size_t)>((const char *)val.start, val.len * sizeof(T));
        }
    };
}