#pragma once

namespace ss{ namespace iter{

    struct FieldFiller {
        size_t row_offset;

        FieldFiller(size_t row_offset) : row_offset(row_offset) {}
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

        StringFiller(SlotPointer slot, size_t offset, size_t len) : FieldFiller(offset) {
            throw_if(ValueError, len == 0, "Zero length strings not supported");
            this->slot = slot;
            this->len = len - 1; // Need 1 char for null terminator
        }

        void fill_cell(void *cell){
            size_t fill_len = slot->len > len ? len : slot->len;
            memcpy(cell, (void*)slot->start, fill_len);
            ((char *)cell)[fill_len] = '\0';
        }
    };

    struct PyObjFiller : FieldFiller {
        const PyObj *slot;

        PyObjFiller(SlotPointer slot, size_t offset) :
            FieldFiller(offset),
            slot(slot)
        {}

        void fill_cell(void *cell) {
            *(PyObject **)cell = slot->obj;
        }
    };

    template<class T>
    struct ScalarFiller : FieldFiller {
        const T *slot;

        ScalarFiller(SlotPointer slot, size_t offset) :
            FieldFiller(offset),
            slot(slot)
        {}

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
                case NPY_BOOL:
                    return new ScalarFiller<bool>(slot, offset);
                case NPY_OBJECT:
                    return new PyObjFiller(slot, offset);
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

    inline void fill_ndarray(PyArrayObject *array, NDArrayFiller *filler, Chain &chain, size_t growth_factor) {
        int ndims = PyArray_NDIM(array);
        throw_if(ValueError, ndims < 1, "Array with zero dimensions");
        PyArray_Dims dims = {
            new npy_intp[ndims],
            ndims
        };
        npy_intp *initial_dims = PyArray_DIMS(array);
        std::copy(initial_dims, initial_dims + ndims, dims.ptr);

        npy_intp cur_index = 0;
        while(true) {
            if (cur_index >= dims.ptr[0]) {
                dims.ptr[0] += growth_factor;
                static_throw_if(PyExceptionRaised, !PyArray_Resize(array, &dims, 1, NPY_CORDER));
            }
            try{
                do_next(chain);
            } catch (StopIterationExc &e) {
                break;
            }
            filler->fill_row(array, cur_index);
            cur_index += 1;
        }
        dims.ptr[0] = cur_index;
        static_throw_if(PyExceptionRaised, !PyArray_Resize(array, &dims, 1, NPY_CORDER));
    }

}}