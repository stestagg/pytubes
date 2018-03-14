#pragma once

namespace ss{ namespace iter{

    struct FieldFiller {
        size_t row_offset;

        virtual ~FieldFiller() = default;

        void fill_row(uint8_t *row_start){
            uint8_t *cell_start = row_start + row_offset;
            fill_cell((void*)cell_start);
        }

        virtual void fill_cell(void *cell) = 0;
    };

    template<class T>
    struct StringFiller : FieldFiller {
        const T *slot;
        size_t len;

        StringFiller(SlotPointer slot, size_t offset, size_t len) {            
            this->slot = slot;
            this->row_offset = offset;
            this->len = len - 1; // Need 1 char for null terminator
        }

        void fill_cell(void *cell){
            size_t fill_len = slot->len > len ? len : slot->len;
            memcpy(cell, (void*)slot->start, fill_len);
            ((char *)cell)[fill_len] = '\0';
        }
    };

    template<class T>
    struct ScalarFiller : FieldFiller {
        const T *slot;

        ScalarFiller(SlotPointer slot, size_t offset) : slot(slot)
        { 
            row_offset = offset;
        }

        void fill_cell(void *cell) {
            *(T *)cell = *slot;
        }
    };

    class NDArrayFiller {
    
    public:
        std::vector<std::unique_ptr<FieldFiller>> fillers;

        NDArrayFiller() : fillers() {}

        FieldFiller *_make_filler(SlotPointer slot, PyArray_Descr *dtype, size_t offset) {
            switch(dtype->type_num) {
                case NPY_STRING:
                    return new StringFiller<ByteSlice>(slot, offset, dtype->elsize);
                case NPY_INT64:
                    return new ScalarFiller<int64_t>(slot, offset);
                case NPY_DOUBLE:
                    return new ScalarFiller<double>(slot, offset);
                default:
                    throw_py<ValueError>("Unknown dtype: ", dtype->type_num);
            }
        }

        void add_field(SlotPointer slot, PyArray_Descr *dtype, size_t offset) {
            FieldFiller *field_filler = _make_filler(slot, dtype, offset);
            fillers.emplace_back(field_filler);
        }

        inline void fill_row(PyArrayObject *array, size_t row_num) {
            uint8_t *row_ptr = (uint8_t *)PyArray_GETPTR1(array, row_num);
            for (auto &field_filler : fillers) {
                field_filler->fill_row((uint8_t *)row_ptr);
            }
        }


    };

}}