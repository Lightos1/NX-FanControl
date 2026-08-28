#include <fancontrol.hpp>
#include "utils.hpp"
#include "pwm.h"

// Global variables for sensor sessions
static bool g_sensorsInitialized = false;
static PwmChannelSession g_fanSession;
static Result pwmCheck = 1;

// Cache last valid fan speed to avoid showing 0% on transient failures
static float g_lastValidFanSpeed = -1.0f;
static u64 g_lastValidFanSpeedTime = 0;

u64 IsRunning() {
    u64 pid = 0;
    pmdmntGetProcessId(&pid, SysFanControlID);
    return pid;
}

void RemoveB2F()
{
    remove(SysFanControlB2FPath);
}

void CreateB2F()
{
    FILE *f = fopen(SysFanControlB2FPath, "w");
    if (f) fclose(f);
}

bool InitializeSensors() {
    if (g_sensorsInitialized) return true;

    if (R_FAILED(i2cInitialize())) {
        return false;
    }

    if (hosversionAtLeast(6,0,0) && R_SUCCEEDED(pwmInitialize())) {
        pwmCheck = pwmOpenSession2(&g_fanSession, 0x3D000001);
    }

    g_sensorsInitialized = true;
    return true;
}

float GetSOCTemperature() {
    if (!g_sensorsInitialized) return -1.0f;

    float temperature = 0.0f;
    Result rc = Tmp451GetSocTemp(&temperature);
    if (R_FAILED(rc)) {
        return -1.0f;
    }
    return temperature;
}

float GetFanSpeed() {
    if (!g_sensorsInitialized) return -1.0f;

    if (R_SUCCEEDED(pwmCheck)) {
        const int MAX_RETRIES = 3;

        for (int retry = 0; retry < MAX_RETRIES; retry++) {
            double temp = 0.0;
            Result rc = pwmChannelSessionGetDutyCycle(&g_fanSession, &temp);

            if (R_SUCCEEDED(rc)) {
                temp *= 10.0;
                temp = trunc(temp);
                temp /= 10.0;
                double fanSpeed = 100.0 - temp;

                g_lastValidFanSpeed = (float)fanSpeed;
                g_lastValidFanSpeedTime = armGetSystemTick();

                return (float)fanSpeed;
            }

            if (retry < MAX_RETRIES - 1) {
                svcSleepThread(1000000); // 1ms in nanoseconds
            }
        }

        u64 currentTick = armGetSystemTick();
        u64 ticksPerSecond = armGetSystemTickFreq();

        // If we have a cached value from within the last 2 seconds, use it
        if (g_lastValidFanSpeed >= 0.0f &&
            (currentTick - g_lastValidFanSpeedTime) < (ticksPerSecond * 2)) {
            return g_lastValidFanSpeed;
        }
    }

    // If we have any cached value at all, return it rather than -1
    if (g_lastValidFanSpeed >= 0.0f) {
        return g_lastValidFanSpeed;
    }

    return -1.0f;
}

void CloseSensors() {
    if (g_sensorsInitialized) {
        if (R_SUCCEEDED(pwmCheck)) {
            pwmChannelSessionClose(&g_fanSession);
        }
        pwmExit();
        i2cExit();
        g_sensorsInitialized = false;

        // Reset cached values
        g_lastValidFanSpeed = -1.0f;
        g_lastValidFanSpeedTime = 0;
    }
}