#pragma once

namespace ss{ namespace iter {
	template<class T> bool is_undefined(T *val){ return false; }
	template<> inline bool is_undefined(const JsonUtf8 *val) {
	    return val->type == json::Type::Unset;
	}
	template<class T> inline bool is_undefined(const Slice<T> *val) {
	    return val->is(Slice<T>::Null());
	}

	static PyObj UNDEFINED;

	template<> inline bool is_undefined(const PyObj *val) {
	    return val->obj == UNDEFINED.obj;
	}

	inline void init_undefined() {
	    if (!UNDEFINED.was_created()) {
	        PyObj builtins = PyObj::fromCall(
	            PyImport_ImportModule("builtins")
	        );
	        PyObj object = PyObj::fromCall(
	        	PyObject_GetAttrString(builtins.obj, "object")
	        );
	        UNDEFINED = PyObj::fromCall(
	        	PyObject_CallFunction(object.obj, NULL)
	        );
	        UNDEFINED.assert_created();
	    }

	}
}}