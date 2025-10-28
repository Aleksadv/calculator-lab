#include "plugin_api.h"
#include <cmath>
#include <stdexcept>

PLG_EXPORT const char* plugin_name() { return "ln"; }
PLG_EXPORT int         plugin_arity(){ return 1; }

PLG_EXPORT double plugin_eval(const double* args, int argc) {
    if (argc != 1) throw std::runtime_error("ln expects 1 arg");
    double x = args[0];
    if (x <= 0.0) throw std::domain_error("ln domain error: x<=0");
    return std::log(x);
}
