#pragma once

#include <math.h>
#include <fancontrol.hpp>
#include <switch.h>

void InitFanController(TemperaturePoint *table);
void FanControllerThreadFunction(void*);
void StartFanControllerThread();
void CloseFanControllerThread();
void WaitFanController();
void SortFanCurveTable(TemperaturePoint *table);
bool ValidateFanCurveTable(const TemperaturePoint *table);
