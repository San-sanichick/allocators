#pragma once

#include "pch.hpp"
#include "stl/alloc/alloc.hpp"


constexpr float RESIZE_COEFF = 1.8f;



namespace stl::container
{
template<typename T>
class Vector
{
TYPE_IS_TRIVIAL(T);

public:
    Vector(alloc::Arena *arena)
        : _capacity(1)
        , _size(0)
        , _arena(arena)
        , _data(reinterpret_cast<T*>(_arena->alloc_buf_aligned(_capacity * sizeof(T), alignof(T))))
    { }

    Vector(size_t capacity, alloc::Arena *arena)
        : _capacity(capacity)
        , _size(0)
        , _arena(arena)
        , _data(static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * _capacity)))
    { }

    [[gnu::hot]] constexpr inline T& operator[](size_t index)
    {
        ASSERT(index < this->_size, "Index out of bounds");
        return this->_data[index];
    }

    [[gnu::hot]] constexpr inline const T& operator[](size_t index) const
    {
        ASSERT(index < this->_size, "Index out of bounds");
        return this->_data[index];
    }


    [[gnu::hot]] constexpr inline void pushBack(const T& el)
    {
        if (this->needsResize())
            this->_resize();

        this->_data[this->_size] = el;
        this->_size++;
    }

    [[gnu::hot]] constexpr inline void pushBack(T&& el)
    {
        if (this->needsResize())
            this->_resize();

        this->_data[this->_size] = std::move(el);
        this->_size++;
    }

    template<class ...Args>
    [[gnu::hot]] constexpr inline void emplaceBack(Args&&... args)
    {
        if (this->needsResize())
            this->_resize();

        // NOTE: we use this neat thing to construct objects at already existing memory
        T* ptr = this->_data + this->_size;
        std::construct_at(ptr, std::forward<Args>(args)...);

        this->_size++;
    }

    [[gnu::hot]] constexpr inline void popBack()
    {
        if (this->_size == 0) return;

        this->_size--;
    }


    [[gnu::hot]] constexpr inline T *erase(size_t index)
    {
        for (; index < this->_size - 1; index++)
        {
            this->_data[index] = this->_data[index + 1];
        }

        this->_size--;

        return &this->_data[index];
    }


    [[gnu::hot]] constexpr inline T *erase(T *it)
    {
        size_t offset = it - this->_data;

        for (; offset < this->_size - 1; offset++)
        {
            this->_data[offset] = this->_data[offset + 1];
        }

        this->_size--;

        return &this->_data[offset];
    }


    [[gnu::hot]] constexpr inline T *erase(T *first, T *last)
    {
        size_t startOffset = first - this->_data;
        size_t endOffset = last - this->_data;
        size_t offset = endOffset - startOffset;

        for (; startOffset < this->_size - 1; startOffset++)
        {
            this->_data[startOffset] = this->_data[startOffset + offset];
        }

        this->_size -= offset;

        return &this->_data[startOffset];
    }


    [[gnu::hot]] constexpr inline T *erase(size_t startOffset, size_t endOffset)
    {
        size_t offset = endOffset - startOffset;

        for (; startOffset < this->_size - 1; startOffset++)
        {
            this->_data[startOffset] = this->_data[startOffset + offset];
        }

        this->_size -= offset;

        return &this->_data[startOffset];
    }


    constexpr inline void shrinkToFit()
    {
        if (this->_capacity == this->_size) return;

        this->_capacity = this->_size;

        T* oldData = this->_data;
        this->_data = reinterpret_cast<T*>(
            this->_arena->alloc_buf_aligned(this->_capacity * sizeof(T), alignof(T))
        );

        std::memcpy(this->_data, oldData, this->_capacity * sizeof(T));
    }

    constexpr inline void resize(size_t newSize)
    {
        this->_capacity = newSize;

        T* oldData = this->_data;
        this->_arena->alloc_buf_aligned(this->_capacity * sizeof(T), alignof(T));

        std::memcpy(this->_data, oldData, this->_capacity * sizeof(T));
    }

    constexpr inline void reserve(size_t newSize)
    {
        this->_size = 0;
        this->_capacity = newSize;

        this->_arena->alloc_buf_aligned(this->_capacity * sizeof(T), alignof(T));
    }

    constexpr inline size_t size() const
    {
        return this->_size;
    }

    constexpr inline size_t capacity() const
    {
        return this->_capacity;
    }

public:
    /* Iterator stuff */
    const T* begin() const
    {
        return this->_data;
    }

    T* begin()
    {
        return this->_data;
    }

    const T* end() const
    {
        return this->_data + this->_size;
    }

    T* end()
    {
        return this->_data + this->_size;
    }

private: 
    [[gnu::hot]] constexpr inline void _resize()
    {
        float newCapacity = std::ceil((float)this->_capacity * RESIZE_COEFF);
        this->_capacity = newCapacity;

        T* oldData = this->_data;
        this->_arena->alloc_buf_aligned(this->_capacity * sizeof(T), alignof(T));

        std::memcpy(this->_data, oldData, this->_capacity * sizeof(T));
    }

    constexpr inline bool needsResize()
    {
        return this->_size + 1 > this->_capacity;
    }

private:
    size_t _capacity;
    size_t _size;
    alloc::Arena *_arena;
    T *_data;
};

