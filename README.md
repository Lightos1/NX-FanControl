# NX-FanControl

**NX-FanControl** lets you fully customize your console’s fan curve.

---

## Features

* **Custom fan curve**: Set your own temperature-to-fan-speed points, with as many points as you need for more precise control.
* **Real-time monitoring**: Watch SoC temperature and fan speed live on a graph.
* **Profiles**: Optionally use a separate fan curve automatically when docked.
* **Refresh interval settings**: Control how often different parts of the sysmodule update:
    - **Docked check interval**: How often the sysmodule checks whether the device is docked.
    - **Custom control check interval**: How often the sysmodule checks whether custom fan control is turned on.
    - **Config reload interval**: How often the sysmodule re-reads your configuration.
    - **High-temperature refresh interval**: A faster update rate used once temperatures get high, for quicker response.
    - **Low-temperature refresh interval**: A slower update rate used when temperatures are low, to save resources.
* **High-temperature threshold**: The temperature at which the sysmodule switches to the faster (high-temperature) refresh interval.

---

## Compiling

Before building, ensure you have the [**devkitPro toolchain**](https://devkitpro.org/wiki/Getting_Started) installed and properly set up.

Clone the repository (including submodules) and build:

```bash
git clone https://github.com/Lightos1/NX-FanControl.git --recurse-submodules
cd NX-FanControl
./build.sh
```

---

## Common Issues & Fixes

**Issue:** Fan always stays on Fix: Add the following configuration to your Atmosphère setup:
[system_settings.ini (GitHub link)](https://github.com/dominatorul/Easy-Setup/blob/main/data/Optimizer/EmuNAND/system_settings.ini)

Place it in:

```
atmosphere/config/
```

---

## Credit
**Zathawo** - Upstream
**Dominaturul** - Original fork
**CtCaer** - Temperature driver

## License

This project is licensed under the **MIT License**.
See the [LICENSE](./LICENSE) file for details.
