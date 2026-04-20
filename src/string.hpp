#pragma once

#include <cstring>
#include "alloc/alloc.hpp"
#include "alloc/arena.hpp"



namespace stl
{

class String
{
private:
    String(stl::alloc::Arena *arena)
        : _arena(arena)
        , _size(0)
        , _capacity(1)
        , _str(nullptr)
    {}

    String(const char *str, size_t size, stl::alloc::Arena *arena)
        : _arena(arena)
        , _size(size)
        , _capacity(_size + 1)
        , _str((char*)arena->alloc_buf_aligned(_capacity, alignof(char)))
    {
        std::strcpy(this->_str, str);
        this->_str[this->_size] = '\0';
    }

    String(const char *str, stl::alloc::Arena *arena)
        : _arena(arena)
        , _size(std::strlen(str))
        , _capacity(_size + 1)
        , _str((char*)arena->alloc_buf_aligned(_capacity, alignof(char)))
    {
        std::strcpy(this->_str, str);
        this->_str[this->_size] = '\0';
    }

    String(const String &o, stl::alloc::Arena *arena)
        : _arena(arena)
        , _size(o._size)
        , _capacity(o._size + 1)
        , _str((char*)arena->alloc_buf_aligned(o._capacity, alignof(char)))
    {
        std::strcpy(this->_str, o._str);
        this->_str[this->_size] = '\0';
    }

    String(String&& o)
        : _arena(o._arena)
        , _size(o._size)
        , _capacity(o._capacity)
        , _str(o._str)
    {
        o._str = nullptr;
        o._size = 0;
    }

public:
    inline static String make(const char* str, alloc::Arena *arena)
    {
        return String(str, arena);
    }

    inline static String copy(const String &src, alloc::Arena *arena)
    {
        return String(src, arena);
    }

    inline static String concat(const String &lhs, const String &rhs, alloc::Arena *arena)
    {
        size_t size = lhs._size + rhs._size + 1;
        char *str = (char*)arena->alloc_buf_aligned(size, alignof(char));

        std::strcpy(str, lhs._str);
        std::strcpy(str + lhs._size, rhs._str);

        str[size] = '\0';

        String res(arena);

        res._str = str;
        res._size = size;
        res._capacity = size + 1;

        return res;
    }

public:
    [[gnu::hot]] constexpr inline char& operator[](size_t index)
    {
        ASSERT(index < this->_size, "Index out of bounds");
        return this->_str[index];
    }

    [[gnu::hot]] constexpr inline String& operator+=(char c)
    {
        if (this->needResize())
            this->resize();

        this->_str[this->_size++] = c;
        this->_str[this->_size] = '\0';

        return *this;
    }

    [[gnu::hot]] constexpr inline String& operator+=(const String& o)
    {
        if (this->needResize(o._size))
            this->resize(o._size);

        std::strcpy(this->_str + this->_size, o._str);
        this->_size += o._size;
        this->_str[this->_size] = '\0';

        return *this;
    }

    constexpr inline size_t size() const
    {
        return this->_size;
    }

    constexpr inline const char *data() const
    {
        return this->_str;
    }

    constexpr inline const alloc::Arena *arena() const
    {
        return this->_arena;
    }

    friend std::ostream& operator<<(std::ostream& os, const String& p);
    friend String operator+(const String& lhs, const String& rhs);

    constexpr operator std::string_view() const noexcept
    {
        return { this->_str, this->_size };
    }

private:
    constexpr inline bool needResize(size_t incr = 1) const
    {
        return this->_size + incr > this->_capacity;
    }

    constexpr inline void resize()
    {
        this->_capacity *= 1.5f;

        char *oldStr = this->_str;
        this->_str = (char*)this->_arena->alloc_buf_aligned(this->_capacity, alignof(char));

        std::strcpy(this->_str, oldStr);
        this->_str[this->_size] = '\0';
    }

    constexpr inline void resize(size_t incr)
    {
        this->_capacity += incr;
        this->_size += incr;

        char *oldStr = this->_str;
        this->_str = (char*)this->_arena->alloc_buf_aligned(this->_capacity, alignof(char));

        std::strcpy(this->_str, oldStr);
        this->_str[this->_size] = '\0';
    }

private:
    stl::alloc::Arena *_arena;
    size_t _size;
    size_t _capacity;
    char *_str;
};

inline String operator+(const String& lhs, const String& rhs)
{
    size_t size = lhs._size + rhs._size + 1;
    auto *arena = lhs._arena;
    char *str = (char*)arena->alloc_buf_aligned(size, alignof(char));

    std::strcpy(str, lhs._str);
    std::strcpy(str + lhs._size, rhs._str);

    str[size] = '\0';

    String res(arena);

    res._str = str;
    res._size = size;
    res._capacity = size + 1;

    return res;
}

inline std::ostream& operator<<(std::ostream& os, const String& p)
{
    for (size_t i = 0; i < p._size; i++)
    {
        os << p._str[i];
    }

    return os;
}

}

