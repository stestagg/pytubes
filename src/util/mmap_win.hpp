#pragma once

#include <cstdio>
#include <sys/stat.h>
#include <fcntl.h>
#include <ios>
#include <cstring>
#include <cerrno>

#include <windows.h>

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
    HANDLE file_mapping = NULL;
public:
    const uint8_t *map;
    size_t size;

    HANDLE get_handle() {
        return (HANDLE)_get_osfhandle(file.descriptor());
    }

public:
    
    Mmap(): file() {}
    Mmap(const char* filename): file(filename) {
        size = file.size;
        if (size == 0) {
            map = (const uint8_t *)"";
            return;
        }
        file_mapping = CreateFileMapping(get_handle(), NULL, PAGE_READONLY, 0, 0, NULL);
        throw_if(IOError, file_mapping == NULL, "Error mapping file");
        map = static_cast<const uint8_t*>(MapViewOfFile(
            file_mapping, FILE_MAP_READ, 0, 0, 0));
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
        if (size) {
            UnmapViewOfFile((LPCVOID)map);
            CloseHandle(file_mapping);
        }
        size = 0;
    }

    ByteSlice slice() const {
        return ByteSlice(map, size);
    }

};

}}