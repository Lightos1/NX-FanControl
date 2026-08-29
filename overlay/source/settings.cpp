#include "settings.hpp"
#include "main_menu.hpp"

Settings::Settings() {
    this->_dockedOverride = IsDockedOverride(ConfigSection);
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

    frame->setContent(list);

    return frame;
}

bool Settings::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) {
    if (keysDown & HidNpadButton_B) {
        g_navJump = "Settings";
        triggerExitFeedback();
        tsl::swapTo<MainMenu>();
        return true;
    }
    return false;
}
