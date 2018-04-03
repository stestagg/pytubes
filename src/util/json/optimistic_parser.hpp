#pragma once

namespace ss{ namespace json{ namespace parse {

    template <class T, Type ty> inline void checkType(const Value<T> &val) {
        throw_if(InvalidJson,
            val.type != ty,
            "Tried to interpret ",
            json_type_name(ty),
            " as ",
            json_type_name(val.type)
        );
    }

    template <class T>
    class OptimisticParser{

        using ObjectView = CollectionView<T, ObjectIter<T, OptimisticParser>>;
        using ArrayView = CollectionView<T, ArrayIter<T, OptimisticParser>>;

    public:
        static inline void parse_null(const Value<T> &val) {
            checkType<T, Type::Null>(val);
        }

        static inline bool parse_bool(const Value<T> &val){
            checkType<T, Type::Bool>(val);
            return *(val.slice.start) == 't';
        }

        static inline double parse_double(const Value<T> &val) {
            checkType<T, Type::Number>(val);
            char *end_ptr;
            auto value = PyOS_string_to_double((char *)val.slice.start, &end_ptr, NULL);
            if (PyErr_Occurred()) { throw PyExceptionRaised; }
            throw_if(ValueError, (unsigned char*)end_ptr != val.slice.end(),
                "could not convert string to float:",
                val.slice);
            return value;
        }

        static inline int64_t parse_int(const Value<T> &val){
            checkType<T, Type::Number>(val);
            T * endptr = 0;
            int64_t rv = strtol((char *)val.slice.start, (char**)&endptr, 10);
            throw_if(ValueError,
                endptr != val.slice.end(),
                "Number '", val.slice, "' could not be interpreted as integer");
            return rv;
        }

        static inline Slice<T> parse_string(const Value<T> &val, std::basic_string<T> &buffer) {
            checkType<T, Type::String>(val);
            return string::decode_str(val.slice, buffer);
        }

        static inline ArrayView parse_array(const Value<T> &val) {
            checkType<T, Type::Array>(val);
            return ArrayView(val.slice);
        }

        static inline ObjectView parse_object(const Value<T> &val) {
            checkType<T, Type::Object>(val);
            return ObjectView(val.slice);
        }

        static inline Slice<T> read_key_val_separator(Slice<T> source) {
            source = stripWhiteSpace(source);
            if (!source.len) { return source; }
            return stripWhiteSpace(source.slice_from(1));
        }

        static inline Slice<T> read_item_separator(Slice<T> source) {
            source = stripWhiteSpace(source);
            if (!source.len) { return source; }
            return stripWhiteSpace(source.slice_from(1));
        }

    };

}}}