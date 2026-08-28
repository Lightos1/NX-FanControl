#include <fancontrol.hpp>
#include <algorithm>

const TemperaturePoint defaultTable[TABLE_ENTRIES] = {
    { .temperature_c = 25,  .fanLevel_f = 0.10f },
    { .temperature_c = 30,  .fanLevel_f = 0.20f },
    { .temperature_c = 35,  .fanLevel_f = 0.30f },
    { .temperature_c = 40,  .fanLevel_f = 0.40f },
    { .temperature_c = 45,  .fanLevel_f = 0.50f },
    { .temperature_c = 50,  .fanLevel_f = 0.60f },
    { .temperature_c = 55,  .fanLevel_f = 0.70f },
    { .temperature_c = 60,  .fanLevel_f = 0.80f },
    { .temperature_c = 65,  .fanLevel_f = 0.90f },
    { .temperature_c = 70,  .fanLevel_f = 1.00f },
};

void SortFanCurveTable(TemperaturePoint *tbl) {
    std::sort(tbl, tbl + TABLE_ENTRIES, [](const TemperaturePoint &a, const TemperaturePoint &b) { return a.temperature_c < b.temperature_c; });
}

float InterpolateFanLevel(const TemperaturePoint *tbl, float tempC) {
    if (tempC <= tbl[0].temperature_c) {
        return tbl[0].fanLevel_f;
    }

    for (size_t i = 0; i < TABLE_ENTRIES - 1; i++) {
        if (tempC <= tbl[i + 1].temperature_c) {
            if (tbl[i].fanLevel_f == tbl[i + 1].fanLevel_f) {
                return tbl[i].fanLevel_f;
            }

            float dT = tbl[i + 1].temperature_c - tbl[i].temperature_c;
            if (dT <= 0.0f) {
                return tbl[i + 1].fanLevel_f;
            }
            float dF = tbl[i + 1].fanLevel_f    - tbl[i].fanLevel_f;
            float t  = (tempC - tbl[i].temperature_c) / dT;
            return tbl[i].fanLevel_f + dF * t;
        }
    }

    return tbl[TABLE_ENTRIES - 1].fanLevel_f;
}
