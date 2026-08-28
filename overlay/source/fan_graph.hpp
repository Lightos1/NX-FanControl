#pragma once

#include <tesla.hpp>
#include <fancontrol.hpp>

constexpr u16 FanGraphHeight = 176;

void DrawFanCurveGraph(tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h, const TemperaturePoint* table, float liveTempC);
