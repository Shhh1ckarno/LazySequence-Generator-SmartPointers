

#ifndef LABA2_DECK_H
#define LABA2_DECK_H
#include "LinkedSequenceMutable.h"

template <typename T>
class Deque {
private:
    LinkedSequenceMutable<T>* sequence;
public:
    Deque();
    Deque(const Deque<T>& other);
    ~Deque();

    void PushTop(const T& item);
    void PushFront(const T& item);
    T PopTop();
    T PopFront();
    T Front() const;
    T Top() const;
    T Get(int index) const;
    int GetSize() const;
    void Clear();
};
template <typename T>
Deque<T>::Deque() {
    sequence = new LinkedSequenceMutable<T>();
}

template <typename T>
Deque<T>::Deque(const Deque<T>& other) {
    sequence = new LinkedSequenceMutable<T>(*other.sequence);
}

template <typename T>
Deque<T>::~Deque() {
    delete sequence;
}

template <typename T>
void Deque<T>::PushTop(const T& item) {
    sequence->Append(item);
}

template <typename T>
void Deque<T>::PushFront(const T& item) {
    sequence->Prepend(item);
}

template <typename T>
T Deque<T>::PopTop() {
    if (GetSize() == 0)
        throw std::out_of_range("Deque is empty");
    T item = sequence->GetLast();
    sequence->Remove(GetSize() - 1);
    return item;
}

template <typename T>
T Deque<T>::PopFront() {
    if (GetSize() == 0)
        throw std::out_of_range("Deque is empty");
    T item = sequence->GetFirst();
    sequence->Remove(0);
    return item;
}

template <typename T>
T Deque<T>::Front() const {
    if (GetSize() == 0)
        throw std::out_of_range("Deque is empty");
    return sequence->GetFirst();
}

template <typename T>
T Deque<T>::Top() const {
    if (GetSize() == 0)
        throw std::out_of_range("Deque is empty");
    return sequence->GetLast();
}

template <typename T>
int Deque<T>::GetSize() const {
    return sequence->GetLength();
}

template <typename T>
void Deque<T>::Clear() {
    while (GetSize() != 0) {
        PopFront();
    }
}
template <typename T>
T Deque<T>::Get(int index) const {
    if (index < 0 || index >= GetSize()) {
        throw std::out_of_range("Index out of range");
    }
    return sequence->Get(index);
}
#endif
