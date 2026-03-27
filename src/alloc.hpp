#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include "debug.hpp"

#ifndef defer

template <typename T>
struct deferrer
{
	T f;
	deferrer(T f) : f(std::move(f)) { };
	deferrer(const deferrer&) = delete;
	~deferrer() { f(); }
};

#define TOKEN_CONCAT_NX(a, b) a ## b
#define TOKEN_CONCAT(a, b) TOKEN_CONCAT_NX(a, b)
#define defer deferrer TOKEN_CONCAT(__deferred, __COUNTER__) =

#endif

namespace stl
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
            this->freeAll();
        }

        delete[] this->_arena;
    }

public:
    constexpr inline void freeAll()
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

    // TODO: implement alignment
    template<typename T>
    constexpr inline T *const alloc(size_t elements)
    {
        const size_t size = sizeof(T) * elements;
        ASSERT((this->_top + size) > (std::byte*)this->_size, "Out of memory");

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

        ASSERT(offset + size > this->_size, "Out of memory");

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
        ASSERT((this->_top + size) > (std::byte*)this->_size, "Out of memory");

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

        ASSERT(offset + size > this->_size, "Out of memory");

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

}
