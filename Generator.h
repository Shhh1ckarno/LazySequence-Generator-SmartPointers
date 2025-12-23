#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <optional>
#include "ArraySequence.h"
#include "DynamicArray.h"  
#include "SmartPointer.h" 
#include "Deck.h"        
#include "Cardinal.h"   
#include <optional>

template <class T>
class Generator {
public:

Generator(const SharedPtr<ArraySequence<T>>& materialised_, std::function<T(Sequence<T>*)> ruleFunc_)
    : materialised(materialised_), rule(ruleFunc_), injHead(0)
{
    if (materialised_) {
        size_t raw_len = materialised_->GetLength(); 
        Cardinal len(raw_len); 

        if (len.IsFinite() && len.GetValue() > 0)
            pos = static_cast<long long>(len.GetValue() - 1); 
        else
            pos = -1;
    } else {
        pos = -1;
    }
}
    Generator(const Generator& other)
        : materialised(other.materialised),
          rule(other.rule),
          pos(other.pos),
          injections(other.injections),
          injHead(other.injHead),
          prependQueue(other.prependQueue),
          removeValues(other.removeValues)
    {}

    T GetNext() {
        if (prependQueue.GetSize() > 0) {
            T val = prependQueue.Get(0);
            prependQueue.PopFront();         
            if (materialised) materialised->Append(val);
            ++pos;
            return val;
        }
        if (injHead < static_cast<size_t>(injections.GetSize())) {
            T val = injections.Get(static_cast<int>(injHead++));
            if (materialised) materialised->Append(val);
            ++pos;
            return val;
        }
        if (!rule) throw std::runtime_error("Generator: no rule and no queued elements");

        while (true) {
            Sequence<T>* seqPtr = materialised.get();
            if (!seqPtr) throw std::runtime_error("Generator: materialised sequence is null");

            T cand = rule(seqPtr);            
            materialised->Append(cand);      

            if (!isRemoved(cand)) {  
                ++pos;
                return cand;
            }

        }
    }

    std::optional<T> TryGetNext() {
        try {
            return GetNext();
        } catch (...) {
            return std::nullopt;
        }
    }

    UniquePtr<Generator<T>> PrependValue(const T& item) const {
        UniquePtr<Generator<T>> g(new Generator<T>(*this));
        g->prependQueue.PushFront(item);
        return g;
    }

    UniquePtr<Generator<T>> PrependSequence(Sequence<T>* seq) const {
        UniquePtr<Generator<T>> g(new Generator<T>(*this));
        Cardinal len = seq->GetLength();
        if (len.IsOmega()) throw std::runtime_error("Cannot prepend infinite sequence");

        size_t n = len.GetValue();
        for (size_t i = n; i > 0; --i)
            g->prependQueue.PushFront(seq->Get(static_cast<int>(i - 1)));
        return g;
    }

    UniquePtr<Generator<T>> AppendValue(const T& item) const {
        UniquePtr<Generator<T>> g(new Generator<T>(*this));
        g->injections.Append(item); 
        return g;
    }

    UniquePtr<Generator<T>> AppendSequence(Sequence<T>* seq) const {
        UniquePtr<Generator<T>> g(new Generator<T>(*this));
        Cardinal len = seq->GetLength();
        if (len.IsOmega()) throw std::runtime_error("Cannot append infinite sequence");

        size_t n = len.GetValue();
        for (size_t i = 0; i < n; ++i)
            g->injections.Append(seq->Get(static_cast<int>(i)));
        return g;
    }

    UniquePtr<Generator<T>> RemoveValue(const T& item) const {
        UniquePtr<Generator<T>> g(new Generator<T>(*this));
        g->removeValues.Append(item);
        return g;
    }

    UniquePtr<Generator<T>> RemoveSequence(Sequence<T>* seq) const {
        UniquePtr<Generator<T>> g(new Generator<T>(*this));
        Cardinal len = seq->GetLength();
        if (len.IsOmega()) throw std::runtime_error("Cannot remove infinite sequence");

        size_t n = len.GetValue();
        for (size_t i = 0; i < n; ++i)
            g->removeValues.Append(seq->Get(static_cast<int>(i)));
        return g;
    }

    long long GetPosition() const { return pos; }

private:

    bool isRemoved(const T& v) const {
        for (int i = 0; i < removeValues.GetSize(); ++i)
            if (removeValues.Get(i) == v) return true;
        return false;
    }

private:
    SharedPtr<ArraySequence<T>> materialised; 
    std::function<T(Sequence<T>*)> rule;      
    long long pos;                             

    DynamicArray<T> injections;                
    size_t injHead;                            
    Deque<T> prependQueue;                   
    DynamicArray<T> removeValues;            
};
