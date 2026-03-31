#pragma once

#include <algorithm>
#include <cstdlib>


namespace stl::container
{

template<typename T>
class CDynArray
{
public:
    CDynArray()
        : _capacity(1)
        , _size(0)
        , _data(static_cast<T*>(std::malloc(sizeof(T) * _capacity)))
    { }

    CDynArray(size_t capcacity)
        : _capacity(capcacity)
        , _size(0)
        , _data(static_cast<T*>(std::malloc(sizeof(T) * _capacity)))
    { }

    CDynArray(const CDynArray&) = delete;

    CDynArray(CDynArray&& o)
        : _capacity(o._capacity)
        , _size(o._size)
        , _data(o._data)
    {
        o._capacity = 0;
        o._size = 0;
        o._data = nullptr;
    }

    ~CDynArray()
    {
        std::free(this->_data);
    }

    void push(T&& el)
    {
        if (this->needsResize()) this->resize();

        this->_data[this->_size] = std::move(el);
        this->_size++;
    }

private:
    bool needsResize()
    {
        return this->_size + 1 > this->_capacity;
    }

    void resize()
    {
        this->_capacity *= 1.8f;
        this->_data = static_cast<T*>(
            std::realloc(
                this->_data,
                sizeof(T) * this->_capacity
            )
        );
    }

private:
    size_t _capacity;
    size_t _size;
    T* _data;
};

}
