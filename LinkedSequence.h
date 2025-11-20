#ifndef LINKED_SEQUENCE_H
#define LINKED_SEQUENCE_H

#include "LinkedList.h"
#include "Sequence.h"

template <typename T>
class LinkedSequence : public Sequence<T> {
protected:
    LinkedList<T>* list;
    virtual LinkedSequence<T>* Instance()=0;
    virtual LinkedSequence<T>* Clone()=0;
public:
    LinkedSequence();
    LinkedSequence(const LinkedSequence<T>& other);
    LinkedSequence(T* items, int count);
    LinkedSequence(const LinkedList<T>& otherList);
    LinkedSequence(LinkedList<T>* otherList);
    ~LinkedSequence();
    T Get(int index) const override;
    T GetFirst() const override;
    T GetLast() const override;
    int GetLength() const override;
    Sequence<T>* Append(const T& item) override;
    Sequence<T>* Prepend(const T& item) override;
    Sequence<T>* Insert(const T& item, int index) override;
    Sequence<T>* Remove(int index);
    Sequence<T>* GetSubsequence(int startIndex, int endIndex) override;
    Sequence<T>* Concat(Sequence<T>* other) override;
};
template <typename T>
LinkedSequence<T>::LinkedSequence() {
    list = new LinkedList<T>();
}
template <typename T>
LinkedSequence<T>::LinkedSequence(const LinkedSequence<T>& other) {
    list = new LinkedList<T>(*other.list);
}
template <typename T>
LinkedSequence<T>::LinkedSequence(T* items, int count) {
    list = new LinkedList<T>(items, count);
}

template <typename T>
LinkedSequence<T>::LinkedSequence(const LinkedList<T>& otherList) {
    list = new LinkedList<T>(otherList);
}
template <typename T>
LinkedSequence<T>::LinkedSequence(LinkedList<T>* otherList){
    list=new LinkedList<T>(*otherList);
}
template <typename T>
LinkedSequence<T>::~LinkedSequence() {
    delete list;
}

template <typename T>
T LinkedSequence<T>::Get(int index) const {
    if (list->GetLength() == 0)
        throw std::out_of_range("Sequence is empty");
    return list->Get(index);
}

template <typename T>
T LinkedSequence<T>::GetFirst() const {
    if (list->GetLength() == 0)
        throw std::out_of_range("Sequence is empty");
    return list->GetFirst();
}

template <typename T>
T LinkedSequence<T>::GetLast() const {
    if (list->GetLength() == 0)
        throw std::out_of_range("Sequence is empty");
    return list->GetLast();
}

template <typename T>
int LinkedSequence<T>::GetLength() const {
    return list->GetLength();
}

template <typename T>
Sequence<T>* LinkedSequence<T>::Append(const T& item) {
    LinkedSequence<T>* instance=Instance();
    instance->list->Append(item);
    return instance;
}

template <typename T>
Sequence<T>* LinkedSequence<T>::Prepend(const T& item) {
    LinkedSequence<T>* instance=Instance();
    instance->list->Prepend(item);
    return instance;
}

template <typename T>
Sequence<T>* LinkedSequence<T>::Insert(const T& item, int index) {
    LinkedSequence<T>* instance=Instance();
    instance->list->Insert(item, index);
    return instance;
}
template <typename T>
Sequence<T>* LinkedSequence<T>::Remove(int index) {
    LinkedSequence<T>* instance=Instance();
    instance->list->Remove(index);
    return instance;
}
template <typename T>
Sequence<T>* LinkedSequence<T>::GetSubsequence(int startIndex, int endIndex) {
    LinkedList<T>* sub = list->GetSublist(startIndex, endIndex);
    LinkedSequence<T>* instance=Clone();
    delete instance->list;
    instance->list = sub;
    return instance;
}

template <typename T>
Sequence<T>* LinkedSequence<T>::Concat(Sequence<T>* other) {
    LinkedSequence<T>* instance=Clone();
    for (int i=0;i<other->GetLength();i++){
        instance->list->Append(other->Get(i));
    }
    return instance;
}


#endif
