#include <fancontrol.hpp>
#include "fancontrol.hpp"
#include "fan_hysteresis.hpp"
#include <atomic>
#include <math.h>
#include <sys/stat.h>

TemperaturePoint *fanControllerTable;
Thread           FanControllerThread;

#define POLL_NORMAL_NS     50000000ULL
#define POLL_FAST_NS       25000000ULL
#define TEMP_FAST_THRESH        55.0f
#define CONFIG_CHECK_INTERVAL_NS 2000000000ULL
#define TEMP_READ_RETRIES         3

void InitFanController(TemperaturePoint *table) {
    fanControllerTable = table;
}

bool ValidateFanCurveTable(const TemperaturePoint *tbl) {
    for (size_t i = 0; i + 1 < TABLE_ENTRIES; i++) {
        if (tbl[i].temperature_c > tbl[i + 1].temperature_c) {
            return false;
        }
    }
    return true;
}

static bool GetConfigStat(const char *path, time_t *mtime, off_t *size) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }
    *mtime = st.st_mtime;
    *size  = st.st_size;
    return true;
}

static time_t GetConfigMTime(const char *path) {
    time_t mtime = 0;
    off_t  size  = 0;
    return GetConfigStat(path, &mtime, &size) ? mtime : 0;
}

static bool TryReloadConfig(time_t &lastMTime) {
    time_t mtime = 0;
    off_t  size  = 0;
    if (!GetConfigStat(FC_CONFIG_FILE, &mtime, &size) || mtime == 0 || mtime == lastMTime) {
        return false;
    }

    if (size != static_cast<off_t>(TABLE_SIZE)) {
        return false;
    }

    TemperaturePoint *newTable = nullptr;
    ReadConfigFile(&newTable);

    SortFanCurveTable(newTable);

    if (!ValidateFanCurveTable(newTable)) {
        free(newTable);
        static time_t lastBadMTime = 0;
        if (mtime != lastBadMTime) {
            WriteLog("Config validation failed");
            lastBadMTime = mtime;
        }
        return false;
    }

    TemperaturePoint *oldTable = fanControllerTable;
    fanControllerTable = newTable;
    free(oldTable);

    lastMTime = mtime;
    return true;
}

static void RefreshConfig(FanHysteresisState *fanState) {
    static time_t lastCfgMTime  = GetConfigMTime(FC_CONFIG_FILE);
    static u64    lastCheckTime = armGetSystemTick();

    u64 now = armGetSystemTick();
    if (armTicksToNs(now - lastCheckTime) >= CONFIG_CHECK_INTERVAL_NS) {
        lastCheckTime = now;
        if (TryReloadConfig(lastCfgMTime)) {
            RebindFanHysteresisTable(fanState, fanControllerTable);
            WriteLog("Config reloaded");
        }
    }
}

void LoopFanController() {
    FanController fc;
    float tempC = 0.0f;

    Result rs = fanOpenController(&fc, 0x3D000001);
    if (R_FAILED(rs)) {
        WriteLog("Error opening fanController");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }

    FanHysteresisState fanState{};
    InitFanHysteresis(&fanState, fanControllerTable, 2.0f);

    for (;;) {
        RefreshConfig(&fanState);

        bool readOk = false;
        for (u32 retry = 0; retry < TEMP_READ_RETRIES; ++retry) {
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
}

/* This should never be called, but just in case */
void CleanupFanController() {
    free(fanControllerTable);
    fanControllerTable = NULL;
}
