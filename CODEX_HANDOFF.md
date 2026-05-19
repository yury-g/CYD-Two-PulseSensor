# Codex Handoff: CYD Two PulseSensor Comparator

Last updated: 2026-05-19 13:04:51 EDT

This repository is an experimental clone of `yury-g/CYD_App_Launcher` for testing two PulseSensor inputs on one ESP32-2432S028 CYD.

## GitHub Repo

```text
yury-g/CYD-Two-PulseSensor
```

Source experiment memory:

```text
yury-g/CYD_App_Launcher
```

Pin scanner used for hardware discovery:

```text
yury-g/CYD_Analog_Pin_Scanner
```

## Current Goal

Compare two raw PulseSensor signal candidates before building full two-channel beat detection.

The firmware intentionally reads raw ADC values instead of using PulseSensorPlayground. It shows:

- top trace: `GPIO35`
- bottom trace: `GPIO27`
- raw value and rolling range for each
- `BEST` badge for stronger non-railed signal
- `REL` percentage showing whether the two signals move together

## Hardware Finding

2026-05-19 13:04:51 EDT: raw scanner testing on the connected ESP32-2432S028 CYD showed `GPIO35` and `GPIO27` as the best two PulseSensor signal candidates.

`GPIO22` also showed usable raw signal in the scanner, but an earlier dashboard experiment using `GPIO22` caused a screen on/off reset loop. Keep `GPIO22` out of the first two-sensor dashboard experiment.

## Wiring

```text
PulseSensor A red    -> 3.3V
PulseSensor A black  -> GND
PulseSensor A purple -> GPIO35

PulseSensor B red    -> 3.3V
PulseSensor B black  -> GND
PulseSensor B purple -> GPIO27
```

Use `3.3V`, not `5V`.

## Build And Flash

```sh
cd /Users/narwhal2/Documents/CYD-Two-PulseSensor
/Users/narwhal2/Library/Python/3.9/bin/pio run -e cyd
/Users/narwhal2/Library/Python/3.9/bin/pio run -e cyd -t upload
```

Current connected board port:

```text
/dev/cu.usbserial-210
```

## Development Style

Keep this MVP easy to tweak:

- small changes
- build before flashing
- save timestamped commits/tags after hardware-tested states
- do not push this experimental work to `WorldFamousElectronics/PulseSensor_CYD`