// template<typename T>
// class Vector
// {
// public:
//     Vector()
//         : _capacity(1)
//         , _size(0)
//         , _data(static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * _capacity)))
//     { }
//
//     Vector(size_t capacity)
//         : _capacity(capacity)
//         , _size(0)
//         , _data(static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * _capacity)))
//     { }
//
//     Vector(const Vector& o)
//         : _capacity(o._capacity)
//         , _size(o._size)
//         , _data(static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * _capacity)))
//     {
//         for (size_t i = 0; i < this->_size; i++)
//         {
//             T* ptr = this->_data + i;
//             std::construct_at(ptr, o[i]);
//         }
//     }
//
//     Vector(Vector&& o)
//         : _capacity(o._capacity)
//         , _size(o._size)
//         , _data(o._data)
//     {
//         o._capacity = 1;
//         o._size = 0;
//         o._data = static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * _capacity));
//     }
//
//     ~Vector()
//     {
//         if (this->_size != 0)
//         {
//             // NOTE: std::vector is required to call destructors in reverse order,
//             // which is what we do. Also we have to do this manually, since we used
//             // std::malloc to allocate this memory, and as such, C++ is not even aware that
//             // it's not an array of bytes, but actually an array of objects.
//             // In C this would be much simpler, since there are no constructors or destructors
//             size_t i = this->_size - 1;
//             do
//             {
//                 this->_data[i].~T();
//                 i--;
//             }
//             while (i != 0);
//         }
//
//         std::free(this->_data);
//     }
//
//
//     [[gnu::hot]] constexpr inline T& operator[](size_t index)
//     {
//         ASSERT(index < this->_size, "Index out of bounds");
//         return this->_data[index];
//     }
//
//     [[gnu::hot]] constexpr inline const T& operator[](size_t index) const
//     {
//         ASSERT(index < this->_size, "Index out of bounds");
//         return this->_data[index];
//     }
//
//
//     constexpr inline void pushBack(const T& el)
//     {
//         if (this->needsResize())
//             this->_resize();
//
//         this->_data[this->_size] = el;
//         this->_size++;
//     }
//
//     [[gnu::hot]] constexpr inline void pushBack(T&& el)
//     {
//         if (this->needsResize())
//             this->_resize();
//
//         this->_data[this->_size] = std::move(el);
//         this->_size++;
//     }
//
//     template<class ...Args>
//     [[gnu::hot]] constexpr inline void emplaceBack(Args&&... args)
//     {
//         if (this->needsResize())
//             this->_resize();
//
//         // NOTE: we use this neat thing to construct objects at already existing memory
//         T* ptr = this->_data + this->_size;
//         std::construct_at(ptr, std::forward<Args>(args)...);
//
//         this->_size++;
//     }
//
//     constexpr inline void popBack()
//     {
//         if (this->_size == 0) return;
//
//         // bruh
//         this->_data[this->_size - 1].~T(); // same thing as in the destructor
//         this->_size--;
//     }
//
//     constexpr inline void shrinkToFit()
//     {
//         if (this->_capacity == this->_size) return;
//
//         this->_capacity = this->_size;
//
//         T* oldData = this->_data;
//         this->_data = static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * this->_capacity));
//         
//         for (size_t i = 0; i < this->_size; i++)
//         {
//             T* ptr = this->_data + i;
//             std::construct_at(ptr, std::move(oldData[i]));
//
//             oldData[i].~T();
//         }
//     }
//
//     constexpr inline void resize(size_t newSize)
//     {
//         this->_capacity = newSize;
//
//         T* oldData = this->_data;
//         this->_data = static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * this->_capacity));
//
//         for (size_t i = 0; i < this->_size; i++)
//         {
//             T* ptr = this->_data + i;
//             std::construct_at(ptr, std::move(oldData[i]));
//
//             oldData[i].~T();
//         }
//
//         std::free(oldData);
//     }
//
//     constexpr inline void reserve(size_t newSize)
//     {
//         if (this->_size != 0)
//         {
//             size_t i = this->_size - 1;
//             do
//             {
//                 this->_data[i].~T();
//                 i--;
//             }
//             while (i != 0);
//         }
//
//         std::free(this->_data);
//
//         this->_size = 0;
//         this->_capacity = newSize;
//
//         this->_data = static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * this->_capacity));
//     }
//
//     constexpr inline size_t size() const
//     {
//         return this->_size;
//     }
//
//     constexpr inline size_t capacity() const
//     {
//         return this->_capacity;
//     }
//
// public:
//     /* Iterator stuff */
//     const T* begin() const
//     {
//         return this->_data;
//     }
//
//     T* begin()
//     {
//         return this->_data;
//     }
//
//     const T* end() const
//     {
//         return this->_data + this->_size;
//     }
//
//     T* end()
//     {
//         return this->_data + this->_size;
//     }
//
// private: 
//     constexpr inline void _resize()
//     {
//         float newCapacity = std::ceil((float)this->_capacity * RESIZE_COEFF);
//         this->_capacity = newCapacity;
//
//         T* oldData = this->_data;
//         this->_data = static_cast<T*>(std::aligned_alloc(alignof(T), sizeof(T) * this->_capacity));
//
//         for (size_t i = 0; i < this->_size; i++)
//         {
//             T* ptr = this->_data + i;
//             std::construct_at(ptr, std::move(oldData[i]));
//
//             oldData[i].~T();
//         }
//     }
//
//     constexpr inline bool needsResize()
//     {
//         return this->_size + 1 > this->_capacity;
//     }
//
// private:
//     size_t _capacity;
//     size_t _size;
//     T* _data;
// };
}
