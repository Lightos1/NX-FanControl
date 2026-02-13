#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/syslimits.h>

#include <switch.h>

#define LOG_DIR "./config/NX-FanControl/"
#define LOG_FILE "./config/NX-FanControl/log.txt"
#define CONFIG_DIR "./config/NX-FanControl/"
#define CONFIG_FILE "./config/NX-FanControl/config.dat"
#define TABLE_SIZE sizeof(TemperaturePoint) * 10


typedef struct
{
    int     temperature_c;
    float   fanLevel_f;
} TemperaturePoint;

void WriteConfigFile(const TemperaturePoint *table);
void ReadConfigFile(TemperaturePoint **table_out);

void InitFanController(TemperaturePoint *table);
void FanControllerThreadFunction(void*);
void StartFanControllerThread();
void CloseFanControllerThread();
void WaitFanController();
void WriteLog(const char *buffer);

#ifdef __cplusplus
}
#endif