#include <switch.h>
#include "fancontrol/apm.hpp"
#include "fancontrol/scope_exit.hpp"

namespace {
    Service service;

    Result ApmInititializeImpl() {
        return smGetService(&service, "apm");
    }

    Result ApmGetPerformanceModeImpl(ApmPerformanceMode *outPerformanceMode) {
        return serviceDispatchOut(&service, 1, *outPerformanceMode);
    }

    void ApmExitImpl() {
        serviceClose(&service);
    }

}

bool IsDocked() {
    if (R_FAILED(ApmInititializeImpl())) {
        return false;
    }

    ON_SCOPE_EXIT { ApmExitImpl(); };

    ApmPerformanceMode mode = ApmPerformanceMode_Normal;
    if (R_FAILED(ApmGetPerformanceModeImpl(&mode))) {
        return false;
    }

    return mode == ApmPerformanceMode_Boost;
}
