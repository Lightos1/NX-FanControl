#pragma once
#include "fan.hpp"

void WriteConfigFile(const TemperaturePoint *table);
void ReadConfigFile(TemperaturePoint **table_out);
