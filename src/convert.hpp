#pragma once

#include <array>

#include "iters/topy.hpp"
#include "util/atoi.hpp"

namespace ss{ namespace iter{

    struct AnyConverter{
        virtual SlotPointer get_slot() = 0;
        virtual void convert() = 0;
        virtual ~AnyConverter() = default;
    };

    template <class F, class T, int Enable=0> struct Converter : AnyConverter{
        const T *to;

        SlotPointer get_slot() { return to; }

        Converter(const F *from, const std::string &codec) {
            throw_py<ValueError>(
                "Unsupported conversion from ",
                ScalarType_t<F>::type_name(),
                " to ",
                ScalarType_t<T>::type_name()
            );
        }
        void convert() {}
    };

    template<class X> struct Converter<X, X> : AnyConverter{
        const X *to;
        Converter(const X *from, const std::string &codec) : to(from) {}
        SlotPointer get_slot() { return to; }
        void convert() {};
    };

    template<> struct Converter<ByteSlice, Utf8> : AnyConverter{
        Utf8 *to;
        Converter(const ByteSlice *from, const std::string &codec) : to((Utf8*)from) {
            throw_if(ValueError, codec != "utf-8", "Conversion from bytes to ",
                "str using the '", codec, "' codec is not supported")
        }
        SlotPointer get_slot() { return to; }
        void convert() {};
    };

    template<class F> struct Converter<F, typename std::enable_if<!std::is_same<F, PyObj>::value, PyObj>::type> : AnyConverter{
        const F *from;
        PyObj current;
        PyObj *to;
        Converter(const F *from, const std::string &codec) : from(from), to(&current) {}
        SlotPointer get_slot() { return to; }

        void convert() {
            current = to_py<F>(from);
        }
    };

    #define simple_convert(F, T, body) template<> struct Converter<F, T> : AnyConverter{ \
        const F *from; \
        T current; \
        T *to; \
        Converter(const F *from, const std::string &codec) : from(from), to(&current) {} \
        SlotPointer get_slot() { return to; } \
        void convert() body \
    };

    simple_convert(bool, int64_t, { current = *from ? 1 : 0; });
    simple_convert(bool, double, { current = *from ? 1 : 0; });
    simple_convert(int64_t, bool, { current = *from != 0; });
    simple_convert(double, bool, { current = *from != 0; });
    simple_convert(ByteSlice, bool, { current = from->len != 0; });
    simple_convert(Utf8, bool, { current = from->len != 0; });
    simple_convert(bool, ByteSlice, { current = *from ? ByteSlice((bytes*)"True", 4) : ByteSlice((bytes*)"False", 5); });
    simple_convert(bool, Utf8, { current = *from ? ByteSlice((bytes*)"True", 4) : ByteSlice((bytes*)"False", 5); });
    simple_convert(int64_t, double, { current = *from; });
    simple_convert(double, int64_t, { current = *from; });
    simple_convert(ByteSlice, TsvRow, { current = TsvRow(*from, NULL); });
    simple_convert(ByteSlice, CsvRow, { current = CsvRow(*from, NULL); });

    simple_convert(ByteSlice, int64_t, { current = slice_to_int(*from); });
    simple_convert(Utf8, int64_t, { current = slice_to_int(*from); });


    #define BYTES_UTF(T, U) typename std::enable_if< \
        std::is_same<T, ByteSlice>::value || \
        std::is_same<T, Utf8>::value \
    , U>::type

    template<class T> struct Converter<BYTES_UTF(T, JsonUtf8), T> : AnyConverter {
        using Parser = json::parse::OptimisticParser<bytes>;

        const JsonUtf8 *from;
        T current;
        std::array<char, 32> fixed_buffer;
        ByteString buffer;
        const T * const to;

        Converter(const JsonUtf8 *from, const std::string &codec)
            : from(from), to(&this->current) {}

        SlotPointer get_slot() { return to; }

        inline void convert() {
            switch(from->type){
                case json::Type::Null:
                    Parser::parse_null(*from);
                    current = T((bytes*)"None", 4);
                    break;
                case json::Type::Bool:
                    if (Parser::parse_bool(*from)) {
                        current = T((bytes*)"True", 4);
                    } else {
                        current = T((bytes*)"False", 5);
                    }
                    break;
                case json::Type::String:
                    current = T(Parser::parse_string(*from, buffer));
                    break;
                default:
                    throw_py<ValueError>(
                        "Cannot convert ",
                        json::json_type_name(from->type),
                        " json value to ",
                        ScalarType_t<T>::type_name());
            }
        }

    };

    inline PyObj encode_str(PyObject *from, const std::string &codec) {
        PyObj encoded;
        if (codec == "fs") {
            encoded = PyObj(PyUnicode_EncodeFSDefault(from), true);
        } else {
            encoded = PyObj(PyUnicode_AsEncodedString(from, codec.c_str(), "strict"), true);
        }
        if(!encoded.obj){ throw PyExceptionRaised; }
        return encoded.give();
    }

