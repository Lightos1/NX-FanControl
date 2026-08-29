#include <fancontrol.hpp>
#include "fancontrol.hpp"
#include "fan_hysteresis.hpp"
#include <atomic>
#include <math.h>
#include <sys/stat.h>

Context *ctx;

#define POLL_NORMAL_NS     50000000ULL
#define POLL_FAST_NS       25000000ULL
#define TEMP_FAST_THRESH        55.0f
#define CONFIG_CHECK_INTERVAL_NS 2000000000ULL
#define DISABLED_SLEEP_NS 2000000000ULL
#define TEMP_READ_RETRIES         3

bool ValidateFanCurveTable(const TemperaturePoint *tbl, u32 count) {
    for (u32 i = 0; i + 1 < count; i++) {
        if (tbl[i].temperature_c > tbl[i + 1].temperature_c) {
            return false;
        }
    }
    return true;
}

static bool LoadCurveAlloc(const char *curveSection, TemperaturePoint **outTable, u32 *outCount) {
    u32 count = GetPointCount(curveSection);
    if (count == 0 || count > MAX_TABLE_ENTRIES) {
        return false;
    }

    TemperaturePoint *table = static_cast<TemperaturePoint *>(malloc(sizeof(TemperaturePoint) * count));
    if (table == nullptr) {
        return false;
    }

    if (LoadCurve(curveSection, table, count) != count) {
        free(table);
        return false;
    }

    SortFanCurveTable(table, count);

    if (!ValidateFanCurveTable(table, count)) {
        free(table);
        return false;
    }

    *outTable = table;
    *outCount = count;
    return true;
}

void InitContext(Context *_ctx) {
    ctx = _ctx;

    ctx->table        = nullptr;
    ctx->tableEntries = 0;
    ctx->tableSize    = 0;
    ctx->enabled      = false;

    TemperaturePoint *table = nullptr;
    u32 count = 0;
    if (!LoadCurveAlloc(CurveSection, &table, &count)) {
        WriteLog("No valid curve at init, starting disabled");
        return;
    }

    ctx->table        = table;
    ctx->tableEntries = count;
    ctx->tableSize    = sizeof(TemperaturePoint) * count;
    ctx->enabled      = IsEnabled(ConfigSection);
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

static bool TryReloadConfig(const char *configSection, const char *curveSection, time_t &lastMTime) {
    time_t mtime = 0;
    off_t  size  = 0;
    if (!GetConfigStat(FC_CONFIG_INI, &mtime, &size) || mtime == 0 || mtime == lastMTime) {
        return false;
    }
    (void) size;

    lastMTime = mtime;

    ctx->enabled = IsEnabled(configSection);

    TemperaturePoint *newTable = nullptr;
    u32 newCount = 0;
    if (!LoadCurveAlloc(curveSection, &newTable, &newCount)) {
        static time_t lastBadMTime = 0;
        if (mtime != lastBadMTime) {
            WriteLog("Config reload failed, keeping current curve");
            lastBadMTime = mtime;
        }
        return false;
    }

    TemperaturePoint *oldTable = ctx->table;
    ctx->table        = newTable;
    ctx->tableEntries = newCount;
    ctx->tableSize    = sizeof(TemperaturePoint) * newCount;
    free(oldTable);

    return true;
}

static void RefreshConfig(const char *configSection, const char *curveSection, FanHysteresisState *fanState) {
    static time_t lastCfgMTime  = GetConfigMTime(FC_CONFIG_INI);
    static u64    lastCheckTime = armGetSystemTick();

    u64 now = armGetSystemTick();
    if (armTicksToNs(now - lastCheckTime) >= CONFIG_CHECK_INTERVAL_NS) {
        lastCheckTime = now;
        if (TryReloadConfig(configSection, curveSection, lastCfgMTime)) {
            RebindFanHysteresisTable(fanState, ctx->table, ctx->tableEntries);
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
    InitFanHysteresis(&fanState, ctx->table, ctx->tableEntries, 2.0f);

    for (;;) {
        RefreshConfig(ConfigSection, CurveSection, &fanState);

        if (!ctx->enabled || ctx->table == nullptr) {
            svcSleepThread(DISABLED_SLEEP_NS);
            continue;
        }

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
    if (ctx == nullptr) {
        return;
    }
    free(ctx->table);
    ctx->table = nullptr;
}
