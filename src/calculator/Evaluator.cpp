#include "Evaluator.h"
#include <cmath>
#include <stdexcept>

static double applyOp(const std::string& op, double a, double b){
    if (op=="+") return a+b;
    if (op=="-") return a-b;
    if (op=="*") return a*b;
    if (op=="/"){ if (b==0.0) throw std::runtime_error("Division by zero"); return a/b; }
    if (op=="^") return std::pow(a,b);
    throw std::runtime_error("Unknown operator: " + op);
}

double Evaluator::evalRPN(const std::vector<RPNItem>& rpn, const FuncRegistry& reg){
    std::vector<double> st; st.reserve(rpn.size());
    for (const auto& it : rpn){
        if (it.kind == RPNItem::PushNumber){
            st.push_back(it.number);
        } else { // ApplyOp
            if (it.ident=="+"||it.ident=="-"||it.ident=="*"||it.ident=="/"||it.ident=="^"){
                if (st.size()<2) throw std::runtime_error("Stack underflow (operator)");
                double b=st.back(); st.pop_back();
                double a=st.back(); st.pop_back();
                st.push_back(applyOp(it.ident,a,b));
            } else {
                int arity=0;
                std::function<double(const std::vector<double>&)> f;
                if (!reg.resolver || !reg.resolver(it.ident, arity, f)) throw std::runtime_error("Function not found: "+it.ident);
                if (arity != it.argc) throw std::runtime_error("Function '"+it.ident+"' arity mismatch");
                if (st.size() < (size_t)arity) throw std::runtime_error("Stack underflow (function)");
                std::vector<double> args(arity);
                for (int i=arity-1;i>=0;--i){ args[i]=st.back(); st.pop_back(); }
                st.push_back(f(args));
            }
        }
    }
    if (st.size()!=1) throw std::runtime_error("Evaluation error: stack size="+std::to_string(st.size()));
    return st.back();
}
