#pragma once

#include <charconv>
#include <cstdint>
#include <cstring>
#include "alloc/alloc.hpp"
#include "alloc/arena.hpp"

constexpr size_t BUF_SIZE = 58;

namespace stl
{
template<typename T>
inline static size_t num_size(T val)
{
    size_t size = 0;
    do
    {
        size += 1;
        val /= 10;
    }
    while (val != 0);

    return size;
}

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

    inline static String to_string(int32_t val, alloc::Arena *arena)
    {
        size_t size = num_size(val);

        char *str = (char*)arena->alloc_buf_aligned(size, alignof(char));

        auto [ptr, ec] = std::to_chars(str, str + size, val);

        ASSERT(ec == std::errc{}, "Integer converstion failed");

        String res(arena);

        res._str = str;
        res._size = size;
        res._capacity = size + 1;

        return res;
    }

    inline static String to_string(size_t val, alloc::Arena *arena)
    {
        size_t size = num_size(val);

        char *str = (char*)arena->alloc_buf_aligned(size, alignof(char));

        auto [ptr, ec] = std::to_chars(str, str + size, val);

        ASSERT(ec == std::errc{}, "Integer converstion failed");

        String res(arena);

        res._str = str;
        res._size = size;
        res._capacity = size + 1;

        return res;
    }

    inline static String to_string(float val, alloc::Arena *arena)
    {
        char buf[BUF_SIZE] {};

        auto [ptr, ec] = std::to_chars(buf, buf + BUF_SIZE, val);

        ASSERT(ec == std::errc{}, "Integer converstion failed");

        size_t size = std::strlen(buf);

        char *str = (char*)arena->alloc_buf_aligned(size, alignof(char));
        std::strcpy(str, buf);
        str[size] = '\0';

        String res(arena);

        res._str = str;
        res._size = size;
        res._capacity = size + 1;

        return res;
    }

    [[gnu::hot]] inline static String concat(const String &lhs, const String &rhs, alloc::Arena *arena)
    {
        size_t size = lhs._size + rhs._size;
        size_t capacity = size + 1;
        char *str = (char*)arena->alloc_buf_aligned(capacity, alignof(char));

        std::strcpy(str, lhs._str);
        std::strcpy(str + lhs._size, rhs._str);

        str[size] = '\0';

        String res(arena);

        res._str = str;
        res._size = size;
        res._capacity = capacity;

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

    friend std::ostream &operator<<(std::ostream &os, const String &p);

    friend String operator+(const String &lhs, const String &rhs);
    friend String operator+(const String &lhs, const char *rhs);
    friend String operator+(const char *lhs, const String &rhs);

    friend bool operator==(const String &lhs, const String &rhs);
    friend bool operator!=(const String &lhs, const String &rhs);

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

inline String operator+(const String &lhs, const String &rhs)
{
    size_t size = lhs._size + rhs._size;
    size_t capacity = size + 1;
    auto *arena = lhs._arena;
    char *str = (char*)arena->alloc_buf_aligned(capacity, alignof(char));

    std::strcpy(str, lhs._str);
    std::strcpy(str + lhs._size, rhs._str);

    str[size] = '\0';

    String res(arena);

    res._str = str;
    res._size = size;
    res._capacity = capacity;

    return res;
}

inline String operator+(const char *lhs, const String &rhs)
{
    size_t lhsSize = std::strlen(lhs);
    size_t size = lhsSize + rhs._size;
    size_t capacity = size + 1;

    auto *arena = rhs._arena;
    char *str = (char*)arena->alloc_buf_aligned(capacity, alignof(char));

    std::strcpy(str, lhs);
    std::strcpy(str + lhsSize, rhs._str);

    str[size] = '\0';

    String res(arena);

    res._str = str;
    res._size = size;
    res._capacity = capacity;

    return res;
}

inline String operator+(const String &lhs, const char *rhs)
{
    size_t rhsSize = std::strlen(rhs);
    size_t size = lhs._size + rhsSize;
    size_t capacity = size + 1;

    auto *arena = lhs._arena;
    char *str = (char*)arena->alloc_buf_aligned(capacity, alignof(char));

    std::strcpy(str, lhs._str);
    std::strcpy(str + lhs._size, rhs);

    str[size] = '\0';

    String res(arena);

    res._str = str;
    res._size = size;
    res._capacity = capacity;

    return res;
}

inline bool operator==(const String &lhs, const String &rhs)
{
    if (lhs._str == rhs._str) return true;
    if (lhs._size != rhs._size) return false;

    return std::strncmp(lhs._str, rhs._str, lhs._size);
}

inline bool operator!=(const String &lhs, const String &rhs)
{
    if (lhs._str == rhs._str) return false;

    return !std::strncmp(lhs._str, rhs._str, lhs._size);
}

inline std::ostream &operator<<(std::ostream &os, const String &p)
{
    for (size_t i = 0; i < p._size; i++)
    {
        os << p._str[i];
    }

    return os;
}

}

