# Codex Handoff: CYD Two PulseSensor Playground A/B

Last updated: 2026-05-19 13:31:33 EDT

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

Compare two PulseSensor hardware candidates using PulseSensorPlayground's BPM, IBI, and `sawStartOfBeat()` behavior.

The firmware now uses one `PulseSensorPlayground(2)` detector with indexed channels. It shows:

- compact raw truth strip for `GPIO35`
- compact raw truth strip for `GPIO27`
- Playground BPM and IBI for each sensor
- light green beat flashes when Playground reports `sawStartOfBeat()`
- pickup quality bars for each sensor
- `BETTER PICKUP` verdict for A/B testing

It also emits compact `AB,...` serial telemetry at 115200 baud for `webserial.html`, a Chrome Web Serial A/B dashboard with a larger raw overlap comparator, BPM/IBI trends, beat-event markers, quality bars, and a demo mode. The design rule is additive: keep the CYD fully useful on its own, and let the browser be the expanded lab view.

## Current Visual Direction

2026-05-19 13:47:28 EDT: the one-screen layout remains the standalone CYD instrument, and the firmware now also streams telemetry for the browser dashboard. This pass has built, flashed, and the browser demo mode has been visually checked; live CYD+browser feedback is pending.

Rendered reference:

```text
docs/screenshots/playground-ab-20260519-133133-EDT.svg
```

Browser dashboard:

```text
webserial.html
```

Decision log:

```text
docs/decision-log.md
```

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
