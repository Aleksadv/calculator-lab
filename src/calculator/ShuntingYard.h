#pragma once
#include "Tokenizer.h"
#include <vector>
#include <string>

struct RPNItem {
    enum { PushNumber, ApplyOp } kind;
    double number{};
    std::string ident; // имя оператора или функции
    int argc{};
};

class ShuntingYard {
public:
    static std::vector<RPNItem> toRPN(const std::string& expr);
};
