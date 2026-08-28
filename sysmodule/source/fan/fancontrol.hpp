#pragma once

#include <math.h>
#include <fancontrol.hpp>
#include <switch.h>

void InitFanController(TemperaturePoint *table);
void LoopFanController();
bool ValidateFanCurveTable(const TemperaturePoint *table);
void CleanupFanController();
