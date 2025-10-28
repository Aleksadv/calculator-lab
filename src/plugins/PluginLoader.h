#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

struct LoadedFunction {
    int arity;
    std::function<double(const std::vector<double>&)> call;
};

class PluginLoader {
public:
    void loadAll(const std::string& dir = "./plugins");
    const std::unordered_map<std::string, LoadedFunction>& functions() const { return funcs; }
    const std::vector<std::string>& errors() const { return errs; }
private:
    std::unordered_map<std::string, LoadedFunction> funcs;
    std::vector<void*> handles;
    std::vector<std::string> errs;
};
