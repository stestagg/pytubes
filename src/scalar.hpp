#pragma once

#include "util/slice.hpp"
#include "util/pyobj.hpp"
#include "util/json/json.hpp"
#include "util/tsv.hpp"

namespace ss{ namespace iter{

    static const char null = 0;

    using NullType = std::tuple<>;

    struct Utf8 : ByteSlice {
        Utf8() : ByteSlice() {}
        Utf8(const uint8_t *start, size_t len): ByteSlice(start, len) {}
        Utf8(const uint8_t *begin, const uint8_t *end, bool _): ByteSlice(begin, end, _) {}
        Utf8(const Slice<uint8_t> &src) : ByteSlice(src) {}
        Utf8(Utf8 &src) : ByteSlice(src.start, src.len) {}
        Utf8(const std::basic_string<uint8_t> &src) : ByteSlice(src) {}
    
        Utf8(const std::vector<uint8_t> &src) : ByteSlice(src) {}
    };
    using JsonUtf8 = json::Value<uint8_t>;

    enum class ScalarType{
        Null,
        Bool,
        Int64,
        Float,
        ByteSlice,
        Utf8,
        Object,
        JsonUtf8,
        Tsv
    };

    template<class T> struct ScalarType_t {  };
    template<> struct ScalarType_t<NullType> {
        constexpr static const ScalarType scalar_type = ScalarType::Null;
        using type = NullType; 
        static const char * const type_name() { return "Null"; };
    };
    template<> struct ScalarType_t<bool> {
        constexpr static const ScalarType scalar_type = ScalarType::Bool;
        using type = bool; 
        static const char * const type_name() { return "Bool"; };
    };
    template<> struct ScalarType_t<int64_t> {
        constexpr static const ScalarType scalar_type = ScalarType::Int64;
        using type = int64_t; 
        static const char * const type_name() { return "Int64"; };
    };
    template<> struct ScalarType_t<double> {
        constexpr static const ScalarType scalar_type = ScalarType::Float;
        using type = double; 
        static const char * const type_name() { return "Float"; };
    };
    template<> struct ScalarType_t<ByteSlice> {
        constexpr static const ScalarType scalar_type = ScalarType::ByteSlice;
        using type = ByteSlice; 
        static const char * const type_name() { return "Bytes"; };
    };
    template<> struct ScalarType_t<Utf8> {
        constexpr static const ScalarType scalar_type = ScalarType::Utf8;
        using type = Utf8; 
        static const char * const type_name() { return "Utf8"; };
    };
    template<> struct ScalarType_t<PyObj> {
        constexpr static const ScalarType scalar_type = ScalarType::Object;
        using type = PyObj; 
        static const char * const type_name() { return "Object"; };
    };
    template<> struct ScalarType_t<JsonUtf8> {
        constexpr static const ScalarType scalar_type = ScalarType::JsonUtf8;
        using type = JsonUtf8; 
        static const char * const type_name() { return "Json"; };
    };
    template<> struct ScalarType_t<TsvRow> {
        constexpr static const ScalarType scalar_type = ScalarType::Tsv;
        using type = TsvRow; 
        static const char * const type_name() { return "Tsv"; };
    };

    template<class T>
    struct type_name_op{
        constexpr inline const char *operator()() const { return ScalarType_t<T>::type_name(); }
    };

    template<class T>
    constexpr ScalarType dtype_from_type() { return ScalarType_t<T>::scalar_type; }

    template<template <class U> class T, class... Args> 
    /* constexpr(>c++14) */ inline
    decltype(T<NullType>()(std::declval<Args &&>()...))
    dispatch_type(ScalarType type, Args &&... args) {
        switch (type) {
            case ScalarType::Null: return T<NullType>()(args...);
            case ScalarType::Bool: return T<bool>()(args...);
            case ScalarType::Int64: return T<int64_t>()(args...);
            case ScalarType::Float: return T<double>()(args...);
            case ScalarType::ByteSlice: return T<ByteSlice>()(args...);
            case ScalarType::Utf8: return T<Utf8>()(args...);
            case ScalarType::Object: return T<PyObj>()(args...);
            case ScalarType::JsonUtf8: return T<JsonUtf8>()(args...);
            case ScalarType::Tsv: return T<TsvRow>()(args...);
            default:  throw_py<RuntimeError>("Got unexpected dtype value:  ", (size_t)type);
        }
        return T<NullType>()(args...);
    }


    template<class T> struct field_type_t {using type = NullType;};
    template<> struct field_type_t<JsonUtf8> {using type = JsonUtf8;};
    template<> struct field_type_t<TsvRow> {using type = ByteSlice;};

    template<class T>
    constexpr inline ScalarType field_dtype() { 
        using field_type = typename field_type_t<T>::type;
        return ScalarType_t<field_type>::scalar_type;
    }

    template<class T>
    struct field_dtype_op{
        constexpr inline ScalarType operator()() const { return field_dtype<T>(); }
    };

    inline ScalarType field_dtype_from_dtype(ScalarType dtype) {
        return dispatch_type<field_dtype_op>(dtype);
    }


    inline const char *type_name(ScalarType type) {
        return dispatch_type<type_name_op>(type);
    }


    inline std::ostream & operator<< (std::ostream &out, NullType const &t) {
        return out << "Null";
    }

}}