#include <fancontrol.hpp>
#include "select_menu.hpp"
#include "curve_menu.hpp"
#include "utils.hpp"

#include <algorithm>
#include <functional>
#include <vector>

namespace {

    constexpr tsl::Color DeleteFill(0xF, 0x3, 0x3, 0xF);
    constexpr u64 HoldDurationNs = 500000000ULL;
    constexpr int MinTemp = 20;
    constexpr int MaxTemp = 80;

    class HoldToDeleteItem : public tsl::elm::ListItem {
    public:
        HoldToDeleteItem(bool enabled, std::function<void()> onFilled)
            : tsl::elm::ListItem("Delete Point", enabled ? "hold A" : "can't delete (min. points)"), _enabled(enabled), _onFilled(onFilled) {
            this->m_flags.m_useClickAnimation = false;
        }

        virtual void draw(tsl::gfx::Renderer* renderer) override {
            if (this->_progress > 0.0f) {
                const s32 barWidth = (s32)((this->getWidth() - 8) * this->_progress);
                renderer->drawRect(this->getX() + 4, this->getY() + 1, barWidth, this->getHeight() - 2, renderer->a(DeleteFill));
            }
            tsl::elm::ListItem::draw(renderer);
        }

        virtual bool onClick(u64 keys) override {
            return false;
        }

        virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override {
            if (this->_enabled && (keysHeld & HidNpadButton_A) && !(keysHeld & ~HidNpadButton_A & ALL_KEYS_MASK)) {
                if (this->_holdStart == 0) {
                    this->_holdStart = armGetSystemTick();
                }
                const u64 elapsed = armTicksToNs(armGetSystemTick() - this->_holdStart);
                this->_progress = std::min(1.0f, (float)elapsed / (float)HoldDurationNs);
                if (this->_progress >= 1.0f) {
                    this->_holdStart = 0;
                    this->_progress = 0.0f;
                    this->_onFilled();
                }
                return true;
            }
            this->_holdStart = 0;
            this->_progress = 0.0f;
            return false;
        }

    private:
        bool _enabled;
        std::function<void()> _onFilled;
        u64 _holdStart = 0;
        float _progress = 0.0f;
    };

}

SelectMenu::SelectMenu(u32 i) {
    this->_i = i;
    this->_tempLabel = new tsl::elm::CategoryHeader(std::to_string(g_editCurve->points[i].temperature_c) + "C", true);
    this->_fanLabel = new tsl::elm::CategoryHeader(std::to_string((int)(g_editCurve->points[i].fanLevel_f * 100)) + "%", true);
}

void SelectMenu::exitPoint() {
    std::string label = FormatPointLabel(g_editCurve->points[this->_i]);
    g_editCurve->persist();
    g_navJump = label;
    triggerExitFeedback();
    tsl::swapTo<CurveMenu>(SwapDepth(2));
}

tsl::elm::Element* SelectMenu::createUI() {
    auto frame = new tsl::elm::OverlayFrame("NX-FanControl", "Curve Point");

    auto list = new tsl::elm::List();

    const int curTemp = g_editCurve->points[this->_i].temperature_c;

    std::vector<int> freeTemps;
    for (int t = MinTemp; t <= MaxTemp; t += 5) {
        if (!g_editCurve->tempTaken(t, this->_i)) {
            freeTemps.push_back(t);
        }
    }

    list->addItem(this->_tempLabel);
    if (freeTemps.size() >= 2) {
        int startIdx = 0;
        for (size_t k = 0; k < freeTemps.size(); k++) {
            if (freeTemps[k] == curTemp) {
                startIdx = (int)k;
            }
        }

        auto stepTemp = new tsl::elm::StepTrackBar("C", freeTemps.size());
        stepTemp->setValueChangedListener([this, freeTemps](u16 value) {
            if (value >= freeTemps.size()) {
                return;
            }
            int newTemp = freeTemps[value];
            if (g_editCurve->trySetTemp(this->_i, newTemp)) {
                this->_tempLabel->setText(std::to_string(newTemp) + "C");
            }
        });
        stepTemp->setProgress(startIdx);
        list->addItem(stepTemp);
    }

    list->addItem(this->_fanLabel);
    auto stepFanL = new tsl::elm::StepTrackBar("%", 21);
    stepFanL->setValueChangedListener([this](u16 value) {
        this->_fanLabel->setText(std::to_string(value * 5) + "%");
        g_editCurve->setLevel(this->_i, (float)(value * 5) / 100.0f);
    });
    stepFanL->setProgress(((int)(g_editCurve->points[this->_i].fanLevel_f * 100)) / 5);
    list->addItem(stepFanL);

    list->addItem(new HoldToDeleteItem(g_editCurve->count > 2, [this]() {
        std::string neighbour = FormatPointLabel(g_editCurve->points[this->_i == 0 ? 1 : this->_i - 1]);
        if (g_editCurve->removePoint(this->_i)) {
            g_navJump = neighbour;
            triggerExitFeedback();
            tsl::swapTo<CurveMenu>(SwapDepth(2));
        }
    }));

    frame->setContent(list);

    return frame;
}

bool SelectMenu::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) {
    if (keysDown & HidNpadButton_B) {
        this->exitPoint();
        return true;
    }
    return false;
}
