#pragma once

#include "pch.hpp"

#include "alloc/alloc.hpp"
#include "alloc/arena.hpp"
#include <type_traits>

constexpr size_t BUF_SIZE = 58;

namespace stl
{
template<typename T> requires std::is_integral_v<T>
constexpr inline static size_t num_size(T val)
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
public:
    static String make(const char* str, alloc::Arena *arena);
    static String make(const char* str, size_t size, alloc::Arena *arena);
    static String make(char ch, alloc::Arena *arena);
    static String make_buf(size_t capacity, alloc::Arena *arena);

    static void getline(std::istream &is, String &dest);

    static String copy(const String &src, alloc::Arena *arena);

    template<typename T> requires std::is_integral_v<T>
    static String to_string(T val, alloc::Arena *arena)
    {
        size_t size = num_size(val);

        char *str = (char*)arena->alloc_buf_aligned(size + 1, alignof(char));

        auto [ptr, ec] = std::to_chars(str, str + size + 1, val);

        ASSERT(ec == std::errc{}, "Integer converstion failed");

        String res;

        res._arena = arena;
        res._str = str;
        res._size = size;
        res._capacity = size + 1;

        return res;
    }


    template<typename T> requires std::is_floating_point_v<T>
    static String to_string(T val, alloc::Arena *arena)
    {
        char buf[BUF_SIZE] {};

        auto [ptr, ec] = std::to_chars(buf, buf + BUF_SIZE, val);

        ASSERT(ec == std::errc{}, "Integer converstion failed");

        size_t size = std::strlen(buf);

        char *str = (char*)arena->alloc_buf_aligned(size, alignof(char));
        std::strcpy(str, buf);
        str[size] = '\0';

        String res;

        res._arena = arena;
        res._str = str;
        res._size = size;
        res._capacity = size + 1;

        return res;
    }

    static String concat(const String &lhs, const String &rhs, alloc::Arena *arena);

public:
    char& operator[](size_t index);

    String& operator+=(char c);
    String& operator+=(const String& o);

    const alloc::Arena *arena() const;

    friend std::ostream &operator<<(std::ostream &os, const String &p);

    friend String operator+(const String &lhs, const String &rhs);
    friend String operator+(const String &lhs, const char *rhs);
    friend String operator+(const String &lhs, char rhs);
    friend String operator+(const char *lhs, const String &rhs);

    friend bool operator==(const String &lhs, const String &rhs);
    friend bool operator!=(const String &lhs, const String &rhs);

    operator std::string_view() const noexcept;

public:
    String& to_upper();
    String& to_lower();

    String& char_to_upper(size_t index);
    String& char_to_lower(size_t index);

    void reset();

public:
    /* Iterator stuff */
    size_t size() const;

    const char *data() const;
    char *data();

    const char* begin() const;
    char* begin();

    const char* end() const;
    char* end();


private:
    bool needResize(size_t incr = 1) const;
    void resize();
    void resize(size_t incr);

private:
    stl::alloc::Arena *_arena;
    size_t _size;
    size_t _capacity;
    char *_str;
};


class StringView
{
public:
    char *data;
    size_t size;

public:
    inline static StringView make(String &src, stl::alloc::Arena *arena)
    {
        char *str = (char*)arena->alloc_buf_aligned(src.size() + 1, alignof(char));
        std::strncpy(str, src.data(), src.size());
        str[src.size()] = '\0';

        return {
            .data = str,
            .size = src.size(),
        };
    }

    inline static StringView from_string(String &src)
    {
        return {
            .data = src.data(),
            .size = src.size(),
        };
    }
};


i32 to_int(const String& str);
i32 to_int(const StringView& str);
f32 to_float(const String& str);
f32 to_float(const StringView& str);

}

