#pragma once
#include "fan.hpp"

#define FC_CONFIG_DIR     "./config/NX-FanControl/"
#define FC_CONFIG_INI     "./config/NX-FanControl/config.ini"
#define FC_CONFIG_INI_TMP "./config/NX-FanControl/config.ini.tmp"

constexpr const char *ConfigSection              = "config";
constexpr const char *CurveSection               = "defaultCurve";
constexpr const char *DockedOverrideCurveSection = "dockedCurveOverride";

constexpr const char *KeySlowRefreshIntervalMs   = "low_temp_interval_ms";
constexpr const char *KeyFastRefreshIntervalMs   = "high_temp_interval_ms";
constexpr const char *KeyConfigRefreshIntervalMs = "config_refresh_interval_ms";
constexpr const char *KeyEnableRefreshIntervalMs = "enable_refresh_interval_ms";
constexpr const char *KeyDockedRefreshIntervalMs = "docked_refresh_interval_ms";

constexpr u32 MinFastRefreshTempC     = 20;
constexpr u32 MaxFastRefreshTempC     = 80;
constexpr u32 DefaultFastRefreshTempC = 55;

constexpr u32 MinPollIntervalMs  = 5;
constexpr u32 MaxPollIntervalMs  = 200;
constexpr u32 MinCheckIntervalMs = 100;
constexpr u32 MaxCheckIntervalMs = 5000;

constexpr u32 DefaultSlowRefreshIntervalMs   = 50;
constexpr u32 DefaultFastRefreshIntervalMs   = 25;
constexpr u32 DefaultConfigRefreshIntervalMs = 2000;
constexpr u32 DefaultEnableRefreshIntervalMs = 2000;
constexpr u32 DefaultDockedRefreshIntervalMs = 1000;

u32 LoadCurve(const char *section, TemperaturePoint *points, u32 maxPoints);
u32 GetPointCount(const char *section);
bool IsEnabled(const char *section);

bool SaveCurve(const char *section, const TemperaturePoint *points, u32 count);
bool SetEnabled(const char *section, bool enabled);

bool IsDockedOverride(const char *section);
bool SetDockedOverride(const char *section, bool enabled);

u32 GetFastRefreshTemperatureC(const char *section);
bool SetFastRefreshTemperatureC(const char *section, u32 temp);

u32 GetRefreshInterval(const char *section, const char *key, u32 defaultMs);
bool SetRefreshInterval(const char *section, const char *key, u32 ms);
