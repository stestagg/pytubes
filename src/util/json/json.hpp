#pragma once

#include <ostream>

#include <Python.h>

#include "../slice.hpp"

namespace ss{ namespace json{

    class InvalidJson: public std::invalid_argument {
    public:
        InvalidJson(std::string what) : std::invalid_argument(what) {}
    };

    enum class Type{
        Unset=0,
        Null,
        Bool,
        Number,
        String,
        Array,
        Object,
    };

    template<class T>
    struct Value{
        using KeyCh = T;

        Slice<T> slice;
        Type type;

        Value(): slice(Slice<T>::Null()), type(Type::Unset) {}
        Value(Type type, Slice<T> slice): slice(slice), type(type) {}
        Value(const Value &src) : slice(src.slice), type(src.type) {}
    };

    inline const char *json_type_name(Type t) {
        switch (t) {
            case Type::Unset: return "unset";
            case Type::Null: return "null";
            case Type::Bool: return "bool";
            case Type::Number: return "number";
            case Type::String: return "string";
            case Type::Array: return "array";
            case Type::Object: return "object";
        }
        return "unknown";
    }

    inline std::ostream& operator<< (std::ostream &out, const json::Value<unsigned char> &v) {
        switch(v.type) {
            case json::Type::Unset:
                throw_py<ValueError>("Tried to print unset json value");
                break;
            case json::Type::String:
                out << "\"" << v.slice << "\""; break;
            case json::Type::Array:
                out << "[" << v.slice << "]"; break;
            case json::Type::Object:
                out << "{" << v.slice << "}"; break;
            default:
                out << v.slice;
        }
        return out;
    }

}}

#include "../error.hpp"

#include "string.hpp"
#include "token.hpp"
#include "iter.hpp"
#include "optimistic_parser.hpp"
