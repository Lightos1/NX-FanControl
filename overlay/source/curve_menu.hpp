#include <tesla.hpp>
#include <fancontrol.hpp>
#include "fan_graph.hpp"
#include "curve.hpp"
#include "utils.hpp"

class CurveMenu : public tsl::Gui {
private:
    float _liveTemp = -1.0f;

public:
    CurveMenu();

    virtual tsl::elm::Element* createUI() override;

    virtual void update() override;

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override;
};
