#pragma once

#include <utility>



namespace stl::ptr
{

template<class T>
class Ref
{
public:
    Ref() = delete;
    Ref(const Ref&) = delete;

    Ref(T *const ptr)
        : ptr(ptr)
    { }

    Ref(Ref&& o)
        : ptr(o.ptr)
    {
        o.ptr = nullptr;
    }

    ~Ref()
    {
        if (this->ptr != nullptr)
            delete this->ptr;
    }

    template<class ...Args>
    static Ref<T> make(Args&&... args)
    {
        return Ref(new T(std::forward<Args>(args)...));
    }

    constexpr inline T *const operator*() const
    {
        return this->ptr;
    }

    constexpr inline T *const operator->() const
    {
        return this->ptr;
    }

    constexpr inline T *const get() const
    {
        return this->ptr;
    }

private:
    T *ptr;
};

template<class T>
class RefCounted
{
public:
    RefCounted() = delete;
    RefCounted(RefCounted&&) = delete;

    RefCounted(T *const ptr)
        : count(new std::size_t {1})
        , _ptr(ptr)
    {
    }

    RefCounted(const RefCounted& o)
        : count(o.count)
        , _ptr(o._ptr)
    {
        (*o.count)++;
    }

    ~RefCounted()
    {
        (*this->count)--;
        if ((*this->count) == 0)
        {
            delete this->_ptr;
            delete this->count;
        }
    }


    template<class ...Args>
    static RefCounted<T> make(Args&&... args)
    {
        return RefCounted(new T(std::forward<Args>(args)...));
    }

    constexpr inline T *const operator*() const
    {
        return this->_ptr;
    }

    constexpr inline T *const operator->() const
    {
        return this->_ptr;
    }

    constexpr inline T *const get() const
    {
        return this->_ptr;
    }

    constexpr inline std::size_t get_count() const
    {
        return *this->count;
    }


private:
    std::size_t *count;
    T *_ptr;
};

}

