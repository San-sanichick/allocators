#include "string.hpp"
#include <charconv>

constexpr size_t BUF_SIZE = 58;


namespace stl
{
    String::String(stl::alloc::Arena *arena)
        : _arena(arena)
        , _size(0)
        , _capacity(1)
        , _str(nullptr)
    {}

    String::String(size_t capacity, stl::alloc::Arena *arena)
        : _arena(arena)
        , _size(0)
        , _capacity(capacity)
        , _str((char*)arena->alloc_buf_aligned(_capacity, alignof(char)))
    { }

    String::String(const char *str, size_t size, stl::alloc::Arena *arena)
        : _arena(arena)
        , _size(size)
        , _capacity(_size + 1)
        , _str((char*)arena->alloc_buf_aligned(_capacity, alignof(char)))
    {
        std::strcpy(this->_str, str);
        this->_str[this->_size] = '\0';
    }

    String::String(const char *str, stl::alloc::Arena *arena)
        : _arena(arena)
        , _size(std::strlen(str))
        , _capacity(_size + 1)
        , _str((char*)arena->alloc_buf_aligned(_capacity, alignof(char)))
    {
        std::strcpy(this->_str, str);
        this->_str[this->_size] = '\0';
    }

    String::String(const String &o, stl::alloc::Arena *arena)
        : _arena(arena)
        , _size(o._size)
        , _capacity(o._size + 1)
        , _str((char*)arena->alloc_buf_aligned(o._capacity, alignof(char)))
    {
        std::strcpy(this->_str, o._str);
        this->_str[this->_size] = '\0';
    }

    String::String(String&& o)
        : _arena(o._arena)
        , _size(o._size)
        , _capacity(o._capacity)
        , _str(o._str)
    {
        o._str = nullptr;
        o._size = 0;
    }

    String String::make(const char* str, alloc::Arena *arena)
    {
        return String(str, arena);
    }

    String String::make(const char* str, size_t size, alloc::Arena *arena)
    {
        return String(str, size, arena);
    }

    String String::make(char ch, alloc::Arena *arena)
    {
        String res(arena);
        char *str = arena->alloc<char>(2);
        str[0] = ch;

        res._str = str;
        res._capacity = 2;
        res._size = 1;
        res._str[res._size] = '\0';

        return res;
    }

    String String::make_buf(size_t size, alloc::Arena *arena)
    {
        return String(size, arena);
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
        return String(src, arena);
    }

    String String::to_string(int32_t val, alloc::Arena *arena)
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

    String String::to_string(size_t val, alloc::Arena *arena)
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

    String String::to_string(float val, alloc::Arena *arena)
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


    String String::concat(const String &lhs, const String &rhs, alloc::Arena *arena)
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

    std::strcpy(str, lhs._str);
    std::strcpy(str + lhs._size, rhs._str);

    str[size] = '\0';

    String res(arena);

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

    std::strcpy(str, lhs);
    std::strcpy(str + lhsSize, rhs._str);

    str[size] = '\0';

    String res(arena);

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

    std::strcpy(str, lhs._str);
    std::strcpy(str + lhs._size, rhs);

    str[size] = '\0';

    String res(arena);

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

int32_t to_int(const String& str)
{
    int32_t result {};

    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    ASSERT(ec == std::errc{}, "Integer converstion failed");

    return result;
}

int32_t to_int(const StringView& str)
{
    int32_t result {};

    auto [ptr, ec] = std::from_chars(str.data, str.data + str.size, result);
    ASSERT(ec == std::errc{}, "Integer converstion failed");

    return result;
}

float to_float(const String& str)
{
    float result {};

    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    ASSERT(ec == std::errc{}, "Integer converstion failed");

    return result;
}

float to_float(const StringView& str)
{
    float result {};

    auto [ptr, ec] = std::from_chars(str.data, str.data + str.size, result);
    ASSERT(ec == std::errc{}, "Integer converstion failed");

    return result;
}
}
