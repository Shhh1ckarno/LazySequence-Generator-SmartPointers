#ifndef ARRAYSEQUENCE_H
#define ARRAYSEQUENCE_H

#include "Sequence.h"
#include "DynamicArray.h"
#include <stdexcept>

template <typename T>
class ArraySequence : public Sequence<T> {
protected:
    DynamicArray<T> array;
    explicit ArraySequence(const DynamicArray<T>& arr);
    virtual ArraySequence<T>* Instance()=0;
    virtual ArraySequence<T>* Clone()=0;

public:
    ArraySequence()=default;
    explicit ArraySequence(int size);
    ArraySequence(T* items, int count);
    ArraySequence(const ArraySequence<T>& other);
    virtual ~ArraySequence()=default;
    T GetFirst() const override;
    T GetLast() const override;
    T Get(int index) const override;
    int GetLength() const override;
    Sequence<T>* GetSubsequence(int startIndex, int endIndex) override;
    Sequence<T>* Append(const T& item) override;
    Sequence<T>* Prepend(const T& item) override;
    Sequence<T>* Insert(const T& item, int index) override;
    Sequence<T>* Concat(Sequence<T>* other) override;
};
template <typename T>
ArraySequence<T>::ArraySequence(const DynamicArray<T>& arr) : array(arr) {}

template <typename T>
ArraySequence<T>::ArraySequence(int size) : array(size) {}

template <typename T>
ArraySequence<T>::ArraySequence(T* items, int count)
        : array(DynamicArray<T>(items, count)) {}

template <typename T>
ArraySequence<T>::ArraySequence(const ArraySequence<T>& other)
        : array(other.array) {}

template <typename T>
T ArraySequence<T>::GetFirst() const {
    if (array.GetSize() == 0)
        throw std::out_of_range("Sequence is empty");
    return array.Get(0);
}

template <typename T>
T ArraySequence<T>::GetLast() const {
    if (array.GetSize() == 0)
        throw std::out_of_range("Sequence is empty");
    return array.Get(array.GetSize() - 1);
}

template <typename T>
T ArraySequence<T>::Get(int index) const {
    if (index < 0 or index>=array.GetSize())
        throw std::out_of_range("Sequence is empty");
    return array.Get(index);
}

template <typename T>
int ArraySequence<T>::GetLength() const {
    return array.GetSize();
}

template <typename T>
Sequence<T>* ArraySequence<T>::GetSubsequence(int startIndex, int endIndex) {
    if (startIndex < 0 || endIndex >= array.GetSize() || startIndex > endIndex)
        throw std::out_of_range("Invalid index range");
    DynamicArray<T> instance(endIndex-startIndex+1);
    for (int i=0;i<(endIndex-startIndex+1);i++){
        instance.Set(i,array.Get(startIndex+i));
    }
    ArraySequence<T>* result=Clone();
    result->array=instance;
    return result;

}
template <typename T>
Sequence<T>* ArraySequence<T>::Append(const T& item) {
    ArraySequence<T>* instance=Instance();
    int OldSize=instance->array.GetSize();
    instance->array.Resize(OldSize + 1);
    instance->array.Set(OldSize, item);
    return instance;
}

template <typename T>
Sequence<T>* ArraySequence<T>::Prepend(const T& item) {
    ArraySequence<T>* instance=Instance();
    int OldSize=instance->array.GetSize();
    instance->array.Resize(OldSize + 1);
    for (int i = OldSize; i > 0; --i) {
        instance->array.Set(i, instance->array.Get(i-1));
    }
    instance->array.Set(0, item);
    return instance;
}

template <typename T>
Sequence<T>* ArraySequence<T>::Insert(const T& item, int index) {
    if (index < 0 || index > array.GetSize())
        throw std::out_of_range("Index out of range");
    ArraySequence<T>* instance=Instance();
    int OldSize=instance->array.GetSize();
    instance->array.Resize(OldSize + 1);
    for (int i = OldSize; i > index; --i) {
        instance->array.Set(i, instance->array.Get(i - 1));
    }
    instance->array.Set(index, item);
    return instance;
}

template <typename T>
Sequence<T>* ArraySequence<T>::Concat(Sequence<T>* other) {
    DynamicArray<T> NewArray(this->GetLength()+other->GetLength());
    for (int i=0;i<(array.GetSize());i++){
        NewArray.Set(i,this->Get(i));
    }
    for (int i=0;i<(other->GetLength());i++){
        NewArray.Set(this->GetLength()+i,other->Get(i));
    }
    ArraySequence<T>* result=Clone();
    result->array=NewArray;
    return result;
}

#endif
