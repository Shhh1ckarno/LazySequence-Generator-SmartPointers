#pragma once

#include <cstddef>
#include <utility>
#include <new>
#include <typeinfo>
#include <cassert>
#include <iostream>


template <class T>
class UniquePtr {
public:
    typedef T element_type;

    UniquePtr() : ptr_(0) {}
    explicit UniquePtr(T* p) : ptr_(p) {}
    UniquePtr(UniquePtr&& other) : ptr_(other.release()) {}

    template<class U>
    UniquePtr(UniquePtr<U>&& other) {
        U* up = other.get();
        if (!up) { ptr_ = 0; other.release(); return; }
        T* casted = dynamic_cast<T*>(up);
        if (casted) { ptr_ = casted; other.release(); }
        else throw std::bad_cast();
    }

    UniquePtr& operator=(UniquePtr&& other) {
        if (this != &other) reset(other.release());
        return *this;
    }

    template<class U>
    UniquePtr& operator=(UniquePtr<U>&& other) {
        if ((void*)this == (void*)&other) return *this;
        U* up = other.get();
        if (!up) { reset(0); other.release(); return *this; }
        T* casted = dynamic_cast<T*>(up);
        if (casted) { reset(casted); other.release(); }
        else throw std::bad_cast();
        return *this;
    }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;
    ~UniquePtr() { delete ptr_; }

    T* get() const { return ptr_; }
    T& operator*() const { assert(ptr_); return *ptr_; }
    T* operator->() const { return ptr_; }
    operator bool() const { return ptr_ != 0; }

    T* release() { return std::exchange(ptr_, (T*)0); }
    void reset(T* p = 0) {
        if (ptr_ != p) { delete ptr_; ptr_ = p; }
    }

    void swap(UniquePtr& other) { T* tmp = ptr_; ptr_ = other.ptr_; other.ptr_ = tmp; }

private:
    T* ptr_;
};

template<class T, class... Args>
UniquePtr<T> MakeUnique(Args&&... args) {
    return UniquePtr<T>(new T(static_cast<Args&&>(args)...));
}


template <class T>
class UniquePtr<T[]> {
public:
    typedef T element_type;

    UniquePtr() : ptr_(0) {}
    explicit UniquePtr(T* p) : ptr_(p) {}
    UniquePtr(UniquePtr&& other) : ptr_(other.release()) {}
    UniquePtr& operator=(UniquePtr&& other) { reset(other.release()); return *this; }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;
    ~UniquePtr() { delete[] ptr_; }

    T* get() const { return ptr_; }
    T& operator[](std::size_t i) const { return ptr_[i]; }
    T* release() { return std::exchange(ptr_, (T*)0); }
    void reset(T* p = 0) {
        if (ptr_ != p) { delete[] ptr_; ptr_ = p; }
    }
    operator bool() const { return ptr_ != 0; }

private:
    T* ptr_;
};


template<class T>
UniquePtr<T[]> MakeUnique(std::size_t n) {
    return UniquePtr<T[]>(new T[n]());
}


struct ControlBlockBase {
    std::size_t strong;
    std::size_t weak;
    ControlBlockBase() : strong(1), weak(0) {}
    virtual void destroy_object() = 0;
    virtual ~ControlBlockBase() {}
};


template<class U>
struct ControlBlock : ControlBlockBase {
    U* ptr;
    explicit ControlBlock(U* p) : ptr(p) {}
    void destroy_object() override { delete ptr; ptr = 0; }
};


template<class U>
struct ControlBlockArray : ControlBlockBase {
    U* ptr;
    explicit ControlBlockArray(U* p) : ptr(p) {}
    void destroy_object() override { delete[] ptr; ptr = 0; }
};

template<class T>
class SharedPtr {
public:
    typedef T element_type;

    SharedPtr() : ptr_(0), cb_(0) {}
    explicit SharedPtr(T* p) {
        if (p) cb_ = new ControlBlock<T>(p); else cb_ = 0;
        ptr_ = p;
    }

    SharedPtr(const SharedPtr& other) { acquire(other.ptr_, other.cb_); }

    template<class U>
    SharedPtr(const SharedPtr<U>& other) {
        U* up = other.ptr_;
        if (!up) { ptr_ = 0; cb_ = 0; return; }
        T* casted = dynamic_cast<T*>(up);
        if (casted) acquire(casted, other.cb_); else throw std::bad_cast();
    }

    SharedPtr(SharedPtr&& other) : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = 0; other.cb_ = 0;
    }

    template<class U>
    SharedPtr(SharedPtr<U>&& other) {
        U* up = other.ptr_;
        if (!up) { ptr_ = 0; cb_ = 0; other.ptr_ = 0; other.cb_ = 0; return; }
        T* casted = dynamic_cast<T*>(up);
        if (casted) { ptr_ = casted; cb_ = other.cb_; other.ptr_ = 0; other.cb_ = 0; }
        else throw std::bad_cast();
    }

    ~SharedPtr() { release(); }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) { release(); acquire(other.ptr_, other.cb_); }
        return *this;
    }

    template<class U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        if ((void*)this == (void*)&other) return *this;
        U* up = other.ptr_;
        if (!up) { release(); return *this; }
        T* casted = dynamic_cast<T*>(up);
        if (casted) { release(); acquire(casted, other.cb_); } else throw std::bad_cast();
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        if (this != &other) { release(); ptr_ = other.ptr_; cb_ = other.cb_; other.ptr_ = 0; other.cb_ = 0; }
        return *this;
    }

    template<class U>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        if ((void*)this == (void*)&other) return *this;
        U* up = other.ptr_;
        if (!up) { release(); return *this; }
        T* casted = dynamic_cast<T*>(up);
        if (casted) { release(); ptr_ = casted; cb_ = other.cb_; other.ptr_ = 0; other.cb_ = 0; }
        else throw std::bad_cast();
        return *this;
    }

    T* get() const { return ptr_; }
    T& operator*() const { assert(ptr_); return *ptr_; }
    T* operator->() const { return ptr_; }
    operator bool() const { return ptr_ != 0; }
    std::size_t use_count() const { return cb_ ? cb_->strong : 0; }

    void reset() { release(); }
    void reset(T* p) {
        release();
        if (p) { cb_ = new ControlBlock<T>(p); ptr_ = p; } else { cb_ = 0; ptr_ = 0; }
    }
    void swap(SharedPtr& other) { std::swap(ptr_, other.ptr_); std::swap(cb_, other.cb_); }

    template<class U> friend class SharedPtr;

private:
    template<class U>
    void acquire(U* p, ControlBlockBase* cb) {
        ptr_ = static_cast<T*>(p);
        cb_ = cb;
        if (cb_) cb_->strong += 1;
    }

    void release() {
        if (!cb_) return;
        if (cb_->strong > 0) {
            cb_->strong -= 1;
            if (cb_->strong == 0) {
                cb_->destroy_object();
                if (cb_->weak == 0) delete cb_;
            }
        }
        ptr_ = 0; cb_ = 0;
    }

    T* ptr_;
    ControlBlockBase* cb_;
};

template<class T, class... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    T* raw = new T(static_cast<Args&&>(args)...);
    return SharedPtr<T>(raw);
}

template<class T>
SharedPtr<T[]> MakeSharedArray(std::size_t n) {
    T* raw = new T[n]();
    SharedPtr<T[]> sp;
    sp.ptr_ = raw;
    sp.cb_ = new ControlBlockArray<T>(raw);
    return sp;
}
