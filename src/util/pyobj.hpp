#pragma once

#include "Python.h"

namespace ss {


class PyObj{

    // Simple wrapper for a PyObject* that manages ref counting and scope
    // according to C++ rules, using move semantics to reduce refcount
    // calls

public:
    PyObject * obj;

    PyObj(): obj(0) {}
    PyObj(PyObject *obj, bool already_retained=false): obj(obj) {
        if (obj && !already_retained) { Py_INCREF(obj); }
    }
    PyObj(const PyObj &other) : obj(other.obj) {
        if(obj) { Py_INCREF(obj); }
    }

    PyObj static fromCall(PyObject *obj, bool already_retained=true) {
        if (obj == 0) throw PyExceptionRaised;
        return PyObj(obj, already_retained);
    }

    inline bool has_attr(const char *attr) const {
        return PyObject_HasAttrString(obj, attr);
    }

    inline PyObj get_attr(const char *attr) const {
        return PyObj::fromCall(PyObject_GetAttrString(obj, attr));
    }

    inline bool was_created() const {
        return obj != 0;
    }

    inline void assert_created() const {
        if(obj == 0) throw PyExceptionRaised;
    }

    inline PyObj(PyObj&& o) noexcept : obj(o.obj) { o.obj = 0; }
    inline PyObj& operator=(const PyObj& other) { Py_XDECREF(obj); obj = other.obj; Py_INCREF(obj); return *this;}
    inline PyObj& operator=(PyObject *other) { Py_XDECREF(obj); obj = other; return *this; }
    inline PyObj& operator=(PyObj&& other) { Py_XDECREF(obj); obj = other.obj; other.obj=0; return *this;}

    // give() is used to move the owned pointer to the receiver without calling DECREF
    inline PyObject *give() { auto rv = obj; obj = 0; return rv;}
    inline PyObj acquire() const { return PyObj(obj); }
    inline void incref() const { Py_INCREF(obj);}
    inline void release() { Py_XDECREF(obj); obj = 0; }
    ~PyObj() {
        Py_XDECREF(obj);
    }
};

inline std::ostream & operator<< (std::ostream &out, PyObj const &t) {
    out << "PyObj[";
    if (t.obj == 0) {
        out << "NULL";
    } else {
        PyObj repr = PyObj::fromCall(PyObject_Repr(t.obj));
        Py_ssize_t size;
        const char * dest = PyUnicode_AsUTF8AndSize(repr.obj, &size);
        out << std::string((char *)dest, size);
    }
    out << "]";
    return out;
}

}

namespace std{
    template<> struct hash<ss::PyObj>{
        inline std::size_t operator()(const ss::PyObj& val) const {
            return std::hash<long>()(val.obj ? PyObject_Hash(val.obj) : 0);
        }
    };
}