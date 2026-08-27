#pragma once
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

static inline float InterpolateFanLevel(const TemperaturePoint *tbl, float tempC) {
    if (tempC <= tbl[0].temperature_c) {
        return tbl[0].fanLevel_f;
    }

    for (size_t i = 0; i < TABLE_ENTRIES - 1; i++) {
        if (tempC <= tbl[i + 1].temperature_c) {
            if (tbl[i].fanLevel_f == tbl[i + 1].fanLevel_f) {
                return tbl[i].fanLevel_f;
            }

            float dT = tbl[i + 1].temperature_c - tbl[i].temperature_c;
            float dF = tbl[i + 1].fanLevel_f    - tbl[i].fanLevel_f;
            float t  = (tempC - tbl[i].temperature_c) / dT;
            return tbl[i].fanLevel_f + dF * t;
        }
    }

    return tbl[TABLE_ENTRIES - 1].fanLevel_f;
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
