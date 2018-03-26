 #pragma once

#include "../util/pyobj.hpp"
#include "../iter.hpp"

#include "../util/map.hpp"
#include "../convert.hpp"


namespace ss{ namespace iter{

    template<class T>
    class EnumIter : public Iter {
    /*<-
        Fn:
            - "Iter *enum_from_iter(AnyIter, string) except +"
        EnumIter: 
            template: T
            init: [AnyIter, string]
        Tube:
            Enum:
                props: [Tube parent, {'type': 'bytes', 'name': 'codec', 'default': 'b"utf-8"'}]
                dtype: return (Object, )
                custom_iter: |
                    cdef Iter *iter = enum_from_iter(parent.iter, self.codec)
    ->*/

        const T* from;
        Converter<T, PyObj> converter;
        PyObj *convert_slot;
        PyObj cur_val;
        SlotPointer slot;
        HashTable<T, PyObj, void> enum_values;
        
    public:

        EnumIter(AnyIter parent, const std::string &codec) 
            : from(parent->get_slots()[0]), 
              converter(parent->get_slots()[0], codec),
              convert_slot(converter.to),
              slot(&cur_val),
              enum_values()
            {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next() {
            auto &match = enum_values[*from];
            if (!match.obj) {
                converter.convert();
                match.obj = convert_slot->give();
            }
            cur_val = match;
        }
    };

    template<class T, class Enable=bool>
    struct enum_iter_op{
        inline Iter *operator()(const AnyIter parent, const std::string &codec) {
            throw_py<ValueError>("Cannot treat ", ScalarType_t<T>::type_name(), " as enum");
        } 
    };

    template<class T>
    struct enum_iter_op<T, decltype(std::declval<const T>() == std::declval<const T>())>{
        inline Iter *operator()(const AnyIter parent, const std::string &codec) {
            return new EnumIter<T>(parent, codec);
        }
    };

    Iter *enum_from_iter(const AnyIter parent, const std::string codec) {
        return dispatch_type<enum_iter_op>(parent->get_slots()[0].type, parent, codec);
    }


}}