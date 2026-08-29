#include <tesla.hpp>
#include <fancontrol.hpp>
#include "fan_graph.hpp"
#include "curve.hpp"
#include "utils.hpp"

class MainMenu : public tsl::Gui {
private:
    bool _enabled;

    tsl::elm::ToggleListItem* _enabledBtn;

    tsl::elm::ListItem* _socTempLabel;
    tsl::elm::ListItem* _fanSpeedLabel;
    tsl::elm::ListItem* _curveBtn;

    float _liveTemp = -1.0f;

public:
    MainMenu();
    ~MainMenu();

    virtual tsl::elm::Element* createUI() override;

    virtual void update() override;
};
