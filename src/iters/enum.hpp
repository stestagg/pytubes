 #pragma once

#include "../util/pyobj.hpp"
#include "../iter.hpp"

#include <unordered_map>


namespace ss{ namespace iter{
    template<typename... Ts> struct make_void { typedef void type;};
    template<typename... Ts> using void_t = typename make_void<Ts...>::type;

    template<class K, class V, class Enable=void>
    class HashTable{
        std::unordered_map<K, V> map;

    public:
        HashTable() : map() {}

        inline V& operator[](const K &key){
            return map[key];
        }
    };

    template<class K, class V> 
    class HashTable<K, V, void_t<typename K::el_t>> {
        using T = typename K::el_t;
        std::unordered_map<Slice<T>, V> map;
        std::vector<std::unique_ptr<T[]>> items;
    public:
        HashTable() : map() {}

        inline K store_key(Slice<T> const &src) {
            auto stored = new T[src.len];
            memcpy((void*)stored, (void*)src.start, src.len);
            size_t index = items.size();
            items.push_back(std::unique_ptr<T[]>(stored));
            return K(&items[index][0], src.len);
        }

        inline V& operator[](const K &key){
            auto match = map.find(key); 
            if (match == map.end()) {
                auto stored_key = store_key(key);
                auto ref = map.emplace(stored_key, PyObj());
                return ref.first->second;
            }
            return match->second;
        }
    };

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

    template<class T, class U=bool>
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

    template <typename T> using enum_iter_spec = enum_iter_op<T, bool>;

    Iter *enum_from_iter(const AnyIter parent, const std::string codec) {
        return dispatch_type<enum_iter_spec>(parent->get_slots()[0].type, parent, codec);
    }


}}