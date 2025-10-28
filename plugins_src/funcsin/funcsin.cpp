#include "plugin_api.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <stdexcept>
PLG_EXPORT const char* plugin_name(){ return "sin"; }
PLG_EXPORT int         plugin_arity(){ return 1; }

PLG_EXPORT double plugin_eval(const double* args, int argc){
    if (argc!=1) throw std::runtime_error("sin expects 1 arg");
    double deg = args[0];
    double rad = deg * M_PI / 180.0;
    return std::sin(rad);
}
