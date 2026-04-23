#pragma once

#include "arena.hpp"
#include "debug.hpp"
#include "utils.hpp"
#include <cstdint>


namespace stl::alloc
{

class Stack
{
private:
    struct StackHeader
    {
        size_t prevOffset;
        size_t padding;
    };

public:
    Stack(size_t size, alloc::Arena *arena)
        : _size(size)
        , _offset(0)
        , _prevOffset(0)
        , _arena(arena)
        , _stack(_arena->alloc_buf(_size))
    {}

    template<typename T>
    T *alloc(size_t size)
    {
        size_t align = alignof(T);

        ASSERT(is_power_of_two(align), "Incorrect alignment");

        if (align > 128) align = 128;

        uintptr_t currAddr = reinterpret_cast<uintptr_t>(this->_stack) + reinterpret_cast<uintptr_t>(this->_offset);
        uintptr_t padding = this->calcPaddingWithHeader(currAddr, align, sizeof(StackHeader));

        ASSERT(this->_offset + padding + size < this->_size, "Out of memory");

        this->_prevOffset = this->_offset;
        this->_offset += padding;

        uintptr_t nextAddr = currAddr + static_cast<uintptr_t>(padding);

        StackHeader *header = reinterpret_cast<StackHeader*>(nextAddr - sizeof(StackHeader));
        header->padding = static_cast<uint8_t>(padding);
        header->prevOffset = this->_prevOffset;

        this->_offset += size;

        return (T*)std::memset(reinterpret_cast<std::byte*>(nextAddr), 0, size);
    }


    template<class T, class ...Args>
    T *make(Args&&... args)
    {
        size_t align = alignof(T);
        size_t size = sizeof(T);

        ASSERT(is_power_of_two(align), "Incorrect alignment");

        if (align > 128) align = 128;

        uintptr_t currAddr = reinterpret_cast<uintptr_t>(this->_stack) + static_cast<uintptr_t>(this->_offset);
        uintptr_t padding = this->calcPaddingWithHeader(currAddr, align, sizeof(StackHeader));

        ASSERT(this->_offset + padding + size < this->_size, "Out of memory");

        this->_prevOffset = this->_offset;
        this->_offset += padding;

        uintptr_t nextAddr = currAddr + (uintptr_t)padding;

        StackHeader *header = reinterpret_cast<StackHeader*>(nextAddr - sizeof(StackHeader));
        header->padding = static_cast<uint8_t>(padding);
        header->prevOffset = this->_prevOffset;

        this->_offset += size;

        return new (reinterpret_cast<std::byte*>(nextAddr)) T(std::forward<Args>(args)...);
    }

    template<typename T>
    void free(T *ptr)
    {
        ASSERT(ptr != nullptr, "ptr is null");

        // BUG: here's a question - what happens when you call a destructor
        // on a pointer to an array?
        // That's right, it'll destroy only the first element in that array

        // ptr->~T(); // we can only do this on non-trivial types

        uintptr_t start = reinterpret_cast<uintptr_t>(this->_stack);
        uintptr_t end = start + static_cast<uintptr_t>(this->_size);
        uintptr_t currAddr = reinterpret_cast<uintptr_t>(ptr);

        ASSERT(start <= currAddr && currAddr <= end, "Out of bounds pointer");

        if (currAddr >= start + static_cast<uintptr_t>(this->_offset))
        {
            return;
        }

        StackHeader *header = reinterpret_cast<StackHeader*>(currAddr - sizeof(StackHeader));

        size_t prevOffset = static_cast<size_t>(currAddr - static_cast<uintptr_t>(header->padding) - start);

        ASSERT(prevOffset == header->prevOffset, "Out of order free");

        this->_offset = prevOffset;
        this->_prevOffset = header->prevOffset;
    }

    void freeAll()
    {
        this->_offset = 0;
    }

private:
    size_t calcPaddingWithHeader(uintptr_t ptr, uintptr_t align, size_t headerSize)
    {
        ASSERT(is_power_of_two(align), "Invalid alignment");

        uintptr_t p = ptr;
        uintptr_t a = align;
        uintptr_t modulo = p & (a - 1);

        uintptr_t padding = 0;

        if (modulo != 0) padding = a - modulo;

        uintptr_t neededSpace = headerSize;

        if (padding < neededSpace)
        {
            neededSpace -= padding;

            if ((neededSpace & (a - 1)) != 0)
            {
                padding += a * (1 + (neededSpace / a));
            }
            else
            {
                padding += a * (neededSpace / a);
            }
        }

        return static_cast<size_t>(padding);
    }

private:
    size_t _size;
    size_t _offset;
    size_t _prevOffset;
    alloc::Arena *_arena;
    std::byte *_stack;
};

}

