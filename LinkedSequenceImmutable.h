#ifndef LINKEDSEQUENCE_IMMUTABLE_H
#define LINKEDSEQUENCE_IMMUTABLE_H

#include "LinkedSequence.h"
#include "Sequence.h"
#include <stdexcept>

template <typename T>
class LinkedSequenceImmutable : public LinkedSequence<T> {
private:
    LinkedSequence<T>* Instance() override {
        return Clone();
    }
    LinkedSequence<T>* Clone() override{
        return new LinkedSequenceImmutable<T>(*this);
    }
public:
    using LinkedSequence<T>::LinkedSequence;
};

#endif
