#pragma once
#include <fancontrol.hpp>

struct FanHysteresisState {
    const TemperaturePoint *tbl;
    float margin;
    bool  hasLast;
    float lastTempC;
    float lastLevel;
};

void InitFanHysteresis(FanHysteresisState *state, const TemperaturePoint *tbl, float margin);
float UpdateFanHysteresis(FanHysteresisState *state, float tempC);
void RebindFanHysteresisTable(FanHysteresisState *state, const TemperaturePoint *newTable);
