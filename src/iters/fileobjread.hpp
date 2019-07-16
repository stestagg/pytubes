#pragma once

#include "../iter.hpp"

namespace ss{ namespace iter{


class ReadFileObjIter : public Iter {
    /*<-
        Iter:
            ReadFileObjIter: [Chain, AnyIter, size_t]
        Tube:
            ReadFileObj:
                props: [{type: Tube, name: parent, dtypes: [Object]}, {type: size_t, name: size, default: 8_388_608}]
                dtype: return (ByteSlice, )
                chains: ((self.parent, ), )
                iter: [ReadFileObjIter, ["iters_to_c_chain(chains[0])", parent.iter, self.size]]
    ->*/

    const PyObj *file_obj;
    const PyObj buffer_size;
    const PyObj read_str;
    bool file_finished;
    Chain chain;
    ByteSlice buffer_slice;
    SlotPointer slot;

public:
    ReadFileObjIter(Chain chain, AnyIter parent, size_t buffer_size)
        : file_obj(parent->get_slots()[0]),
          buffer_size(PyLong_FromLong(buffer_size)),
          read_str(PyUnicode_FromString("read")),
          file_finished(true),
          chain(chain),
          buffer_slice(),
          slot(&buffer_slice)
        {}

    Slice<SlotPointer> get_slots(){
        return Slice<SlotPointer>(&slot, 1);
    }

    void next() {
        if (file_finished) {
            do_next(chain);
            throw_if(
                ValueError,
                !file_obj->has_attr("read"),
                "read_fileobj only accepts objects with a .read() method"
            )
            file_finished = false;
        }

        PyObj buf = PyObject_CallMethodObjArgs(file_obj->obj, read_str.obj, buffer_size.obj, NULL);
        throw_if(ValueError, !PyBytes_Check(buf.obj), "read_fileobj expects binary data");
        size_t num_bytes = PyBytes_GET_SIZE(buf.obj);
        if (num_bytes == 0) { file_finished = true; return next(); }
        buffer_slice = ByteSlice(PyBytes_AsString(buf.obj), num_bytes);
    }

};

}}