#pragma once 

#include <cstddef>
#include <cassert>
#include <atomic>

template <typename T>
class SharedPtr 
{
public:
    SharedPtr() : _ptr(nullptr), _refs(nullptr) {}
    explicit SharedPtr(T* ptr) : _ptr(ptr), _refs(_ptr ? new std::atomic<size_t>(0) : nullptr) { ref(); }

    ~SharedPtr() { unref(); }
    
    SharedPtr(const SharedPtr& p) : _ptr(p._ptr), _refs(p._refs) { ref(); }

    SharedPtr& operator=(const SharedPtr& p) {
        if (*this != p) {
            unref();
            _ptr = p._ptr;
            _refs = p._refs;
            ref();
        }
        return *this;
    }

    SharedPtr(SharedPtr&& p) noexcept : _ptr(p._ptr), _refs(p._refs) {
        p._ptr = nullptr;
        p._refs = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& p) {
        if (*this != p) {
            unref();
            _ptr = p._ptr;
            _refs = p._refs;
            p._ptr = nullptr;
            p._refs = nullptr;
        }
        return *this;
    }

    T& operator*() const { return *_ptr; }
    T* operator->() const { return _ptr; }
    T* get() const { return _ptr; }

    bool operator==(const SharedPtr& other) const { return _ptr == other._ptr; }
    bool operator!=(const SharedPtr& other) const { return !(*this == other); }

    size_t count() const { return _refs->load(std::memory_order_acquire); }

private:
    void ref() {
        if (_refs != nullptr) 
            _refs->fetch_add(1, std::memory_order_relaxed); 
    }

    void unref() {
        if (_refs && _refs->fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete _ptr;
            delete _refs;
        }
    }

private:
    T* _ptr;
    std::atomic<size_t>* _refs;
};
