#include <fancontrol.hpp>
#include <algorithm>

void SortFanCurveTable(TemperaturePoint *tbl, u32 count) {
    if (tbl == nullptr || count < 2) {
        return;
    }
    std::sort(tbl, tbl + count, [](const TemperaturePoint &a, const TemperaturePoint &b) { return a.temperature_c < b.temperature_c; });
}

float InterpolateFanLevel(const TemperaturePoint *tbl, u32 count, float tempC) {
    if (tbl == nullptr || count == 0) {
        return 0.0f;
    }

    if (count == 1 || tempC <= tbl[0].temperature_c) {
        return tbl[0].fanLevel_f;
    }

    for (u32 i = 0; i < count - 1; i++) {
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

    return tbl[count - 1].fanLevel_f;
}
