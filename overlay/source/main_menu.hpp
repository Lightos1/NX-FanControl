#include <tesla.hpp>
#include <fancontrol.hpp>
#include "utils.hpp"

class MainMenu : public tsl::Gui {
private:
    TemperaturePoint* _fanCurveTable;
    bool _tableIsChanged;

    tsl::elm::ToggleListItem* _enabledBtn;

    // Real-time monitoring labels
    tsl::elm::ListItem* _socTempLabel;
    tsl::elm::ListItem* _fanSpeedLabel;

    tsl::elm::ListItem* _pLabels[TABLE_ENTRIES];

    std::string formatPointLabel(int i) const;

public:
    MainMenu();
    ~MainMenu();

    virtual tsl::elm::Element* createUI() override;

    virtual void update() override;
};