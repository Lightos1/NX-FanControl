/*
 * Copyright (c) Souldbminer, Lightos_ and Horizon OC Contributors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* --------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <p-sam@d3vs.net>, <natinusala@gmail.com>, <m4x@m4xw.net>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If you meet any of us some day, and you think this
 * stuff is worth it, you can buy us a beer in return.  - The sys-clk authors
 * --------------------------------------------------------------------------
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <switch.h>

Result apmExtInitialize(void);
void apmExtExit(void);

Result apmExtGetPerformanceMode(u32 *out_mode);
Result apmExtSysRequestPerformanceMode(u32 mode);
Result apmExtGetCurrentPerformanceConfiguration(u32 *out_conf);
Result apmExtSysRequestPerformanceMode(u32 mode);
Result apmExtSysSetCpuBoostMode(u32 mode);

Result apmExtGetPerformanceMode(u32 *out_mode);
Result apmExtGetCurrentPerformanceConfiguration(u32 *out_conf);

inline bool apmExtIsCPUBoosted(u32 conf_id) {  // CPU boosted to 1785 MHz
    return (conf_id == 0x92220009 || conf_id == 0x9222000A);
};
inline bool apmExtIsBoostMode(u32 conf_id) {  // GPU throttled to 76.8 MHz
    return (conf_id >= 0x92220009 && conf_id <= 0x9222000C);
};

#ifdef __cplusplus
}
#endif
