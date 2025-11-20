
#ifndef LABA2_STACK_H
#define LABA2_STACK_H

#include "LinkedSequenceMutable.h"

template <typename T>
class Stack {
private:
    LinkedSequenceMutable<T>* sequence;
public:
    Stack();
    Stack(const Stack<T>& other);
    ~Stack();

    void Push(const T& item);
    T Pop();
    T Top() const;
    int GetSize() const;
    void Clear();
};

template <typename T>
Stack<T>::Stack() {
    sequence = new LinkedSequenceMutable<T>();
}

template <typename T>
Stack<T>::Stack(const Stack<T>& other) {
    sequence = new LinkedSequenceMutable<T>(*other.sequence);
}

template <typename T>
Stack<T>::~Stack() {
    delete sequence;
}

template <typename T>
void Stack<T>::Push(const T& item) {
    sequence->Append(item);
}

template <typename T>
T Stack<T>::Pop() {
    if (GetSize()==0)
        throw std::out_of_range("Stack is empty");
    T item = sequence->GetLast();
    sequence->Remove(sequence->GetLength() - 1);
    return item;
}

template <typename T>
T Stack<T>::Top() const {
    if (GetSize()==0)
        throw std::out_of_range("Stack is empty");
    return sequence->GetLast();
}

template <typename T>
int Stack<T>::GetSize() const {
    return sequence->GetLength();
}

template <typename T>
void Stack<T>::Clear() {
    while (GetSize()!=0) {
        Pop();
    }
}
#endif
