#include "fancontrol.h"
#include "tmp451.h"
#include <stdatomic.h>
#include <math.h>

/* ── Fan curve table (10 presets) ─────────────────────────────────── */

const TemperaturePoint defaultTable[] =
{
    { .temperature_c = 25,  .fanLevel_f = 0.10f },
    { .temperature_c = 30,  .fanLevel_f = 0.20f },
    { .temperature_c = 35,  .fanLevel_f = 0.30f },
    { .temperature_c = 40,  .fanLevel_f = 0.40f },
    { .temperature_c = 45,  .fanLevel_f = 0.50f },
    { .temperature_c = 50,  .fanLevel_f = 0.60f },
    { .temperature_c = 55,  .fanLevel_f = 0.70f },
    { .temperature_c = 60,  .fanLevel_f = 0.80f },
    { .temperature_c = 65,  .fanLevel_f = 0.90f },
    { .temperature_c = 70,  .fanLevel_f = 1.00f },
};

#define TABLE_ENTRIES  (sizeof(defaultTable) / sizeof(defaultTable[0]))

/* ── State ────────────────────────────────────────────────────────── */

TemperaturePoint     *fanControllerTable;
Thread                FanControllerThread;
static atomic_bool    fanControllerThreadExit = false;

/* ── Tuning constants ─────────────────────────────────────────────── */

#define POLL_NORMAL_NS     50000000ULL   /*  50 ms – normal rate          */
#define POLL_FAST_NS       25000000ULL   /*  25 ms – when temp is high    */
#define TEMP_FAST_THRESH        55.0f    /* switch to fast poll above this */
#define TEMP_READ_RETRIES         3

/* ── CreateDir ────────────────────────────────────────────────────── */

void CreateDir(char *dir)
{
    char buf[PATH_MAX];
    size_t len = strlen(dir);

    if (len == 0)
        return;
    if (len >= PATH_MAX)
        len = PATH_MAX - 1;

    memcpy(buf, dir, len);
    buf[len] = '\0';

    for (size_t i = 1; i <= len; i++)
    {
        if (buf[i] == '/' || buf[i] == '\0')
        {
            char saved = buf[i];
            buf[i] = '\0';
            if (access(buf, F_OK) == -1)
                mkdir(buf, 0777);
            buf[i] = saved;
        }
    }
}

/* ── Logging ──────────────────────────────────────────────────────── */

void InitLog(void)
{
    if (access(LOG_DIR, F_OK) == -1)
        CreateDir(LOG_DIR);

    if (access(LOG_FILE, F_OK) != -1)
        remove(LOG_FILE);
}

void WriteLog(const char *buffer)
{
    FILE *log = fopen(LOG_FILE, "a");
    if (log == NULL)
        return;
    fprintf(log, "%s\n", buffer);
    fclose(log);
}

/* ── Config persistence ───────────────────────────────────────────── */

void WriteConfigFile(const TemperaturePoint *table)
{
    const TemperaturePoint *src = table ? table : defaultTable;

    if (access(CONFIG_DIR, F_OK) == -1)
        CreateDir(CONFIG_DIR);

    FILE *config = fopen(CONFIG_FILE, "w");
    if (config == NULL)
    {
        WriteLog("WriteConfigFile: fopen failed");
        return;
    }
    fwrite(src, 1, TABLE_SIZE, config);
    fclose(config);
}

