#pragma once

namespace ss{ namespace json{ namespace parse {

    template <class T>
    inline bool is_empty(T &&container) {
        return container.begin() == container.end();
    }

    template <class T>
    class FailsafeParser{

        using ObjectView = CollectionView<T, ObjectIter<T, FailsafeParser>>;
        using ArrayView = CollectionView<T, ArrayIter<T, FailsafeParser>>;

    public:
        static inline void parse_null(const Value<T> &val) {}

        static inline bool parse_bool(const Value<T> &val){
            switch(val.type) {
                case Type::Bool:
                    return *(val.slice.start) == 't';
                case Type::String:
                    return val.slice.len > 0;
                case Type::Number:
                    return OptimisticParser<T>::parse_double(val) != 0;
                case Type::Array:
                    return !is_empty(OptimisticParser<T>::parse_array(val));
                case Type::Object:
                    return !is_empty(OptimisticParser<T>::parse_object(val));
                default:
                    return false;
            };
        }

        static inline double parse_double(const Value<T> &val) {
            switch(val.type) {
                case Type::Number:
                    return OptimisticParser<T>::parse_double(val);
                case Type::Bool:
                    return OptimisticParser<T>::parse_bool(val) ? 1 : 0;
                default:
                    return 0;
            }
        }

        static inline int64_t parse_int(const Value<T> &val){
            return parse_double(val);
        }

        static inline Slice<T> parse_string(const Value<T> &val, std::basic_string<T> &buffer) {
            switch(val.type) {
                case Type::String:
                    return string::decode_str(val.slice, buffer);
                default:
                    return Slice<T>();
            };
        }

        static inline ArrayView parse_array(const Value<T> &val) {
            switch(val.type) {
                case Type::Array:
                    return ArrayView(val.slice);
                default:
                    return ArrayView(Slice<T>());
            }
        }

        static inline ObjectView parse_object(const Value<T> &val) {
            switch(val.type) {
                case Type::Object:
                    return ObjectView(val.slice);
                default:
                    return ObjectView(Slice<T>::Null());
            }
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