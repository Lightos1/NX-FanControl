#include <switch.h>
#include "fancontrol/apm.hpp"

bool IsDocked() {
    ApmPerformanceMode mode = ApmPerformanceMode_Normal;
    if (R_FAILED(apmGetPerformanceMode(&mode))) {
        return false;
    }
    return mode == ApmPerformanceMode_Boost;
}
