#include "linked_list.hpp"
#include "alloc/alloc.hpp"
#include "utils.hpp"
#include "ref.hpp"
// #include "src/vector.hpp"


struct Point
{
    int x, y;

    // Point() = default;
    // Point(int x, int y) : x(x), y(y) {}
    // Point(const Point&) = default;
    // Point(Point&&) = default;
    // ~Point() = default;

    int len()
    {
        return x * x + y * y;
    }

    inline constexpr std::string toString() const
    {
        return std::format("({}, {})", x, y);
    }
};




int main()
{
    stl::alloc::Arena arena(4096); // creates an arena of 4KB
    stl::container::LinkedList<Point> list(&arena, 5);

    for (int i = 0; i < 5; i++)
    {
        list.push({ i, i + 1 });
    }

    auto *next = list.head();
    while (next != nullptr)
    {
        LOG(next->data.toString());
        next = next->next;
    }


    // // we can do this as many times as we want,
    // // since freeing memory in the arena is
    // // a simple assignment operation
    // for (size_t i = 0; i < 10; i++)
    // {
    //     // NOTE: if we use a defer macro here,
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
    //
    // LOG("");
    //
    // // NOTE: here we don't defer, as the data is going
    // // to be freed by the arena's destructor
    //
    // // WARN: these allocations never call destructors.
    // // either call them manually, if needed,
    // // or, preferably, make sure these types
    // // don't need destructors at all
    // Point *const p1 = arena.make_aligned<Point>(69, 420); // Make calls the constructor
    // Point *const p2 = arena.make_aligned<Point>(420, 69);
    //
    // LOG(p1->toString());
    // LOG(p2->toString());
    //
    //
    // constexpr size_t SIZE = 32;
    // int32_t *const arr = arena.alloc_aligned<int32_t>(SIZE); // NOTE: Allocates 32 integers,
    //                                                          // without calling any constructors
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
    // arena.free_all(); // let's just free the arena, why not
    //
    // stl::alloc::Pool<Point> pool(20, &arena); // creates a pool of 20 Point-s using the arena
    // Point *const p4 = pool.alloc(23, 32); // calls the constructor
    // LOG(p4->toString());
    //
    // pool.free(p4); // NOTE: This DOES call the destructor
    //                // WARN: pool.free_all() does NOT call any destructors,
    //                // as that would be stupid, it has no way of knowing if
    //                // any given chunk is a valid object or not
    //                // NOTE: Since we call freeAll() at the end anyway,
    //                // forgetting an individual free() is not that big of a deal here.
    //                // Might be a bigger deal elsewhere, so, uh, don't forget to call it there
    //
    // {
    //     // HACK: RAII is actually kinda shit, lmao
    //     // Technically we could pass an allocator into
    //     // the RefCounted::make, but that sounds like a crime,
    //     // so let's not do that
    //     stl::ptr::RefCounted<Point> p1 = stl::ptr::RefCounted<Point>::make(2, 3);
    //     LOG(std::to_string(p1.get_count()));
    //     LOG(p1->toString());
    //
    //     stl::ptr::RefCounted<Point> copy = p1;
    //     LOG(std::to_string(copy.get_count()));
    //     LOG(copy->toString());
    // }
    //
    // // NOTE: Pool's destructor doesn't really do much,
    // // all of the memory gets cleaned up by the Arena's destructor

    // stl::container::Vector<Point> v;
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
