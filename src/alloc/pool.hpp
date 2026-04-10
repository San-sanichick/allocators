#pragma once

#include "utils.hpp"
#include "arena.hpp"


namespace stl::alloc
{

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
        // C++ has a whole system for tracking which objects
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
