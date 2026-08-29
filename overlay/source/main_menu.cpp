#include "main_menu.hpp"
#include "curve_menu.hpp"
#include "settings.hpp"

MainMenu::MainMenu() {
    this->_dockedOverride = IsDockedOverride(ConfigSection);

    g_curve.loadOrDefault();
    if (this->_dockedOverride) {
        g_dockedCurve.loadOrDefault();
        this->_isDocked = IsConsoleDocked();
    }

    this->_enabled = IsEnabled(ConfigSection);

    this->_enabledBtn = new tsl::elm::ToggleListItem("Enabled", this->_enabled);
    this->_settingBtn = new tsl::elm::ListItem("Settings", ">");
    this->_modeLabel = new tsl::elm::ListItem("State: --");
    this->_socTempLabel = new tsl::elm::ListItem("SOC Temp: --C");
    this->_fanSpeedLabel = new tsl::elm::ListItem("Fan Speed: --%");
    this->_curveBtn = new tsl::elm::ListItem(HandheldCurveButtonLabel(), ">");
    if (this->_dockedOverride) {
        this->_dockedCurveBtn = new tsl::elm::ListItem(DockedCurveButtonLabel, ">");
    }
}

MainMenu::~MainMenu() {}

CurveStore* MainMenu::activeGraphCurve() {
    if (this->_dockedOverride && this->_isDocked) {
        return &g_dockedCurve;
    }
    return &g_curve;
}

tsl::elm::Element* MainMenu::createUI() {
    auto frame = new tsl::elm::OverlayFrame("NX-FanControl", "v1.1.0");

    auto list = new tsl::elm::List();

    this->_enabledBtn->setStateChangedListener([this](bool state) {
        this->_enabled = state;
        SetEnabled(ConfigSection, state);
        return true;
    });
    list->addItem(this->_enabledBtn);

    list->addItem(new tsl::elm::CategoryHeader("Settings", true));
    this->_settingBtn->setClickListener([](uint64_t keys) {
        if (keys & HidNpadButton_A) {
            g_navJump.clear();
            tsl::swapTo<Settings>();
            return true;
        }
        return false;
    });
    list->addItem(this->_settingBtn);

    list->addItem(new tsl::elm::CategoryHeader("Real-time Monitoring", true));
    list->addItem(this->_modeLabel);
    list->addItem(this->_socTempLabel);
    list->addItem(this->_fanSpeedLabel);

    list->addItem(new tsl::elm::CategoryHeader("Fan Curve", true));
    list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h) { CurveStore* c = this->activeGraphCurve(); DrawFanCurveGraph(r, x, y, w, h, c->points, c->count, this->_liveTemp); }), FanGraphHeight);

    this->_curveBtn->setClickListener([](uint64_t keys) {
        if (keys & HidNpadButton_A) {
            g_editCurve = &g_curve;
            g_navJump.clear();
            tsl::swapTo<CurveMenu>();
            return true;
        }
        return false;
    });
    list->addItem(this->_curveBtn);

    if (this->_dockedCurveBtn != nullptr) {
        this->_dockedCurveBtn->setClickListener([](uint64_t keys) {
            if (keys & HidNpadButton_A) {
                g_editCurve = &g_dockedCurve;
                g_navJump.clear();
                tsl::swapTo<CurveMenu>();
                return true;
            }
            return false;
        });
        list->addItem(this->_dockedCurveBtn);
    }

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
        this->_isDocked = IsConsoleDocked();
        this->_modeLabel->setText(std::string("State: ") + (this->_isDocked ? "Docked" : "Handheld"));

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
