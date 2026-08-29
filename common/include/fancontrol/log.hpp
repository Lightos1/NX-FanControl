#pragma once

#define FC_LOG_DIR     "./config/NX-FanControl/"
#define FC_LOG_FILE    "./config/NX-FanControl/log.txt"
#define FC_CONFIG_DIR     "./config/NX-FanControl/"
#define FC_CONFIG_INI     "./config/NX-FanControl/config.ini"
#define FC_CONFIG_INI_TMP "./config/NX-FanControl/config.ini.tmp"

void CreateDir(const char *dir);
void InitLog(void);

void WriteLog(const char *buffer);
