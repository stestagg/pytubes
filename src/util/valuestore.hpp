#pragma once


template <class T, class Enable=bool>
struct ValueStorage{};


template<class T>
struct ValueStorage<T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type> {
    T value;

    inline void set(T value) { this->value = value; }
    inline T *get_pointer() { return &this->value; }
};


template<class T>
struct ValueStorage<T, typename std::enable_if<std::is_arithmetic<typename T::el_t>::value, bool>::type> {
    using ElementType = typename T::el_t;
    std::basic_string<ElementType> value;
    T value_slice;

    inline void set(const T &value) { this->value = value; value_slice = T(this->value); }
    inline T *get_pointer() { return &value_slice; }
};
