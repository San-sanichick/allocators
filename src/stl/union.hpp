#pragma once

#include "debug.hpp"
#include "pch.hpp"
#include "stl/alloc/utils.hpp"


namespace stl
{
template<typename...Ts>
struct TaggedUnion
{
TYPE_IS_TRIVIAL(Ts...);

private:
    template<typename F, typename...Tss>
    struct union_helper
    {
        static const size_t size =
            sizeof(F) > union_helper<Tss...>::size ? sizeof(F) : union_helper<Tss...>::size;
    };

    template<typename F>
    struct union_helper<F>
    {
        static const size_t size = sizeof(F);
    };

    template<size_t size>
    struct raw_data { std::byte data[size]; };

public:
    template<typename C>
    inline static TaggedUnion make(C value)
    {
        raw_data<union_helper<Ts...>::size> data;
        
        // HACK: C++ is a perfectly adequate language with no flaws at all
        new (data.data) C(value);
        return {
            .data = data, // we allocated on the stack,
                          // and then copied that data into the Union object,
                          // so there shouldn't be any issues. BUT.
                          // This will only work for TRIVIAL types
            .type_id = typeid(C).hash_code(),
        };
    }

    template<typename C>
    void set(C value)
    {
        new (this->data.data) C(value);
        this->type_id = typeid(C).hash_code();
    }

    template<typename C>
    const C& get() const
    {
        ASSERT(this->type_id == typeid(C).hash_code(), "Invalid cast");
        return *reinterpret_cast<const C*>(this->data.data);
    }

    template<typename C>
    C& get()
    {
        ASSERT(this->type_id == typeid(C).hash_code(), "Invalid cast");
        return *reinterpret_cast<C*>(this->data.data);
    }

public:
    alignas(Ts...) raw_data<union_helper<Ts...>::size> data;
    size_t type_id;
};
}