    inline PyObj decode_str(PyObject *from, const std::string &codec) {
        PyObj decoded;
        const char *buf = PyBytes_AsString(from);
        Py_ssize_t buf_len = PyBytes_GET_SIZE(from);
        if (codec == "fs") {
            decoded = PyObj(PyUnicode_DecodeFSDefaultAndSize(buf, buf_len), true);
        } else {
            decoded = PyObj(PyUnicode_Decode(buf, buf_len, codec.c_str(), "strict"), true);
        }
        if(!decoded.obj){ throw PyExceptionRaised; }
        return decoded.give();
    }

    // PyObject to anything..
    template<class T> struct Converter<typename std::enable_if<!std::is_same<T, PyObj>::value, PyObj>::type, T> : AnyConverter{
        const PyObj *from;
        PyObj buffer;
        T current;
        const T *to;
        const std::string codec;

        SlotPointer get_slot() { return to; }

        Converter(const PyObj *from, const std::string &codec) :
            from(from),
            to(&current),
            codec(codec)
        {}

        template<class F> inline void convert_from() {
            PyObj uni = PyObj(PyObject_Repr(from->obj), true);
            Py_ssize_t size;
            const char * obj_desc = PyUnicode_AsUTF8AndSize(uni.obj, &size);
            throw_py<ValueError>(
                "Cannot convert from ",
                Slice<char>(obj_desc, size),
                " to ",
                ScalarType_t<T>::type_name()
            );
        }

        void convert() {
            PyObject *p = from->obj;
            if (p == Py_True) {
                convert_from<std::true_type>();
            } else if (p == Py_False) {
                convert_from<std::false_type>();
            } else if (p == Py_None) {
                convert_from<NullType>();
            } else if (PyUnicode_Check(p)) {
                convert_from<Py_UNICODE*>();
            } else if (PyBytes_Check(p)) {
                convert_from<bytes*>();
            } else if (PyFloat_Check(p)) {
                convert_from<double>();
            } else if (PyLong_Check(p)) {
                convert_from<int64_t>();
            } else {
                convert_from<PyObject *>();
            }
        }
    };

    #define py_convert_fn(F, T) template<> template<> inline void Converter<PyObj, T>::convert_from<F>()

    // PyObj > Null
    py_convert_fn(NullType, NullType) { current = std::tuple<>(); }

    // PyObj > Bytes
    py_convert_fn(std::true_type, ByteSlice) { current = ByteSlice((const bytes*)"True", 4); }
    py_convert_fn(std::false_type, ByteSlice) { current = ByteSlice((const bytes*)"False", 5); }
    py_convert_fn(NullType, ByteSlice) { current = ByteSlice((const bytes*)"None", 4); }
    py_convert_fn(Py_UNICODE*, ByteSlice){
        buffer = encode_str(from->obj, codec);
        const char *buf = PyBytes_AsString(buffer.obj); // Owned by buffer
        current = ByteSlice((const bytes*)buf, PyBytes_GET_SIZE(buffer.obj));
    }
    py_convert_fn(bytes*, ByteSlice) {
        buffer = PyObj(from->obj); // keep a ref, to make sure it doesn't go away
        const char *buf = PyBytes_AsString(from->obj);
        if(!buf){ throw PyExceptionRaised; }
        current = ByteSlice((const bytes*)buf, PyBytes_GET_SIZE(from->obj));
    }
    py_convert_fn(int64_t, ByteSlice) {
        buffer = PyObj(PyObject_Str(from->obj), true);
        Py_ssize_t len;
        const char *buf = PyUnicode_AsUTF8AndSize(buffer.obj, &len);
        if(!buf){ throw PyExceptionRaised; }
        current = ByteSlice((const bytes*)buf, len);
    }
    py_convert_fn(double, ByteSlice) {
        buffer = PyObj(PyObject_Str(from->obj), true);
        Py_ssize_t len;
        const char *buf = PyUnicode_AsUTF8AndSize(buffer.obj, &len);
        if(!buf){ throw PyExceptionRaised; }
        current = ByteSlice((const bytes*)buf, len);
    }

    // PyObj > str
    py_convert_fn(std::true_type, Utf8) { current = Utf8((const bytes*)"True", 4); }
    py_convert_fn(std::false_type, Utf8) { current = Utf8((const bytes*)"False", 5); }
    py_convert_fn(NullType, Utf8) { current = Utf8((const bytes*)"None", 4); }
    py_convert_fn(Py_UNICODE*, Utf8) {
        Py_ssize_t size;
        buffer = PyObj(from->obj); // keep a ref, to make sure it doesn't go away
        const char *buf = PyUnicode_AsUTF8AndSize(buffer.obj, &size);
        if (!buf) { throw PyExceptionRaised; }
        current = Utf8((const bytes*)buf, size);
    }
    py_convert_fn(bytes*, Utf8) {
        buffer = decode_str(from->obj, codec);
        Py_ssize_t size;
        const char *buf = PyUnicode_AsUTF8AndSize(buffer.obj, &size);
        if (!buf) { throw PyExceptionRaised; }
        current = Utf8((const bytes*)buf, size);
    }
    py_convert_fn(int64_t, Utf8) {
        buffer = PyObj(PyObject_Str(from->obj), true);
        Py_ssize_t len;
        const char *buf = PyUnicode_AsUTF8AndSize(buffer.obj, &len);
        if(!buf){ throw PyExceptionRaised; }
        current = Utf8((const bytes*)buf, len);
    }
    py_convert_fn(double, Utf8) {
        buffer = PyObj(PyObject_Str(from->obj), true);
        Py_ssize_t len;
        const char *buf = PyUnicode_AsUTF8AndSize(buffer.obj, &len);
        if(!buf){ throw PyExceptionRaised; }
        current = Utf8((const bytes*)buf, len);
    }

