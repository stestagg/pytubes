 #pragma once

#include "../util/pyobj.hpp"
#include "../iter.hpp"


namespace ss{ namespace iter{

    using ToPyFn = PyObj (*) (const void *);

    template <class T> inline PyObj to_py(const void *val);

    template <> inline PyObj to_py<NullType>(const void *val) {
        return PyObj(Py_None);
    }

    template <> inline PyObj to_py<bool>(const void *val) {
        return PyObj(*(bool*)val ? Py_True : Py_False);
    }

    template<> inline PyObj to_py<int64_t>(const void *val) {
        return PyObj(PyLong_FromLongLong(*(const int64_t *)val));
    }

    template<> inline PyObj to_py<double>(const void *val) {
        return PyObj(PyFloat_FromDouble(*(const double *)val), true);
    }

    template <> inline PyObj to_py<ByteSlice>(const void *val) {
        ByteSlice *v = (ByteSlice *)val;
        return PyObj(PyBytes_FromStringAndSize((const char *)v->start, v->len), true);
    }

    template <> inline PyObj to_py<Utf8>(const void *val) {
        ByteSlice *v = (ByteSlice *)val;
        return PyObj(PyUnicode_FromStringAndSize((const char *)v->start, v->len), true);
    }

    template <> inline PyObj to_py<PyObj>(const void *val) {
        const PyObj *v = (const PyObj *)val;
        return PyObj(v->obj);
    }

    PyObj py_long_from_string(const ByteSlice &slice) {
        //PyLong_FromString requires a null-terminated char array to parse
        // so make a copy, and parse that
        std::basic_string<uint8_t> buffer(slice);
        return PyLong_FromString((const char *)buffer.c_str(), NULL, 10);
    }

    template <> inline PyObj to_py<JsonUtf8>(const void *_val) {
    const JsonUtf8 *val = (const JsonUtf8 *)_val;
    std::basic_string<uint8_t> buffer;
    Slice<uint8_t> slice;
    PyObj container;
    using Parser = json::parse::OptimisticParser<uint8_t>;
    switch(val->type) {
        case json::Type::Unset:
            throw_py<MissingValue>("Missing Json value");
        case json::Type::Null:
            Parser::parse_null(*val);
            return PyObj(Py_None);
        case json::Type::Bool:
            return PyObj(Parser::parse_bool(*val) ? Py_True : Py_False);
        case json::Type::String:
            slice = Parser::parse_string(*val, buffer);
            container = PyObj(PyUnicode_Decode((const char *)slice.start, slice.len, "utf-8", "surrogatepass"), true);
            if (!container.obj) { throw PyExceptionRaised; }
            return container; 
        case json::Type::Number:
            container = py_long_from_string(val->slice);
            if(PyErr_Occurred()){
                PyErr_Clear();
                return PyObj(PyFloat_FromDouble(Parser::parse_double(*val)), true);
            }
            return container;
        case json::Type::Array:
            container = PyObj(PyList_New(0), true);
            if (!container.obj) { throw std::bad_alloc(); }
            for (auto v : Parser::parse_array(*val)) {
                PyList_Append(container.obj, to_py<JsonUtf8>((const void *)&v).obj);
            }
            return container;
        case json::Type::Object:
            container = PyObj(PyDict_New(), true);
            if (!container.obj) { throw std::bad_alloc(); }
            for (auto item : Parser::parse_object(*val)) {
                auto name = to_py<JsonUtf8>((const void *)&item.first);
                auto value = to_py<JsonUtf8>((const void *)&item.second);
                auto result = PyDict_SetItem(container.obj, name.obj, value.obj);
                if (result == -1){
                    throw PyExceptionRaised;
                }
            }
            return container;
        };
        return PyObj();
    }

    template <> inline PyObj to_py<TsvRow>(const void *_val) {
        auto row = (TsvRow *)_val;
        PyObj container= PyObj(PyList_New(0), true);
        if (!container.obj) { throw std::bad_alloc(); }

        for (auto v : *row) {
            PyList_Append(container.obj, to_py<ByteSlice>((const void *)&v).obj);
        }
        return container;
    }


    template<class T, class Enable>
    struct to_py_op{
        constexpr inline ToPyFn operator()() const { return to_py<T>; }
    };


    class ToPyIter : public Iter {
    /*<-
        Iter: 
            ToPyIter:
                init: [AnyIter]
                extra: >
                    const PyObj &get(size_t index)
        Tube:
            ToPy:
                props: [Tube parent]
                dtype: return (Object, ) * len(self.parent.dtype)
                iter: [ToPyIter, [parent.iter]]
    ->*/

        Slice<SlotPointer> input_slots;
        std::vector<ToPyFn> fns;
        std::unique_ptr<PyObj[]> values;

    public:

        Slice<SlotPointer> get_slots(){
            return input_slots;
        }

        ToPyIter(AnyIter parent) : input_slots(parent.get()->get_slots()) { 
            values = std::unique_ptr<PyObj[]>(new PyObj[input_slots.len]);
            for (auto slot : input_slots) {
                fns.push_back(dispatch_type<to_py_op>(slot.type));
            }
        } 

        void next() {
            for (size_t i = 0; i < fns.size(); ++i) {
                values[i] = fns[i](input_slots[i]);
            }
        }

        const PyObj &get(size_t index) {
            return values[index];
        }
    };

}}