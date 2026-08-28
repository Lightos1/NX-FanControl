#include <cmath>
#include <fancontrol.hpp>
#include "fan_hysteresis.hpp"

void InitFanHysteresis(FanHysteresisState *state, const TemperaturePoint *tbl, float margin) {
    state->tbl        = tbl;
    state->margin     = margin;
    state->hasLast    = false;
    state->lastTempC  = 0.0f;
    state->lastLevel  = 0.0f;
}

static float ResetFanHysteresis(FanHysteresisState *state, float tempC) {
    state->lastLevel = InterpolateFanLevel(state->tbl, tempC);
    state->lastTempC = tempC;
    state->hasLast = true;
    return state->lastLevel;
}

float UpdateFanHysteresis(FanHysteresisState *state, float tempC) {
    if (!state->hasLast) {
        return ResetFanHysteresis(state, tempC);
    }

    if (std::fabs(tempC - state->lastTempC) >= state->margin) {
        state->lastLevel = InterpolateFanLevel(state->tbl, tempC);
    }

    return state->lastLevel;
}

void RebindFanHysteresisTable(FanHysteresisState *state, const TemperaturePoint *newTable) {
    state->tbl     = newTable;
    state->hasLast = false;
}
