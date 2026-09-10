#include <switch.h>
#include "fancontrol/apm.hpp"
#include "fancontrol/scope_exit.hpp"
#include "fancontrol/log.hpp"

namespace {
    Service service;

    Result ApmInititializeImpl() {
        Result rc = smInitialize();
        if (R_FAILED(rc)) {
            return rc;
        }

        rc = smGetService(&service, "apm");
        if (R_FAILED(rc)) {
            smExit();
        }

        return rc;
    }

    Result ApmGetPerformanceModeImpl(ApmPerformanceMode *outPerformanceMode) {
        return serviceDispatchOut(&service, 1, *outPerformanceMode);
    }

    void ApmExitImpl() {
        serviceClose(&service);
        smExit();
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
