#pragma once

#include "../util/default.hpp"
#include "../iter.hpp"

namespace ss{ namespace iter{

    template<class T, class Enable=bool>
    struct index_get_impl{
        using Enabled = false_type;
        inline void read(const T &parent);
    };

    template<> struct index_get_impl<JsonUtf8, bool> {
        using Enabled = true_type;
        using Parser = json::parse::FailsafeParser<uint8_t>;

        SkipList<JsonUtf8> skips;

        index_get_impl(SkipList<JsonUtf8> skips) : skips(skips) {}

        inline void read(const JsonUtf8 &parent) {
            if (parent.type == json::Type::Array) {
                auto field_iter = Parser::parse_array(parent);
                auto cur_field = field_iter.begin();
                for (auto &field : skips) {
                    auto to_skip = field.skip;
                    while(to_skip--){
                        ++cur_field;
                        if (cur_field == field_iter.end()) {
                            return;
                        }
                    }
                    *(field.destination) = *cur_field;
                    ++cur_field;
                }
            }
        }
    };

    template<class T> struct index_get_impl<T, enable_if_t<T::IsXsv::value, bool> > {
        using Enabled = true_type;
        SkipList<ByteSlice> skips;
        Array<ByteString> buffers;

        index_get_impl(SkipList<ByteSlice> skips) : skips(skips), buffers(skips.size()) {}

        inline void read(const T &parent) {
            parent.populate_slots(skips, buffers);
        }
    };

    template<> struct index_get_impl<PyObj, bool> {
        using Enabled = true_type;

        SkipList<PyObj> skips;
        SkipList<PyObj> indexes;

        index_get_impl(SkipList<PyObj> skips) : skips(skips) {
            // Reconstruct the indexes from skiplist
            size_t cur = 0;
            indexes.reserve(skips.size());
            for (auto const &skip: skips) {
                cur += skip.skip;
                indexes.push_back(SkipListItem<PyObj>(cur, skip.destination));
                cur += 1;
            }
        }

        inline void read_nosequence(const PyObj& obj) {
            for (auto &index : indexes) {
                *(index.destination) = (index.skip == 0) ? obj : UNDEFINED;
            }
        }

        inline void read_sequence(const PyObj& obj) {
            Py_ssize_t  len = PySequence_Fast_GET_SIZE(obj.obj);
            for (auto &index : indexes) {
                if (index.skip < (size_t)len) {
                    (*index.destination) = PyObj(PySequence_Fast_GET_ITEM(obj.obj, index.skip));
                } else {
                    (*index.destination) = UNDEFINED;
                }
            }
        }

        inline void read(const PyObj &parent) {
            PyObj value = PyObj(PySequence_Fast(parent.obj, "Ignore"), true);
            if (!value.was_created()) {
                PyErr_Clear();
                read_nosequence(parent);
            } else {
                read_sequence(value);
            }
        }
    };

    template<class T>
    SkipList<T> _make_skip_list(const std::vector<size_t> &indexes, const std::vector<size_t> &skips, const Array<T> &slots){
        SkipList<T> skip_list;
        throw_if(ValueError, indexes.size() != skips.size(), "Inconsistent number of indices and skips values");
        size_t index = 0;
        for (auto &slot_index : indexes) {
            auto skip = skips[index];
            ++ index;
            skip_list.emplace_back(skip, &slots[slot_index]);
        }
        return skip_list;
    }

    template<class T>
    class IndexLookupIter : public Iter {
        /*<-
        Fn:
            - "Iter *index_lookup_from_dtype(AnyIter, vector[size_t] &, vector[size_t] &) except +"
        Tube:
            IndexLookup:
                props: [Tube parent, list indexes, {type: dict, name: _index_lookups, default: None, print: False}]
                unnamed_props: [indexes]
                dtype: |
                    cdef DType dt = self.parent.dtype[0]
                    return (c_dtype_to_dtype(field_dtype_from_dtype(dt.type)), ) * len(self.indexes)
                custom_iter: |
                    cdef vector[size_t] indexes
                    cdef vector[size_t] skips
                    cdef size_t slot_index
                    cdef size_t row_index
                    cdef size_t last_row_index
                    by_row_index = [(r_i, s_i) for (s_i, r_i) in enumerate(self.indexes)]
                    if by_row_index:
                        by_row_index = sorted(by_row_index)
                        first_row_index, first_slot_index = by_row_index[0]
                        indexes.push_back(first_slot_index)
                        skips.push_back(first_row_index)
                        last_row_index = first_row_index
                        for row_index, slot_index in by_row_index[1:]:
                            indexes.push_back(slot_index)
                            skips.push_back(row_index - last_row_index - 1)
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

        index_get_impl<T> impl;

    public:
        IndexLookupIter(AnyIter parent, std::vector<size_t> &indexes, std::vector<size_t> &skips)
            : parent(parent->get_slots()[0]),
              values(indexes.size()),
              slots(make_slots_from_array(this->values)),
              impl(_make_skip_list<ValueType>(indexes, skips, values))
        {}

        Slice<SlotPointer> get_slots(){ return slots.slice(); }

        void next(){
            for(size_t index=0; index < slots.size; ++index) {
                values[index] = ValueType();
            }
            impl.read(*parent);
        }
    };

    template<class T, class Enable>
    struct index_lookup_iter_op{
        inline Iter *operator()(AnyIter parent, std::vector<size_t> &indexes, std::vector<size_t> &skips) {
            throw_py<ValueError>(
                "Index-based lookup has not been implemented on iterators of type ",
                ScalarType_t<T>::type_name()
                );
            return NULL;
        }
    };

    template<class T>
    struct index_lookup_iter_op<T, enable_if_t<index_get_impl<T>::Enabled::value, bool>> {
        inline Iter *operator()(AnyIter parent, std::vector<size_t> &indexes, std::vector<size_t> &skips){
            return new IndexLookupIter<T>(parent, indexes, skips);
        }
    };

    Iter *index_lookup_from_dtype(AnyIter parent, std::vector<size_t> &indexes, std::vector<size_t> &skips) {
        auto slots = parent->get_slots();
        return dispatch_type<index_lookup_iter_op>(slots[0].type, parent, indexes, skips);
    }

}}