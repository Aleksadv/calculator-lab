#include "ShuntingYard.h"
#include <stack>
#include <stdexcept>

struct Op { std::string name; int precedence; bool rightAssoc; int arity; };

static Op opInfo(char c){
    switch(c){
        case '+': return {"+",2,false,2};
        case '-': return {"-",2,false,2};
        case '*': return {"*",3,false,2};
        case '/': return {"/",3,false,2};
        case '^': return {"^",4,true ,2};
        default: throw std::runtime_error("Not an operator");
    }
}

std::vector<RPNItem> ShuntingYard::toRPN(const std::string& expr){
    Tokenizer tz(expr);
    std::vector<RPNItem> out;
    std::stack<char> ops;             // операторы и скобки
    std::stack<std::string> funcs;    // имена функций
    std::stack<int> argCounts;        // счётчик аргументов функции

    Token t = tz.next();
    bool mayUnary = true;

    while (t.type != TokType::End){
        if (t.type==TokType::Number){
            out.push_back({RPNItem::PushNumber, t.value, "", 0});
            mayUnary = false;
        } else if (t.type==TokType::Ident){
            funcs.push(t.text);
            argCounts.push(1);
            Token p = tz.peek();
            if (p.type != TokType::LParen) throw std::runtime_error("Identifier must be used as function: " + t.text);
            mayUnary = true;
        } else if (t.type==TokType::LParen){
            ops.push('(');
            mayUnary = true;
        } else if (t.type==TokType::Comma){
            while(!ops.empty() && ops.top()!='('){
                char top=ops.top(); ops.pop();
                Op oi = opInfo(top);
                out.push_back({RPNItem::ApplyOp, 0.0, oi.name, oi.arity});
            }
            if (argCounts.empty()) throw std::runtime_error("Comma outside function");
            argCounts.top() += 1;
            mayUnary = true;
        } else if (t.type==TokType::RParen){
            while(!ops.empty() && ops.top()!='('){
                char top=ops.top(); ops.pop();
                Op oi = opInfo(top);
                out.push_back({RPNItem::ApplyOp, 0.0, oi.name, oi.arity});
            }
            if (ops.empty()) throw std::runtime_error("Mismatched ')'");
            ops.pop(); // '('
            if (!funcs.empty()){
                std::string fname = funcs.top(); funcs.pop();
                int argc = argCounts.top(); argCounts.pop();
                out.push_back({RPNItem::ApplyOp, 0.0, fname, argc});
            }
            mayUnary = false;
        } else {
            // оператор
            if (t.type==TokType::Minus && mayUnary){
                // унарный минус как (0 - x)
                out.push_back({RPNItem::PushNumber, 0.0, "", 0});
            }
            char c = t.text[0];
            Op o1 = opInfo(c);
            while(!ops.empty()){
                char top = ops.top();
                if (top=='(') break;
                // если top — буква, это не оператор (в этом варианте не кладём сюда функции)
                if (top=='+'||top=='-'||top=='*'||top=='/'||top=='^'){
                    Op o2 = opInfo(top);
                    if ((!o1.rightAssoc && o1.precedence<=o2.precedence) || (o1.rightAssoc && o1.precedence<o2.precedence)){
                        ops.pop();
                        out.push_back({RPNItem::ApplyOp, 0.0, o2.name, o2.arity});
                        continue;
                    }
                }
                break;
            }
            ops.push(c);
            mayUnary = true;
        }
        t = tz.next();
    }
    while(!ops.empty()){
        char top = ops.top(); ops.pop();
        if (top=='(') throw std::runtime_error("Mismatched parentheses");
        Op oi = opInfo(top);
        out.push_back({RPNItem::ApplyOp, 0.0, oi.name, oi.arity});
    }
    if (!funcs.empty() || !argCounts.empty()) throw std::runtime_error("Function arg counter mismatch");
    return out;
}
