#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include "debug.hpp"

namespace stl::alloc
{

class Arena
{
public:
    Arena() = delete;
    Arena(const Arena&) = delete;
    Arena(Arena&&) = delete;

    Arena(size_t bytes)
        : _size(bytes)
          , _arena(new std::byte[_size])
          , _top(_arena)
    { }

    ~Arena()
    {
        if (this->_top != this->_arena)
        {
            this->free_all();
        }

        delete[] this->_arena;
    }

public:
    constexpr inline void free_all()
    {
        this->_top = this->_arena;
    }

    constexpr inline std::byte *const alloc_buf(size_t bytes)
    {
        ASSERT((this->_top + bytes) > (std::byte*)this->_size, "Out of memory");

        std::byte *ptr = this->_top;
        std::memset(ptr, 0, bytes);

        this->_top += bytes;

        return ptr;
    }

    constexpr inline std::byte *const alloc_buf_aligned(size_t bytes, size_t align)
    {
        uintptr_t offset = this->align_forward((uintptr_t)this->_top, align);

        ASSERT(offset + bytes > this->_size, "Out of memory");

        this->_top = reinterpret_cast<std::byte*>(offset);
        std::byte *ptr = this->_top;
        std::memset(ptr, 0, bytes);

        this->_top += bytes;

        return ptr;
    }

    template<typename T>
    constexpr inline T *const alloc(size_t elements)
    {
        const size_t size = sizeof(T) * elements;
        ASSERT((this->_top + size) < (std::byte*)(this->_arena + this->_size), "Out of memory");

        T *ptr = reinterpret_cast<T*>(this->_top);
        std::memset(ptr, 0, size);

        this->_top += size;

        return ptr;
    }

    template<typename T>
    constexpr inline T *const alloc_aligned(size_t elements)
    {
        const size_t size = sizeof(T);
        const size_t alignment = alignof(T);

        uintptr_t offset = this->align_forward((uintptr_t)this->_top, alignment);

        ASSERT(offset + size < (uintptr_t)(this->_arena + this->_size), "Out of memory");

        this->_top = reinterpret_cast<std::byte*>(offset);
        T *ptr = reinterpret_cast<T*>(this->_top);
        std::memset(ptr, 0, size);

        this->_top += size;

        return ptr;
    }

    template<class T, class ...Args>
    constexpr inline T *const make(Args&&... args)
    {
        const size_t size = sizeof(T);
        ASSERT((this->_top + size) < (std::byte*)(this->_arena + this->_size), "Out of memory");

        std::byte *ptr = this->_top;
        this->_top += size;

        return new (ptr) T(std::forward<Args>(args)...);
    }

    template<class T, class ...Args>
    constexpr inline T *const make_aligned(Args&&... args)
    {
        const size_t size = sizeof(T);
        const size_t alignment = alignof(T);
        uintptr_t offset = this->align_forward((uintptr_t)this->_top, alignment);

        ASSERT(offset + size < (uintptr_t)(this->_arena + this->_size), "Out of memory");

        this->_top = reinterpret_cast<std::byte*>(offset);
        std::byte *ptr = this->_top;
        this->_top += size;

        return new (ptr) T(std::forward<Args>(args)...);
    }

    constexpr inline const size_t size() const
    {
        return this->_size;
    }

private:
    uintptr_t align_forward(uintptr_t ptr, size_t align) const
    {
        uintptr_t p = ptr;
        uintptr_t a = (uintptr_t)align;
        uintptr_t modulo = p & (a - 1);

        if (modulo != 0)
        {
            p += a - modulo;
        }

        return p;
    }

private:
    size_t _size;
    std::byte *_arena;
    std::byte *_top;
};


template<typename T>
class Pool
{
private:
    struct PoolNode
    {
        PoolNode *next;
    };

public:
    Pool(size_t size)
        : _count(size)
        , _arena(nullptr)
        , _pool(new std::byte[_count * _chunk_size])
    {
        // init the linked list
        // world's first good performance linked list - it's all in contiguous memory
        this->free_all();
    }

    Pool(size_t size, Arena *arena)
        : _count(size)
        , _arena(arena)
        , _pool(arena->alloc_buf_aligned(_count * _chunk_size, alignof(T)))
    {
        this->free_all();
    }

    ~Pool()
    {
        this->free_all();

        // if we didn't use the arena, then we just use the OS
        if (this->_arena == nullptr)
        {
            delete[] this->_pool;
        }
    }


    template<class ...Args>
    T *const alloc(Args&&... args)
    {
        PoolNode *node = this->_head;

        ASSERT(node != nullptr, "Out of memory");

        this->_head = this->_head->next;

        return new (node) T(std::forward<Args>(args)...);
    }

    void free(T *const ptr)
    {
        if (ptr == nullptr) return;

        std::byte *start = this->_pool;
        std::byte *end = &this->_pool[this->_count * this->_chunk_size];

        ASSERT((start <= (std::byte*)ptr && (std::byte*)ptr < end), "Pointer does not belong to this pool");

        ptr->~T(); // we go out of our way to actually call the destructor here,
                   // although it would be preferable to skip this entirely

        // This is invalid C++, by the way.
        // C++ had a whole system for tracking which objects
        // actually exist, as in were created through a constructor.
        // If they weren't created through a constructor,
        // they might as well not exist.
        //
        // This code still works though,
        // and since this only holds a pointer,
        // we don't really care what C++ thinks.
        PoolNode *node = reinterpret_cast<PoolNode*>(ptr);
        node->next = this->_head;
        this->_head = node;
    }

    void free_all()
    {
        for (size_t i = 0; i < this->_count; i++)
        {
            std::byte *ptr = &this->_pool[i + this->_chunk_size];
            PoolNode *node = reinterpret_cast<PoolNode*>(ptr);
            node->next = this->_head;
            this->_head = node;
        }
    }

    const size_t capacity() const
    {
        return this->_count;
    }

private:
    size_t _chunk_size = sizeof(T);
    size_t _count;
    Arena *_arena;
    std::byte *_pool;

    PoolNode *_head = nullptr;
};


}
