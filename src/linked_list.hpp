#pragma once

#include "alloc/alloc.hpp"


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
    LinkedList(alloc::Arena *arena, size_t size)
        : _pool(alloc::Pool<ListNode>(size, arena))
        , _head(nullptr)
        , _tail(nullptr)
    { }

    ~LinkedList()
    {
        this->_pool.free_all();
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
            auto* head = this->_head;
            this->_pool.free(head);
            this->_head = this->_head->next;
            return;
        }

        if (node == this->_tail)
        {
            auto* tail = this->_tail;
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

private:
    alloc::Pool<ListNode> _pool;
    ListNode *_head;
    ListNode *_tail;
};

}
