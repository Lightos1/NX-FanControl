#include "main_menu.hpp"
#include "curve_menu.hpp"

MainMenu::MainMenu() {
    g_curve.loadOrDefault();
    this->_enabled = IsEnabled(ConfigSection);

    this->_socTempLabel = new tsl::elm::ListItem("SOC Temp: --C");
    this->_fanSpeedLabel = new tsl::elm::ListItem("Fan Speed: --%");
    this->_curveBtn = new tsl::elm::ListItem("Edit Fan Curve", ">");

    this->_enabledBtn = new tsl::elm::ToggleListItem("Enabled", this->_enabled);
}

MainMenu::~MainMenu() {}

tsl::elm::Element* MainMenu::createUI() {
    auto frame = new tsl::elm::OverlayFrame("NX-FanControl", "v1.1.0");

    auto list = new tsl::elm::List();

    this->_enabledBtn->setStateChangedListener([this](bool state) {
        this->_enabled = state;
        SetEnabled(ConfigSection, state);
        return true;
    });
    list->addItem(this->_enabledBtn);

    list->addItem(new tsl::elm::CategoryHeader("Real-time Monitoring", true));
    list->addItem(this->_socTempLabel);
    list->addItem(this->_fanSpeedLabel);

    list->addItem(new tsl::elm::CategoryHeader("Fan Curve", true));
    list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h) { DrawFanCurveGraph(r, x, y, w, h, g_curve.points, g_curve.count, this->_liveTemp); }), FanGraphHeight);

    this->_curveBtn->setClickListener([](uint64_t keys) {
        if (keys & HidNpadButton_A) {
            g_navJump.clear();
            tsl::swapTo<CurveMenu>();
            return true;
        }
        return false;
    });
    list->addItem(this->_curveBtn);

    if (!g_navJump.empty()) {
        list->jumpToItem(g_navJump);
        g_navJump.clear();
    }

    frame->setContent(list);

    return frame;
}

void MainMenu::update() {
    static u64 counter = 0;
    counter++;

    if (counter % 6 == 0) {
        float socTemp = GetSOCTemperature();
        this->_liveTemp = socTemp;
        if (socTemp >= 0) {
            this->_socTempLabel->setText("SOC Temp: " + std::to_string((int)socTemp) + "C");
        } else {
            this->_socTempLabel->setText("SOC Temp: Error");
        }

        float fanSpeed = GetFanSpeed();
        if (fanSpeed >= 0) {
            this->_fanSpeedLabel->setText("Fan Speed: " + std::to_string((int)fanSpeed) + "%");
        } else {
            this->_fanSpeedLabel->setText("Fan Speed: Error");
        }
    }
}
