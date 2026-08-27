#pragma once

#include <switch.h>

struct TemperaturePoint {
    int  temperature_c;
    float fanLevel_f;
};

#define TABLE_ENTRIES  10
#define TABLE_SIZE     (sizeof(TemperaturePoint) * TABLE_ENTRIES)

extern const TemperaturePoint defaultTable[TABLE_ENTRIES];
