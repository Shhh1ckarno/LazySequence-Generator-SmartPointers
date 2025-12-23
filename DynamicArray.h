#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include <stdexcept>
#include <algorithm> 
#include <new>

template <typename T>
class DynamicArray {
public:
    DynamicArray();
    DynamicArray(T* items, int count);
    explicit DynamicArray(int s);
    DynamicArray(const DynamicArray& other);
    DynamicArray(int s, const T& initialValue);
    DynamicArray& operator=(const DynamicArray& other);
    ~DynamicArray();

    const T& Get(int index) const;
    T& Get(int index);
    int GetSize() const;
    void Set(int index, const T& value);

    void Reserve(int newCapacity);
    void Resize(int NewSize);
    void Append(const T& value);

    bool operator==(const DynamicArray& other) const {
        if (size != other.size) return false;
        for(int i = 0; i < size; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }
    bool operator!=(const DynamicArray& other) const {
        return !(*this == other);
    }

private:
    int size;
    int capacity;
    T* data;

    void ensureCapacity(int minCapacity);
};

template <typename T>
DynamicArray<T>::DynamicArray()
    : size(0), capacity(1), data(new T[1]()) 
{ }

template <typename T>
DynamicArray<T>::DynamicArray(int s)
    : size(s > 0 ? s : 0), capacity(s > 0 ? s : 1), data(new T[capacity]())
{ }

template <typename T>
DynamicArray<T>::DynamicArray(T* items, int count)
    : size(count > 0 ? count : 0), capacity(count > 0 ? count : 1), data(new T[capacity]())
{
    for (int i = 0; i < size; ++i) data[i] = items[i];
}

template <typename T>
DynamicArray<T>::DynamicArray(const DynamicArray& other)
    : size(other.size), capacity(other.capacity > 0 ? other.capacity : 1), data(new T[capacity]())
{
    for (int i = 0; i < size; ++i) data[i] = other.data[i];
}

template <typename T>
DynamicArray<T>::DynamicArray(int s, const T& initialValue)
    : size(s > 0 ? s : 0), capacity(s > 0 ? s : 1), data(new T[capacity]())
{
    for (int i = 0; i < size; ++i) data[i] = initialValue;
}

template <typename T>
DynamicArray<T>& DynamicArray<T>::operator=(const DynamicArray& other) {
    if (this == &other) return *this;

    int newCap = other.capacity > 0 ? other.capacity : 1;
    T* newData = new T[newCap]();
    for (int i = 0; i < other.size; ++i) newData[i] = other.data[i];

    delete[] data;
    data = newData;
    size = other.size;
    capacity = newCap;
    return *this;
}

template <typename T>
DynamicArray<T>::~DynamicArray() {
    delete[] data;
}

template <typename T>
const T& DynamicArray<T>::Get(int index) const {
    if (index < 0 || index >= size) {
        throw std::out_of_range("DynamicArray::Get: index out of range");
    }
    return data[index];
}

template <typename T>
T& DynamicArray<T>::Get(int index) {
    if (index < 0 || index >= size) {
        throw std::out_of_range("DynamicArray::Get: index out of range");
    }
    return data[index];
}

template <typename T>
int DynamicArray<T>::GetSize() const {
    return size;
}

template <typename T>
void DynamicArray<T>::Set(int index, const T& value) {
    if (index < 0 || index >= size) {
        throw std::out_of_range("DynamicArray::Set: index out of range");
    }
    data[index] = value;
}

template <typename T>
void DynamicArray<T>::ensureCapacity(int minCapacity) {
    if (capacity >= minCapacity) return;
    int newCap = std::max(capacity * 2, minCapacity);
    if (newCap <= 0) newCap = 1;
    T* newData = new T[newCap]();
    for (int i = 0; i < size; ++i) newData[i] = data[i];
    delete[] data;
    data = newData;
    capacity = newCap;
}

template <typename T>
void DynamicArray<T>::Reserve(int newCapacity) {
    if (newCapacity <= capacity) return;
    ensureCapacity(newCapacity);
}

template <typename T>
void DynamicArray<T>::Resize(int NewSize) {
    if (NewSize < 0) {
        throw std::invalid_argument("DynamicArray::Resize: NewSize must be non-negative");
    }
    if (NewSize == size) return;
    if (NewSize < size) {
        size = NewSize;
        return;
    }
    ensureCapacity(NewSize);
    for (int i = size; i < NewSize; ++i) data[i] = T();
    size = NewSize;
}

template <typename T>
void DynamicArray<T>::Append(const T& value) {
    if (size >= capacity) {
        ensureCapacity(capacity > 0 ? capacity * 2 : 1);
    }
    data[size++] = value;
}

#endif