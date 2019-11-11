#pragma once

// TODO: this feels wrong...
#undef __PYX_HAVE__tubes
#undef __PYX_HAVE_API__tubes
//extern "C" ss::PyObj pyarrow_make_double_array(size_t, std::shared_ptr<arrow::Buffer> );
#include "../../pyx/tubes.h"

#include "error.hpp"

#include <arrow/array/builder_primitive.h>
#include <arrow/array/builder_binary.h>

namespace ss{ namespace iter{

    template<class T> struct PaType_t {  };
    template<> struct PaType_t<NullType> {
        using Enabled = true_type;
        using PaType = arrow::NullType;
        using PaBuilder = arrow::NullBuilder;
        static constexpr const char *PaTypeName = "null";
    };
    template<> struct PaType_t<bool> {
        using Enabled = true_type;
        using PaType = arrow::BooleanType;
        using PaBuilder = arrow::BooleanBuilder;
        static constexpr const char *PaTypeName = "bool_";
    };
    template<> struct PaType_t<int64_t> {
        using Enabled = true_type;
        using PaType = arrow::Int64Type;
        using PaBuilder = arrow::Int64Builder;
        static constexpr const char *PaTypeName = "int64";
    };
    template<> struct PaType_t<double> {
        using Enabled = true_type;
        using PaType = arrow::DoubleType;
        using PaBuilder = arrow::DoubleBuilder;
        static constexpr const char *PaTypeName = "float64";
    };
    template<> struct PaType_t<ByteSlice> {
        using Enabled = true_type;
        using PaType = arrow::BinaryType;
        using PaBuilder = arrow::StringBuilder;
        static constexpr const char *PaTypeName = "binary";
    };
    template<> struct PaType_t<Utf8> {
        using Enabled = true_type;
        using PaType = arrow::StringType;
        using PaBuilder = arrow::StringBuilder;
        static constexpr const char *PaTypeName = "string";
    };
    /*template<> struct PaType_t<PyObj> {
        using PaBuilder = arrow:: NullBuilder;
    };
    template<> struct PaType_t<JsonUtf8> {
        using PaBuilder = arrow:: NullBuilder;
    };
    template<> struct PaType_t<TsvRow> {
        using PaBuilder = arrow:: NullBuilder;
    };
    template<> struct PaType_t<CsvRow> {
        using PaBuilder = arrow:: NullBuilder;
    };*/

    struct PaArrayFiller {
        virtual ~PaArrayFiller() = default;
        virtual void fill() = 0;
        virtual PyObj GetPyobj() = 0;
    };

    template<class T>
    struct PAArrayFillerImpl: PaArrayFiller {
        using PaType = PaType_t<T>;
        using Builder = typename PaType::PaBuilder;
        Builder builder;
        const T *ptr;

        PAArrayFillerImpl(const T * ptr)
            : builder(), ptr(ptr) {}


        void fill();
        PyObj GetPyobj();

    };

    /* fill() */
    template<class T>
    void PAArrayFillerImpl<T>::fill() {
        assert_arrow(builder.Append(*ptr));
    }

    template<>
    void PAArrayFillerImpl<NullType>::fill() {
        assert_arrow(builder.AppendNull());
    }

    template<>
    void PAArrayFillerImpl<ByteSlice>::fill() {
            assert_arrow(builder.Reserve(1));
            assert_arrow(builder.ReserveData(ptr->len));
            builder.UnsafeAppend(ptr->cbegin(), ptr->len);
    }

    template<>
    void PAArrayFillerImpl<Utf8>::fill() {
            assert_arrow(builder.Reserve(1));
            assert_arrow(builder.ReserveData(ptr->len));
            builder.UnsafeAppend(ptr->cbegin(), ptr->len);
    }
    /* End fill() */

    template<class T>
    PyObj PAArrayFillerImpl<T>::GetPyobj() {
        shared_ptr<arrow::ArrayData> array;
        assert_arrow(builder.FinishInternal(&array));
        PyObj rv = PyObj::fromCall(pyarrow_make_simple_array(
            PaType_t<T>::PaTypeName,
            array
        ));
        if (PyErr_Occurred()) { throw PyExceptionRaised;}
        return rv;
    }

    template<>
    PyObj PAArrayFillerImpl<Utf8>::GetPyobj() {
        shared_ptr<arrow::ArrayData> array;
        assert_arrow(builder.FinishInternal(&array));
        PyObj rv = PyObj::fromCall(pyarrow_make_str_array(array));
        if (PyErr_Occurred()) { throw PyExceptionRaised;}
        return rv;
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

    ///
    template<class T, class Enable>
    struct pa_datatype_from_dtype{
        NORETURN(inline std::shared_ptr<arrow::DataType>) operator()() {
            throw_py<ValueError>(
                "Unable to create pa table from ",
                ScalarType_t<T>::type_name()
                );
        }
    };

    template<class T>
    struct pa_datatype_from_dtype<T, enable_if_t<PaType_t<T>::Enabled::value, bool>> {
        inline std::shared_ptr<arrow::DataType> operator()() {
            return std::make_shared<typename PaType_t<T>::PaType>();
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
            arrays.push_back(fillers[index]->GetPyobj());
        }
        return arrays;
    }
}}
