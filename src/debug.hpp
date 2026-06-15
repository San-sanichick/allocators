#pragma once

#include "stl/print.hpp"
#include <cassert>
#include <iostream>
#include <source_location>

enum class Build
{
    Debug,
    Release,
};


#if defined(NDEBUG)
    constexpr Build BUILD = Build::Release;
#else
    constexpr Build BUILD = Build::Debug;
#endif

inline constexpr void LOG(std::string_view str)
{
    if constexpr (BUILD == Build::Debug)
    {
        std::cout << str << std::endl;
    }
}

inline constexpr void ERR(std::string_view str)
{
    if constexpr (BUILD == Build::Debug)
    {
        std::cout << str << std::endl;
    }
}

inline constexpr void _LOG(const char *str, std::source_location loc, ...)
{
    if constexpr (BUILD == Build::Debug)
    {
        va_list args;

        va_start(args, loc);
        std::cout << loc.file_name() << ':' << loc.line() << ' ' << stl::vfmt(str, args) << std::endl;
        va_end(args);
    }
}

#define LOG(str, ...) _LOG(str, std::source_location::current(), __VA_ARGS__)

inline constexpr void ERR(const char *str, ...)
{
    if constexpr (BUILD == Build::Debug)
    {
        va_list args;

        va_start(args, str);
        std::cout << stl::vfmt(str, args) << std::endl;
        va_end(args);
    }
}

#if defined(NDEBUG)
    #define ASSERT(x, msg)
#else
    #define ASSERT(x, msg) assert((x) && (msg))
#endif
