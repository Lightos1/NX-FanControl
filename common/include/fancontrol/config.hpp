#pragma once
#include "fan.hpp"

constexpr const char *ConfigSection = "config";
constexpr const char *CurveSection  = "defaultCurve";

u32  LoadCurve(const char *section, TemperaturePoint *points, u32 maxPoints);
u32  GetPointCount(const char *section);
bool IsEnabled(const char *section);

bool SaveCurve(const char *section, const TemperaturePoint *points, u32 count);
bool SetEnabled(const char *section, bool enabled);
