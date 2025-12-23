#pragma once

#include <string>
#include <functional>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

#include "DynamicArray.h" 
#include "SmartPointer.h" 


class EndOfStream : public std::out_of_range {
public:
    EndOfStream() : std::out_of_range("EndOfStream: attempt to read beyond stream") {}
};


template <class T>
using Deserializer = std::function<T(const std::string&)>;


template <class T>
class ReadOnlyStream {
private:
    struct IStreamImpl {
        virtual ~IStreamImpl() = default;
        virtual T Read() = 0;
        virtual bool IsEOS() const = 0;
        virtual bool CanSeek() const = 0;
        virtual int GetPos() const = 0; 
        virtual void Seek(int index) = 0; 
        virtual void Reset() = 0; 
    };

    class ArrayImpl : public IStreamImpl {
        const DynamicArray<T>* array_ptr; 
        int pos; 
    public:
        ArrayImpl(const DynamicArray<T>* arr) : array_ptr(arr), pos(0) {
            if (!array_ptr) throw std::invalid_argument("ReadOnlyStream: DynamicArray is null");
        }
        T Read() override {
            if (IsEOS()) throw EndOfStream();
            return array_ptr->Get(pos++);
        }
        bool IsEOS() const override {
            return pos >= array_ptr->GetSize(); 
        }
        bool CanSeek() const override { return true; }
        int GetPos() const override { return pos; }
        void Seek(int index) override {
            if (index > array_ptr->GetSize()) {
                throw std::out_of_range("Seek index out of bounds");
            }
            pos = index;
        }
        void Reset() override { pos = 0; }
    };

    class IoStreamImpl : public IStreamImpl {
        std::istream* stream;
        UniquePtr<std::istream> ownedStream; 
        Deserializer<T> deserializer;
        int pos; 
        
    public:
        IoStreamImpl(std::istream* strm, Deserializer<T> deser) 
            : stream(strm), deserializer(deser), pos(0) {
            if (!stream) throw std::invalid_argument("Stream is null");
        }
        
        T Read() override {
            if (IsEOS()) throw EndOfStream();
            std::string line;
            if (!std::getline(*stream, line)) {
                throw EndOfStream();
            }
            pos++;
            return deserializer(line);
        }
        bool IsEOS() const override { return stream->eof(); }
        bool CanSeek() const override { return false; }
        int GetPos() const override { return pos; }
        void Seek(int index) override { throw std::logic_error("Seek not supported for generic Input Streams"); }
        void Reset() override { stream->clear(); pos = 0; }
    };

    UniquePtr<IStreamImpl> impl; 
    bool isOpen;

public:
    explicit ReadOnlyStream(const DynamicArray<T>* arr) 
        : impl(new ArrayImpl(arr)), isOpen(true) {}
    ReadOnlyStream(std::istream* stream, Deserializer<T> deser)
        : impl(new IoStreamImpl(stream, deser)), isOpen(true) {}
    T Read() {
        if (!isOpen) throw std::logic_error("Stream is closed");
        return impl->Read();
    }
    bool IsEndOfStream() const { return !isOpen || impl->IsEOS(); }
};