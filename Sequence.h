//
// Created by ggwp2 on 23.04.2025.
//

#ifndef SEQUENCE_H
#define SEQUENCE_H
#include <stdexcept>
#include <string>

template <typename T>
class Sequence {
public:
    virtual ~Sequence()=default;
    virtual T GetFirst() const=0;
    virtual T GetLast() const=0;
    virtual T Get(int index) const=0;
    virtual int GetLength() const=0;
    virtual Sequence<T>* GetSubsequence(int startindex, int endindex) = 0;
    virtual Sequence<T>* Append(const T& item)=0;
    virtual Sequence<T>* Prepend(const T& Item)=0;
    virtual Sequence<T>* Insert(const T& item,int index)=0;
    virtual Sequence<T>* Concat(Sequence<T>* other) =0;

};


#endif
