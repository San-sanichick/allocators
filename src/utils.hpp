#pragma once

#include <algorithm>


#ifndef defer

template <typename T>
struct deferrer
{
	T f;
	constexpr deferrer(T f) : f(std::move(f)) { };
	constexpr deferrer(const deferrer&) = delete;
	constexpr ~deferrer() { f(); }
};

#define TOKEN_CONCAT_NX(a, b) a ## b
#define TOKEN_CONCAT(a, b) TOKEN_CONCAT_NX(a, b)
#define defer deferrer TOKEN_CONCAT(__deferred, __COUNTER__) =

#endif

