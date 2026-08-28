#include <fancontrol.hpp>

const TemperaturePoint defaultTable[TABLE_ENTRIES] = {
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
