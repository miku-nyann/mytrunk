#pragma once 

#include <cstddef>
#include <cassert>
#include <vector>
#include <atomic>

template <typename T, size_t Capacity>
class RingBuffer
{
    static_assert(!(Capacity & (Capacity - 1)), "Capacity must be a power of 2");

public:
    RingBuffer() : _buffer(Capacity), _head(0), _tail(0) {}

    bool push(T&& v) {
        size_t oldtail = _tail.load(std::memory_order_acquire);
        if (full())
            return false;

        _buffer[oldtail] = std::move(v);

        size_t newtail = (oldtail + 1) & (Capacity - 1);
        if (_tail.compare_exchange_strong(oldtail, newtail, std::memory_order_release)) {
            _size.fetch_add(1, std::memory_order_release);
            return true;
        }
        return false;
    }

    bool pop(T& v) {
        size_t oldhead = _head.load(std::memory_order_acquire);
        if (empty())
            return false;
        
        v = std::move(_buffer[oldhead]);

        size_t newhead = (oldhead + 1) & (Capacity - 1);
        if (_head.compare_exchange_strong(oldhead, newhead, std::memory_order_release)) {
            _size.fetch_sub(1, std::memory_order_release);
            return true;
        }
        return false;
    }

private:
    inline bool full() const { return _size.load(std::memory_order_acquire) == Capacity; }

    inline bool empty() const { return _size.load(std::memory_order_acquire) == 0; }

private:
    std::vector<T> _buffer;
    std::atomic<size_t> _head;
    std::atomic<size_t> _tail;
    std::atomic<size_t> _size;
};
