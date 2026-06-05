#include "string.hpp"
#include <charconv>


namespace stl
{
    String String::make(const char* str, alloc::Arena *arena)
    {
        size_t size = std::strlen(str);

        char *_str = (char*)arena->alloc_buf_aligned(size, alignof(char));
        std::strncpy(_str, str, size);
        _str[size] = '\0';

        String res;

        res._arena = arena;
        res._str = _str;
        res._size = size;
        res._capacity = size + 1;

        return res;
    }

    String String::make(const char* str, size_t size, alloc::Arena *arena)
    {
        char *_str = (char*)arena->alloc_buf_aligned(size, alignof(char));
        std::strncpy(_str, str, size);
        _str[size] = '\0';

        String res;

        res._arena = arena;
        res._str = _str;
        res._size = size;
        res._capacity = size + 1;

        return res;
    }

    String String::make(char ch, alloc::Arena *arena)
    {
        String res;
        char *str = arena->alloc<char>(2);
        str[0] = ch;

        res._arena = arena;
        res._str = str;
        res._capacity = 2;
        res._size = 1;
        res._str[res._size] = '\0';

        return res;
    }

    String String::make_buf(size_t capacity, alloc::Arena *arena)
    {
        char *_str = (char*)arena->alloc_buf_aligned(capacity, alignof(char));
        _str[0] = '\0';

        String res;

        res._arena = arena;
        res._str = _str;
        res._size = 0;
        res._capacity = capacity + 1;

        return res;
    }

    void String::getline(std::istream &is, String &dest)
    {
        char ch;
        size_t size = 0;
        while ((ch = is.get()) != '\n')
        {
            if (size > dest._capacity) break;

            dest._str[size] = ch;
            size++;
        }

        dest._str[size] = '\0';
        dest._size = size;
    }

    String String::copy(const String &src, alloc::Arena *arena)
    {
        char *_str = (char*)arena->alloc_buf_aligned(src._size + 1, alignof(char));
        std::strncpy(_str, src._str, src._size);
        _str[src._size] = '\0';

        String res;

        res._arena = arena;
        res._str = _str;
        res._size = src._size;
        res._capacity = src._size + 1;

        return res;
    }

    String String::concat(const String &lhs, const String &rhs, alloc::Arena *arena)
    {
        size_t size = lhs._size + rhs._size;
        size_t capacity = size + 1;
        char *str = (char*)arena->alloc_buf_aligned(capacity, alignof(char));

        std::strcpy(str, lhs._str);
        std::strcpy(str + lhs._size, rhs._str);

        str[size] = '\0';

        String res;

        res._arena = arena;
        res._str = str;
        res._size = size;
        res._capacity = capacity;

        return res;
    }

    char& String::operator[](size_t index)
    {
        ASSERT(index < this->_size, "Index out of bounds");
        return this->_str[index];
    }

    [[gnu::hot]] String& String::operator+=(char c)
    {
        if (this->needResize())
            this->resize();

        this->_str[this->_size++] = c;
        this->_str[this->_size] = '\0';

        return *this;
    }

    [[gnu::hot]] String& String::operator+=(const String& o)
    {
        if (this->needResize(o._size))
            this->resize(o._size);

        std::strcpy(this->_str + this->_size, o._str);
        this->_size += o._size;
        this->_str[this->_size] = '\0';

        return *this;
    }

    const alloc::Arena *String::arena() const
    {
        return this->_arena;
    }

    String::operator std::string_view() const noexcept
    {
        return { this->_str, this->_size };
    }

    // constexpr operator StringView() const noexcept
    // {
    //     return { .data = this->_str, .size = this->_size };
    // }

    String& String::to_upper()
    {
        for (size_t i = 0; i < this->_size; i++)
        {
            this->_str[i] = std::toupper(static_cast<unsigned char>(this->_str[i]));
        }

        return *this;
    }

    String& String::to_lower()
    {
        for (size_t i = 0; i < this->_size; i++)
        {
            this->_str[i] = std::tolower(static_cast<unsigned char>(this->_str[i]));
        }

        return *this;
    }

    String& String::char_to_upper(size_t index)
    {
        this->_str[index] = std::toupper(static_cast<unsigned char>(this->_str[index]));

        return *this;
    }

    String& String::char_to_lower(size_t index)
    {
        this->_str[index] = std::tolower(static_cast<unsigned char>(this->_str[index]));

        return *this;
    }

    void String::reset()
    {
        this->_size = 0;
        this->_str[this->_size] = '\0';
    }

    /* Iterator stuff */
    size_t String::size() const
    {
        return this->_size;
    }

    const char *String::data() const
    {
        return this->_str;
    }

    char *String::data()
    {
        return this->_str;
    }

