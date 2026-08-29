#include <tesla.hpp>
#include <fancontrol.hpp>
#include "fan_graph.hpp"
#include "utils.hpp"

class MainMenu : public tsl::Gui {
private:
    TemperaturePoint _curve[MAX_TABLE_ENTRIES];
    u32 _count;
    bool _enabled;
    bool _tableIsChanged;

    tsl::elm::ToggleListItem* _enabledBtn;

    // Real-time monitoring labels
    tsl::elm::ListItem* _socTempLabel;
    tsl::elm::ListItem* _fanSpeedLabel;

    float _liveTemp = -1.0f;

    tsl::elm::ListItem* _pLabels[MAX_TABLE_ENTRIES];

    std::string formatPointLabel(u32 i) const;

public:
    MainMenu();
    ~MainMenu();

    virtual tsl::elm::Element* createUI() override;

    virtual void update() override;
};
