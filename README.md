# NX-FanControl

**NX-FanControl** lets you fully customize your console’s fan curve.

---

## Features

* **Custom fan curve**: Define a dynamic amount of temperature points with corresponding fan speeds.
* **Real-time monitoring**: View the current SoC temperature and fan speed in real time on a graph.
* **Profiles**: Use a secondary fan curve in docked mode (Opt in).
* **Configuration**: Configure refresh intervals:
    - **Docked refresh interval**: The interval used for checking docked state
    - **Enable refresh interval**: Responsible for checking whever custom fan control is enabled
    - **Config refresh interval**: How fast the config gets refreshed
    - **High temperature refresh interval**: Faster refresh interval at higher temperatures
    - **Low temperature frefresh interval**: Opposite of high temperature refresh
* **High refresh temperature**: the temperature where high temp refresh starts

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
