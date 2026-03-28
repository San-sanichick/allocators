#include <cstdlib>
#include <format>
#include <string>
#include "src/alloc.hpp"
// #include "src/vector.hpp"


struct Point
{
    int x, y;

    Point() = default;
    Point(int x, int y) : x(x), y(y) {}
    Point(const Point&) = default;
    Point(Point&&) = default;
    ~Point() = default;


    int len()
    {
        return x * x + y * y;
    }

    inline constexpr std::string toString() const
    {
        return std::format("({}, {})", x, y);
    }
};


template<typename T>
class DynArray
{
public:
    DynArray()
        : _capacity(1)
        , _size(0)
        , _data(static_cast<T*>(std::malloc(sizeof(T) * _capacity)))
    { }

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
        this->_data = static_cast<T*>(std::realloc(this->_data, sizeof(T) * this->_capacity));
    }

private:
    size_t _capacity;
    size_t _size;
    T* _data;
};



struct FooA
{
    size_t a;
    char b;
    char c;
};

struct FooB
{
    char b;
    size_t a;
    char c;
};


int main()
{
    // stl::Arena arena(4096);
    //
    // for (size_t i = 0; i < 10; i++)
    // {
    //     // NOTE: if use a defer macro here,
    //     // we're good to go, as this is going to be
    //     // freed at the end of the scope
    //     defer ([&] { arena.free_all(); });
    //
    //     Point *const p1 = arena.make<Point>(i, i + 1);
    //     Point *const p2 = arena.make<Point>(i + 2, i + 3);
    //
    //     LOG(p1->toString());
    //     LOG(p2->toString());
    //
    //     // NOTE: or just manually free it at the end
    //     // arena.freeAll();
    // }
    // //
    // // LOG("");
    //
    // // NOTE: here we don't defer, as the data is going
    // // to be freed by the arenas destructor
    //
    // // NOTE: these allocations never call destructors.
    // // either call them manually, if needed,
    // // or, preferably, make sure these types don't have
    // // non-trivial destructors
    // Point *const p1 = arena.make_aligned<Point>(69, 420);
    // Point *const p2 = arena.make_aligned<Point>(420, 69);
    //
    // LOG(p1->toString());
    // LOG(p2->toString());
    //
    //
    // constexpr size_t SIZE = 32;
    // int32_t *const arr = arena.alloc_aligned<int32_t>(SIZE); // NOTE: Allocates 32 integers
    //
    // for (size_t i = 0; i < SIZE; i++)
    // {
    //     arr[i] = i;
    // }
    //
    // for (size_t i = 0; i < SIZE; i++)
    // {
    //     LOG(std::to_string(arr[i]));
    // }
    //
    // Point *const p3 = arena.make_aligned<Point>(32, 89);
    // LOG(p3->toString());
    //
    // for (size_t i = 0; i < SIZE; i++)
    // {
    //     LOG(std::to_string(arr[i]));
    // }
    //
    // LOG(p1->toString());

    stl::Pool<Point> pool(20);
    Point *const p1 = pool.alloc(23, 32);
    LOG(p1->toString());
    pool.free(p1);

    // stl::Vector<Point> v;
    //
    // for (int i = 0; i < 10; i++)
    // {
    //     v.emplaceBack(i, i + 1);
    // }
    //
    // v.popBack();
    //
    // LOG("push result");
    //
    // for (const auto& el : v)
    // {
    //     LOG(el.toString());
    // }
    //
    // LOG("pop");
    // for (size_t i = 0; i < 5; i++)
    // {
    //     auto& val = v[v.size() - 1];
    //     LOG(val.toString());
    //
    //     v.popBack();
    // }
    //
    // v.emplaceBack(69, 420);
    //
    // LOG("pop result and then a single push");
    // for (const auto& el : v)
    // {
    //     LOG(el.toString());
    // }
    //
    //
    // LOG("copy");
    // stl::Vector<Point> move = std::move(v);
    //
    // for (const auto& el : move)
    // {
    //     LOG(el.toString());
    // }
}
