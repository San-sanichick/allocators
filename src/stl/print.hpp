#pragma once

#include <cstdio>
#include <cstdarg>
#include <string>


namespace stl
{
inline std::string vfmt(const char *fmt, va_list args)
{
    va_list copy;

    va_copy(copy, args);
    int n = vsnprintf(nullptr, 0, fmt, copy);
    va_end(copy);

    if (n < 0) return "";

    std::string s(n, '\0');

    vsprintf(s.data(), fmt, args);

    return s;
}

inline std::string fmt(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    auto s = vfmt(fmt, args);
    va_end(args);

    return s;
}

inline void print(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    std::cout << vfmt(fmt, args) << std::flush;
    va_end(args);
}

inline void println(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    std::cout << vfmt(fmt, args) << std::endl;
    va_end(args);
}
}
