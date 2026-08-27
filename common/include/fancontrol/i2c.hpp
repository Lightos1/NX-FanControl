#pragma once

#include <switch.h>

Result I2cReadRegHandler16(u8 reg, I2cDevice dev, u16 *out);

Result I2cReadRegHandler8(u8 reg, I2cDevice dev, u8 *out);
