#include "stl/alloc/alloc.hpp"

#include "stl/container/linked_list.hpp"
#include "stl/container/vector.hpp"

#include "stl/ref.hpp"
#include "stl/string.hpp"

#include "stl/union.hpp"
#include "types.hpp"
#include "utils.hpp"


struct Point
{
    i32 x, y;

    i32 len()
    {
        return x * x + y * y;
    }


    constexpr inline bool operator<(const Point& o)
    {
        return this->x < o.x && this->y < o.y;
    }

    inline constexpr std::string toString() const
    {
        return std::format("({}, {})", x, y);
    }
};

enum class TokenKind : i8
{
    IDENT,
    OP,
    eof,
};

struct Token
{
    TokenKind kind;
    stl::TaggedUnion<char, f32> value;
};

bool isDelimiter(char ch)
{
    return ch == ' ';
}

bool isOperator(char ch)
{
    return ch == '+'
        || ch == '-'
        || ch == '*'
        || ch == '/';
}



class AST
{
public:
    struct ASTNode
    {
        TokenKind kind;
        stl::TaggedUnion<char, float> value;
        ASTNode *left;
        ASTNode *right;
    };

    Token eof = {
        .kind = TokenKind::eof,
        .value = stl::TaggedUnion<char, float>::make(0),
    };

public:

    AST(stl::container::Vector<Token> &tokens, stl::alloc::Arena *arena)
        : tokens(tokens)
        , pool(stl::alloc::Pool<ASTNode>(64, arena))
    {}

    i8 bp(const Token &token)
    {
        if (token.kind == TokenKind::IDENT) return 0;
        if (token.kind == TokenKind::eof) return 0;

        switch (token.value.get<char>())
        {
            case '+': return 10; 
            case '-': return 10;
            case '*': return 20;
            case '/': return 20;
        }

        return -1;
    }


    ASTNode* nud(const Token &token)
    {
        auto *node = this->pool.alloc();

        node->kind = token.kind;
        node->value = token.value;
        node->left = nullptr;
        node->right = nullptr;

        return node;
    }

    ASTNode* led(ASTNode *left, const Token &nextToken)
    {
        ASTNode* right = this->expr(bp(nextToken));

        auto *node = this->pool.alloc();
        node->kind = nextToken.kind;
        node->value = nextToken.value;
        node->left = left;
        node->right = right;

        return node;
    }

    const Token next()
    {
        Token next = tokens[tokens.size() - 1];
        tokens.popBack();

        return next;
    }

    const Token &peek() const
    {
        if (tokens.size() == 0) return this->eof;

        return this->tokens[tokens.size() - 1];
    }

    ASTNode *expr(i8 rbp = 0)
    {
        ASTNode* left = this->nud(this->next());

        while (bp(this->peek()) > rbp)
        {
            left = this->led(left, this->next());
        }

        return left;
    }

    float walk(ASTNode *node)
    {
        if (node->kind == TokenKind::OP)
        {
            char op = node->value.get<char>();
            switch (op)
            {
                case '+':
                {
                    return this->walk(node->left) + this->walk(node->right);
                }
                case '-':
                {
                    return this->walk(node->left) - this->walk(node->right);
                }
                case '*':
                {
                    return this->walk(node->left) * this->walk(node->right);
                }
                case '/':
                {
                    return this->walk(node->left) / this->walk(node->right);
                }
            }
        }

        return node->value.get<f32>();
    }

private:
    stl::container::Vector<Token> &tokens;
    stl::alloc::Pool<ASTNode> pool;
};




