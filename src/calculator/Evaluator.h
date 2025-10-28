#pragma once
#include "ShuntingYard.h"
#include <functional>
#include <string>

struct FuncRegistry {
    std::function<bool(const std::string&, int&, std::function<double(const std::vector<double>&)>&)> resolver;
};

class Evaluator {
public:
    static double evalRPN(const std::vector<RPNItem>& rpn, const FuncRegistry& reg);
};