    // PyObj > bool
    py_convert_fn(std::true_type, bool) { current = true; }
    py_convert_fn(std::false_type, bool) { current = false; }
    py_convert_fn(int64_t, bool) { current = PyLong_AsLongLong(from->obj); }
    py_convert_fn(bytes*, bool) { current = PyBytes_Size(from->obj) > 0; }
    py_convert_fn(Py_UNICODE*, bool) {
        PyUnicode_READY(from->obj);
        current = PyUnicode_GET_LENGTH(from->obj) > 0;
    }

    // PyObj > int64
    py_convert_fn(std::true_type, int64_t) { current = 1; }
    py_convert_fn(std::false_type, int64_t) { current = 0; }
    py_convert_fn(int64_t, int64_t) { current = PyLong_AsLongLong(from->obj); }
    py_convert_fn(double, int64_t) { current = (int64_t)PyFloat_AsDouble(from->obj); }
    py_convert_fn(Py_UNICODE*, int64_t) {
        auto num = PyObj(PyNumber_Long(from->obj), true);
        current = PyLong_AsLongLong(num.obj);
        if (PyErr_Occurred()) { throw PyExceptionRaised;}
    }

    // PyObj > double
    py_convert_fn(std::true_type, double) { current = 1; }
    py_convert_fn(std::false_type, double) { current = 0; }
    py_convert_fn(int64_t, double) { current = (double)PyLong_AsLongLong(from->obj); }
    py_convert_fn(double, double) { current = PyFloat_AsDouble(from->obj); }
    py_convert_fn(Py_UNICODE*, double) {
        auto num = PyObj(PyNumber_Float(from->obj), true);
        current = PyFloat_AsDouble(num.obj);
        if (PyErr_Occurred()) { throw PyExceptionRaised;}
    }

    // PyObj > json
    py_convert_fn(Py_UNICODE*, JsonUtf8) {
        Py_ssize_t size;
        buffer = PyObj(from->obj); // keep a ref, to make sure it doesn't go away
        const char *buf = PyUnicode_AsUTF8AndSize(buffer.obj, &size);
        current = json::tokenize_entire(ByteSlice((bytes *)buf, size));
    }
    py_convert_fn(bytes*, JsonUtf8) {
        buffer = PyObj(from->obj); // keep a ref, to make sure it doesn't go away
        const char *buf = PyBytes_AsString(from->obj);
        if(!buf){ throw PyExceptionRaised; }
        auto slice = ByteSlice((bytes *)buf, PyBytes_GET_SIZE(from->obj));
        current = json::tokenize_entire(slice);
    }

     // PyObj > Tsv
    py_convert_fn(Py_UNICODE*, TsvRow) {
        Py_ssize_t size;
        buffer = PyObj(from->obj); // keep a ref, to make sure it doesn't go away
        const char *buf = PyUnicode_AsUTF8AndSize(buffer.obj, &size);
        current = TsvRow(ByteSlice((bytes *)buf, size), NULL);
    }
    py_convert_fn(bytes*, TsvRow) {
        buffer = PyObj(from->obj); // keep a ref, to make sure it doesn't go away
        const char *buf = PyBytes_AsString(from->obj);
        if(!buf){ throw PyExceptionRaised; }
        auto slice = ByteSlice((bytes *)buf, PyBytes_GET_SIZE(from->obj));
        current = TsvRow(slice, NULL);
    }
    py_convert_fn(Py_UNICODE*, CsvRow) {
        Py_ssize_t size;
        buffer = PyObj(from->obj); // keep a ref, to make sure it doesn't go away
        const char *buf = PyUnicode_AsUTF8AndSize(buffer.obj, &size);
        current = CsvRow(ByteSlice((bytes *)buf, size), NULL);
    }
    py_convert_fn(bytes*, CsvRow) {
        buffer = PyObj(from->obj); // keep a ref, to make sure it doesn't go away
        const char *buf = PyBytes_AsString(from->obj);
        if(!buf){ throw PyExceptionRaised; }
        auto slice = ByteSlice((bytes *)buf, PyBytes_GET_SIZE(from->obj));
        current = CsvRow(slice, NULL);
    }

}}