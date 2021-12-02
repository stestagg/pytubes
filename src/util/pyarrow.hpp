#pragma once

#include "../util/arrow/buffer.hpp"
#include "../util/arrow/containers.hpp"

// TODO: this feels wrong...
#undef __PYX_HAVE__tubes
#undef __PYX_HAVE_API__tubes
#include "../../pyx/tubes.h"

#include "error.hpp"


namespace ss{ namespace iter{

    template<class T> struct PaType_t {  };
    template<> struct PaType_t<NullType> {
        using Enabled = true_type;
        using ContainerType = arrow::NullContainer;
        static constexpr const char *PaTypeName = "null";
    };
    template<> struct PaType_t<bool> {
        using Enabled = true_type;
        using ContainerType = arrow::BoolContainer;
        static constexpr const char *PaTypeName = "bool_";
    };
    template<> struct PaType_t<int64_t> {
        using Enabled = true_type;
        using ContainerType = aligned64_vector<int64_t>;
        static constexpr const char *PaTypeName = "int64";
    };
    template<> struct PaType_t<double> {
        using Enabled = true_type;
        using ContainerType = aligned64_vector<double>;
        static constexpr const char *PaTypeName = "float64";
    };
    template<> struct PaType_t<Utf8> {
        using Enabled = true_type;
        using ContainerType = arrow::StringContainer<uint8_t>;
        static constexpr const char *PaTypeName = "string";
    };

    struct PaArrayFiller {
        virtual ~PaArrayFiller() = default;
        virtual void fill() = 0;
        virtual PyObj GetPaArray() = 0;
    };

    template<class T>
    struct PAArrayFillerImpl: PaArrayFiller {
        using PaType = PaType_t<T>;
        using ContainerType = typename PaType::ContainerType;
        ContainerType container;
        const T *ptr;

        PAArrayFillerImpl(const T *ptr) : container(), ptr(ptr) {}

        void fill();
        PyObj GetPaArray();

    };

    /* fill() */
    template<class T>
    void PAArrayFillerImpl<T>::fill() {
        container.emplace_back(*ptr);
    }

    template<>
    void PAArrayFillerImpl<NullType>::fill() {
        container.increment();
    }

    // template<>
    // void PAArrayFillerImpl<ByteSlice>::fill() {
    //         assert_arrow(builder.Reserve(1));
    //         assert_arrow(builder.ReserveData(ptr->len));
    //         builder.UnsafeAppend(ptr->cbegin(), ptr->len);
    // }

    // template<>
    // void PAArrayFillerImpl<Utf8>::fill() {
    //         assert_arrow(builder.Reserve(1));
    //         assert_arrow(builder.ReserveData(ptr->len));
    //         builder.UnsafeAppend(ptr->cbegin(), ptr->len);
    // }
    // /* End fill() */

    template<>
    PyObj PAArrayFillerImpl<NullType>::GetPaArray() {
        PyObj buffers = PyObj::fromCall(PyList_New(1));
        PyList_SET_ITEM(buffers.obj, 0, PyObj(Py_None).give());
        PyObj array = PyObj::fromCall(pyarrow_make_array(
            PaType_t<NullType>::PaTypeName,
            container.size(),
            buffers.obj
        ));
        return array;
    }

    template<>
    PyObj PAArrayFillerImpl<bool>::GetPaArray() {
        size_t size = container.size();
        auto buf = arrow::Buffer<uint8_t>::unique(std::move(container.contents));
        PyObj bridged = PyObj::fromCall(pyarrow_make_buffer(std::move(buf)));
        PyObj buffers = PyObj::fromCall(PyList_New(2));
        PyList_SET_ITEM(buffers.obj, 0, PyObj(Py_None).give());
        PyList_SET_ITEM(buffers.obj, 1, bridged.give());
        PyObj array = PyObj::fromCall(pyarrow_make_array(
            PaType_t<bool>::PaTypeName,
            size,
            buffers.obj
        ));
        return array;
    }

    template<class T>
    PyObj PAArrayFillerImpl<T>::GetPaArray() {
        using Buf = arrow::Buffer<typename ContainerType::value_type>;
        size_t size = container.size();
        std::unique_ptr<Buf> buf(new Buf(std::move(container)));
        PyObj bridged = PyObj::fromCall(
            pyarrow_make_buffer(std::move(buf))
        );
        PyObj buffers = PyObj::fromCall(PyList_New(2));
        PyList_SET_ITEM(buffers.obj, 0, PyObj(Py_None).give());
        PyList_SET_ITEM(buffers.obj, 1, bridged.give());
        PyObj array = PyObj::fromCall(pyarrow_make_array(
            PaType_t<T>::PaTypeName,
            size,
            buffers.obj
        ));
        return array;
    }

    template<>
    PyObj PAArrayFillerImpl<Utf8>::GetPaArray() {
        size_t size = container.size();

        auto contents = arrow::Buffer<uint8_t>::unique(std::move(container.contents));
        auto offsets = arrow::Buffer<int32_t>::unique(std::move(container.offsets));

        return PyObj::fromCall(pyarrow_make_str_array(
            size,
            PyObj::fromCall(pyarrow_make_buffer(std::move(offsets))).obj,
            PyObj::fromCall(pyarrow_make_buffer(std::move(contents))).obj
        ));
    }

    template<class T, class Enable>
    struct make_pa_filler{
        NORETURN(inline std::unique_ptr<PaArrayFiller>) operator()(const SlotPointer ptr) {
            throw_py<ValueError>(
                "Unable to create pa table from ",
                ScalarType_t<T>::type_name()
                );

        }
    };

    template<class T>
    struct make_pa_filler<T, enable_if_t<PaType_t<T>::Enabled::value, bool>> {
        inline std::unique_ptr<PaArrayFiller> operator()(const SlotPointer ptr) {
            return std::unique_ptr<PaArrayFiller>(new PAArrayFillerImpl<T>((const T*)ptr));
        }
    };

    std::vector<PyObj> fill_arrays(
        AnyIter iter,
        Chain &chain
    ) {
        Slice<SlotPointer> slots = iter.get()->get_slots();
        std::vector<std::unique_ptr<PaArrayFiller>> fillers{};
        fillers.reserve(slots.len);

        for(auto &slot : slots) {
            fillers.push_back(dispatch_type<make_pa_filler>(slot.type, slot));
        }

        while(true) {
            try{
                do_next(chain);
            } catch (StopIterationExc &e) {
                break;
            }
            for (auto &filler : fillers) {
                filler->fill();
            }
        }

        std::vector<PyObj> arrays;
        for(size_t index=0; index < slots.len; ++index) {
            arrays.push_back(fillers[index]->GetPaArray());
        }
        return arrays;
    }
}
}
