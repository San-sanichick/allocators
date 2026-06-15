#include "stl/alloc/alloc.hpp"

#include "stl/container/vector.hpp"

#include "stl/print.hpp"
#include "stl/string.hpp"

#include "stl/union.hpp"
#include "types.hpp"


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
        return stl::fmt("(%d, %d)", x, y);
    }
};

enum class TokenKind : i8
{
    IDENT,
    OP,
    OPEN_PAREN,
    CLOSE_PAREN,
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

bool isParen(char ch)
{
    return ch == '(' || ch == ')';
}

bool isOperator(char ch)
{
    return ch == '+'
        || ch == '-'
        || ch == '*'
        || ch == '/'
        || ch == '^';
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
            case ')': return 0;
            case '+': return 10;
            case '-': return 10;
            case '*': return 20;
            case '/': return 20;
            case '^': return 40;
            case '(': return 50;
        }

        return -1;
    }


    ASTNode *nud(const Token &token)
    {
        auto *node = this->pool.alloc();

        node->kind = token.kind;
        node->value = token.value;
        node->left = nullptr;
        node->right = nullptr;

        return node;
    }

    ASTNode *led(ASTNode *left, const Token &nextToken)
    {
        i8 bp = this->bp(nextToken);
        if (
            nextToken.kind == TokenKind::OP &&
            nextToken.value.get<char>() == '^'
        )
        {
            bp--;
        }

        ASTNode* right = this->expr(bp);

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
                case '^':
                {
                    return std::pow(this->walk(node->left), this->walk(node->right));
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
    stl::alloc::Arena arena(4096); // 4KB general-purpose arena
    stl::alloc::Arena scratch(4096); // 4KB for other stuff

    stl::String inputBuf = stl::String::make_buf(512, &scratch); // yoink like an 8th of the arena,
                                                                 // why not, it's free

    stl::print("Input an expression: ");
    stl::String::getline(std::cin, inputBuf);

    stl::String expr = stl::String::copy(inputBuf, &arena); // copy from buffer string into
                                                            // a proper string

    scratch.free_all(); // free up the scratch arena, we'll need it later

    stl::container::Vector<Token> tokens(128, &arena);
    stl::String parseBuf = stl::String::make_buf(64, &scratch);
    bool eof = false;

    for (size_t i = 0; i < expr.size(); i++)
    {
        char ch = expr[i];

        if (i == expr.size() - 1)
        {
            if (parseBuf.size() == 0) parseBuf += ch;
            else if (!eof) parseBuf += ch;

            auto value = stl::TaggedUnion<char, f32>::make(stl::to_float(parseBuf));
            tokens.emplaceBack(TokenKind::IDENT, value);
            parseBuf.reset();

            break;
        }

        if (isDelimiter(ch)) continue;

        if (isOperator(ch))
        {
            if (parseBuf.size() != 0)
            {
                auto value = stl::TaggedUnion<char, f32>::make(stl::to_float(parseBuf));
                tokens.emplaceBack(TokenKind::IDENT, value);
            }
            parseBuf.reset();

            {
                auto value = stl::TaggedUnion<char, f32>::make(ch);
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

    stl::println("Parsed tokens:");
    for (const auto &token : tokens)
    {
        if (token.kind == TokenKind::OP)
        {
            stl::println("%d: %c", (i32)token.kind, token.value.get<char>());
        }
        else
        {
            stl::println("%d: %f", (i32)token.kind, token.value.get<f32>());
        }
    }

    scratch.free_all();

    LOG("check: %d", 86);
    AST ast(tokens, &arena);
    auto *head = ast.expr();

    stl::println("Calculated result");
    float res = ast.walk(head);
    stl::println("%f", res);

    return 0;
}
