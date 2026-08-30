#pragma once

#define FC_LOG_DIR     "./config/NX-FanControl/"
#define FC_LOG_FILE    "./config/NX-FanControl/log.txt"

void CreateDir(const char *dir);
void InitLog(void);

void WriteLog(const char *buffer);
