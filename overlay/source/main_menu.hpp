#include <tesla.hpp>
#include <fancontrol.hpp>
#include "fan_graph.hpp"
#include "curve.hpp"
#include "utils.hpp"

class MainMenu : public tsl::Gui {
private:
    bool _enabled;
    bool _dockedOverride;
    bool _isDocked = false;

    tsl::elm::ToggleListItem* _enabledBtn;

    tsl::elm::ListItem* _socTempLabel;
    tsl::elm::ListItem* _fanSpeedLabel;
    tsl::elm::ListItem* _modeLabel;
    tsl::elm::ListItem* _settingBtn;
    tsl::elm::ListItem* _curveBtn;
    tsl::elm::ListItem* _dockedCurveBtn = nullptr;

    float _liveTemp = -1.0f;

    CurveStore* activeGraphCurve();

public:
    MainMenu();
    ~MainMenu();

    virtual tsl::elm::Element* createUI() override;

    virtual void update() override;
};
