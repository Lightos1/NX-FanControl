#include "main_menu.hpp"
#include "select_menu.hpp"

MainMenu::MainMenu() {
    ReadConfigFile(&this->_fanCurveTable);

    // Initialize sensors
    InitializeSensors();

    // Initialize temperature and fan speed labels
    this->_socTempLabel = new tsl::elm::ListItem("SOC Temp: --C");
    this->_fanSpeedLabel = new tsl::elm::ListItem("Fan Speed: --%");

    for (int i = 0; i < TABLE_ENTRIES; i++) {
        this->_pLabels[i] = new tsl::elm::ListItem(this->formatPointLabel(i));
    }

    if (IsRunning() != 0) {
        this->_enabledBtn = new tsl::elm::ToggleListItem("Enabled", true);
    } else {
        this->_enabledBtn = new tsl::elm::ToggleListItem("Enabled", false);
    }
}

MainMenu::~MainMenu() {
    CloseSensors();
}

std::string MainMenu::formatPointLabel(int i) const {
    const TemperaturePoint* point = this->_fanCurveTable + i;
    return "P" + std::to_string(i) + ": " + std::to_string(point->temperature_c) + "C | " + std::to_string((int)(point->fanLevel_f * 100)) + "%";
}

tsl::elm::Element* MainMenu::createUI() {
    this->_tableIsChanged = false;

    auto frame = new tsl::elm::OverlayFrame("NX-FanControl", "v1.1.0");

    auto list = new tsl::elm::List();

    this->_enabledBtn->setStateChangedListener([this](bool state) {
        if (state) {
            CreateB2F();
            const NcmProgramLocation programLocation{
                .program_id = SysFanControlID,
                .storageID = NcmStorageId_None,
            };
            u64 pid = 0;
            pmshellLaunchProgram(0, &programLocation, &pid);
            return true;
        } else {
            RemoveB2F();
            pmshellTerminateProgram(SysFanControlID);
            return true;
        }
        return false;
    });
    list->addItem(this->_enabledBtn);

    // Add real-time monitoring section
    list->addItem(new tsl::elm::CategoryHeader("Real-time Monitoring", true));
    list->addItem(this->_socTempLabel);
    list->addItem(this->_fanSpeedLabel);

    list->addItem(new tsl::elm::CategoryHeader("Fan Curve", true));
    for (int i = 0; i < TABLE_ENTRIES; i++) {
        this->_pLabels[i]->setClickListener([this, i](uint64_t keys) {
            if (keys & HidNpadButton_A) {
                tsl::changeTo<SelectMenu>(i, this->_fanCurveTable, &this->_tableIsChanged);
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
        for (int i = 0; i < TABLE_ENTRIES; i++) {
            this->_pLabels[i]->setText(this->formatPointLabel(i));
        }

        this->_tableIsChanged = false;
    }
}
