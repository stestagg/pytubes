#pragma once

#include "../convert.hpp"
#include "../iter.hpp"

namespace ss{ namespace iter{

    namespace cmp {

        const char *op_name(int op) {
            switch(op){
                case Py_LT: return "<";
                case Py_LE: return "<=";
                case Py_EQ: return "==";
                case Py_NE: return "!=";
                case Py_GT: return ">";
                case Py_GE: return ">=";
                default: return "unknown";
            }
        }

        template<class T, int Op, class U=bool> 
        struct Cmp{
            static inline bool cmp(const T &a, const T &b){ 
                throw_py<ValueError>("Cannot '", op_name(Op), "' compare ", ScalarType_t<T>::type_name());
                return false;
            }
        };

        template<class T> 
        struct Cmp<T, Py_EQ, decltype(std::declval<T>() == std::declval<T>())>{static inline bool cmp(const T &a, const T &b){  
            return a == b; 
        }};

        template<class T> 
        struct Cmp<T, Py_GT, decltype(std::declval<T>() > std::declval<T>())>{static inline bool cmp(const T &a, const T &b){  
            return a > b; 
        }};

        template<class T> 
        struct Cmp<T, Py_LT, decltype(std::declval<T>() < std::declval<T>())>{static inline bool cmp(const T &a, const T &b){  
            return a < b; 
        }};

        template<int Op>
        struct Cmp<PyObj, Op, bool>{
            static inline bool cmp(const PyObj &a, const PyObj &b) {
                int result = PyObject_RichCompareBool(a.obj, b.obj, Op);
                if (result == -1) { throw PyExceptionRaised; }
                return result;
            }
        };

    }

    template<class T, class Cmp>
    class CompareIter : public Iter {
        /*<-
        Fn:
            - "Iter *compare_iter_from_cmp_dtype(AnyIter, int, PyObj&) except +"
        Iter: 
            CompareIter: {
                template: [T, Cmp],
                init: [AnyIter, T]
            }
        Tube:
            Compare:
                props: [Tube parent, int op, object value]
                dtype: return (Bool, )
                custom_iter: |
                    cdef PyObj value_ob = PyObj(<PyObject*>self.value)
                    cdef Iter *iter = compare_iter_from_cmp_dtype(parent.iter, self.op, value_ob)
        ->*/
        
        const T *parent;
        PyObj value;
        Converter<PyObj, T> converter;
        bool result;
        SlotPointer slot;

    public:
        CompareIter(AnyIter parent, PyObj &value)
            : parent(parent->get_slots()[0]),
              value(value),
              converter(&this->value, std::string("utf-8")),
              slot(&this->result)
            {
                converter.convert();
            }

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next(){
            result = Cmp::cmp(*parent, *converter.to);
        }

    };

    template<class T, class Enable>
    struct compare_iter_op{

        inline Iter *operator()(AnyIter parent, int op, PyObj &value) {
            switch(op) {
                case Py_LT: return new CompareIter<T, cmp::Cmp<T, Py_LT, bool> >(parent, value);
                case Py_LE: return new CompareIter<T, cmp::Cmp<T, Py_LE, bool> >(parent, value);
                case Py_EQ: return new CompareIter<T, cmp::Cmp<T, Py_EQ, bool> >(parent, value);
                case Py_NE: return new CompareIter<T, cmp::Cmp<T, Py_NE, bool> >(parent, value);
                case Py_GT: return new CompareIter<T, cmp::Cmp<T, Py_GT, bool> >(parent, value);
                case Py_GE: return new CompareIter<T, cmp::Cmp<T, Py_GE, bool> >(parent, value); 
                default: throw_py<ValueError>("Unknown comparison type");
            }
        } 
    };

    Iter *compare_iter_from_cmp_dtype(AnyIter parent, int op, PyObj &value) {
        return dispatch_type<compare_iter_op>(parent->get_slots()[0].type, parent, op, value);
    }

}}