i32 main()
{
    stl::alloc::Arena arena(4096); // general-purpose arena of 4KB
    stl::alloc::Arena scratch(8192); // 8KB for other stuff

    stl::String inputBuf = stl::String::make_buf(512, &scratch); // yoink like an 16th of the arena, why not, it's free

    LOG("Input an expression:");
    stl::String::getline(std::cin, inputBuf);

    stl::String expr = stl::String::copy(inputBuf, &arena); // copy from buffer string into
                                                            // a proper string

    scratch.free_all(); // free up the scratch arena, we'll need it later

    stl::String parseBuf = stl::String::make_buf(64, &scratch);
    bool eof = false;
    stl::container::Vector<Token> tokens(128, &arena);
    for (size_t i = 0; i < expr.size(); i++)
    {
        char ch = expr[i];

        if (i == expr.size() - 1)
        {
            if (parseBuf.size() == 0) parseBuf += ch;
            else if (!eof) parseBuf += ch;

            auto value = stl::TaggedUnion<char, f32>::make<float>(stl::to_float(parseBuf));
            tokens.emplaceBack(TokenKind::IDENT, value);
            parseBuf.reset();

            break;
        }

        if (isDelimiter(ch)) continue;

        if (isOperator(ch))
        {
            if (parseBuf.size() != 0)
            {
                auto value = stl::TaggedUnion<char, f32>::make<float>(stl::to_float(parseBuf));
                tokens.emplaceBack(TokenKind::IDENT, value);
            }
            parseBuf.reset();

            {
                auto value = stl::TaggedUnion<char, f32>::make<char>(ch);
                tokens.emplaceBack(TokenKind::OP, value);
            }

            if (i < expr.size())
            {
                if (i + 1 == expr.size() - 1) eof = true;

                char next = expr[i + 1];
                if (!isDelimiter(next))
                {
                    parseBuf += next;
                }

                continue;
            }
        }

        parseBuf += ch;
    }

    scratch.free_all();

    tokens.shrinkToFit();

    LOG("Parsed tokens:");
    for (const auto &token : tokens)
    {
        if (token.kind == TokenKind::OP)
        {
            LOG(stl::String::to_string((i32)token.kind, &scratch) + ": " + token.value.get<char>());
        }
        else
        {
            LOG(
                stl::String::to_string((i32)token.kind, &scratch)
                + ": "
                + stl::String::to_string(token.value.get<f32>(), &scratch)
            );
        }

    }

    scratch.free_all();

    AST ast(tokens, &scratch);
    auto *head = ast.expr();

    LOG("Calculated result");
    std::cout << ast.walk(head) << std::endl;

    return 0;


    {
        defer ([&] { arena.free_all(); });

        stl::alloc::Stack stack(128, &arena);
        Point *point1 = stack.make<Point>(2, 3);
        Point *point2 = stack.make<Point>(3, 4);
        i32 *_arr = stack.alloc<i32>(12);

        for (size_t i = 0; i < 12; i++)
        {
            _arr[i] = i;
        }

        stack.free(_arr);
        LOG("yay");
        stack.free(point2);
        LOG("yay");
        stack.free(point1);
        LOG("yay");

        LOG("Used arena memory: " + stl::String::to_string(arena.used(), &scratch) + " bytes"); // 128 bytes
    }

    LOG("Used arena memory: " + stl::String::to_string(arena.used(), &scratch) + " bytes"); // 0 bytes

    {
        stl::String str1 = stl::String::make("hello ", &arena);
        LOG("Used arena memory: " + stl::String::to_string(arena.used(), &scratch) + " bytes"); // 7 bytes

        stl::String str2 = stl::String::make("world", &arena);
        LOG("Used arena memory: " + stl::String::to_string(arena.used(), &scratch) + " bytes"); // 7 + 6 = 13 bytes

        stl::String res1 = str1 + str2; // NOTE: uses the same arena as str1, ambiguous

        LOG(res1);

        res1.to_upper();
        for (auto &ch : res1)
        {
            std::cout << ch;
        }
        std::cout << std::endl;

        LOG("Equal: " + std::to_string(str1 == str2));
        LOG("Used arena memory: " + std::to_string(arena.used()) + " bytes");
    }

    arena.free_all();

    LOG("Hello " + stl::String::to_string(3.0f, &arena) + " world");
    LOG("Used bytes will include the call to String::to_string: " + stl::String::to_string(arena.used(), &arena) + " bytes");

    arena.free_all();

    stl::String str = stl::String::make("😳", &arena);
    LOG(str);

    LOG("Used arena memory: " + stl::String::to_string(arena.used(), &scratch) + " bytes"); // 5 bytes

    arena.free_all();
    scratch.free_all();

    {
        auto state = arena.save();
        defer ([&] { arena.restore(state); });

        LOG("Create linked list");
        stl::container::LinkedList<Point> list(5, &arena);

        for (int i = 0; i < 5; i++)
        {
            list.push({ i, i + 1 });
        }

        // Iterators kinda fucking suck in C++
        list.for_each([](Point &point)
        {
            LOG(point.toString());
        });

        list.reverse_for_each([](Point &point)
        {
            LOG(point.toString());
        });

        LOG("Used arena memory: " + stl::String::to_string(arena.used(), &scratch) + " bytes");
        LOG("Destroy linked list");
    }

    LOG("Used arena memory: " + stl::String::to_string(arena.used(), &scratch) + " bytes");

    return 0;


    // we can do this as many times as we want,
    // since freeing memory in the arena is
    // a simple assignment operation
    for (size_t i = 0; i < 10; i++)
    {
        // NOTE: if we use a defer macro here,
        // we're good to go, as this is going to be
        // freed at the end of the scope
        defer ([&] { arena.free_all(); });

        Point *const p1 = arena.make<Point>(i, i + 1);
        Point *const p2 = arena.make<Point>(i + 2, i + 3);

        LOG(p1->toString());
        LOG(p2->toString());

        LOG("Used arena memory: " + std::to_string(arena.used()) + " bytes");
        // NOTE: or just manually free it at the end
        // arena.freeAll();
    }

    LOG("");

    // NOTE: here we don't defer, as the data is going
    // to be freed by the arena's destructor

    // WARN: these allocations never call destructors.
    // either call them manually, if needed,
    // or, preferably, make sure these types
    // don't need destructors at all
    Point *const p1 = arena.make_aligned<Point>(69, 420); // Make calls the constructor
    Point *const p2 = arena.make_aligned<Point>(420, 69);

    LOG("Used arena memory: " + std::to_string(arena.used()) + " bytes");
    LOG(p1->toString());
    LOG(p2->toString());


    constexpr size_t SIZE = 32;
    i32 *const arr = arena.alloc_aligned<i32>(SIZE); // NOTE: Allocates 32 integers,
                                                             // without calling any constructors
    LOG("");
    LOG("Used arena memory: " + std::to_string(arena.used()) + " bytes");

    for (size_t i = 0; i < SIZE; i++)
    {
        arr[i] = i;
    }

    for (size_t i = 0; i < SIZE; i++)
    {
        LOG(std::to_string(arr[i]));
    }

    Point *const p3 = arena.make_aligned<Point>(32, 89);
    LOG(p3->toString());

    for (size_t i = 0; i < SIZE; i++)
    {
        LOG(std::to_string(arr[i]));
    }

    LOG(p1->toString());
    arena.free_all(); // let's just free the arena, why not

    stl::alloc::Pool<Point> pool(20, &arena); // creates a pool of 20 Point-s using the arena
    Point *const p4 = pool.alloc(23, 32); // calls the constructor
    LOG(p4->toString());

    pool.free(p4); // NOTE: This DOES call the destructor
                   // WARN: pool.free_all() does NOT call any destructors,
                   // as that would be stupid, it has no way of knowing if
                   // any given chunk is a valid object or not
                   // NOTE: Since we call freeAll() at the end anyway,
                   // forgetting an individual free() is not that big of a deal here.
                   // Might be a bigger deal elsewhere, so, uh, don't forget to call it there

    {
        // HACK: RAII is actually kinda shit, lmao
        // Technically we could pass an allocator into
        // the RefCounted::make, but that sounds like a crime,
        // so let's not do that
        stl::ptr::RefCounted<Point> p1 = stl::ptr::RefCounted<Point>::make(2, 3);
        LOG(std::to_string(p1.get_count()));
        LOG(p1->toString());

        stl::ptr::RefCounted<Point> copy = p1;
        LOG(std::to_string(copy.get_count()));
        LOG(copy->toString());
    }

    // NOTE: Pool's destructor doesn't really do much,
    // all of the memory gets cleaned up by the Arena's destructor

    stl::container::Vector<Point> v(&arena);

    for (i32 i = 10; i >= 0; i--)
    {
        v.emplaceBack(i, i + 1);
    }

    LOG("before sort");
    for (const auto &el : v)
    {
        LOG(el.toString());
    }

    std::sort(v.begin(), v.end());

    LOG("after sort");
    for (const auto &el : v)
    {
        LOG(el.toString());
    }

    v.popBack();

    LOG("push result");

    for (const auto &el : v)
    {
        LOG(el.toString());
    }

    LOG("pop");
    for (size_t i = 0; i < 5; i++)
    {
        auto& val = v[v.size() - 1];
        LOG(val.toString());

        v.popBack();
    }

    v.emplaceBack(69, 420);

    LOG("pop result and then a single push");
    for (const auto &el : v)
    {
        LOG(el.toString());
    }

    v.erase(2, 4);

    for (const auto &el : v)
    {
        LOG(el.toString());
    }

    // LOG("copy");
    // stl::container::Vector<Point> move = std::move(v);
    //
    // for (const auto& el : move)
    // {
    //     LOG(el.toString());
    // }
}
