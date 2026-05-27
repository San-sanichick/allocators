#pragma once

#include "pch.hpp"
#include "stl/alloc/alloc.hpp"


namespace stl::container
{

template<typename T>
class LinkedList
{
private:
    struct ListNode
    {
        T data;
        ListNode *prev;
        ListNode *next;
    };

public:
    LinkedList(size_t size, alloc::Arena *arena)
        : _pool(alloc::Pool<ListNode>(size, arena))
        , _head(nullptr)
        , _tail(nullptr)
    { }

    ~LinkedList()
    {
        // this->_pool.free_all();
    }

    void push(T &&el)
    {
        if (!this->_tail)
        {
            this->_head = this->_pool.alloc();
            this->_head->data = el;
            this->_head->next = nullptr;
            this->_head->prev = nullptr;

            this->_tail = this->_head;
            return;
        }

        ListNode *node = this->_pool.alloc();
        node->data = el;
        node->next = nullptr;
        node->prev = this->_tail;

        this->_tail->next = node;
        this->_tail = node;
    }

    ListNode *head() const
    {
        return this->_head;
    }

    ListNode *tail() const
    {
        return this->_tail;
    }

    void remove(ListNode &node)
    {
        if (node == this->_head)
        {
            auto *head = this->_head;
            this->_pool.free(head);
            this->_head = this->_head->next;
            return;
        }

        if (node == this->_tail)
        {
            auto *tail = this->_tail;
            this->_pool.free(tail);
            this->_tail = this->_tail->prev;
            return;
        }

        ListNode *next = node->next;
        ListNode *prev = node->prev;
        next->prev = prev;
        prev->next = next;

        this->_pool.free(node);
    }

    void for_each(const std::function<void(T&)> &callback)
    {
        if (this->_head == nullptr) return;

        ListNode *next = this->_head;
        do
        {
            callback(next->data);
        }
        while ((next = next->next));
    }

    void reverse_for_each(const std::function<void(T&)> &callback)
    {
        if (this->_tail == nullptr) return;

        ListNode *prev = this->_tail;
        do
        {
            callback(prev->data);
        }
        while ((prev = prev->prev));
    }

private:
    alloc::Pool<ListNode> _pool;
    ListNode *_head;
    ListNode *_tail;
};

}
