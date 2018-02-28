#pragma once
#include <string>

extern "C" PyObject *UNDEFINED_OBJ;

#include "../iters/convert.hpp"

namespace ss::iter {
    using namespace std::literals;

    template<class T> struct DefaultValue{
        bool have_default;
        Converter<PyObj, T> converter;
        T value;

        DefaultValue(PyObj &value) : have_default(value.obj != UNDEFINED_OBJ), converter(&value, "utf-8"s) {
            if (have_default) {
                converter.convert();
                this->value = *converter.to;
            }
        }

        template<class ... Args>
        inline T default_or(Args... args) {
            throw_if(MissingValue, !have_default, args...);
            return value;
        }
    };

}