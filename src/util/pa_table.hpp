#pragma once

namespace ss{ namespace iter{

    template<class T> struct PaType_t {  };
    template<> struct PaType_t<NullType> {
        using Enabled = true_type;
        using PaType = arrow::NullType;
        using PaBuilder = arrow::NullBuilder;
    };
    template<> struct PaType_t<bool> {
        using Enabled = true_type;
        using PaType = arrow::BooleanType;
        using PaBuilder = arrow::BooleanBuilder;
    };
    template<> struct PaType_t<int64_t> {
        using Enabled = true_type;
        using PaType = arrow::Int64Type;
        using PaBuilder = arrow::Int64Builder;
    };
    template<> struct PaType_t<double> {
        using Enabled = true_type;
        using PaType = arrow::DoubleType;
        using PaBuilder = arrow::DoubleBuilder;
    };
    template<> struct PaType_t<ByteSlice> {
        using Enabled = true_type;
        using PaType = arrow::BinaryType;
        using PaBuilder = arrow::StringBuilder;
    };
    template<> struct PaType_t<Utf8> {
        using Enabled = true_type;
        using PaType = arrow::StringType;
        using PaBuilder = arrow::StringBuilder;
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
        virtual std::shared_ptr<arrow::Array> Finish() = 0;
    };

    template<class T>
    struct PAArrayFillerImpl: PaArrayFiller {
        using Builder = typename PaType_t<T>::PaBuilder;
        Builder builder;
        const T *ptr;

        PAArrayFillerImpl(const T * ptr)
            : builder(), ptr(ptr) {}


        std::shared_ptr<arrow::Array> Finish() {
            std::shared_ptr<arrow::Array> array;
            builder.Finish(&array);
            return array;
        }

        void fill();

    };

    template<class T>
    void PAArrayFillerImpl<T>::fill() {
        builder.Append(*ptr);
    }

    template<>
    void PAArrayFillerImpl<NullType>::fill() {}

    template<>
    void PAArrayFillerImpl<ByteSlice>::fill() {
            builder.Reserve(1);
            builder.ReserveData(ptr->len);
            builder.UnsafeAppend(ptr->cbegin(), ptr->len);
    }

    template<>
    void PAArrayFillerImpl<Utf8>::fill() {
            builder.Reserve(1);
            builder.ReserveData(ptr->len);
            builder.UnsafeAppend(ptr->cbegin(), ptr->len);
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

    std::shared_ptr<arrow::Table> fill_table(
        AnyIter iter,
        std::vector<std::string> field_names,
        Chain &chain
    ) {
        Slice<SlotPointer> slots = iter.get()->get_slots();
        throw_if(ValueError, field_names.size() != slots.len,
            "fields must be exactly the same length as the number of tube slots.  ",
            field_names.size(),
            " field names provided, tube has ",
            slots.len,
            " slots");
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

        std::vector<std::shared_ptr<arrow::Field>> fields;
        std::vector<std::shared_ptr<arrow::Array>> arrays;

        for(size_t index=0; index < slots.len; ++index) {
            fields.push_back(
                arrow::field(field_names[index],
                dispatch_type<pa_datatype_from_dtype>(slots[index].type)
            ));
            arrays.push_back(fillers[index]->Finish());
        }
        auto schema = arrow::schema(fields);
        return arrow::Table::Make(schema, arrays);
    }
}}
