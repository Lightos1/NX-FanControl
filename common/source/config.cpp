#include <fancontrol.hpp>

void WriteConfigFile(const TemperaturePoint *table) {
    const TemperaturePoint *src = table ? table : defaultTable;

    if (access(FC_CONFIG_DIR, F_OK) == -1)
        CreateDir(FC_CONFIG_DIR);

    FILE *config = fopen(FC_CONFIG_FILE, "w");
    if (config == NULL) {
        WriteLog("WriteConfigFile: fopen failed");
        return;
    }
    fwrite(src, 1, TABLE_SIZE, config);
    fclose(config);
}

void ReadConfigFile(TemperaturePoint **table_out) {
    *table_out = (TemperaturePoint *)malloc(sizeof(defaultTable));
    if (*table_out == NULL) {
        WriteLog("ReadConfigFile: malloc failed");
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
    }
    memcpy(*table_out, defaultTable, sizeof(defaultTable));

    if (access(FC_CONFIG_DIR, F_OK) == -1) {
        CreateDir(FC_CONFIG_DIR);
        WriteConfigFile(NULL);
        WriteLog("Missing config dir");
        return;
    }

    if (access(FC_CONFIG_FILE, F_OK) == -1) {
        WriteConfigFile(NULL);
        WriteLog("Missing config file");
        return;
    }

    FILE *config = fopen(FC_CONFIG_FILE, "r");
    if (config == NULL) {
        WriteLog("ReadConfigFile: fopen failed, using defaults");
        return;
    }

    if (fread(*table_out, 1, TABLE_SIZE, config) != TABLE_SIZE) {
        WriteLog("ReadConfigFile: short read, using defaults");
        memcpy(*table_out, defaultTable, sizeof(defaultTable));
    }
    fclose(config);
    WriteLog("config file exist");
}
