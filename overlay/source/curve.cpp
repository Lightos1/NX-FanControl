#include "curve.hpp"

#include <algorithm>

CurveStore g_curve;
std::string g_navJump;

constexpr int MinTemp = 20;
constexpr int MaxTemp = 80;
constexpr int TempStep = 5;

static const TemperaturePoint DefaultCurve[] = {
    { .temperature_c = 25, .fanLevel_f = 0.00f },
    { .temperature_c = 30, .fanLevel_f = 0.00f },
    { .temperature_c = 35, .fanLevel_f = 0.00f },
    { .temperature_c = 40, .fanLevel_f = 0.00f },
    { .temperature_c = 45, .fanLevel_f = 0.00f },
    { .temperature_c = 50, .fanLevel_f = 0.30f },
    { .temperature_c = 55, .fanLevel_f = 0.40f },
    { .temperature_c = 60, .fanLevel_f = 0.60f },
    { .temperature_c = 65, .fanLevel_f = 0.70f },
    { .temperature_c = 70, .fanLevel_f = 1.00f },
};
static const u32 DefaultCurveCount = sizeof(DefaultCurve) / sizeof(DefaultCurve[0]);

std::string FormatPointLabel(const TemperaturePoint& point) {
    return std::to_string(point.temperature_c) + "C  |  " + std::to_string((int)(point.fanLevel_f * 100)) + "%";
}

void CurveStore::loadOrDefault() {
    this->count = LoadCurve(CurveSection, this->points, MAX_TABLE_ENTRIES);
    if (this->count == 0) {
        memcpy(this->points, DefaultCurve, sizeof(DefaultCurve));
        this->count = DefaultCurveCount;
        SaveCurve(CurveSection, this->points, this->count);
        SetEnabled(ConfigSection, true);
    }
    this->sortByTemp();
}

void CurveStore::sortByTemp() {
    SortFanCurveTable(this->points, this->count);
}

bool CurveStore::persist() {
    this->sortByTemp();
    return SaveCurve(CurveSection, this->points, this->count);
}

bool CurveStore::tempTaken(int temperature_c, u32 exceptIndex) const {
    for (u32 i = 0; i < this->count; i++) {
        if (i != exceptIndex && this->points[i].temperature_c == temperature_c) {
            return true;
        }
    }
    return false;
}

bool CurveStore::addPoint() {
    if (this->count >= MAX_TABLE_ENTRIES) {
        return false;
    }

    int freeTemp = -1;
    for (int t = MinTemp; t <= MaxTemp; t += TempStep) {
        if (!this->tempTaken(t, this->count)) {
            freeTemp = t;
            break;
        }
    }
    if (freeTemp < 0) {
        return false;
    }

    this->points[this->count].temperature_c = freeTemp;
    this->points[this->count].fanLevel_f = std::clamp(InterpolateFanLevel(this->points, this->count, (float)freeTemp), 0.0f, 1.0f);
    this->count++;

    return this->persist();
}

bool CurveStore::removePoint(u32 index) {
    if (this->count <= 2 || index >= this->count) {
        return false;
    }

    for (u32 i = index; i + 1 < this->count; i++) {
        this->points[i] = this->points[i + 1];
    }
    this->count--;

    return this->persist();
}

bool CurveStore::trySetTemp(u32 index, int temperature_c) {
    if (index >= this->count || temperature_c < MinTemp || temperature_c > MaxTemp || this->tempTaken(temperature_c, index)) {
        return false;
    }
    this->points[index].temperature_c = temperature_c;
    return true;
}

void CurveStore::setLevel(u32 index, float level) {
    if (index < this->count) {
        this->points[index].fanLevel_f = std::clamp(level, 0.0f, 1.0f);
    }
}
