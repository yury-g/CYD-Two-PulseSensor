# CYD Hardware Experiment Log

## 2026-05-19 14:54 EDT — IR + Green PPG Demo 2 Private Lab

Branch: `codex/ir-green-led-testing`

Status: visual map and browser Demo mode built; firmware set to `IR+Green PPG` `Demo2`; ready for connected CYD flash and real-line testing.

### Goal

Explore whether an IR PPG channel on `GPIO35` and a green-light PPG channel on `GPIO27` produce a useful private correlation signal. The branch intentionally avoids SpO2 claims.

### Built

- Visual UI target: `docs/screenshots/ir-green-demo2-ui-map-20260519-145456-EDT.svg`
- Browser screenshot: `docs/screenshots/ir-green-demo2-browser-20260519-145456-EDT.png`
- Spec: `docs/ir-green-demo2-spec.md`
- Browser dashboard relabeled for IR/green private lab work.
- Added `G/IR` ratio trend and human-entered reference markers.
- Added session buttons: `REST`, `HOLD`, `MOVE`, `NOTE`.
- Kept Web Serial auto-connect off by default.

### Current Interpretation

- `a*` telemetry fields are IR.
- `b*` telemetry fields are green.
- `G/IR` is green amplitude divided by IR amplitude, not oxygen saturation.

### Next Real-Line Test

Flash Demo 2 to the connected CYD, connect IR to `GPIO35`, connect green to `GPIO27`, and observe whether the ratio trend changes repeatably during rest, breath hold, movement, and placement changes.

## 2026-05-19 11:39 EDT — Signal Dashboard / Finger Coach Side Quest

Branch: `codex/finger-coach-dashboard-20260519-111641-EDT`

Tested on connected ESP32-2432S028 CYD hardware.

## 2026-05-19 13:04:51 EDT — Best Dual PulseSensor Pin Candidates

Raw pin-scanner testing on the connected ESP32-2432S028 CYD showed `GPIO35` and `GPIO27` as the best two PulseSensor signal candidates. `GPIO22` also showed usable raw signal, but a dashboard experiment using `GPIO22` as the PulseSensor input caused the screen to turn on/off in a reset loop. Use `GPIO35` and `GPIO27` for the first two-PulseSensor dashboard experiment.

### Tried

- Auto-expanding Signal Dashboard when the PulseSensor signal was not yet qualified.
- Finger Coach guidance states such as place finger, press firmer, press lighter, hold steady, and signal acquired.
- A 5-step Playground lock game with heart progress.
- Visual math strip for amplitude, live range, min/max, threshold, quality, and clipping.
- Friendly sound cues to gamify signal acquisition.

### IRL Result

The expanded dashboard did not improve the experience on the actual CYD display. It felt too busy, and the active bottom panel introduced visible flicker and over-drawing artifacts.

### Kept

- Cyan dotted threshold line on the graph.
- `THR 550` label on the graph.

### Rejected For Now

- Full Signal Dashboard screen.
- Finger Coach copy/states.
- Lock-game bottom panel.
- Dense visual math strip.
- Coaching nudge sounds.

### Current Direction

Return to the original one-screen dashboard: waveform, BPM, IBI, and compact signal panel. Keep the threshold affordance from the experiment, and use only a small signal-quality harmony tied to the bottom-right `SIGNAL` quality progress.
