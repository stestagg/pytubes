#pragma once

#include "../iter.hpp"

namespace ss{ namespace iter{

#define BUFFER_SIZE (8 * 1024 * 1024)

class ReadFileObjIter : public Iter {
    /*<-
        Iter:
            ReadFileObjIter: [Chain, AnyIter]
        Tube:
            ReadFileObj:
                props: [{type: Tube, name: parent, dtypes: [Object]}]
                dtype: return (ByteSlice, )
                chains: ((self.parent, ), )
                iter: [ReadFileObjIter, ["iters_to_c_chain(chains[0])", parent.iter]]
    ->*/

    const PyObj *file_obj;
    const PyObj buffer_size;
    const PyObj read_str;
    bool file_finished;
    Chain chain;
    ByteSlice buffer_slice;
    SlotPointer slot;

public:
    ReadFileObjIter(Chain chain, AnyIter parent)
        : file_obj(parent->get_slots()[0]),
          file_finished(true),
          read_str(PyUnicode_FromString("read")),
          buffer_size(PyLong_FromLong(BUFFER_SIZE)),
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