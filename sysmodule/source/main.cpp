#include "fan/fancontrol.hpp"

#define INNER_HEAP_SIZE 0x1110

#ifdef __cplusplus
extern "C" {
#endif

u32 __nx_applet_type = AppletType_None;

u32 __nx_fs_num_sessions = 1;

void __libnx_initheap(void) {
    static u8 inner_heap[INNER_HEAP_SIZE];
    extern void* fake_heap_start;
    extern void* fake_heap_end;

    fake_heap_start = inner_heap;
    fake_heap_end   = inner_heap + sizeof(inner_heap);
}

void __appInit(void) {
    Result rc;

    rc = smInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    rc = setsysInitialize();
    if (R_SUCCEEDED(rc)) {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if (R_SUCCEEDED(rc))
            hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }

    rc = fsInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    rc = fsdevMountSdmc();;
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    rc = fanInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));

    rc = i2cInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));

    smExit();
}

void __appExit(void) {
    CleanupFanController();
    fanExit();
    i2cExit();
    fsExit();
    fsdevUnmountAll();
}

#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[]) {
    TemperaturePoint *table;

    InitLog();
    ReadConfigFile(&table);
    SortFanCurveTable(table);

    if (!ValidateFanCurveTable(table)) {
        WriteLog("Invalid config!");
        memcpy(table, defaultTable, sizeof(defaultTable));
    }

    InitFanController(table);
    LoopFanController();

    __builtin_unreachable();

    return 0;
}
