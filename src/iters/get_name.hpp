#pragma once

#include <unordered_map>

#include "../util/default.hpp"
#include "../util/array.hpp"
#include "../iter.hpp"

namespace ss{ namespace iter{

    template<class T, class Enable=bool>
    class SingleNameLookupIter : public Iter {
        using ValueType = typename field_type_t<T>::type;
        const T *parent;
        std::string name;
        ByteSlice name_slice;
        ValueType value;
        std::basic_string<uint8_t> key_buffer;
        SlotPointer slot;

    public:
        SingleNameLookupIter(AnyIter parent, std::string &name)
        : parent(parent->get_slots()[0]),
          name(name),
          name_slice((const uint8_t *)this->name.c_str(), this->name.length()),
          slot(&value) {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next();
    };

    template<> void SingleNameLookupIter<JsonUtf8, bool>::next() {
        using Parser = json::parse::OptimisticParser<uint8_t>;
        value = JsonUtf8();
        if (parent->type == json::Type::Object) {
            for (auto item : Parser::parse_object(*parent)){
                auto key = Parser::parse_string(item.first, key_buffer);
                if (key == name_slice) {
                    value = item.second;
                    return;
                }
            }
        }
    }

    template<class T, class Enable=bool> class NameLookupIter;
    /*<-
        Fn:
            - "Iter *name_lookup_from_dtype(AnyIter, vector[string] &) except +"
        Iter:
            NameLookupIter:
                template: T
                init: [AnyIter, "vector[string]"]
        Tube:
            NameLookup:
                props: [Tube parent, list items, {type: dict, name: _name_lookups, default: None, print: False}]
                dtype: |
                    cdef DType dt = self.parent.dtype[0]
                    return (c_dtype_to_dtype(field_dtype_from_dtype(dt.type)), ) * len(self.items)
                custom_iter: |
                    cdef vector[string] fields = [f.encode('utf-8') for f in self.items]
                    cdef Iter *iter = name_lookup_from_dtype(parent.iter, fields)
                methods: >
                    cdef lookup_name(self, str name, default):
                        try:
                            index = self.items.index(name)
                        except ValueError:
                            self.items.append(name)
                            index = len(self.items) - 1
                        return self.get_slot_shared(index, default)

                    cdef get_slot_shared(self, size_t index, default):
                        cdef Tube tube = SlotGet(self, index, default)
                        if self._name_lookups is None:
                            self._name_lookups = {}
                        if index not in self._name_lookups:
                            shared_index_get = SlotGet(self, index, b"")
                            self._name_lookups[index] = NameLookup(shared_index_get, [])
                        tube._set_name_lookup(self._name_lookups[index])
                        return tube

        ->*/

    template<>
    class NameLookupIter<JsonUtf8, bool> : public Iter {
        using Parser = json::parse::OptimisticParser<uint8_t>;

        const JsonUtf8 *parent;
        const Array<std::string> names;
        const Array<JsonUtf8> values;
        const Array<SlotPointer> slots;
        std::basic_string<uint8_t> key_buffer;
        std::unordered_map<ByteSlice, JsonUtf8*> fields;

    public:
        NameLookupIter(AnyIter parent, std::vector<std::string> &names)
            : parent(parent->get_slots()[0]),
              names(names),
              values(names.size()),
              slots(make_slots_from_array(this->values))
        {
            size_t index = 0;
            for (auto &name: this->names) {
                auto name_slice = ByteSlice((uint8_t *)name.c_str(), name.length());
                this->fields[name_slice] = &values[index];
                index += 1;
            }
        }

        Slice<SlotPointer> get_slots(){ return slots.slice(); };

        void next(){
            for(auto &value : values) {
                value = JsonUtf8();
            }
            if (parent->type == json::Type::Object) {
                for (auto item : Parser::parse_object(*parent)){
                    auto key = Parser::parse_string(item.first, key_buffer);
                    auto match = fields.find(key);
                    if (match != fields.end()) {
                        *(match->second) = item.second;
                    }
                }
            }
        }
    };


    template<class X> class NameLookupIter<X, typename std::enable_if<X::IsXsv::value, bool>::type > : public Iter {
        using header_t = XsvHeader<typename X::iterator>;
        const X *parent;
        const Array<std::string> names;
        const Array<ByteSlice> name_slices;
        const Array<ByteSlice> values;

        const Array<SlotPointer> slots;

        // 0x01 is used here as an initial value so that it will always be different
        // from an XSV row header pointer (which can be null)
        // This takes the null header check out of the critical path, arguably
        // saving a few cycles in the hot loop.
        header_t *cur_header = (header_t *)0x01;
        SkipList<ByteSlice> skip_list;
    public:
        NameLookupIter(AnyIter parent, std::vector<std::string> &names):
            parent(parent->get_slots()[0]),
            names(names),
            name_slices(names.size()),
            values(names.size()),
            slots(make_slots_from_array(this->values))
        {
            std::transform(this->names.begin(), this->names.end(), name_slices.begin(), [](std::string &x){ return ByteSlice(x); });
        }

        Slice<SlotPointer> get_slots(){ return slots.slice(); }

        void next() {
            if (cur_header != parent->header) {
                cur_header = parent->header;
                throw_if(ValueError,
                    cur_header == NULL,
                    "Getting ",
                    X::variant_name(),
                    " values by name only supported with a header row"
                );
                skip_list = cur_header->make_skip_list(name_slices, values);
            }
            parent->populate_slots(skip_list);
        }

    };

    template<class T, class Enable>
    struct name_lookup_iter_op{
        inline Iter *operator()(AnyIter parent, std::vector<std::string> &names) {
            throw_py<ValueError>(
                "Field lookup has not been implemented on iterators of type ",
                ScalarType_t<T>::type_name()
                );
            return NULL;
        }
    };

    template<> inline Iter *name_lookup_iter_op<JsonUtf8, bool>::operator() (AnyIter parent, std::vector<std::string> &names) {
        if (names.size() == 1) {
            return new SingleNameLookupIter<JsonUtf8>(parent, names[0]);
        }
        return new NameLookupIter<JsonUtf8>(parent, names);
    }

    template<> inline Iter *name_lookup_iter_op<TsvRow, bool>::operator() (AnyIter parent, std::vector<std::string> &names) {
        return new NameLookupIter<TsvRow>(parent, names);
    }


    Iter *name_lookup_from_dtype(AnyIter parent, std::vector<std::string> &names) {
        auto slots = parent->get_slots();
        return dispatch_type<name_lookup_iter_op>(slots[0].type, parent, names);
    }

}}