#ifndef LINKEDSEQUENCE_MUTABLE_H
#define LINKEDSEQUENCE_MUTABLE_H

#include "LinkedSequence.h"
#include "Sequence.h"
#include <stdexcept>
template <typename T>
class LinkedSequenceMutable : public LinkedSequence<T> {
private:
    LinkedSequence<T>* Instance() override {
        return this;
    }
    LinkedSequence<T>* Clone() override{
    return new LinkedSequenceMutable<T>(*this);
    }
public:
    using LinkedSequence<T>::LinkedSequence;
};
#endif
