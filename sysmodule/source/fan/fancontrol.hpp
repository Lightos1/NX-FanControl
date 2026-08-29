#pragma once

#include <math.h>
#include <fancontrol.hpp>
#include <switch.h>

void InitContext(Context *_ctx);
void LoopFanController();
bool ValidateFanCurveTable(const TemperaturePoint *table, u32 count);
void CleanupFanController();
