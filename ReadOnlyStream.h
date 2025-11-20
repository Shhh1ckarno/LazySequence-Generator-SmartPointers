#pragma once

#include <cstddef>
#include <utility>
#include <new>
#include <typeinfo>
#include <cassert>
#include <iostream>
#include <istream>
#include <functional>
#include <stdexcept>
#include <optional>
#include <string>

#include "LazySequence.h"
#include "SmartPointer.h"

template <class T>
class ReadOnlyStream {
public:
    explicit ReadOnlyStream(Sequence<T>* seq)
        : seqSource(make_shared< CoreLazySequence<T> >(seq)),
          lazySource(),
          inSource(nullptr),
          deserializer(nullptr),
          pos(0),
          opened(true),
          stringBuf(),
          stringPos(0)
    {}

    explicit ReadOnlyStream(LazySequence<T>* lazy)
        : seqSource(),
          lazySource(make_unique< LazySequence<T> >(*lazy)),
          inSource(nullptr),
          deserializer(nullptr),
          pos(0),
          opened(true),
          stringBuf(),
          stringPos(0)
    {}

    ReadOnlyStream(std::istream* in, std::function<T(const std::string&)> deserial)
        : seqSource(),
          lazySource(),
          inSource(in),
          deserializer(deserial),
          pos(0),
          opened(true),
          stringBuf(),
          stringPos(0)
    {
        if (!inSource) throw std::invalid_argument("ReadOnlyStream: null istream");
    }

    ReadOnlyStream(const std::string& s, std::function<T(const std::string&)> deserial)
        : seqSource(),
          lazySource(),
          inSource(nullptr),
          deserializer(deserial),
          pos(0),
          opened(false),
          stringBuf(s),
          stringPos(0)
    {}

    bool IsEndOfStream() const {
        if (inSource) return inSource->eof();
        if (!stringBuf.empty()) return stringPos >= stringBuf.size();
        if (lazySource) {
            Cardinal l = lazySource->GetLength();
            if (l.IsOmega()) return false;
            return pos >= l.GetValue();
        }
        if (seqSource) {
            Cardinal l = seqSource->GetLength();
            if (l.IsOmega()) return false;
            return pos >= l.GetValue();
        }
        return true;
    }

    T Read() {
        if (!opened) Open();
        if (IsEndOfStream()) throw std::out_of_range("EndOfStream");
        if (inSource) {
            std::string line;
            if (!std::getline(*inSource, line)) throw std::out_of_range("EndOfStream");
            if (!deserializer) throw std::runtime_error("No deserializer");
            ++pos;
            return deserializer(line);
        }
        if (!stringBuf.empty()) {
            if (!deserializer) throw std::runtime_error("No deserializer");
            size_t start = stringPos;
            size_t next = stringBuf.find('\n', start);
            std::string token;
            if (next == std::string::npos) {
                token = stringBuf.substr(start);
                stringPos = stringBuf.size();
            } else {
                token = stringBuf.substr(start, next - start);
                stringPos = next + 1;
            }
            ++pos;
            return deserializer(token);
        }
        if (lazySource) {
            T v = lazySource->Get(pos);
            ++pos;
            return v;
        }
        if (seqSource) {
            T v = seqSource->Get(pos);
            ++pos;
            return v;
        }
        throw std::runtime_error("No source");
    }

    size_t GetPosition() const { return pos; }
    bool IsCanSeek() const { return static_cast<bool>(seqSource) || static_cast<bool>(lazySource); }
    size_t Seek(size_t index) {
        if (!IsCanSeek()) throw std::runtime_error("Seek not supported");
        pos = index;
        if (lazySource && pos > lazySource->GetMaterializedCount() && lazySource->HasGenerator()) {
            // do not materialize here — caller can call Read which will materialize as needed
        }
        return pos;
    }
    bool IsCanGoBack() const { return IsCanSeek(); }

    void Open() { opened = true; if (!stringBuf.empty()) stringPos = 0; }
    void Close() { opened = false; }

private:
    SharedPtr< CoreLazySequence<T> > seqSource;
    UniquePtr< LazySequence<T> > lazySource;
    std::istream* inSource;
    std::function<T(const std::string&)> deserializer;
    size_t pos;
    bool opened;
    std::string stringBuf;
    size_t stringPos;
};
