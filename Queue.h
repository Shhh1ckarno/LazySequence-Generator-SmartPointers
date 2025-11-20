
#ifndef LABA2_QUEUE_H
#define LABA2_QUEUE_H
#include "LinkedSequenceMutable.h"

template <typename T>
class Queue {
private:
    LinkedSequenceMutable<T>* sequence;
public:
    Queue();
    Queue(const Queue<T>& other);
    ~Queue();

    void Push(const T& item);
    T Pop();
    T Front() const;
    int GetSize() const;
    void Clear();
};

template <typename T>
Queue<T>::Queue()  {
    sequence = new LinkedSequenceMutable<T>();
}

template <typename T>
Queue<T>::Queue(const Queue<T>& other) {
    sequence = new LinkedSequenceMutable<T>(*other.sequence);
}

template <typename T>
Queue<T>::~Queue() {
    delete sequence;
}

template <typename T>
void Queue<T>::Push(const T& item) {
    sequence->Append(item);
}

template <typename T>
T Queue<T>::Pop() {
    if (GetSize()==0)
        throw std::out_of_range("Stack is empty");
    T item = sequence->GetFirst();
    sequence->Remove(0);
    return item;
}

template <typename T>
T Queue<T>::Front() const {
    if (GetSize()==0)
        throw std::out_of_range("Stack is empty");
    return sequence->GetFirst();
}

template <typename T>
int Queue<T>::GetSize() const {
    return sequence->GetLength();
}

template <typename T>
void Queue<T>::Clear() {
    while (GetSize()!=0) {
        Pop();
    }
}
#endif