#pragma once

#include "pch.hpp"


template<typename... Ts>
concept Trivial = std::conjunction_v<
    std::is_trivially_constructible<Ts>...,
    std::is_trivially_destructible<Ts>...
>;

template<typename T>
concept Number = std::is_integral_v<T> || std::is_floating_point_v<T>;

template<typename T>
concept Integer = std::is_integral_v<T>;

template<typename T>
concept Float = std::is_floating_point_v<T>;


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
