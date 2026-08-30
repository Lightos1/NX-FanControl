#include <fancontrol.hpp>
#include "fancontrol.hpp"
#include "fan_hysteresis.hpp"
#include <algorithm>
#include <atomic>
#include <math.h>
#include <sys/stat.h>

Context *ctx;

#define TEMP_READ_RETRIES 3

static u64 MsToNs(u64 ms) {
    return ms * 1'000'000ULL;
}

bool ValidateFanCurveTable(const TemperaturePoint *tbl, u32 count) {
    for (u32 i = 0; i + 1 < count; i++) {
        if (tbl[i].temperature_c > tbl[i + 1].temperature_c) {
            return false;
        }
    }
    return true;
}

static bool IntervalElapsed(u64 *last, u64 intervalNs) {
    u64 now = armGetSystemTick();
    if (armTicksToNs(now - *last) < intervalNs) {
        return false;
    }
    *last = now;
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

static const char *GetProfileCurve() {
    if (ctx->isDocked) {
        return DockedOverrideCurveSection;
    }

    return CurveSection;
}

static bool SwapCurveTable(const char *curveSection, FanHysteresisState *fanState) {
    TemperaturePoint *newTable = nullptr;
    u32 newCount = 0;
    if (!LoadCurveAlloc(curveSection, &newTable, &newCount)) {
        return false;
    }

    TemperaturePoint *oldTable = ctx->table;
    ctx->table        = newTable;
    ctx->tableEntries = newCount;
    free(oldTable);
    RebindFanHysteresisTable(fanState, ctx->table, ctx->tableEntries);
    return true;
}

static void LoadConfig() {
    ctx->enabled                               = IsEnabled(ConfigSection);
    ctx->refreshConfig.fastRefreshTemperatureC = std::clamp(GetFastRefreshTemperatureC(ConfigSection), MinFastRefreshTempC, MaxFastRefreshTempC);
    ctx->refreshConfig.slowRefreshIntervalMs   = std::max(GetRefreshInterval(ConfigSection, KeySlowRefreshIntervalMs, DefaultSlowRefreshIntervalMs), MinPollIntervalMs);
    ctx->refreshConfig.fastRefreshIntervalMs   = std::max(GetRefreshInterval(ConfigSection, KeyFastRefreshIntervalMs, DefaultFastRefreshIntervalMs), MinPollIntervalMs);
    ctx->refreshConfig.configRefreshIntervalMs = std::max(GetRefreshInterval(ConfigSection, KeyConfigRefreshIntervalMs, DefaultConfigRefreshIntervalMs), MinCheckIntervalMs);
    ctx->refreshConfig.enableRefreshIntervalMs = std::max(GetRefreshInterval(ConfigSection, KeyEnableRefreshIntervalMs, DefaultEnableRefreshIntervalMs), MinCheckIntervalMs);
    ctx->refreshConfig.dockedRefreshIntervalMs = std::max(GetRefreshInterval(ConfigSection, KeyDockedRefreshIntervalMs, DefaultDockedRefreshIntervalMs), MinCheckIntervalMs);
}

void InitContext(Context *_ctx) {
    ctx = _ctx;

    ctx->table          = nullptr;
    ctx->tableEntries   = 0;
    ctx->dockedOverride = IsDockedOverride(ConfigSection);
    ctx->isDocked       = ctx->dockedOverride && IsDocked();

    LoadConfig();

    TemperaturePoint *table = nullptr;
    u32 count = 0;
    if (!LoadCurveAlloc(GetProfileCurve(), &table, &count)) {
        WriteLog("No valid curve at init, starting disabled");
        return;
    }

    ctx->table        = table;
    ctx->tableEntries = count;
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

static bool TryReloadConfig(const char *configSection, FanHysteresisState *fanState, time_t &lastMTime) {
    time_t mtime = 0;
    off_t  size  = 0;
    if (!GetConfigStat(FC_CONFIG_INI, &mtime, &size) || mtime == 0 || mtime == lastMTime) {
        return false;
    }
    (void) size;

    lastMTime = mtime;

    ctx->dockedOverride = IsDockedOverride(configSection);
    ctx->isDocked       = ctx->dockedOverride && IsDocked();
    LoadConfig();

    if (!SwapCurveTable(GetProfileCurve(), fanState)) {
        static time_t lastBadMTime = 0;
        if (mtime != lastBadMTime) {
            WriteLog("Config reload failed, keeping current curve");
            lastBadMTime = mtime;
        }
        return false;
    }

    return true;
}

static void RefreshConfig(const char *configSection, FanHysteresisState *fanState) {
    static time_t lastCfgMTime  = GetConfigMTime(FC_CONFIG_INI);
    static u64    lastCheckTime = armGetSystemTick();

    if (IntervalElapsed(&lastCheckTime, MsToNs(ctx->refreshConfig.configRefreshIntervalMs))) {
        if (TryReloadConfig(configSection, fanState, lastCfgMTime)) {
            WriteLog("Config reloaded");
        }
    }
}

static bool HasDockChanged(bool newState) {
    return newState != ctx->isDocked;
}

static void HandleDockRefresh(FanHysteresisState *fanState) {
    bool docked = IsDocked();
    if (HasDockChanged(docked)) {
        ctx->isDocked = docked;
        SwapCurveTable(GetProfileCurve(), fanState);
    }
}

static void RefreshDockedState(FanHysteresisState *fanState) {
    if (!ctx->dockedOverride) {
        return;
    }

    static u64 lastCheckTime = armGetSystemTick();

    if (IntervalElapsed(&lastCheckTime, MsToNs(ctx->refreshConfig.dockedRefreshIntervalMs))) {
        HandleDockRefresh(fanState);
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
        RefreshConfig(ConfigSection, &fanState);
        RefreshDockedState(&fanState);

        if (!ctx->enabled || ctx->table == nullptr) {
            svcSleepThread(MsToNs(ctx->refreshConfig.enableRefreshIntervalMs));
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

        u32 intervalMs = (tempC >= ctx->refreshConfig.fastRefreshTemperatureC) ? ctx->refreshConfig.fastRefreshIntervalMs : ctx->refreshConfig.slowRefreshIntervalMs;
        svcSleepThread(MsToNs(intervalMs));
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
