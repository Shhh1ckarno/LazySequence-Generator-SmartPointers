#ifndef MUTABLEARRAYSEQUENCE_H
#define MUTABLEARRAYSEQUENCE_H

#include "ArraySequence.h"
#include <stdexcept>

template <typename T>
class MutableArraySequence : public ArraySequence<T> {
private:
    ArraySequence<T>* Instance() override {
        return this;
    }
    ArraySequence<T>* Clone() override{
        return new MutableArraySequence<T>(*this);
    }
public:
    using ArraySequence<T>::ArraySequence;

};
#endif