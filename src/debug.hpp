#pragma once

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

#define ASSERT(x, msg) assert((x) && (msg))
