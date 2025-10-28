#include <iostream>
#include <string>
#include "calculator/ShuntingYard.h"
#include "calculator/Evaluator.h"
#include "plugins/PluginLoader.h"

int main(){
    try{
        PluginLoader loader;
        loader.loadAll("../plugins");
        for (auto& e : loader.errors()){
            std::cerr << "[plugin warning] " << e << "\n";
        }

        FuncRegistry reg{
            [&loader](const std::string& name, int& arity, std::function<double(const std::vector<double>&)>& f)->bool{
                const auto& map = loader.functions();
                auto it = map.find(name);
                if (it==map.end()) return false;
                arity = it->second.arity;
                f = it->second.call;
                return true;
            }
        };

        std::cout << "| Enter expression (empty line to exit): |\n";
        std::string line;
        while (true){
            std::cout << "> ";
            if (!std::getline(std::cin, line) || line.empty()) break;
            try{
                auto rpn = ShuntingYard::toRPN(line);
                double res = Evaluator::evalRPN(rpn, reg);
                std::cout << res << "\n";
            } catch (const std::exception& e){
                std::cout << "error: " << e.what() << "\n";
            }
        }
        return 0;
    } catch (const std::exception& e){
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}
