#include <fancontrol.hpp>
#include <algorithm>
#include "fancontrol.hpp"
#include "fan_hysteresis.hpp"
#include <atomic>
#include <math.h>

TemperaturePoint     *fanControllerTable;
Thread                FanControllerThread;
static std::atomic_bool fanControllerThreadExit{false};

#define POLL_NORMAL_NS     50000000ULL   /*  50 ms – normal rate          */
#define POLL_FAST_NS       25000000ULL   /*  25 ms – when temp is high    */
#define TEMP_FAST_THRESH        55.0f    /* switch to fast poll above this */
#define TEMP_READ_RETRIES         3

void InitFanController(TemperaturePoint *table) {
    fanControllerTable = table;

    if (R_FAILED(threadCreate(&FanControllerThread, FanControllerThreadFunction, NULL, NULL, 0x4000, 0x3F, -2))) {
        WriteLog("Error creating FanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
}

static void SortFanCurveTable(TemperaturePoint *tbl, size_t count) {
    std::sort(tbl, tbl + count, [](const TemperaturePoint &a, const TemperaturePoint &b) { return a.temperature_c < b.temperature_c; });
}

[[maybe_unused]] static bool ValidateFanCurveTable(const TemperaturePoint *tbl, size_t count) {
    if (count == 0) {
        return false;
    }
    for (size_t i = 0; i + 1 < count; i++) {
        if (tbl[i].temperature_c >= tbl[i + 1].temperature_c) {
            return false;
        }
    }
    return true;
}

void FanControllerThreadFunction(void *arg)
{
    (void)arg;

    FanController fc;
    float tempC        =  0.0f;

    Result rs = fanOpenController(&fc, 0x3D000001);
    if (R_FAILED(rs)) {
        WriteLog("Error opening fanController");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }

    FanHysteresisState fanState{};
    InitFanHysteresis(&fanState, fanControllerTable, 2.0f);
    SortFanCurveTable(fanControllerTable, TABLE_ENTRIES);

    while (!fanControllerThreadExit.load(std::memory_order_relaxed)) {
        bool readOk = false;
        for (int retry = 0; retry < TEMP_READ_RETRIES; ++retry) {
            rs = Tmp451GetSocTemp(&tempC);
            if (R_SUCCEEDED(rs)) {
                readOk = true;
                break;
            }
        }

        if (!readOk) {
            WriteLog("Tmp451GetSocTemp failed after retries");
            tempC = 70.0f;
        }

        float target = UpdateFanHysteresis(&fanState, tempC);

        rs = fanControllerSetRotationSpeedLevel(&fc, target);
        if (R_FAILED(rs)) {
            WriteLog("fanControllerSetRotationSpeedLevel error");
            diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
        }

        u64 interval = (tempC >= TEMP_FAST_THRESH) ? POLL_FAST_NS : POLL_NORMAL_NS;
        svcSleepThread(interval);
    }

    fanControllerClose(&fc);
}

void StartFanControllerThread(void) {
    if (R_FAILED(threadStart(&FanControllerThread))) {
        WriteLog("Error starting FanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
}

void CloseFanControllerThread(void) {
    fanControllerThreadExit.store(true, std::memory_order_release);

    Result rs = threadWaitForExit(&FanControllerThread);
    if (R_FAILED(rs)) {
        WriteLog("Error waiting fanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }

    threadClose(&FanControllerThread);

    fanControllerThreadExit.store(false, std::memory_order_relaxed);

    free(fanControllerTable);
    fanControllerTable = NULL;
}

void WaitFanController(void) {
    if (R_FAILED(threadWaitForExit(&FanControllerThread))) {
        WriteLog("Error waiting fanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
}
