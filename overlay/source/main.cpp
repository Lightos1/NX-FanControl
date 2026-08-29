// main.cpp
// Define TESLA_INIT_IMPL before including tesla.hpp to get the implementations
#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <apm_ext.h>
#include "main_menu.hpp"
#include "utils.hpp"

class NxFanControlOverlay : public tsl::Overlay {
public:
    virtual void initServices() override {
        fsdevMountSdmc();
        pmshellInitialize();
        apmExtInitialize();
        InitializeSensors();
    }

    virtual void exitServices() override {
        CloseSensors();
        apmExtExit();
        fsdevUnmountAll();
        pmshellExit();
    }

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<MainMenu>();
    }
};

int main(int argc, char **argv) {
    return tsl::loop<NxFanControlOverlay>(argc, argv);
}