    const char* String::begin() const
    {
        return this->_str;
    }

    char* String::begin()
    {
        return this->_str;
    }

    const char* String::end() const
    {
        return this->_str + this->_size;
    }

    char* String::end()
    {
        return this->_str + this->_size;
    }


    bool String::needResize(size_t incr) const
    {
        return this->_size + incr > this->_capacity;
    }

    void String::resize()
    {
        this->_capacity *= 1.5f;

        char *oldStr = this->_str;
        this->_str = (char*)this->_arena->alloc_buf_aligned(this->_capacity, alignof(char));

        std::strcpy(this->_str, oldStr);
        this->_str[this->_size] = '\0';
    }

    void String::resize(size_t incr = 1)
    {
        this->_capacity += incr;
        this->_size += incr;

        char *oldStr = this->_str;
        this->_str = (char*)this->_arena->alloc_buf_aligned(this->_capacity, alignof(char));

        std::strcpy(this->_str, oldStr);
        this->_str[this->_size] = '\0';
    }


String operator+(const String &lhs, const String &rhs)
{
    size_t size = lhs._size + rhs._size;
    size_t capacity = size + 1;
    auto *arena = lhs._arena;
    char *str = (char*)arena->alloc_buf_aligned(capacity, alignof(char));

    std::strncpy(str, lhs._str, lhs._size);
    std::strncpy(str + lhs._size, rhs._str, rhs._size);

    str[size] = '\0';

    String res;

    res._arena = arena;
    res._str = str;
    res._size = size;
    res._capacity = capacity;

    return res;
}

String operator+(const char *lhs, const String &rhs)
{
    size_t lhsSize = std::strlen(lhs);
    size_t size = lhsSize + rhs._size;
    size_t capacity = size + 1;

    auto *arena = rhs._arena;
    char *str = (char*)arena->alloc_buf_aligned(capacity, alignof(char));

    std::strncpy(str, lhs, lhsSize);
    std::strncpy(str + lhsSize, rhs._str, rhs._size);

    str[size] = '\0';

    String res;

    res._arena = arena;
    res._str = str;
    res._size = size;
    res._capacity = capacity;

    return res;
}

String operator+(const String &lhs, const char *rhs)
{
    size_t rhsSize = std::strlen(rhs);
    size_t size = lhs._size + rhsSize;
    size_t capacity = size + 1;

    auto *arena = lhs._arena;
    char *str = (char*)arena->alloc_buf_aligned(capacity, alignof(char));

    std::strncpy(str, lhs._str, lhs._size);
    std::strncpy(str + lhs._size, rhs, rhsSize);

    str[size] = '\0';

    String res;

    res._arena = arena;
    res._str = str;
    res._size = size;
    res._capacity = capacity;

    return res;
}

String operator+(const String &lhs, char rhs)
{
    size_t rhsSize = 1;
    size_t size = lhs._size + rhsSize;
    size_t capacity = size + 1;

    auto *arena = lhs._arena;
    char *str = (char*)arena->alloc_buf_aligned(capacity, alignof(char));

    std::strncpy(str, lhs._str, lhs._size);
    str[lhs._size] = rhs;

    str[size] = '\0';

    String res;

    res._arena = arena;
    res._str = str;
    res._size = size;
    res._capacity = capacity;

    return res;
}


bool operator==(const String &lhs, const String &rhs)
{
    if (lhs._str == rhs._str) return true;
    if (lhs._size != rhs._size) return false;

    return std::strncmp(lhs._str, rhs._str, lhs._size);
}

bool operator!=(const String &lhs, const String &rhs)
{
    if (lhs._str == rhs._str) return false;

    return !std::strncmp(lhs._str, rhs._str, lhs._size);
}

std::ostream &operator<<(std::ostream &os, const String &p)
{
    for (size_t i = 0; i < p._size; i++)
    {
        os << p._str[i];
    }

    return os;
}

i32 to_int(const String &str)
{
    i32 result {};

    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    ASSERT(ec == std::errc{}, "Integer converstion failed");

    return result;
}

i32 to_int(const StringView &str)
{
    i32 result {};

    auto [ptr, ec] = std::from_chars(str.data, str.data + str.size, result);
    ASSERT(ec == std::errc{}, "Integer converstion failed");

    return result;
}

f32 to_float(const String &str)
{
    f32 result {};

    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    ASSERT(ec == std::errc{}, "Integer converstion failed");

    return result;
}

f32 to_float(const StringView &str)
{
    f32 result {};

    auto [ptr, ec] = std::from_chars(str.data, str.data + str.size, result);
    ASSERT(ec == std::errc{}, "Integer converstion failed");

    return result;
}
}
