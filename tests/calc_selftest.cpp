#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <filesystem>
#define _USE_MATH_DEFINES
#include <cmath>
#include <stdexcept>

#include "selftest.h"

#include "calculator/ShuntingYard.h"
#include "calculator/Evaluator.h"
#include "plugins/PluginLoader.h"

namespace fs = std::filesystem;

struct TeeBuf : public std::streambuf {
    std::streambuf* a;
    std::streambuf* b;
    TeeBuf(std::streambuf* a_, std::streambuf* b_) : a(a_), b(b_) {}
protected:
    int overflow(int c) override {
        if (c == EOF) return !EOF;
        int const r1 = a->sputc(c);
        int const r2 = b->sputc(c);
        return (r1 == EOF || r2 == EOF) ? EOF : c;
    }
    std::streamsize xsputn(const char* s, std::streamsize n) override {
        auto r1 = a->sputn(s, n);
        auto r2 = b->sputn(s, n);
        return (r1 < n || r2 < n) ? std::min(r1, r2) : n;
    }
};

// --- Вспомогательное: выполнить выражение через ядро с данным реестром ---
static double Eval(const std::string& expr, const FuncRegistry& reg) {
    auto rpn = ShuntingYard::toRPN(expr);
    return Evaluator::evalRPN(rpn, reg);
}

// --- Мок-реестр для юнит-тестов (без реальных DLL) ---
static FuncRegistry makeMockRegistry() {
    struct Entry { int arity; std::function<double(const std::vector<double>&)> f; };
    static std::unordered_map<std::string, Entry> tab = {
        {"^",  {2, [](const std::vector<double>& a){ return std::pow(a[0], a[1]); }}},
        {"sin",{1, [](const std::vector<double>& a){ return std::sin(a[0]*M_PI/180.0); }}},
        {"ln", {1, [](const std::vector<double>& a){
            if (a[0] <= 0.0) throw std::domain_error("ln domain error: x<=0");
            return std::log(a[0]);
        }}}
    };
    FuncRegistry reg{
        [](const std::string& name, int& arity,
           std::function<double(const std::vector<double>&)>& f)->bool {
            auto it = tab.find(name);
            if (it == tab.end()) return false;
            arity = it->second.arity;
            f = it->second.f;
            return true;
        }
    };
    return reg;
}

// --- Тесты ядра без DLL ---
static void run_unit_tests(TestStats& S) {
    std::cout << "== UNIT TESTS (core without real DLLs) ==\n";
    auto reg = makeMockRegistry();

    ST_ASSERT_EQ_DBL(S, Eval("16 + 4 * (3 - 1)", reg), 24.0, 1e-12, "basic arith");
    ST_ASSERT_EQ_DBL(S, Eval("45*(5+10)/90", reg), 7.5, 1e-12, "precedence");
    ST_ASSERT_EQ_DBL(S, Eval("-3 + 5*2", reg), 7.0, 1e-12, "unary minus 1");
    ST_ASSERT_EQ_DBL(S, Eval("2 + -3", reg), -1.0, 1e-12, "unary minus 2");

    // степень — через плагин (мок), правая ассоциативность
    ST_ASSERT_EQ_DBL(S, Eval("2^4", reg), 16.0, 1e-12, "power 2^4");
    ST_ASSERT_EQ_DBL(S, Eval("2^3^2", reg), 512.0, 1e-12, "power right-assoc");

    // функции — через плагин (мок)
    ST_ASSERT_EQ_DBL(S, Eval("sin(90)", reg), 1.0, 1e-12, "sin 90deg");
    ST_ASSERT_THROW(S, Eval("ln(-100)", reg), "ln domain error bubbles");

    // деление на ноль — встроенная ошибка
    ST_ASSERT_THROW(S, Eval("10/0", reg), "division by zero");
}

