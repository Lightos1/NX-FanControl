#include <tesla.hpp>
#include <fancontrol.hpp>
#include "utils.hpp"

class MainMenu : public tsl::Gui
{
private:
    TemperaturePoint* _fanCurveTable;
    bool _tableIsChanged;

    tsl::elm::ToggleListItem* _enabledBtn;

    // Real-time monitoring labels
    tsl::elm::ListItem* _socTempLabel;
    tsl::elm::ListItem* _fanSpeedLabel;

    tsl::elm::ListItem* _p0Label;
    tsl::elm::ListItem* _p1Label;
    tsl::elm::ListItem* _p2Label;
    tsl::elm::ListItem* _p3Label;
    tsl::elm::ListItem* _p4Label;
    tsl::elm::ListItem* _p5Label;
    tsl::elm::ListItem* _p6Label;
    tsl::elm::ListItem* _p7Label;
    tsl::elm::ListItem* _p8Label;
    tsl::elm::ListItem* _p9Label;


public:
    MainMenu();
    ~MainMenu();

    virtual tsl::elm::Element* createUI() override;

    virtual void update() override;
};