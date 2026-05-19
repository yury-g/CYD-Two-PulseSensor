# Codex Handoff: CYD IR + Green PPG Testing

Last updated: 2026-05-19 15:06:30 EDT

This repo contains two important states:

1. A validated two-PulseSensor A/B build on `main`.
2. A private experimental IR + Green PPG Demo 2 branch on `codex/ir-green-led-testing`.

Do not confuse the two. The current development branch is the IR/Green branch.

## Repository Boundary

Active repo:

```text
git@github.com:yury-g/CYD-Two-PulseSensor.git
```

Do not push to the original one-sensor launcher repo unless explicitly asked:

```text
git@github.com:yury-g/CYD_App_Launcher.git
```

Pin scanner reference repo:

```text
git@github.com:yury-g/CYD_Analog_Pin_Scanner.git
```

## Current Branch State

Current branch:

```text
codex/ir-green-led-testing
```

Current local commit:

```text
6228eb2 Build IR green PPG demo 2 lab
```

Planned GitHub save:

```text
branch: codex/ir-green-led-testing
tag: ir-green-demo2-flashed-20260519-150630-EDT
```

This branch is intended as private experimental work. If pushed to GitHub, its visibility follows the repository visibility. Do not make medical claims in public-facing language.

## Stable Base State

Last validated A/B mainline:

```text
commit: 374fc75 Add Web Serial A-B validation dashboard
tag: webserial-ab-validated-20260519-142635-EDT
branch: main
```

Earlier hardware-flashed A/B checkpoint:

```text
commit: b7c06a5 Add Playground A-B dashboard 20260519-133738-EDT
tag: playground-ab-flashed-20260519-133738-EDT
```

## Current IR + Green Demo 2 Goal

Private research question:

```text
Can IR PPG on GPIO35 and green-light PPG on GPIO27 produce useful, repeatable relationship signals?
```

This branch is for comparing:

- raw IR and green PPG waveforms
- shared beat timing
- drift and solo-beat suspects
- pulse pickup quality
- experimental `G/IR` amplitude ratio
- human-entered reference readings from an external pulse oximeter

This branch does not calculate SpO2. It explores whether IR/green features correlate with reference observations.

## Current Hardware Mapping

| Channel | Meaning | Signal Pin | Power | Ground |
| --- | --- | --- | --- | --- |
| IR | infrared PPG | `GPIO35` | `3.3V` | `GND` |
| GRN | green-light PPG | `GPIO27` | `3.3V` | `GND` |

Use `3.3V`, not `5V`.

Avoid `GPIO22` for now. It showed raw scanner signal, but an earlier dashboard experiment using it caused a CYD screen on/off reset loop.

## Current Firmware

Entrypoint:

```text
PulseSensor_CYD.ino
```

Current screen state:

- title: `IR+Green PPG`
- version: `Demo2`
- header includes `no SpO2`
- channel labels: `IR GPIO35`, `GRN GPIO27`
- verdict title: `CLEANER PPG`
- verdict values: `IR`, `GRN`, or `EVEN`
- bottom metric: `G/IR` experimental amplitude ratio

Detector setup:

```text
PulseSensorPlayground(2)
10-bit ADC
threshold 550
telemetry every about 40 ms
serial baud 115200
```

Telemetry remains compatible with the A/B browser parser:

```text
AB,t,aSample,aBpm,aIbi,aQuality,aBeats,aAmp,aRange,aLocked,aBeat,bSample,bBpm,bIbi,bQuality,bBeats,bAmp,bRange,bLocked,bBeat,bpmDiff,winner
```

On this branch:

- `a*` fields mean IR.
- `b*` fields mean green.

## Current Browser Lab

File:

```text
webserial.html
```

Run:

```sh
cd /Users/narwhal2/Documents/CYD-Two-PulseSensor
python3 -m http.server 8765
```

Open:

```text
http://localhost:8765/webserial.html
```

Demo 2 browser features:

- IR + Green PPG private lab title
- Live PPG relationship chart
- shared-pulse agreement, drift, and solo-suspect markers
- `G/IR` ratio readout and trend
- human-entered reference markers
- session buttons: `REST`, `HOLD`, `MOVE`, `NOTE`
- browser Demo mode for safe UI validation without hardware
- Web Serial `Auto` starts unchecked

Do not click `Connect` or enable `Auto` from automated tests unless the human explicitly asks.

## Visuals And Specs

Demo 2 visual map:

```text
docs/screenshots/ir-green-demo2-ui-map-20260519-145456-EDT.svg
```

Demo 2 browser screenshot:

```text
docs/screenshots/ir-green-demo2-browser-20260519-145456-EDT.png
```

Demo 2 spec:

```text
docs/ir-green-demo2-spec.md
```

Repo memory:

```text
README.md
CHANGELOG.md
docs/decision-log.md
docs/experiment-log.md
```

## Build And Flash

Build:

```sh
cd /Users/narwhal2/Documents/CYD-Two-PulseSensor
/Users/narwhal2/Library/Python/3.9/bin/pio run -e cyd
```

Flash connected IR/Green test CYD:

```sh
cd /Users/narwhal2/Documents/CYD-Two-PulseSensor
/Users/narwhal2/Library/Python/3.9/bin/pio run -e cyd -t upload --upload-port /dev/cu.usbserial-110
```

Previous A/B board often appeared as:

```text
/dev/cu.usbserial-210
```

Always check ports before flashing:

```sh
/Users/narwhal2/Library/Python/3.9/bin/pio device list
```

If flashing fails because the serial port cannot be opened, Chrome may be holding Web Serial. Disconnect or close the browser serial tab first.

## Last Verified Actions

2026-05-19:

- Created branch `codex/ir-green-led-testing`.
- Flashed Demo 1 to `/dev/cu.usbserial-110`.
- Reframed README for IR/Green branch state.
- Built Demo 2 visual map first.
- Wrote Demo 2 spec.
- Rebuilt browser UI for private IR/Green lab workflow.
- Updated firmware label to `Demo2`.
- Browser Demo mode validated with Auto unchecked.
- Added reference marker and session marker UI.
- Flashed Demo 2 successfully to `/dev/cu.usbserial-110`.
- Committed Demo 2 as `6228eb2`.

## Next Good Development Steps

1. Do real-line testing with IR sensor on `GPIO35` and green sensor on `GPIO27`.
2. Use browser `Mark Ref` with an external pulse oximeter reading.
3. Use `REST`, `HOLD`, `MOVE`, and `NOTE` markers during sessions.
4. Observe whether `G/IR` trend changes repeatably.
5. If useful, extend telemetry with a branch-specific prefix such as `IG,...`.
6. Consider a CSV export for private analysis.
7. Only after real data looks meaningful, design calibration helpers.

## Language Boundary

Acceptable:

```text
private IR/green PPG ratio exploration
reference-correlation experiment
experimental G/IR trend
not calibrated
not diagnostic
```

Avoid:

```text
measures oxygen
calculates SpO2
medical monitor
diagnostic result
```
