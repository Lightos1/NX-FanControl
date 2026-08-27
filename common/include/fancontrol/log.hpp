#pragma once

#define FC_LOG_DIR     "./config/NX-FanControl/"
#define FC_LOG_FILE    "./config/NX-FanControl/log.txt"
#define FC_CONFIG_DIR  "./config/NX-FanControl/"
#define FC_CONFIG_FILE "./config/NX-FanControl/config.dat"

void CreateDir(const char *dir);
void InitLog(void);

void WriteLog(const char *buffer);
