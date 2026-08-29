#pragma once

#include <fancontrol.hpp>
#include <string>

class CurveStore {
public:
    TemperaturePoint points[MAX_TABLE_ENTRIES];
    u32 count = 0;

    void loadOrDefault();
    bool persist();
    void sortByTemp();

    bool addPoint();
    bool removePoint(u32 index);

    bool trySetTemp(u32 index, int temperature_c);
    void setLevel(u32 index, float level);

    bool tempTaken(int temperature_c, u32 exceptIndex) const;
};

extern CurveStore g_curve;
extern std::string g_navJump;

std::string FormatPointLabel(const TemperaturePoint& point);
