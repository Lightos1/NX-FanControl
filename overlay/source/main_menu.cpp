#include "main_menu.hpp"
#include "select_menu.hpp"

static const TemperaturePoint kDefaultCurve[] = {
    { .temperature_c = 25, .fanLevel_f = 0.00f },
    { .temperature_c = 30, .fanLevel_f = 0.00f },
    { .temperature_c = 35, .fanLevel_f = 0.00f },
    { .temperature_c = 40, .fanLevel_f = 0.00f },
    { .temperature_c = 45, .fanLevel_f = 0.00f },
    { .temperature_c = 50, .fanLevel_f = 0.30f },
    { .temperature_c = 55, .fanLevel_f = 0.40f },
    { .temperature_c = 60, .fanLevel_f = 0.60f },
    { .temperature_c = 65, .fanLevel_f = 0.70f },
    { .temperature_c = 70, .fanLevel_f = 1.00f },
};
static const u32 kDefaultCurveCount = sizeof(kDefaultCurve) / sizeof(kDefaultCurve[0]);

MainMenu::MainMenu() {
    this->_count = LoadCurve(CurveSection, this->_curve, MAX_TABLE_ENTRIES);
    if (this->_count == 0) {
        memcpy(this->_curve, kDefaultCurve, sizeof(kDefaultCurve));
        this->_count = kDefaultCurveCount;
        SaveCurve(CurveSection, this->_curve, this->_count);
        SetEnabled(ConfigSection, true);
    }
    this->_enabled = IsEnabled(ConfigSection);

    // Initialize sensors
    InitializeSensors();

    // Initialize temperature and fan speed labels
    this->_socTempLabel = new tsl::elm::ListItem("SOC Temp: --C");
    this->_fanSpeedLabel = new tsl::elm::ListItem("Fan Speed: --%");

    for (u32 i = 0; i < this->_count; i++) {
        this->_pLabels[i] = new tsl::elm::ListItem(this->formatPointLabel(i));
    }

    this->_enabledBtn = new tsl::elm::ToggleListItem("Enabled", this->_enabled);
}

MainMenu::~MainMenu() {
    CloseSensors();
}

std::string MainMenu::formatPointLabel(u32 i) const {
    const TemperaturePoint* point = this->_curve + i;
    return "P" + std::to_string(i) + ": " + std::to_string(point->temperature_c) + "C | " + std::to_string((int)(point->fanLevel_f * 100)) + "%";
}

tsl::elm::Element* MainMenu::createUI() {
    this->_tableIsChanged = false;

    auto frame = new tsl::elm::OverlayFrame("NX-FanControl", "v1.1.0");

    auto list = new tsl::elm::List();

    this->_enabledBtn->setStateChangedListener([this](bool state) {
        this->_enabled = state;
        SetEnabled(ConfigSection, state);
        return true;
    });
    list->addItem(this->_enabledBtn);

    // Add real-time monitoring section
    list->addItem(new tsl::elm::CategoryHeader("Real-time Monitoring", true));
    list->addItem(this->_socTempLabel);
    list->addItem(this->_fanSpeedLabel);

    list->addItem(new tsl::elm::CategoryHeader("Fan Curve", true));
    list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h) { DrawFanCurveGraph(r, x, y, w, h, this->_curve, this->_count, this->_liveTemp); }), FanGraphHeight);

    list->addItem(new tsl::elm::CategoryHeader("Curve Points", true));
    for (u32 i = 0; i < this->_count; i++) {
        this->_pLabels[i]->setClickListener([this, i](uint64_t keys) {
            if (keys & HidNpadButton_A) {
                tsl::changeTo<SelectMenu>(i, this->_curve, this->_count, &this->_tableIsChanged);
                return true;
            }
            return false;
        });
        list->addItem(this->_pLabels[i]);
    }

    frame->setContent(list);

    return frame;
}

void MainMenu::update() {
    static u64 counter = 0;
    counter++;

    // Update readings every 6-12 frames (0.1-0.2s at 60 FPS)
    if (counter % 6 == 0) {
        // Get SOC temperature
        float socTemp = GetSOCTemperature();
        this->_liveTemp = socTemp;
        if (socTemp >= 0) {
            this->_socTempLabel->setText("SOC Temp: " + std::to_string((int)socTemp) + "C");
        } else {
            this->_socTempLabel->setText("SOC Temp: Error");
        }

        // Get fan speed
        float fanSpeed = GetFanSpeed();
        if (fanSpeed >= 0) {
            this->_fanSpeedLabel->setText("Fan Speed: " + std::to_string((int)fanSpeed) + "%");
        } else {
            this->_fanSpeedLabel->setText("Fan Speed: Error");
        }
    }

    if (this->_tableIsChanged) {
        for (u32 i = 0; i < this->_count; i++) {
            this->_pLabels[i]->setText(this->formatPointLabel(i));
        }

        this->_tableIsChanged = false;
    }
}
