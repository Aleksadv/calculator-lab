#include "Tokenizer.h"
#include <cctype>
#include <stdexcept>

Tokenizer::Tokenizer(std::string s) : src(std::move(s)) {}

void Tokenizer::skipSpace(){ while (i < src.size() && std::isspace((unsigned char)src[i])) ++i; }

Token Tokenizer::readNumber(){
    size_t start = i; bool dot=false;
    while (i < src.size() && (std::isdigit((unsigned char)src[i]) || src[i]=='.')){
        if (src[i]=='.'){ if (dot) break; dot=true; } ++i;
    }
    Token t{TokType::Number};
    t.text = src.substr(start, i-start);
    t.value = std::stod(t.text);
    return t;
}

Token Tokenizer::readIdent(){
    size_t start=i;
    while (i < src.size() && (std::isalnum((unsigned char)src[i]) || src[i]=='_')) ++i;
    return Token{TokType::Ident, src.substr(start, i-start), 0.0};
}

Token Tokenizer::next(){
    if (hasCache){ hasCache=false; return cache; }
    skipSpace();
    if (i >= src.size()) return Token{TokType::End};
    char c = src[i++];
    switch (c){
        case '+': return {TokType::Plus, "+"};
        case '-': return {TokType::Minus,"-"};
        case '*': return {TokType::Star, "*"};
        case '/': return {TokType::Slash,"/"};
        case '^': return {TokType::Caret,"^"};
        case '(': return {TokType::LParen,"("};
        case ')': return {TokType::RParen,")"};
        case ',': return {TokType::Comma, ","};
        default:
            if (std::isdigit((unsigned char)c) || c=='.'){ --i; return readNumber(); }
            if (std::isalpha((unsigned char)c) || c=='_'){ --i; return readIdent(); }
            throw std::runtime_error(std::string("Unexpected char: ")+c);
    }
}

Token Tokenizer::peek() const{
    if (!hasCache){
        Tokenizer* self = const_cast<Tokenizer*>(this);
        self->cache = self->next();
        self->hasCache = true;
    }
    return cache;
}