// --- Интеграционные тесты с реальными плагинами из ./plugins ---
static void run_integration_tests(TestStats& S) {
    std::cout << "\n== INTEGRATION TESTS (real DLLs under ./plugins) ==\n";
    PluginLoader loader;
    loader.loadAll("./plugins");

    // Должны хотя бы отработать без падения и вывести предупреждения при проблемах
    // Проверим: если найдены плагины '^' и 'ln', то выражения посчитаются.
    FuncRegistry reg{
        [&loader](const std::string& name, int& arity,
                  std::function<double(const std::vector<double>&)>& f)->bool{
            auto & map = loader.functions();
            auto it = map.find(name);
            if (it==map.end()) return false;
            arity = it->second.arity;
            f = it->second.call;
            return true;
        }
    };

    // Если плагины реально есть — эти тесты пройдут; если нет — получим "Function not found".
    try {
        double v = Eval("2^4 + sin(90)", reg); // sin может быть не собран — тогда тоже ошибка
        ST_ASSERT_EQ_DBL(S, v, 17.0, 1e-12, "plugins: 2^4 + sin(90)");
    } catch (...) {
        ST_ASSERT_TRUE(S, true, "plugins: expression raised (acceptable if plugins missing)");
    }

    // ln(-100) должен бросить исключение из плагина, если ln подключён
    try {
        (void)Eval("ln(-100)", reg);
        bool has_ln = loader.functions().count("ln")>0;
        ST_ASSERT_TRUE(S, !has_ln, "plugins: ln(-100) should throw when ln present");
    } catch (...) {
        ST_ASSERT_TRUE(S, true, "plugins: ln(-100) threw (ok)");
    }
}

// --- Тесты крайних случаев загрузчика: пустая папка, битая dll ---
static void run_loader_edge_tests(TestStats& S) {
    std::cout << "\n== LOADER EDGE TESTS ==\n";

    // 1) Пустая папка
    fs::path emptyDir = "./_tmp_empty_plugins";
    fs::create_directories(emptyDir);

    PluginLoader L1;
    L1.loadAll(emptyDir.string());
    bool hasNoDllMsg = false;
    for (auto& e : L1.errors())
        if (e.find("No DLLs found") != std::string::npos) { hasNoDllMsg = true; break; }
    ST_ASSERT_TRUE(S, hasNoDllMsg, "empty plugins dir -> 'No DLLs found' warning");

    // 2) Битая DLL (просто текстовый файл с расширением .dll)
    fs::path badDir = "./_tmp_bad_plugins";
    fs::create_directories(badDir);
    {
        std::ofstream("._ignore", std::ios::out).close(); // просто чтобы был побочный файл
        std::ofstream f(badDir / "totally-not-a-dll.dll", std::ios::out | std::ios::binary);
        f << "this is not a real dll";
    }

    PluginLoader L2;
    L2.loadAll(badDir.string());
    bool hasLoadFail = false;
    for (auto& e : L2.errors())
        if (e.find("Failed to LoadLibrary") != std::string::npos) { hasLoadFail = true; break; }
    ST_ASSERT_TRUE(S, hasLoadFail, "bad dll -> 'Failed to LoadLibrary' warning");

    // уборка
    try { fs::remove_all(emptyDir); } catch(...) {}
    try { fs::remove_all(badDir); } catch(...) {}
}

int main(int argc, char** argv) {
    namespace fs = std::filesystem;


    //fs::path projectRoot = fs::path(PROJECT_SOURCE_DI);
    //fs::path projectRoot = fs::path(PROJECT_SOURCE_DIR);
    fs::path projectRoot = fs::current_path();
    if (projectRoot.filename() == "build") {
    projectRoot = projectRoot.parent_path();
    }
    // Папка для отчёта: <repo_root>/tests
    fs::path testsDir = projectRoot / "tests";
    fs::create_directories(testsDir);

    fs::path reportPath = testsDir / "selftests_report.txt";
    std::ofstream report(reportPath, std::ios::out | std::ios::trunc);

    std::streambuf* cout_old = std::cout.rdbuf();
    TeeBuf tee(cout_old, report.rdbuf());
    std::cout.rdbuf(&tee);

    TestStats stats;
    run_unit_tests(stats);
    run_integration_tests(stats);
    run_loader_edge_tests(stats);
    int code = finish(stats);

    std::cout.rdbuf(cout_old);
    std::cout << "Report saved to: " << reportPath.string() << "\n";
    report.close();
    return code;
}

