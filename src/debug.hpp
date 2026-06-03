#pragma once

#include "stl/print.hpp"
#include <cassert>
#include <iostream>

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

inline constexpr void LOG(const char *str, ...)
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
