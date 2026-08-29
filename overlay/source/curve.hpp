#pragma once

#include <fancontrol.hpp>
#include <string>

class CurveStore {
public:
    TemperaturePoint points[MAX_TABLE_ENTRIES];
    u32 count = 0;
    const char* section = CurveSection;

    CurveStore() = default;
    explicit CurveStore(const char* section) : section(section) {}

    bool isDockedProfile() const;

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
extern CurveStore g_dockedCurve;
extern CurveStore* g_editCurve;
extern std::string g_navJump;

std::string FormatPointLabel(const TemperaturePoint& point);

std::string HandheldCurveButtonLabel();
constexpr const char* DockedCurveButtonLabel = "Edit Docked Curve";
