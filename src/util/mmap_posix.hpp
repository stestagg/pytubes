#pragma once

#include <cstdio>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ios>
#include <cstring>
#include <cerrno>

#include "error.hpp"
#include "slice.hpp"

namespace ss { namespace mmap{

class OpenFile{
public:
    FILE * fd;
    size_t size;

    OpenFile() : fd(0), size(0) {}

    OpenFile(const char *filename) {
        fd = fopen(filename, "r");
        throw_if(IOError, !fd, "Could not open file ", filename, ": ", strerror(errno));
        size = _size();
    }

    ~OpenFile() {
        close();
    }
    
    inline bool is_open() {
        return fd;
    }

    void close(){
        if (fd) {
            throw_if(IOError, fclose(fd) == EOF, "Could not close file: ", strerror(errno));
            fd = NULL;
        }
        size = 0;
    }

    inline OpenFile& operator=(OpenFile&& other){
        close();
        fd = other.fd; 
        size = other.size;
        other.fd = 0; 
        other.size = 0;
        return *this;
    }

    int descriptor() {
        throw_if(RuntimeError, !fd, "Tried to get file descriptor of closed file");;
        return fileno(fd);
    }

    size_t _size() {
        throw_if(RuntimeError, !fd, "Tried to get size of closed file");;
        struct stat file_info;
        throw_if(IOError, fstat(descriptor(), &file_info),
            "Could not get size of file: ", strerror(errno));
        return file_info.st_size;
    }

};

class Mmap{
    OpenFile file;
public:
    const uint8_t *map;
    size_t size;

public:
    
    Mmap(): file() {}
    Mmap(const char* filename): file(filename){
        size = file.size;
        auto rv = ::mmap(NULL, size, PROT_READ, MAP_PRIVATE, file.descriptor(), 0);
        throw_if(IOError, rv == MAP_FAILED, "Could not map file: ", strerror(errno));
        map = static_cast<const uint8_t*>(rv);
    }
    Mmap(const Mmap &o) = delete;
    inline Mmap& operator=(Mmap&& other){
        map = other.map;
        size = other.size;
        file = std::move(other.file);
        other.map = 0;
        other.size = 0;
        return *this;
    };
    ~Mmap() {
        munmap((void*)(map), size);
    }

    ByteSlice slice() const {
        return ByteSlice(map, size);
    }

};

}}