#pragma once


#include <cmath>
namespace stl::math
{
template<typename T>
struct Vec2
{
    union
    {
        struct
        {
            T x, y;
        };
        T vals[2];
    };
};

template<typename T>
struct Vec3
{
    union
    {
        struct
        {
            T x, y, z;
        };
        struct
        {
            T r, g, b;
        };
        T vals[3];
    };
};


template<typename T>
float length(Vec2<T> vec)
{
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

template<typename T>
float length(Vec3<T> vec)
{
    return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

}

