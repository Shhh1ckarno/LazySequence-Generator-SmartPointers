#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <optional>
#include "ArraySequence.h" // ArraySequence<T>
#include "DynamicArray.h"  // DynamicArray<T>
#include "SmartPointer.h"  // SharedPtr<T>, UniquePtr<T>
#include "Deck.h"         // Deque<T>
#include "Cardinal.h"      // Cardinal
#include <optional>

template <class T>
class Generator {
public:
    // -----------------------------
    // Конструктор генератора
    // -----------------------------
    Generator(const SharedPtr<ArraySequence<T>>& materialised_, std::function<T(Sequence<T>*)> ruleFunc_)
        : materialised(materialised_), rule(ruleFunc_), injHead(0)
    {
        // pos — индекс последнего сгенерированного элемента
        // Если массив пуст, pos = -1
        if (materialised_ && materialised_->GetLength() > 0)
            pos = static_cast<long long>(materialised_->GetLength() - 1);
        else
            pos = -1;
    }

    // -----------------------------
    // Копирующий конструктор
    // -----------------------------
    Generator(const Generator& other)
        : materialised(other.materialised),
          rule(other.rule),
          pos(other.pos),
          injections(other.injections),
          injHead(other.injHead),
          prependQueue(other.prependQueue),
          removeValues(other.removeValues)
    {}

    // -----------------------------
    // Получение следующего элемента
    // -----------------------------
    T GetNext() {
        // 1. Сначала проверяем очередь prepend
        if (prependQueue.GetSize() > 0) {
            T val = prependQueue.Get(0);
            prependQueue.PopFront();          // O(1)
            if (materialised) materialised->Append(val);
            ++pos;
            return val;
        }

        // 2. Потом очередь append/injections
        if (injHead < static_cast<size_t>(injections.GetSize())) {
            T val = injections.Get(static_cast<int>(injHead++));
            if (materialised) materialised->Append(val);
            ++pos;
            return val;
        }

        // 3. Генерация по правилу
        if (!rule) throw std::runtime_error("Generator: no rule and no queued elements");

        while (true) {
            Sequence<T>* seqPtr = materialised.get();
            if (!seqPtr) throw std::runtime_error("Generator: materialised sequence is null");

            T cand = rule(seqPtr);            // генерируем следующий элемент
            materialised->Append(cand);       // материализуем

            if (!isRemoved(cand)) {           // фильтруем удаленные значения
                ++pos;
                return cand;
            }
            // если cand удалён, продолжаем цикл
        }
    }

    // -----------------------------
    // Попытка получить следующий элемент
    // -----------------------------
    std::optional<T> TryGetNext() {
        try {
            return GetNext();
        } catch (...) {
            return std::nullopt;
        }
    }

    // -----------------------------
    // Методы добавления/удаления элементов
    // -----------------------------
    UniquePtr<Generator<T>> PrependValue(const T& item) const {
        UniquePtr<Generator<T>> g(new Generator<T>(*this));
        g->prependQueue.PushFront(item); // O(1)
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
        g->injections.Append(item); // амортизированно O(1)
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
        g->removeValues.Append(item); // добавляем элемент в фильтр удаления
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

    // -----------------------------
    // Получение позиции (индекса последнего сгенерированного)
    // -----------------------------
    long long GetPosition() const { return pos; }

private:
    // -----------------------------
    // Проверка, удалён ли элемент
    // -----------------------------
    bool isRemoved(const T& v) const {
        for (int i = 0; i < removeValues.GetSize(); ++i)
            if (removeValues.Get(i) == v) return true;
        return false;
    }

private:
    SharedPtr<ArraySequence<T>> materialised; // общий кэш
    std::function<T(Sequence<T>*)> rule;       // правило генерации
    long long pos;                             // позиция последнего сгенерированного элемента

    DynamicArray<T> injections;                // очередь append
    size_t injHead;                            // индекс следующего элемента injections

    Deque<T> prependQueue;                     // очередь prepend (O(1))
    DynamicArray<T> removeValues;              // элементы для удаления
};
