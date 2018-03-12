#pragma once

#include "Python.h"

#include <iostream>
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

    inline PyObj(PyObj&& o) noexcept : obj(o.obj) { o.obj = 0; }
    inline PyObj& operator=(const PyObj& other) { Py_XDECREF(obj); obj = other.obj; Py_INCREF(obj); return *this;}
    inline PyObj& operator=(PyObj&& other) { Py_XDECREF(obj); obj = other.obj; other.obj=0; return *this;}

    // give() is used to move the owned pointer to the receiver without calling DECREF
    inline PyObject *give() { auto rv = obj; obj = 0; return rv;}
    inline PyObj acquire() const { return PyObj(obj); }
    inline void incref() const { Py_INCREF(obj);}
    ~PyObj() { Py_XDECREF(obj); }
};

}


namespace std{
    template<> struct hash<ss::PyObj>{
        inline std::size_t operator()(const ss::PyObj& val) const {
            return std::hash<void*>()(val.obj);
        }
    };
}