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

    TemperaturePoint *table;
    u32 tableEntries;
};

#define MAX_TABLE_ENTRIES  32

void SortFanCurveTable(TemperaturePoint *table, u32 count);
float InterpolateFanLevel(const TemperaturePoint *table, u32 count, float temperature_c);
