#include <fancontrol.hpp>
#include "fancontrol.hpp"
#include <atomic>
#include <math.h>

/* ── State ────────────────────────────────────────────────────────── */

TemperaturePoint     *fanControllerTable;
Thread                FanControllerThread;
static std::atomic_bool fanControllerThreadExit{false};

/* ── Tuning constants ─────────────────────────────────────────────── */

#define POLL_NORMAL_NS     50000000ULL   /*  50 ms – normal rate          */
#define POLL_FAST_NS       25000000ULL   /*  25 ms – when temp is high    */
#define TEMP_FAST_THRESH        55.0f    /* switch to fast poll above this */
#define TEMP_READ_RETRIES         3

/* ── Interpolation ────────────────────────────────────────────────── */

static inline float InterpolateFanLevel(const TemperaturePoint *tbl, float tempC)
{
    if (tempC <= tbl[0].temperature_c)
        return tbl[0].fanLevel_f;

    for (size_t i = 0; i < TABLE_ENTRIES - 1; i++)
    {
        if (tempC <= tbl[i + 1].temperature_c)
        {
            float dT = tbl[i + 1].temperature_c - tbl[i].temperature_c;
            float dF = tbl[i + 1].fanLevel_f    - tbl[i].fanLevel_f;
            float t  = (tempC - tbl[i].temperature_c) / dT;
            return tbl[i].fanLevel_f + dF * t;
        }
    }

    return tbl[TABLE_ENTRIES - 1].fanLevel_f;
}

/* ── Fan controller ───────────────────────────────────────────────── */

void InitFanController(TemperaturePoint *table)
{
    fanControllerTable = table;

    if (R_FAILED(threadCreate(&FanControllerThread,
                              FanControllerThreadFunction,
                              NULL, NULL, 0x4000, 0x3F, -2)))
    {
        WriteLog("Error creating FanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
}

void FanControllerThreadFunction(void *arg)
{
    (void)arg;

    FanController fc;
    float tempC        =  0.0f;

    Result rs = fanOpenController(&fc, 0x3D000001);
    if (R_FAILED(rs))
    {
        WriteLog("Error opening fanController");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }

    while (!fanControllerThreadExit.load(std::memory_order_relaxed))
    {
        /* ── Read temperature with retry ────────────────────────── */
        bool readOk = false;
        for (int retry = 0; retry < TEMP_READ_RETRIES; retry++)
        {
            rs = Tmp451GetSocTemp(&tempC);
            if (R_SUCCEEDED(rs))
            {
                readOk = true;
                break;
            }
        }

        if (!readOk)
        {
            WriteLog("Tmp451GetSocTemp failed after retries");
            tempC = 70.0f;
        }

        /* ── Compute target fan level ───────────────────────────── */
        float target  = InterpolateFanLevel(fanControllerTable, tempC);

        /* ── Always update fan speed for immediate response ─────── */
        rs = fanControllerSetRotationSpeedLevel(&fc, target);
        if (R_FAILED(rs))
        {
            WriteLog("fanControllerSetRotationSpeedLevel error");
            diagAbortWithResult(
                MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
        }

        /* ── Adaptive sleep ─────────────────────────────────────── */
        u64 interval = (tempC >= TEMP_FAST_THRESH)
                      ? POLL_FAST_NS
                      : POLL_NORMAL_NS;
        svcSleepThread(interval);
    }

    fanControllerClose(&fc);
}

/* ── Thread lifecycle ─────────────────────────────────────────────── */

void StartFanControllerThread(void)
{
    if (R_FAILED(threadStart(&FanControllerThread)))
    {
        WriteLog("Error starting FanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
}

void CloseFanControllerThread(void)
{
    fanControllerThreadExit.store(true, std::memory_order_release);

    Result rs = threadWaitForExit(&FanControllerThread);
    if (R_FAILED(rs))
    {
        WriteLog("Error waiting fanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }

    threadClose(&FanControllerThread);

    fanControllerThreadExit.store(false, std::memory_order_relaxed);

    free(fanControllerTable);
    fanControllerTable = NULL;
}

void WaitFanController(void)
{
    if (R_FAILED(threadWaitForExit(&FanControllerThread)))
    {
        WriteLog("Error waiting fanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
}