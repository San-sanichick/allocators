#pragma once

#include "utils.hpp"

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
        uintptr_t offset = align_forward((uintptr_t)this->_top, align);

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

        uintptr_t offset = align_forward((uintptr_t)this->_top, alignment);

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
        uintptr_t offset = align_forward((uintptr_t)this->_top, alignment);

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
    size_t _size;
    std::byte *_arena;
    std::byte *_top;
};
}
