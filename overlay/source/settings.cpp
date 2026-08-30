#include "settings.hpp"
#include "main_menu.hpp"
#include <algorithm>

namespace {
    constexpr u32 FastRefreshTempStepC = 5;
    constexpr u32 PollIntervalStepMs   = 5;
    constexpr u32 CheckIntervalStepMs  = 100;

    u32 StepIndex(u32 value, u32 minValue, u32 step, u32 numSteps) {
        if (value <= minValue) {
            return 0;
        }
        return std::min((value - minValue + step / 2) / step, numSteps - 1);
    }
}

Settings::Settings() {
    this->_dockedOverride          = IsDockedOverride(ConfigSection);
    this->_fastRefreshTemperatureC = GetFastRefreshTemperatureC(ConfigSection);
    this->_slowRefreshIntervalMs   = GetRefreshInterval(ConfigSection, KeySlowRefreshIntervalMs, DefaultSlowRefreshIntervalMs);
    this->_fastRefreshIntervalMs   = GetRefreshInterval(ConfigSection, KeyFastRefreshIntervalMs, DefaultFastRefreshIntervalMs);
    this->_configRefreshIntervalMs = GetRefreshInterval(ConfigSection, KeyConfigRefreshIntervalMs, DefaultConfigRefreshIntervalMs);
    this->_enableRefreshIntervalMs = GetRefreshInterval(ConfigSection, KeyEnableRefreshIntervalMs, DefaultEnableRefreshIntervalMs);
    this->_dockedRefreshIntervalMs = GetRefreshInterval(ConfigSection, KeyDockedRefreshIntervalMs, DefaultDockedRefreshIntervalMs);
}

void Settings::addStepper(tsl::elm::List* list, const std::string& title, const char* unit, u32* value, u32 minValue, u32 maxValue, u32 step) {
    const u32 numSteps = (maxValue - minValue) / step + 1;

    auto header = new tsl::elm::CategoryHeader(title + ": " + std::to_string(*value) + " " + unit, true);
    list->addItem(header);

    auto bar = new tsl::elm::StepTrackBar("", numSteps);
    bar->setValueChangedListener([=](u16 index) {
        *value = minValue + index * step;
        header->setText(title + ": " + std::to_string(*value) + " " + unit);
    });
    bar->setProgress(StepIndex(*value, minValue, step, numSteps));
    list->addItem(bar);
}

tsl::elm::Element* Settings::createUI() {
    auto frame = new tsl::elm::OverlayFrame("NX-FanControl", "Settings");

    auto list = new tsl::elm::List();

    auto dockedOverrideBtn = new tsl::elm::ToggleListItem("Docked Profiles", this->_dockedOverride);
    dockedOverrideBtn->setStateChangedListener([this](bool state) {
        this->_dockedOverride = state;
        if (state) {
            g_dockedCurve.loadOrDefault();
        }
        SetDockedOverride(ConfigSection, state);
    });
    list->addItem(dockedOverrideBtn);

    list->addItem(new tsl::elm::CategoryHeader("Refresh Settings", true));

    this->addStepper(list, "High Refresh Temp", "C", &this->_fastRefreshTemperatureC, MinFastRefreshTempC, MaxFastRefreshTempC, FastRefreshTempStepC);
    this->addStepper(list, "Low Temp Interval", "ms", &this->_slowRefreshIntervalMs, MinPollIntervalMs, MaxPollIntervalMs, PollIntervalStepMs);
    this->addStepper(list, "High Temp Interval", "ms", &this->_fastRefreshIntervalMs, MinPollIntervalMs, MaxPollIntervalMs, PollIntervalStepMs);
    this->addStepper(list, "Config Refresh Interval", "ms", &this->_configRefreshIntervalMs, MinCheckIntervalMs, MaxCheckIntervalMs, CheckIntervalStepMs);
    this->addStepper(list, "Enable Refresh Interval", "ms", &this->_enableRefreshIntervalMs, MinCheckIntervalMs, MaxCheckIntervalMs, CheckIntervalStepMs);
    this->addStepper(list, "Docked Refresh Interval", "ms", &this->_dockedRefreshIntervalMs, MinCheckIntervalMs, MaxCheckIntervalMs, CheckIntervalStepMs);

    frame->setContent(list);

    return frame;
}

bool Settings::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) {
    if (keysDown & HidNpadButton_B) {
        SetFastRefreshTemperatureC(ConfigSection, this->_fastRefreshTemperatureC);
        SetRefreshInterval(ConfigSection, KeySlowRefreshIntervalMs, this->_slowRefreshIntervalMs);
        SetRefreshInterval(ConfigSection, KeyFastRefreshIntervalMs, this->_fastRefreshIntervalMs);
        SetRefreshInterval(ConfigSection, KeyConfigRefreshIntervalMs, this->_configRefreshIntervalMs);
        SetRefreshInterval(ConfigSection, KeyEnableRefreshIntervalMs, this->_enableRefreshIntervalMs);
        SetRefreshInterval(ConfigSection, KeyDockedRefreshIntervalMs, this->_dockedRefreshIntervalMs);
        g_navJump = "Settings";
        triggerExitFeedback();
        tsl::swapTo<MainMenu>();
        return true;
    }
    return false;
}
