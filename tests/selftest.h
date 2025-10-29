#pragma once
#include <iostream>
#include <cmath>
#include <string>
#include <functional>

struct TestStats {
    int passed = 0;
    int failed = 0;
};

#define ST_ASSERT_TRUE(stats, cond, msg) \
    do { \
        if (cond) { (stats).passed++; std::cout << "[PASS] " << msg << "\n"; } \
        else { (stats).failed++; std::cout << "[FAIL] " << msg << "\n"; } \
    } while(0)

#define ST_ASSERT_EQ_DBL(stats, got, exp, eps, msg) \
    do { \
        double __g = (got); double __e = (exp); \
        if (std::fabs(__g - __e) <= (eps)) { \
            (stats).passed++; std::cout << "[PASS] " << msg << " -> " << __g << "\n"; \
        } else { \
            (stats).failed++; std::cout << "[FAIL] " << msg << " -> got " << __g << ", expect " << __e << "\n"; \
        } \
    } while(0)

#define ST_ASSERT_THROW(stats, stmt, msg) \
    do { \
        bool __thrown=false; \
        try { stmt; } catch (...) { __thrown=true; } \
        if (__thrown) { (stats).passed++; std::cout << "[PASS] " << msg << " (thrown)\n"; } \
        else { (stats).failed++; std::cout << "[FAIL] " << msg << " (no throw)\n"; } \
    } while(0)

inline int finish(const TestStats& s) {
    std::cout << "\n==== SUMMARY: " << s.passed << " passed, " << s.failed << " failed ====\n";
    return s.failed == 0 ? 0 : 1;
}
