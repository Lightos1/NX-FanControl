#include <tesla.hpp>

class Settings : public tsl::Gui {
private:
    bool _dockedOverride;

public:
    Settings();

    virtual tsl::elm::Element* createUI() override;

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override;
};
