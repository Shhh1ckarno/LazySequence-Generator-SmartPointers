#pragma once

#include <cstddef>
#include <utility>
#include <new>
#include <typeinfo>
#include <cassert>
#include <iostream>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <optional>
#include <string>

#include "ArraySequence.h"
#include "LazySequence.h"
#include "Cardinal.h"
#include "SmartPointer.h"

template <class T>
class WriteOnlyStream {
public:
    // Конструктор — запись в существующую Sequence (ожидается ArraySequence<T>).
    explicit WriteOnlyStream(Sequence<T>* seq)
        : seqTarget(seq), outStream(nullptr), ownedOutStream(nullptr), serializer(nullptr)
    {
        if (seqTarget) {
            Cardinal len = seqTarget->GetLength();
            if (len.IsFinite()) pos = len.GetValue();
            else pos = 0;
        } else {
            pos = 0;
        }
    }

    // Конструктор — запись в существующий std::ostream (не владеем им).
    WriteOnlyStream(std::ostream* out, std::function<std::string(const T&)> serializer_)
        : seqTarget(nullptr), outStream(out), ownedOutStream(nullptr), serializer(serializer_), pos(0)
    {
        if (!outStream) throw std::invalid_argument("WriteOnlyStream: null out stream");
        if (!serializer) throw std::invalid_argument("WriteOnlyStream: null serializer");
    }

    // Конструктор — записывать в файл (владеем открытым ofstream).
    WriteOnlyStream(const std::string& filename, std::function<std::string(const T&)> serializer_)
        : seqTarget(nullptr), outStream(nullptr),
          ownedOutStream(new std::ofstream(filename, std::ios::out | std::ios::binary)),
          serializer(serializer_), pos(0)
    {
        if (!ownedOutStream || !static_cast<std::ofstream*>(ownedOutStream.get())->is_open())
            throw std::runtime_error("WriteOnlyStream: failed to open file");
        outStream = ownedOutStream.get();
        if (!serializer) throw std::invalid_argument("WriteOnlyStream: null serializer");
    }

    // Запись возвращает позицию следующей записи (т.е. количество записанных элементов после операции)
    size_t Write(const T& v) {
        if (seqTarget) {
            ArraySequence<T>* arr = dynamic_cast< ArraySequence<T>* >(seqTarget);
            if (!arr) throw std::runtime_error("WriteOnlyStream: write target must be ArraySequence");
            arr->Append(v);
            ++pos;
            return pos;
        }
        if (outStream) {
            if (!serializer) throw std::runtime_error("WriteOnlyStream: no serializer provided");
            std::string s = serializer(v);
            (*outStream) << s;
            // записываем перевод строки для удобства текстового формата
            (*outStream) << '\n';
            if (!(*outStream)) throw std::runtime_error("WriteOnlyStream: write to stream failed");
            ++pos;
            return pos;
        }
        throw std::runtime_error("WriteOnlyStream: no write target available");
    }

    size_t GetPosition() const { return pos; }

    void Open() {
        // Ничего не делаем для non-owning потоков/sequence.
        // Для файлов — уже открыт в конструкторе.
    }

    void Close() {
        if (outStream) outStream->flush();
        // ownedOutStream (ofstream) будет закрыт при уничтожении unique_ptr
    }

    ~WriteOnlyStream() {
        try { Close(); } catch (...) {}
    }

private:
    Sequence<T>* seqTarget;                                  // не владеем Sequence
    std::ostream* outStream;                                 // не владеем поток, если nullptr — смотрим ownedOutStream
    std::unique_ptr<std::ostream> ownedOutStream;            // владеем (обычно ofstream при записи в файл)
    std::function<std::string(const T&)> serializer;         // обязателен для записи в поток
    size_t pos;                                              // позиция (сколько элементов записано)
};
