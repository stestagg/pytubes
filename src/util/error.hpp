#pragma once

#include <sstream>
#include <ostream>
#include <type_traits>
#include <typeinfo>
#include <new>
#include <stdexcept>

// This is a micro-optimization based on throws being unlikely
// Taken from rapidjson source.
#ifndef _SS_ERROR_UNLIKELY
#if defined(__GNUC__) || defined(__clang__)
#define _SS_ERROR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define _SS_ERROR_UNLIKELY(x) (x)
#endif
#endif

#ifndef _NOEXCEPT
#define _NOEXCEPT noexcept
#endif

#if defined(__clang__) || defined(_MSC_VER)

#define NORETURN_A [[noreturn]]
#define NORETURN_B

#else

#define NORETURN_A
#define NORETURN_B __attribute__((noreturn))

#endif

#define NORETURN(x) NORETURN_A x NORETURN_B




template<typename... Ts> struct make_void { typedef void type;};
template<typename... Ts> using void_t = typename make_void<Ts...>::type;

namespace ss{

    using MemoryError = std::bad_alloc;
    using TypeError = std::bad_cast;
    using ValueError = std::invalid_argument;
    using IOError = std::ios_base::failure;
    using IndexError = std::out_of_range;
    using OverflowError = std::overflow_error;
    using ArithmeticError = std::range_error;
    using RuntimeError = std::runtime_error;

    class StopIterationExc: public std::exception {};
    static StopIterationExc StopIteration;

    class PyExceptionRaisedExc: public std::exception {};
    static PyExceptionRaisedExc PyExceptionRaised;

    class MissingValue: public std::exception {
        std::string why;

    public:
        MissingValue(std::string why): why(why) {}
        const char *what() const _NOEXCEPT {
            return why.c_str();
        }
    };

    template<class T, class Enable=void>
    struct has_ostream_operator : std::false_type {};
    template<class T>
    struct has_ostream_operator<T, void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())> > : std::true_type {};
    static_assert(has_ostream_operator<int>::value, "has_ostream_operator failed, int has ostream << defined");

    inline void _append_item(std::stringstream &ss){}

    template<class T>
    inline typename std::enable_if<has_ostream_operator<T>::value, void>::type
    _append_item(std::stringstream &ss, T const &first){
        ss << first;
    }

    template<class T>
    inline typename std::enable_if<!has_ostream_operator<T>::value, void>::type
    _append_item(std::stringstream &ss, T const &first){
        static_assert(has_ostream_operator<T>::value == false, "enable_if failed");
        ss << std::string(first);
    }

    template<class T, class ...Args>
    inline void _append_item(std::stringstream &ss, T first, Args const &... rest) {
        _append_item(ss, first);
        _append_item(ss, rest...);
    }

    template<class ...Args>
    inline std::string make_str(Args const &... parts) {
        std::stringstream ss;
        _append_item(ss, parts...);
        return ss.str();
    }


    #define throw_if(Ty, cond, ...) if(_SS_ERROR_UNLIKELY(cond)) { throw_py<Ty>(__VA_ARGS__);}
    #define static_throw_if(Ty, cond) if(_SS_ERROR_UNLIKELY(cond)) { throw Ty;}

    template<class T, class ...Args>
    NORETURN(inline void) throw_py(Args const &... args){
        throw T(make_str(args...));
    }


}