
#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H
#include <stdexcept>
#include <string>
#include <iostream>
template <typename T>
class DynamicArray {
public:
    DynamicArray();
    DynamicArray(T* items,int count);
    explicit DynamicArray(int s);
    DynamicArray(const DynamicArray& other);
    DynamicArray(int s, const T& initialValue);
    DynamicArray& operator=(const DynamicArray& other);
    ~DynamicArray();
    const T& Get(int index) const;
    int GetSize() const;
    void Set(int index,T value);
    void Resize(int NewSize);
    void Append(T value);
private:
    int size;
    int capacity;
    T* data;
};
template <typename T>
DynamicArray<T>::DynamicArray():size(0),capacity(1),data(new T[1]){};
template <typename T>
DynamicArray<T>::DynamicArray(int s):size(s),capacity(s),data(new T[s]){};
template <typename T>
DynamicArray<T>::DynamicArray(const DynamicArray& other):size(other.size),capacity(other.capacity),data(nullptr){
    if (size > 0) {
        data = new T[capacity];
        for (int i = 0; i < size; i++) {
            data[i] = other.data[i];
        }
    }

};
template <typename T>
DynamicArray<T>::DynamicArray(T* items,int count):size(count),capacity(count),data(new T[count]){
    for (int i=0;i<count;i++){
        data[i]=items[i];
    }
}
template <typename T>
DynamicArray<T>::DynamicArray(int s, const T& initialValue) : size(s), capacity(s), data(new T[s]) {
    for (int i = 0; i < size; i++) {
        data[i] = initialValue;
    }
}
template <typename T>
DynamicArray<T>& DynamicArray<T>::operator=(const DynamicArray& other) {
    if (this == &other) return *this;
    delete[] data;
    size = other.size;
    capacity = other.capacity;
    data = new T[capacity];
    for (int i = 0; i < size; i++) {
        data[i] = other.data[i];
    }
    return *this;
}
template <typename T>
const T& DynamicArray<T>::Get(int index) const{
    if (index>=size or index<0){
        throw std::out_of_range("Index out of range");
    }
    return data[index];
}
template <typename T>
int DynamicArray<T>::GetSize() const{
    return size;
}
template <typename T>
void DynamicArray<T>::Resize(int NewSize){
    if (NewSize==size){
        return;
    }
    if (NewSize<0){
        throw std::invalid_argument("Size must be greater than 0");
    }
    T* NewData = new T[NewSize];
    int elementsToCopy = (NewSize < size) ? NewSize : size;

    for (int i = 0; i < elementsToCopy; i++) {
        NewData[i] = data[i];
    }

    delete[] data;
    data = NewData;
    size = NewSize;
    capacity = NewSize;
}
template <typename T>
void DynamicArray<T>::Set(int index,T value){
    if (index < 0 || index >= size) {
        throw std::out_of_range("Index out of range");
    }
    data[index] = value;
}
template <typename T>
void DynamicArray<T>::Append(T value) {
    if (size >= capacity) {
        if (capacity==0){
            Resize(1);
            capacity=1;
        }
        else {
            Resize(capacity * 2);
            capacity*=2;
        }
    }
    data[size] = value;
    size+=1;
}
template <typename T>
DynamicArray<T>::~DynamicArray() {
    delete[] data;
}
#endif