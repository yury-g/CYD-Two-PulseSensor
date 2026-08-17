> **Personal experimental comparison by Yury Gitman.** This repository is not official World Famous Electronics or PulseSensor documentation and does not define supported hardware, wiring, product behavior, or company policy. Current company information lives at [PulseSensor.com](https://pulsesensor.com) and under [`WorldFamousElectronics`](https://github.com/WorldFamousElectronics).

# CYD Two PulseSensor Playground A/B

Experimental ESP32-2432S028 CYD firmware for comparing how two PulseSensor inputs perform with PulseSensorPlayground BPM, IBI, and beat-event detection.

This repo was cloned from `yury-g/CYD_App_Launcher` on 2026-05-19 after raw pin-scanner testing showed `GPIO35` and `GPIO27` as the best two PulseSensor signal candidates on the connected CYD hardware.

> Educational signal-comparison demo only. Not for medical use.

## Current Mapping

| PulseSensor | Red | Black | Purple Signal |
| --- | --- | --- | --- |
| A | `3.3V` | `GND` | `GPIO35` |
| B | `3.3V` | `GND` | `GPIO27` |

Use `3.3V`, not `5V`.

## What The Screen Shows

- Two compact raw truth strips, one for `GPIO35` and one for `GPIO27`
- PulseSensorPlayground BPM and IBI for each sensor
- Light green beat flashes when Playground reports `sawStartOfBeat()`
- Pickup quality bars based on qualified beat streak, recency, amplitude, raw range, and IBI stability
- `BETTER PICKUP` verdict for A/B sensor testing

Current rendered design reference:

![Two PulseSensor Playground A/B screen](docs/screenshots/playground-ab-20260519-133133-EDT.svg)

Earlier raw-comparator references:

![Two PulseSensor comparator screen](docs/screenshots/comparator-textbook-20260519-130920-EDT.svg)

The raw comparator is still represented on the screen as the two waveform strips. The main focus is now which sensor plays better with the existing PulseSensorPlayground detector.

## Chrome Web Serial A/B Lab

The firmware also emits compact `AB,...` telemetry lines at 115200 baud for a browser dashboard:

```text
webserial.html
```

Open it in Chrome, Edge, or Brave from a secure context such as `localhost`, click `Connect`, and choose the CYD serial port. After that first browser permission, the `Auto` toggle can reconnect to the paired port on reload. The browser view shows a larger raw overlap comparator, BPM agreement/drift, beat-event agreement, solo-beat false-positive suspects, pickup quality bars, the current A/B winner, and an IBI microscope that magnifies small millisecond differences between the two detectors. Green highlights mean agreement, orange means drift, and red means solo beat suspects. It also includes a demo mode for checking the layout without hardware attached.

## Build And Flash

```sh
cd /Users/narwhal2/Documents/CYD-Two-PulseSensor
/Users/narwhal2/Library/Python/3.9/bin/pio run -e cyd
/Users/narwhal2/Library/Python/3.9/bin/pio run -e cyd -t upload
```

The current local upload port is:

```text
/dev/cu.usbserial-210
```

## Hardware Notes

- Board: `ESP32-2432S028` CYD
- ESP32 chip seen while flashing: `ESP32-D0WD-V3`, revision `v3.1`
- Display: ILI9341 320x240 TFT
- Backlight: `GPIO21`
- Candidate signal pins from scanner testing:
  - Best pair: `GPIO35`, `GPIO27`
  - Also showed raw signal: `GPIO22`
  - Avoid for dashboard input for now: `GPIO22`, because one dashboard experiment caused a screen on/off reset loop

## Related Repos

- Source experiment memory: [yury-g/CYD_App_Launcher](https://github.com/yury-g/CYD_App_Launcher)
- Pin scanner: [yury-g/CYD_Analog_Pin_Scanner](https://github.com/yury-g/CYD_Analog_Pin_Scanner)
