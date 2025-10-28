#pragma once
#include <cstddef>

#ifdef _WIN32
  #define PLG_EXPORT extern "C" __declspec(dllexport)
#else
  #define PLG_EXPORT extern "C"
#endif

typedef const char* (*plugin_name_t)(); // Имя функции
typedef int (*plugin_arity_t)(); // кол-во аргументов (например 1 для sin(x))
typedef double (*plugin_eval_t)(const double* args, int argc); // Вычисление: args[0..argc-1] -> double (может бросать исключения)

// Экспортируемые символы из каждой DLL:
PLG_EXPORT const char* plugin_name();
PLG_EXPORT int         plugin_arity();
PLG_EXPORT double      plugin_eval(const double* args, int argc);
