#pragma once

#include "pch.hpp"

#include "alloc/alloc.hpp"
#include "alloc/arena.hpp"

namespace stl
{
template<typename T>
constexpr inline static size_t num_size(T val)
{
    static_assert(std::is_integral_v<T>, "T is not an int");

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
    String(stl::alloc::Arena *arena);
    String(size_t capacity, stl::alloc::Arena *arena);
    String(const char *str, size_t size, stl::alloc::Arena *arena);
    String(const char *str, stl::alloc::Arena *arena);
    String(const String &o, stl::alloc::Arena *arena);
    String(String&& o);

public:
    static String make(const char* str, alloc::Arena *arena);
    static String make(const char* str, size_t size, alloc::Arena *arena);
    static String make(char ch, alloc::Arena *arena);
    static String make_buf(size_t size, alloc::Arena *arena);

    static void getline(std::istream &is, String &dest);

    static String copy(const String &src, alloc::Arena *arena);

    static String to_string(int32_t val, alloc::Arena *arena);
    static String to_string(size_t val, alloc::Arena *arena);
    static String to_string(float val, alloc::Arena *arena);

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


int32_t to_int(const String& str);
int32_t to_int(const StringView& str);
float to_float(const String& str);
float to_float(const StringView& str);

}

