#pragma once

#include <memory>

#include "scalar.hpp"

namespace ss{ namespace iter{

    struct SlotPointer{
        ScalarType type;
        const void *ptr;

        SlotPointer() : type(ScalarType::Null), ptr(&null) {}
        SlotPointer(const SlotPointer &other) : type(other.type), ptr(other.ptr) {}
        SlotPointer(ScalarType type, const void *ptr) : type(type), ptr(ptr) {}

        template<class T> SlotPointer(ScalarType type, const T *ptr) : type(type), ptr((const void *)ptr) {
            using T_t = ScalarType_t<T>;
            throw_if(ValueError, T_t::scalar_type != type,
                "Tried to create ",
                type_name(type),
                " slot pointer with ",
                T_t::type_name(),
                " pointer type");
        }

        template<class T> SlotPointer(const T *ptr) :
            type(ScalarType_t<T>::scalar_type),
            ptr((const void *)ptr) { }

        template<class T> inline operator const T*() const {
            using T_t = ScalarType_t<T>;
            throw_if(ValueError, T_t::scalar_type != type,
                "Tried to dereference ",
                type_name(type),
                " slot pointer as ",
                T_t::type_name(),
                " pointer type");
            return (const T*)ptr;
        }

        inline operator const void*() const {
            return ptr;
        }
    };

    struct StoredSlot{
        SlotPointer ptr;
        StoredSlot(SlotPointer ptr) : ptr(ptr) {}
        virtual ~StoredSlot() = default;

        virtual void update(SlotPointer) = 0;
        virtual void update(const void *ptr) = 0;
    };

    template<class T>
    struct TypedStoredSlot : StoredSlot {
        T value;
        TypedStoredSlot() : StoredSlot(&value) {}
        inline void update(SlotPointer ptr) { value = *(const T*)ptr; }
        inline void update(const void *ptr) { value = *(T*)ptr; }
    };

    template<class T, class Enable>
    struct make_typed_slot_op{
        inline std::unique_ptr<StoredSlot> operator()() {
            return std::unique_ptr<StoredSlot>(new TypedStoredSlot<T>());
        }
    };

    inline std::unique_ptr<StoredSlot> make_stored_slot(ScalarType type) {
        return dispatch_type<make_typed_slot_op>(type);
    }

    template<class T>
    Array<SlotPointer> make_slots_from_array(const Array<T> &values) {
        Array<SlotPointer> slots(values.size);
        for (size_t index=0; index < values.size; ++index) {
            slots[index] = &values[index];
        }
        return slots;
    }


}}