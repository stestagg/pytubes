#pragma once

#include <memory>

#include "pyobj.hpp"
#include "slice.hpp"
#include "../scalar.hpp"

namespace ss{ namespace codec{
    static PyObj utf8_encoder = 0;
    static PyObj utf8_decoder = 0;

    PyObj &_get_utf8_encoder() {
        if (utf8_encoder.obj == 0) {
            utf8_encoder.obj = PyCodec_Encoder("utf8");
            utf8_encoder.assert_created();
        }
        return utf8_encoder;
    }

    PyObj &_get_utf8_decoder() {
        if (utf8_decoder.obj == 0) {
            utf8_decoder.obj = PyCodec_Decoder("utf8");
            utf8_decoder.assert_created();
        }
        return utf8_decoder;
    }

    PyObj _get_py_decoder(std::string name) {
        return PyObj::fromCall(PyCodec_Decoder(name.c_str()));
    }

    struct ToUtf8Encoder {
    	iter::Utf8 to;
    	const ByteSlice *from;

    	ToUtf8Encoder(const ByteSlice *from) : from(from) {}
        virtual ~ToUtf8Encoder() = default;
        
        virtual void encode() = 0;
        virtual bool is_transparent() = 0;
    };

    struct NullEncoder : ToUtf8Encoder {
        NullEncoder(const ByteSlice *from) : ToUtf8Encoder(from) {}
        void encode() { return; }
        bool is_transparent() { return true; }
    };

    struct Utf8ToUtf8Encoder : ToUtf8Encoder {
    	
    	Utf8ToUtf8Encoder(const ByteSlice *from) : ToUtf8Encoder(from) {
            to.start = from->start;
            to.len = from->len;
        }
    	bool is_transparent() { return true; }
    	void encode() { return ;}

    };

    struct CodecToUtf8Encoder : ToUtf8Encoder {
        PyObj decoder;
        PyObj src_bytes;
        PyObj uni_str;

        CodecToUtf8Encoder(PyObj decoder, const ByteSlice *from) 
            : ToUtf8Encoder(from), decoder(decoder)
            {
                src_bytes.obj = PyByteArray_FromStringAndSize(Slice<char>::Null().start, 0);
                src_bytes.assert_created();
            }

        bool is_transparent() { return false; }
        void encode() {
            uni_str.release();
            const size_t src_len = from->len;
            PyByteArray_Resize(src_bytes.obj, src_len);
            char * src_dest = PyByteArray_AS_STRING(src_bytes.obj);
            memcpy(src_dest, from->start, src_len);
            PyObj args = PyObj::fromCall(PyTuple_Pack(1, src_bytes.obj));
            PyObj result = PyObj::fromCall(PyObject_CallObject(decoder.obj, args.obj));
            uni_str = PyObj::fromCall(PyTuple_GetItem(result.obj, 0), false);
            Py_ssize_t dest_len = 0;
            const char * dest = PyUnicode_AsUTF8AndSize(uni_str.obj, &dest_len);
            if (!dest) throw PyExceptionRaised;
            to.start = (const unsigned char*)dest;
            to.len = dest_len;
        }

    };

    std::unique_ptr<ToUtf8Encoder> get_encoder(const std::string &codec, const ByteSlice *from) {
        PyObj py_decoder = _get_py_decoder(codec);
        if (from == NULL) { return std::unique_ptr<ToUtf8Encoder>(new NullEncoder(from));}
        if (py_decoder.obj == _get_utf8_decoder().obj) {
            return std::unique_ptr<ToUtf8Encoder>(new Utf8ToUtf8Encoder(from));
        }
        return std::unique_ptr<ToUtf8Encoder>( new CodecToUtf8Encoder(py_decoder, from));
    }

}}
