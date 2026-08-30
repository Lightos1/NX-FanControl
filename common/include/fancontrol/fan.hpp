#pragma once

#include <switch.h>

struct TemperaturePoint {
    int  temperature_c;
    float fanLevel_f;
};

struct Context {
    bool enabled;
    bool dockedOverride;

    bool isDocked;

    struct {
        u32 fastRefreshTemperatureC;
        u32 slowRefreshIntervalMs;
        u32 fastRefreshIntervalMs;
        u32 configRefreshIntervalMs;
        u32 enableRefreshIntervalMs;
        u32 dockedRefreshIntervalMs;
    } refreshConfig;

    TemperaturePoint *table;
    u32 tableEntries;
};

#define MAX_TABLE_ENTRIES  32

void SortFanCurveTable(TemperaturePoint *table, u32 count);
float InterpolateFanLevel(const TemperaturePoint *table, u32 count, float temperature_c);
