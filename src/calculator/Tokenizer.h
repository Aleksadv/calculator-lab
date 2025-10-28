#pragma once
#include <string>

enum class TokType { Number, Ident, LParen, RParen, Comma, Plus, Minus, Star, Slash, Caret, End };

struct Token {
    TokType type{};
    std::string text;
    double value{};
};

class Tokenizer {
public:
    explicit Tokenizer(std::string s);
    Token next();
    Token peek() const;
private:
    std::string src;
    size_t i = 0;
    mutable Token cache;
    mutable bool hasCache = false;
    void skipSpace();
    Token readNumber();
    Token readIdent();
};
