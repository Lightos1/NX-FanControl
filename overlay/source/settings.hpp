#include <tesla.hpp>

class Settings : public tsl::Gui {
private:
    bool _dockedOverride;
    u32 _fastRefreshTemperatureC;
    u32 _slowRefreshIntervalMs;
    u32 _fastRefreshIntervalMs;
    u32 _configRefreshIntervalMs;
    u32 _enableRefreshIntervalMs;
    u32 _dockedRefreshIntervalMs;

    void addStepper(tsl::elm::List* list, const std::string& title, const char* unit, u32* value, u32 minValue, u32 maxValue, u32 step);

public:
    Settings();

    virtual tsl::elm::Element* createUI() override;

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState& touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override;
};
