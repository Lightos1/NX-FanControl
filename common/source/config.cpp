#include <fancontrol.hpp>
#include <minIni.h>

#define KEY_COUNT   "pointCount"
#define KEY_ENABLED "enabled"

static int ClampSpeed(int speed) {
    if (speed < 0) {
        return 0;
    }
    if (speed > 100) {
        return 100;
    }
    return speed;
}

u32 LoadCurve(const char *section, TemperaturePoint *points, u32 maxPoints) {
    if (points == NULL) {
        return 0;
    }

    char key[16], value[32];
    u32 count = 0;

    for (u32 i = 0; i < maxPoints; ++i) {
        snprintf(key, sizeof(key), "p%u", i);
        int len = ini_gets(section, key, "", value, sizeof(value), FC_CONFIG_INI);
        if (len == 0) {
            break;
        }

        int temp;
        int speed;
        if (sscanf(value, "%dC%d%%", &temp, &speed) == 2) {
            points[count].temperature_c = temp;
            points[count].fanLevel_f    = static_cast<float>(ClampSpeed(speed)) / 100.0f;
            ++count;
        }
    }

    return count;
}

u32 GetPointCount(const char *section) {
    long n = ini_getl(section, KEY_COUNT, 0, FC_CONFIG_INI);
    if (n < 0) {
        return 0;
    }
    if (n > MAX_TABLE_ENTRIES) {
        return MAX_TABLE_ENTRIES;
    }
    return static_cast<u32>(n);
}

bool IsEnabled(const char *section) {
    return ini_getbool(section, KEY_ENABLED, 0, FC_CONFIG_INI);
}

static bool BeginConfigWrite(void) {
    if (access(FC_CONFIG_DIR, F_OK) == -1) {
        CreateDir(FC_CONFIG_DIR);
    }
    remove(FC_CONFIG_INI_TMP);

    FILE *src = fopen(FC_CONFIG_INI, "rb");
    if (src == NULL) {
        return true;
    }

    FILE *dst = fopen(FC_CONFIG_INI_TMP, "wb");
    if (dst == NULL) {
        fclose(src);
        WriteLog("BeginConfigWrite: fopen tmp failed");
        return false;
    }

    char buf[512];
    size_t n;
    bool ok = true;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            ok = false;
            break;
        }
    }

    fclose(src);
    if (fclose(dst) != 0) {
        ok = false;
    }
    if (!ok) {
        remove(FC_CONFIG_INI_TMP);
        WriteLog("BeginConfigWrite: copy failed");
    }
    return ok;
}

static bool CommitConfigWrite(void) {
    remove(FC_CONFIG_INI);
    if (rename(FC_CONFIG_INI_TMP, FC_CONFIG_INI) != 0) {
        remove(FC_CONFIG_INI_TMP);
        WriteLog("CommitConfigWrite: rename failed");
        return false;
    }
    return true;
}

bool SaveCurve(const char *section, const TemperaturePoint *points, u32 count) {
    if (points == NULL || count == 0 || count > MAX_TABLE_ENTRIES) {
        return false;
    }

    if (!BeginConfigWrite()) {
        return false;
    }

    u32 oldCount = GetPointCount(section);

    char key[16], value[32];
    for (u32 i = 0; i < count; ++i) {
        int speed = ClampSpeed(static_cast<int>(points[i].fanLevel_f * 100.0f + 0.5f));
        snprintf(key, sizeof(key), "p%u", i);
        snprintf(value, sizeof(value), "%dC%d%%", points[i].temperature_c, speed);
        if (!ini_puts(section, key, value, FC_CONFIG_INI_TMP)) {
            remove(FC_CONFIG_INI_TMP);
            WriteLog("SaveCurve: ini_puts failed");
            return false;
        }
    }

    for (u32 i = count; i < oldCount; ++i) {
        snprintf(key, sizeof(key), "p%u", i);
        ini_puts(section, key, NULL, FC_CONFIG_INI_TMP);
    }

    if (!ini_putl(section, KEY_COUNT, count, FC_CONFIG_INI_TMP)) {
        remove(FC_CONFIG_INI_TMP);
        WriteLog("SaveCurve: ini_putl failed");
        return false;
    }

    return CommitConfigWrite();
}

bool SetEnabled(const char *section, bool enabled) {
    if (!BeginConfigWrite()) {
        return false;
    }

    if (!ini_putl(section, KEY_ENABLED, enabled ? 1 : 0, FC_CONFIG_INI_TMP)) {
        remove(FC_CONFIG_INI_TMP);
        WriteLog("SetEnabled: ini_putl failed");
        return false;
    }

    return CommitConfigWrite();
}
