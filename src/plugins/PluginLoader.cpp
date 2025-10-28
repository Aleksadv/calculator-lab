#include "PluginLoader.h"
#include "plugin_api.h"
#include <windows.h>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

void PluginLoader::loadAll(const std::string& dir){
    funcs.clear(); errs.clear();
    if (!fs::exists(dir)){ errs.push_back("Plugins directory not found: " + dir); return; }
    bool any=false;
    for (auto& p : fs::directory_iterator(dir)){
        if (!p.is_regular_file()) continue;
        if (p.path().extension() != ".dll") continue;
        any=true;

        HMODULE h = LoadLibraryA(p.path().string().c_str());
        if (!h){ errs.push_back("Failed to LoadLibrary: " + p.path().string()); continue; }
        handles.push_back(h);

        auto name = (plugin_name_t)GetProcAddress(h, "plugin_name");
        auto arity= (plugin_arity_t)GetProcAddress(h, "plugin_arity");
        auto eval = (plugin_eval_t)GetProcAddress(h, "plugin_eval");
        if (!name || !arity || !eval){
            errs.push_back("DLL missing required exports: " + p.path().string());
            continue;
        }
        try{
            std::string fname = name();
            int a = arity();
            if (a <= 0 || a > 8){ errs.push_back("Invalid arity in " + p.path().string()); continue; }
            if (funcs.count(fname)){ errs.push_back("Duplicate function name: " + fname); continue; }
            funcs.emplace(fname, LoadedFunction{
                a,
                [eval,a,fname](const std::vector<double>& args)->double{
                    if ((int)args.size()!=a) throw std::runtime_error("Function '"+fname+"' expects "+std::to_string(a)+" args");
                    try{ return eval(args.data(), (int)args.size()); }
                    catch (const std::exception& e){ throw std::runtime_error(std::string("Plugin '")+fname+"' error: "+e.what()); }
                    catch (...){ throw std::runtime_error(std::string("Plugin '")+fname+"' unknown error"); }
                }
            });
        } catch (const std::exception& e){
            errs.push_back(std::string("Init error in ")+p.path().string()+": "+e.what());
        }
    }
    if (!any) errs.push_back("No DLLs found in " + dir);
}
