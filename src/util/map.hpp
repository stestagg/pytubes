#pragma once

#include <unordered_map>

namespace ss{

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

}