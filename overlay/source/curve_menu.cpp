#include "curve_menu.hpp"
#include "main_menu.hpp"
#include "select_menu.hpp"

static const std::string AddPointLabel = "+  Add Point";

CurveMenu::CurveMenu() {}

tsl::elm::Element* CurveMenu::createUI() {
    auto frame = new tsl::elm::OverlayFrame("NX-FanControl", "Fan Curve");

    auto list = new tsl::elm::List();

    list->addItem(new tsl::elm::CategoryHeader("Fan Curve", true));
    list->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h) { DrawFanCurveGraph(r, x, y, w, h, g_curve.points, g_curve.count, this->_liveTemp); }), FanGraphHeight);

    list->addItem(new tsl::elm::CategoryHeader("Curve Points", true));
    for (u32 i = 0; i < g_curve.count; i++) {
        auto item = new tsl::elm::ListItem(FormatPointLabel(g_curve.points[i]));
        item->setClickListener([i](uint64_t keys) {
            if (keys & HidNpadButton_A) {
                g_navJump = FormatPointLabel(g_curve.points[i]);
                tsl::changeTo<SelectMenu>(i);
                return true;
            }
            return false;
        });
        list->addItem(item);
    }

    auto addItem = new tsl::elm::ListItem(AddPointLabel);
    addItem->setClickListener([](uint64_t keys) {
        if (keys & HidNpadButton_A) {
            if (g_curve.addPoint()) {
                g_navJump = AddPointLabel;
                tsl::swapTo<CurveMenu>();
            }
            return true;
        }
        return false;
    });
    list->addItem(addItem);

    if (!g_navJump.empty()) {
        list->jumpToItem(g_navJump);
        g_navJump.clear();
    }

    frame->setContent(list);

    return frame;
}

void CurveMenu::update() {
    static u64 counter = 0;
    counter++;

    if (counter % 6 == 0) {
        this->_liveTemp = GetSOCTemperature();
    }
}

bool CurveMenu::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) {
    if (keysDown & HidNpadButton_B) {
        g_navJump = "Edit Fan Curve";
        triggerExitFeedback();
        tsl::swapTo<MainMenu>();
        return true;
    }
    return false;
}