void ReadConfigFile(TemperaturePoint **table_out)
{
    InitLog();

    *table_out = malloc(sizeof(defaultTable));
    if (*table_out == NULL)
    {
        WriteLog("ReadConfigFile: malloc failed");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
    memcpy(*table_out, defaultTable, sizeof(defaultTable));

    if (access(CONFIG_DIR, F_OK) == -1)
    {
        CreateDir(CONFIG_DIR);
        WriteConfigFile(NULL);
        WriteLog("Missing config dir");
        return;
    }

    if (access(CONFIG_FILE, F_OK) == -1)
    {
        WriteConfigFile(NULL);
        WriteLog("Missing config file");
        return;
    }

    FILE *config = fopen(CONFIG_FILE, "r");
    if (config == NULL)
    {
        WriteLog("ReadConfigFile: fopen failed, using defaults");
        return;
    }

    if (fread(*table_out, 1, TABLE_SIZE, config) != TABLE_SIZE)
    {
        WriteLog("ReadConfigFile: short read, using defaults");
        memcpy(*table_out, defaultTable, sizeof(defaultTable));
    }
    fclose(config);
    WriteLog("config file exist");
}

/* ── Interpolation ────────────────────────────────────────────────── */

static inline float InterpolateFanLevel(const TemperaturePoint *tbl, float tempC)
{
    if (tempC <= tbl[0].temperature_c)
        return tbl[0].fanLevel_f;

    for (size_t i = 0; i < TABLE_ENTRIES - 1; i++)
    {
        if (tempC <= tbl[i + 1].temperature_c)
        {
            float dT = tbl[i + 1].temperature_c - tbl[i].temperature_c;
            float dF = tbl[i + 1].fanLevel_f    - tbl[i].fanLevel_f;
            float t  = (tempC - tbl[i].temperature_c) / dT;
            return tbl[i].fanLevel_f + dF * t;
        }
    }

    return tbl[TABLE_ENTRIES - 1].fanLevel_f;
}

/* ── Fan controller ───────────────────────────────────────────────── */

void InitFanController(TemperaturePoint *table)
{
    fanControllerTable = table;

    if (R_FAILED(threadCreate(&FanControllerThread,
                              FanControllerThreadFunction,
                              NULL, NULL, 0x4000, 0x3F, -2)))
    {
        WriteLog("Error creating FanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
}

void FanControllerThreadFunction(void *arg)
{
    (void)arg;

    FanController fc;
    float tempC        =  0.0f;

    Result rs = fanOpenController(&fc, 0x3D000001);
    if (R_FAILED(rs))
    {
        WriteLog("Error opening fanController");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }

    while (!atomic_load_explicit(&fanControllerThreadExit, memory_order_relaxed))
    {
        /* ── Read temperature with retry ────────────────────────── */
        bool readOk = false;
        for (int retry = 0; retry < TEMP_READ_RETRIES; retry++)
        {
            rs = Tmp451GetSocTemp(&tempC);
            if (R_SUCCEEDED(rs))
            {
                readOk = true;
                break;
            }
        }

        if (!readOk)
        {
            WriteLog("Tmp451GetSocTemp failed after retries");
            tempC = 70.0f;
        }

        /* ── Compute target fan level ───────────────────────────── */
        float target  = InterpolateFanLevel(fanControllerTable, tempC);

        /* ── Always update fan speed for immediate response ─────── */
        rs = fanControllerSetRotationSpeedLevel(&fc, target);
        if (R_FAILED(rs))
        {
            WriteLog("fanControllerSetRotationSpeedLevel error");
            diagAbortWithResult(
                MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
        }

        /* ── Adaptive sleep ─────────────────────────────────────── */
        u64 interval = (tempC >= TEMP_FAST_THRESH)
                      ? POLL_FAST_NS
                      : POLL_NORMAL_NS;
        svcSleepThread(interval);
    }

    fanControllerClose(&fc);
}

/* ── Thread lifecycle ─────────────────────────────────────────────── */

void StartFanControllerThread(void)
{
    if (R_FAILED(threadStart(&FanControllerThread)))
    {
        WriteLog("Error starting FanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
}

void CloseFanControllerThread(void)
{
    atomic_store_explicit(&fanControllerThreadExit, true, memory_order_release);

    Result rs = threadWaitForExit(&FanControllerThread);
    if (R_FAILED(rs))
    {
        WriteLog("Error waiting fanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }

    threadClose(&FanControllerThread);

    atomic_store_explicit(&fanControllerThreadExit, false, memory_order_relaxed);

    free(fanControllerTable);
    fanControllerTable = NULL;
}

void WaitFanController(void)
{
    if (R_FAILED(threadWaitForExit(&FanControllerThread)))
    {
        WriteLog("Error waiting fanControllerThread");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
}