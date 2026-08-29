#include <tesla.hpp>
#include <fancontrol.hpp>
#include "utils.hpp"

class SelectMenu : public tsl::Gui {
private:
    u32 _i = 0;
    TemperaturePoint* _fanCurveTable;
    u32 _count;
    bool* _tableIsChanged;

    tsl::elm::CategoryHeader* _tempLabel;
    tsl::elm::CategoryHeader* _fanLabel;
    tsl::elm::ListItem* _saveBtn;

public:
    SelectMenu(u32 i, TemperaturePoint *tps, u32 count, bool* tableIsChanged);

    virtual tsl::elm::Element* createUI() override;
};
