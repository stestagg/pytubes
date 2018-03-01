#pragma once

#include <unordered_map>

#include "../util/default.hpp"
#include "../iter.hpp"

namespace ss::iter {

    template<class T> bool is_undefined(T *val){ return false; }
    template<> bool is_undefined(const JsonUtf8 *val) { 
        return val->type == json::Type::Unset;
    }

    template<class T>
    class SlotGetIter : public Iter {
        /*<-
        Fn:
            - "Iter *slot_get_iter_from_dtype(AnyIter, size_t, PyObj&) except +"
        Iter: 
            SlotGetIter: [AnyIter, size_t, PyObj]
        Tube:
            SlotGet:
                props: [Tube parent, size_t index, object default_val]
                dtype: return (self.parent.dtype[self.index], )
                init_check: |
                    if index > <size_t>len(parent.dtype):
                        raise IndexError(f"Cannot get slot {index} from parent with {len(parent.dtype)} slots")
                custom_iter: |
                    cdef PyObj default_ob = PyObj(<PyObject*>self.default_val)
                    cdef Iter *iter = slot_get_iter_from_dtype(parent.iter, self.index, default_ob)
        ->*/
        
        const T *parent;
        T value;
        SlotPointer slot;
        DefaultValue<T> default_val;

    public:
        SlotGetIter(AnyIter parent, size_t index, PyObj &default_val)
            : parent(parent->get_slots()[index]),
              slot(&this->value),
              default_val(default_val)
            {}

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slot, 1);
        }

        void next(){
            if (is_undefined(parent)) {
                value = default_val.default_or("Field not found");
            } else {
                value = *parent;
            }
        }

    };

    template<class T>
    struct slot_get_iter_from_dtype_op{
        inline Iter *operator()(AnyIter parent, size_t index, PyObj &default_val) {
            return new SlotGetIter<T>(parent, index, default_val);
        } 
    };


    Iter *slot_get_iter_from_dtype(AnyIter parent, size_t index, PyObj &default_val) {
        return dispatch_type<slot_get_iter_from_dtype_op>(parent->get_slots()[index].type, parent, index, default_val);
    }

    //---------- TODO: split file

    template<class T>
    std::vector<SlotPointer> _make_slots(const std::unique_ptr<T[]> &values, size_t num) {
        std::vector<SlotPointer> slots;
        for (size_t index=0; index < num; ++index) {
            slots.emplace_back(&values[index]);
        }
        return slots;
    }

    template<class T> struct field_type_t {};
    template<> struct field_type_t<JsonUtf8> {using type = JsonUtf8;};

    template<class T>
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

    template<class T>
    class NameLookupIter : public Iter {
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
                dtype: return (self.parent.dtype[0], ) * len(self.items)
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
        using ValueType = typename field_type_t<T>::type;

        const T *parent;
        const std::unique_ptr<std::string[]> names;
        const std::unique_ptr<ValueType[]> values;
        const std::vector<SlotPointer> slots;
        std::basic_string<uint8_t> key_buffer;
        std::unordered_map<ByteSlice, ValueType*> fields;

    public:
        NameLookupIter(AnyIter parent, std::vector<std::string> &names) 
            : parent(parent->get_slots()[0]),
              names(std::make_unique<std::string[]>(names.size())), 
              values(std::make_unique<ValueType[]>(names.size())),
              slots(_make_slots(this->values, names.size()))
        {
            size_t index = 0;
            for (auto name: names) {
                this->names[index] = name;
                auto &name_str = this->names[index];
                auto name_slice = ByteSlice((uint8_t *)name_str.c_str(), name_str.length());
                this->fields[name_slice] = &values[index];
                index += 1;
            }
        }

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(slots.data(), slots.size());
        }

        void next();

    };

    template<> void SingleNameLookupIter<JsonUtf8>::next() {
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

    template<> void NameLookupIter<JsonUtf8>::next() {
        using Parser = json::parse::OptimisticParser<uint8_t>;
        for(size_t index=0; index < slots.size(); ++index) {
            values[index] = JsonUtf8();
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

    template<class T>
    struct name_lookup_iter_op{
        inline Iter *operator()(AnyIter parent, std::vector<std::string> &names) {
            throw_py<ValueError>(
                "Field lookup has not been implemented on iterators of type ",
                ScalarType_t<T>::type_name
                );
            return NULL;
        } 
    };

    template<> inline Iter *name_lookup_iter_op<JsonUtf8>::operator() (AnyIter parent, std::vector<std::string> &names) {
        return new NameLookupIter<JsonUtf8>(parent, names);
    }

    template<class T>
    struct single_name_lookup_iter_op{
        inline Iter *operator()(AnyIter parent, std::string &name) {
            throw_py<ValueError>(
                "Field lookup has not been implemented on iterators of type ",
                ScalarType_t<T>::type_name
                );
            return NULL;
        } 
    };

    template<> inline Iter *single_name_lookup_iter_op<JsonUtf8>::operator() (AnyIter parent, std::string &name) {
        return new SingleNameLookupIter<JsonUtf8>(parent, name);
    }

    Iter *name_lookup_from_dtype(AnyIter parent, std::vector<std::string> &names) {
        auto slots = parent->get_slots();
        if (names.size() == 1) {
            return dispatch_type<single_name_lookup_iter_op>(slots[0].type, parent, names[0]);
        }
        return dispatch_type<name_lookup_iter_op>(slots[0].type, parent, names);
    }

}