#ifndef IMMUTABLEARRAYSEQUENCE_H
#define IMMUTABLEARRAYSEQUENCE_H

#include "ArraySequence.h"
#include <stdexcept>

template <typename T>
class ImmutableArraySequence : public ArraySequence<T> {
private:
    ArraySequence<T>* Instance() override {
        return Clone();
    }
    ArraySequence<T>* Clone() override{
        return new ImmutableArraySequence<T>(*this);
    }
public:
    using ArraySequence<T>::ArraySequence;

};


#endif



