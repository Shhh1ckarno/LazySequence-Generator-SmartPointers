#pragma once

#include <string>
#include <functional>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

#include "DynamicArray.h" 
#include "SmartPointer.h" 


class WriteError : public std::runtime_error {
public:
    WriteError(const std::string& msg) : std::runtime_error("WriteError: " + msg) {}
};

template <class T>
using Serializer = std::function<std::string(const T&)>;


template <class T>
class WriteOnlyStream {
private:
    struct IStreamImpl {
        virtual ~IStreamImpl() = default;
        virtual int Write(const T& value) = 0; 
        virtual int GetPosition() const = 0; 
        virtual void Reset() = 0; 
    };

    class ArrayImpl : public IStreamImpl {
        DynamicArray<T>* array_ptr; 
    public:
        ArrayImpl(DynamicArray<T>* arr) : array_ptr(arr) {
            if (!array_ptr) throw std::invalid_argument("WriteOnlyStream: DynamicArray is null");
        }
        
        int Write(const T& value) override {
            array_ptr->Append(value);
            return array_ptr->GetSize(); 
        }
        
        int GetPosition() const override {
            return array_ptr->GetSize();
        }
        
        void Reset() override {
            array_ptr->Resize(0); 
        }
    };

    class IoStreamImpl : public IStreamImpl {
        std::ostream* stream;
        UniquePtr<std::ostream> ownedStream;
        Serializer<T> serializer;
        int pos; 
        
    public:
        IoStreamImpl(std::ostream* strm, Serializer<T> ser) 
            : stream(strm), serializer(ser), pos(0) {
            if (!stream) throw std::invalid_argument("Stream is null");
        }

        int Write(const T& value) override {
            std::string serialized = serializer(value);
            (*stream) << serialized << "\n";
            if (stream->fail()) {
                throw WriteError("Failed to write data to the underlying stream.");
            }
            pos++;
            return pos;
        }

        int GetPosition() const override { return pos; }
        void Reset() override { stream->clear(); pos = 0; }
    };

    UniquePtr<IStreamImpl> impl; 
    bool isOpen;

public:

    explicit WriteOnlyStream(DynamicArray<T>* arr) 
        : impl(new ArrayImpl(arr)), isOpen(true) {}

    WriteOnlyStream(std::ostream* stream, Serializer<T> ser)
        : impl(new IoStreamImpl(stream, ser)), isOpen(true) {}


    int Write(const T& value) {
        if (!isOpen) throw std::logic_error("Stream is closed");
        return impl->Write(value);
    }
};