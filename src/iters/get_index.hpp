#pragma once

#include "../util/default.hpp"
#include "../iter.hpp"

namespace ss{ namespace iter{

    template<class T>
    SkipList<T> _make_skip_list(const std::vector<size_t> &indexes, const std::vector<size_t> &skips, const Array<T> &slots){
        SkipList<T> skip_list;
        throw_if(ValueError, indexes.size() != skips.size(), "Inconsistent number of indeces and skips values");
        size_t index = 0;
        for (auto &slot_index : indexes) {
            auto skip = skips[index];
            ++ index;
            skip_list.emplace_back(skip, &slots[slot_index]);
        }
        return skip_list;
    }

    template<class T>
    class SingleIndexLookupIter : public Iter {
        using ValueType = typename field_type_t<T>::type;
        const T *parent;
        size_t index;
        ValueType value;
        SlotPointer slot;

    public:
        SingleIndexLookupIter(AnyIter parent, size_t index) 
        : parent(parent->get_slots()[0]),
          index(index),
          slot(&value) {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next();
    };

    template<> void SingleIndexLookupIter<JsonUtf8>::next() {
        using Parser = json::parse::OptimisticParser<uint8_t>;
        value = JsonUtf8();
        if (parent->type == json::Type::Array) {
            size_t left = index;
            for (auto item : Parser::parse_array(*parent)){
                if (left == 0) {
                    value = item;
                    return;
                }
                left -= 1;
            }
        }
    }

    template<> void SingleIndexLookupIter<TsvRow>::next() {
        size_t left = index;
        for (auto item : *parent){
            if (left == 0) {
                    value = item;
                    return;
                }
            left -= 1;
        }
        value = ByteSlice::Null();
    }

    template<class T>
    class IndexLookupIter : public Iter {
        /*<-
        Fn:
            - "Iter *index_lookup_from_dtype(AnyIter, vector[size_t] &, vector[size_t] &) except +"
        Iter:
            IndexLookupIter: 
                template: T
                init: [AnyIter, "vector[size_t]"]
        Tube:
            IndexLookup:
                props: [Tube parent, list indexes, {type: dict, name: _index_lookups, default: None, print: False}]
                dtype: |
                    cdef DType dt = self.parent.dtype[0]
                    return (c_dtype_to_dtype(field_dtype_from_dtype(dt.type)), ) * len(self.indexes)
                custom_iter: |
                    cdef vector[size_t] indexes
                    cdef vector[size_t] skips
                    cdef size_t slot_index
                    cdef size_t row_index
                    by_row_index = [(r_i, s_i) for (s_i, r_i) in enumerate(self.indexes)]
                    cdef size_t last_row_index = 0
                    for row_index, slot_index in sorted(by_row_index):
                        indexes.push_back(slot_index)
                        skips.push_back(row_index - last_row_index)
                        last_row_index = row_index
                    cdef Iter *iter = index_lookup_from_dtype(parent.iter, indexes, skips)
                methods: >
                    cdef lookup_index(self, size_t val_index, default):
                        try:
                            index = self.indexes.index(val_index)
                        except ValueError:
                            self.indexes.append(val_index)
                            index = len(self.indexes) - 1
                        return self.get_slot_shared(index, default)

                    cdef get_slot_shared(self, size_t index, default):
                        cdef Tube tube = SlotGet(self, index, default)
                        if self._index_lookups is None:
                            self._index_lookups = {}
                        if index not in self._index_lookups:
                            shared_index_get = SlotGet(self, index, b"")
                            self._index_lookups[index] = IndexLookup(shared_index_get, [])
                        tube._set_index_lookup(self._index_lookups[index])
                        return tube
                        
        ->*/
        using ValueType = typename field_type_t<T>::type;

        const T *parent;
        const Array<ValueType> values;
        const Array<SlotPointer> slots;

        SkipList<ValueType> fields;

    public:
        IndexLookupIter(AnyIter parent, std::vector<size_t> &indexes, std::vector<size_t> &skips) 
            : parent(parent->get_slots()[0]),
              values(indexes.size()),
              slots(make_slots_from_array(this->values)),
              fields(_make_skip_list<ValueType>(indexes, skips, values))
        {}

        Slice<SlotPointer> get_slots(){ return slots.slice(); }

        void next();
    };

    template<> void IndexLookupIter<JsonUtf8>::next() {
        using Parser = json::parse::OptimisticParser<uint8_t>;
        for(size_t index=0; index < slots.size; ++index) {
            values[index] = JsonUtf8();
        }
        if (parent->type == json::Type::Array) {
            auto field_iter = Parser::parse_array(*parent);
            auto cur_field = field_iter.begin();
            for (auto &field : fields) {
                auto to_skip = field.skip;
                while(to_skip--){
                    ++cur_field;
                    if (cur_field == field_iter.end()) {
                        return;
                    }
                }
                *(field.destination) = *cur_field;
            }
        }
    }

    template<> void IndexLookupIter<TsvRow>::next() {
        for(size_t index=0; index < slots.size; ++index) {
            values[index] = ByteSlice::Null();
        }
        parent->populate_slots(fields);
    }

    template<class T>
    struct index_lookup_iter_op{
        inline Iter *operator()(AnyIter parent, std::vector<size_t> &indexes, std::vector<size_t> &skips) {
            throw_py<ValueError>(
                "Field lookup has not been implemented on iterators of type ",
                ScalarType_t<T>::type_name()
                );
            return NULL;
        } 
    };

    template<> 
    inline Iter *index_lookup_iter_op<JsonUtf8>::operator()(AnyIter parent, std::vector<size_t> &indexes, std::vector<size_t> &skips) {
        return new IndexLookupIter<JsonUtf8>(parent, indexes, skips);
    }
    template<> 
    inline Iter *index_lookup_iter_op<TsvRow>::operator()(AnyIter parent, std::vector<size_t> &indexes, std::vector<size_t> &skips) {
        return new IndexLookupIter<TsvRow>(parent, indexes, skips);
    }

    template<class T>
    struct single_index_lookup_iter_op{
        inline Iter *operator()(AnyIter parent, size_t index) {
            throw_py<ValueError>(
                "Field lookup has not been implemented on iterators of type ",
                ScalarType_t<T>::type_name()
                );
            return NULL;
        } 
    };

    template<> inline Iter *single_index_lookup_iter_op<JsonUtf8>::operator() (AnyIter parent, size_t index) {
        return new SingleIndexLookupIter<JsonUtf8>(parent, index);
    }
    template<> inline Iter *single_index_lookup_iter_op<TsvRow>::operator() (AnyIter parent, size_t index) {
        return new SingleIndexLookupIter<TsvRow>(parent, index);
    }

    Iter *index_lookup_from_dtype(AnyIter parent, std::vector<size_t> &indexes, std::vector<size_t> &skips) {
        auto slots = parent->get_slots();
        if (skips.size() == 1) {
            return dispatch_type<single_index_lookup_iter_op>(slots[0].type, parent, skips[0]);
        }
        return dispatch_type<index_lookup_iter_op>(slots[0].type, parent, indexes, skips);
    }

}}