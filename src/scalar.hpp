#pragma once

#include "util/slice.hpp"
#include "util/pyobj.hpp"
#include "util/json/json.hpp"

namespace ss::iter {

    static const char null = 0;

    using NullType = std::tuple<>;
    using ByteSlice = Slice<uint8_t>;
    struct Utf8 : ByteSlice {
        using ByteSlice::ByteSlice;
        Utf8(const Slice<uint8_t> &src): ByteSlice(src){}
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
    };

    template<class T> struct ScalarType_t {  };
    template<> struct ScalarType_t<NullType> {
        constexpr static const ScalarType scalar_type = ScalarType::Null;
        using type = NullType; 
        static const char * const type_name;
    };
    template<> struct ScalarType_t<bool> {
        constexpr static const ScalarType scalar_type = ScalarType::Bool;
        using type = bool; 
        static const char * const type_name;
    };
    template<> struct ScalarType_t<int64_t> {
        constexpr static const ScalarType scalar_type = ScalarType::Int64;
        using type = int64_t; 
        static const char * const type_name;
    };
    template<> struct ScalarType_t<double> {
        constexpr static const ScalarType scalar_type = ScalarType::Float;
        using type = double; 
        static const char * const type_name;
    };
    template<> struct ScalarType_t<ByteSlice> {
        constexpr static const ScalarType scalar_type = ScalarType::ByteSlice;
        using type = ByteSlice; 
        static const char * const type_name;
    };
    template<> struct ScalarType_t<Utf8> {
        constexpr static const ScalarType scalar_type = ScalarType::Utf8;
        using type = Utf8; 
        static const char * const type_name;
    };
    template<> struct ScalarType_t<PyObj> {
        constexpr static const ScalarType scalar_type = ScalarType::Object;
        using type = PyObj; 
        static const char * const type_name;
    };
    template<> struct ScalarType_t<JsonUtf8> {
        constexpr static const ScalarType scalar_type = ScalarType::JsonUtf8;
        using type = JsonUtf8; 
        static const char * const type_name;
    };

    template<> struct ScalarType_t<uint8_t> {
        static const char * const type_name;
    };
    constexpr const char * const ScalarType_t<NullType>::type_name = "Null";
    constexpr const char * const ScalarType_t<bool>::type_name = "Bool";
    constexpr const char * const ScalarType_t<int64_t>::type_name = "Int64";
    constexpr const char * const ScalarType_t<double>::type_name = "Float";
    constexpr const char * const ScalarType_t<ByteSlice>::type_name = "Bytes";
    constexpr const char * const ScalarType_t<Utf8>::type_name = "Utf8";
    constexpr const char * const ScalarType_t<PyObj>::type_name = "Object";
    constexpr const char * const ScalarType_t<JsonUtf8>::type_name = "Json";

    template<class T>
    struct type_name_op{
        constexpr inline const char *operator()() { return ScalarType_t<T>::type_name; }
    };

    template<template <class U> class T, class... Args> 
    constexpr inline
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
            default:  throw_py<RuntimeError>("Got unexpected dtype value:  ", (size_t)type);
        }
    }

    constexpr inline const char *type_name(ScalarType type) {
        return dispatch_type<type_name_op>(type);
    }


    inline std::ostream & operator<< (std::ostream &out, NullType const &t) {
        return out << "Null";
    }

}