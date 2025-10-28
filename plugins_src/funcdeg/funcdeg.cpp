#include "plugin_api.h"
#include <cmath>
#include <stdexcept>

PLG_EXPORT const char* plugin_name() { return "^"; }
PLG_EXPORT int         plugin_arity(){ return 2; }

PLG_EXPORT double plugin_eval(const double* args, int argc) {
    if (argc != 2) throw std::runtime_error("^ expects 2 args");
    // pow(a, b)
    return std::pow(args[0], args[1]);
}
