#pragma once

#include <vector>

#include "slice.hpp"

namespace ss {

    // Runtime sized array that has a fixed memory address
    // over its lifetime, and doesn't support resizing

    template<class T>
    struct Array{
        using iterator = const T*;
        using const_iterator = const T*;

        size_t size;
        std::unique_ptr<T[]> values;

        Array() : size(0){}

        Array(const std::vector<T> &values) :
            size(values.size()),
            values(new T[values.size()])
        {
            std::copy(values.begin(), values.end(), begin());
        }

        Array(const Array<T> &other) :
            size(other.size),
            values(new T[other.size])
        {
            std::copy(other.begin(), other.end(), begin());
        }

        Array(size_t size) : size(size), values(new T[size]) {}

        inline Array<T> &operator=(const Array<T> &other) {
            throw_if(RuntimeError, values && size, "Tried to assign to already-populated array");
            size = other.size;
            values = std::unique_ptr<T[]>(new T[other.size]);
            std::copy(other.begin(), other.end(), begin());
            return *this;
        }

        inline T& operator[](size_t index) const { return values[index]; }

        T* begin() const  { return &values[0]; }
        T* end() const { return &values[size]; }
        const T* cbegin() const { return &values[0]; }
        const T* cend() const { return &values[size]; }

        const Slice<T> slice() const { return Slice<T>(&values[0], size); }

    };

    template<class T>
    inline std::ostream & operator<< (std::ostream &out, Array<T> const &t) {
        out << "{ ";
        for (auto &item: t) {
            out << item << ", ";
        }
        return out << " }";
    }

};
