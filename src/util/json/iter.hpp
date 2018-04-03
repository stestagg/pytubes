#pragma once

#include <iostream>

namespace ss{ namespace json{

    template<class T> Slice<T> after_value(Slice<T> source, Value<T> &val) {
        switch (val.type) {
            case Type::Array:
            case Type::Object:
            case Type::String:
                return source.slice_from_ptr(val.slice.end() + 1);
            default:
                return source.slice_from_ptr(val.slice.end());
        }
    }

    template<class T, class P> class ArrayIter {

        Value<T> cur;
        Slice<T> slice;

    public:
        ArrayIter(Slice<T> slice): cur(), slice(slice) {}

        inline bool operator==(const ArrayIter<T, P> &other) const {
            return slice.is(other.slice);
        }

        inline bool operator!=(const ArrayIter<T, P> &other) const {
            return !slice.is(other.slice);
        }

        inline ArrayIter<T, P>& operator++() {
            if (slice.is_empty()) { slice = Slice<T>::Null(); return *this; }
            cur = tokenize<T>(slice);
            slice = after_value(slice, cur);
            if (!slice.is_empty()) { slice = P::read_item_separator(slice); }
            return *this;
        }

        inline Value<T> operator*() const {
            return cur;
        }

    };

    template<class T, class P> class ObjectIter {

        Value<T> key;
        Value<T> value;

        Slice<T> slice;

    public:
        ObjectIter(Slice<T> slice) : key(), value(), slice(slice) {}

        inline bool operator!=(const ObjectIter<T, P> &other) const {
            return !slice.is(other.slice);
        }
        inline bool operator==(const ObjectIter<T, P> &other) const {
            return slice.is(other.slice);
        }

        inline ObjectIter<T, P>& operator++() {
            if (slice.is_empty()) { slice = Slice<T>::Null(); return *this; }
            key = tokenize<T>(slice);
            slice = after_value(slice, key);
            if (slice.is_empty()) {
                throw_py<InvalidJson>("Invalid object");
            }
            slice = P::read_key_val_separator(slice);
            value = tokenize<T>(slice);
            slice = after_value(slice, value);
            if (!slice.is_empty()) { slice = P::read_item_separator(slice); }
            return *this;
        }

        inline std::pair<Value<T>, Value<T> > operator*() const {
            return std::make_pair(key, value);
        }

    };

    template<class T, class U> struct CollectionView {
        using iterator = U;
        using const_iterator = const U;

        Slice<T> source;

        CollectionView(Slice<T> source) : source(source) {}

        inline iterator begin() {
            auto it = iterator(source);
            ++it;
            return it;
        }

        inline iterator end() {
            return iterator(Slice<T>::Null());
        }

    };

}}