#pragma once

#include <algorithm>
#include <vector>

#include "../util/pyobj.hpp"
#include "../iter.hpp"
#include "../convert.hpp"

#include "topy.hpp"

namespace ss{ namespace iter{

    template<class T, class Enable> struct make_converter_op{
        template<class F, class Enableb> struct make_converter_inner {
            inline AnyConverter *operator()(SlotPointer from_ptr, const std::string &codec) {
                return new Converter<F, T>(from_ptr, codec);
            }
        };
        inline AnyConverter *operator()(SlotPointer from_ptr, const std::string &codec) {
            return dispatch_type<make_converter_inner>(from_ptr.type, from_ptr, codec);
        }
    };

    AnyConverter *make_converter(SlotPointer from_ptr, ScalarType to_type, const std::string &codec) {
        return dispatch_type<make_converter_op>(to_type, from_ptr, codec);
    }

    std::vector<std::unique_ptr<AnyConverter> > make_converters(Slice<SlotPointer> slots, std::vector<ScalarType> to, const std::string &codec){
        std::vector<std::unique_ptr<AnyConverter>> out;
        throw_if(ValueError, slots.len < to.size(), "Tried to convert ", to.size(), " items but only given ", slots.len, " items");
        for (size_t index=0; index < to.size(); ++ index) {
            const auto &from_ptr = slots[index];
            auto &to_type = to[index];
            out.push_back(std::unique_ptr<AnyConverter>(make_converter(from_ptr, to_type, codec)));
        }
        return out;
    }

    std::vector<SlotPointer> make_slots(const std::vector<std::unique_ptr<AnyConverter> > &converters) {
        std::vector<SlotPointer> out;
        for (auto &converter : converters) {
            out.push_back(converter->get_slot());
        }
        return out;
    }

    void check_can_convert(ScalarType from_type, ScalarType to_type, std::string codec){
        auto from = SlotPointer(from_type, NULL);
        make_converter(from, to_type, codec);
    }

    class ConvertIter : public Iter {
        /*<-
        Fn:
            - void check_can_convert(ScalarType, ScalarType, string) except +
        Iter:
            ConvertIter: [AnyIter, "vector[ScalarType]", string]
        Tube:
            Convert:
                props: [Tube parent, list to_types, {'type': 'bytes', 'name': 'codec', 'default': 'b"utf-8"'}]
                dtype: return tuple(self.to_types)
                iter: [ConvertIter, [parent.iter, self.dtype_vec(), self.codec]]
                init_check: |
                    if len(parent.dtype) < len(self.dtype):
                        raise ValueError("Convert iter cannot have more elements than parent")
                    cdef DType from_dtype
                    cdef DType to_dtype
                    for from_dtype, to_dtype in zip(parent.dtype, self.dtype):
                        check_can_convert(from_dtype.type, to_dtype.type, codec)
                methods: >
                    cdef vector[scalar_type.ScalarType] dtype_vec(self):
                        cdef DType dtype
                        cdef vector[scalar_type.ScalarType] types
                        for dtype in self.to_types:
                            types.push_back(dtype.type)
                        return types
        ->*/

        const size_t num_slots;
        const std::vector<std::unique_ptr<AnyConverter>> converters;
        const std::vector<SlotPointer> slots;

    public:

        Slice<SlotPointer> get_slots(){
            return Slice<SlotPointer>(&slots[0], num_slots);
        }

        ConvertIter(AnyIter parent, std::vector<ScalarType> types, const std::string codec)
            : num_slots(types.size()),
              converters(make_converters(parent->get_slots(), types, codec)),
              slots(make_slots(this->converters))
        {}

        void next() {
            for (auto &converter : converters) {
                converter->convert();
            }
        }

    };

}}