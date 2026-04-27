#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility> // IWYU pragma: export
#include <type_traits> // IWYU pragma: export
#include "debug.hpp"



#define TYPE_IS_TRIVIAL(x) \
    static_assert(std::is_trivially_destructible_v<x>, "T is not trivially destructible"); \
    static_assert(std::is_trivially_default_constructible_v<x>, "T is not trivially constructible");



constexpr static inline bool is_power_of_two(uintptr_t val)
{
    return (val & (val - 1)) == 0;
}

constexpr static inline uintptr_t align_forward(uintptr_t ptr, size_t align)
{
    ASSERT(is_power_of_two(align), "Alignment should be a power of 2");

    uintptr_t p = ptr;
    uintptr_t a = (uintptr_t)align;
    uintptr_t modulo = p & (a - 1);

    if (modulo != 0)
    {
        p += a - modulo;
    }

    return p;
}


