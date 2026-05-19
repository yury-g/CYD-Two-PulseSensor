# IR + Green PPG Demo 2 Spec

Date: 2026-05-19
Branch: `codex/ir-green-led-testing`
Visual target: `docs/screenshots/ir-green-demo2-ui-map-20260519-145456-EDT.svg`

## Goal

Build a private experimental IR/green PPG lab view that helps explore whether the green/IR relationship has repeatable structure when compared with human-entered reference observations.

This is not a public medical dashboard and must not claim SpO2 measurement.

## Sensor Mapping

| Channel | Meaning | Pin |
| --- | --- | --- |
| IR | infrared PPG | `GPIO35` |
| GRN | green-light PPG | `GPIO27` |

## Primary Questions

- Do IR and green beats agree in time?
- Which channel has cleaner pickup during rest, breath holds, movement, and placement changes?
- Does `G/IR` amplitude ratio drift in a repeatable way?
- Does `G/IR` correlate with a manually entered reference pulse-ox reading?

## UI Principles

- Show relationships before numbers.
- Use color and shape more than text.
- Keep "not SpO2" visible.
- Make reference entries explicit human events, not automatic claims.
- Keep the browser fun enough to explore: quick state markers, visible history, low friction.

## Browser Demo 2

Panels:

- Live PPG Relationship
  - IR and green raw traces.
  - beat agreement rungs.
  - drift and solo suspect marks.
- Private Correlation Pad
  - current `G/IR` ratio.
  - reference entry control for external pulse-ox reading.
  - ratio trend with reference markers.
- IR Quality and Green Quality
  - BPM, IBI, AMP, beat count, quality.
- Session Story
  - buttons for `REST`, `HOLD`, `MOVE`, `NOTE`.
  - markers should appear on the history timeline.

## Firmware Demo 2

CYD should remain a standalone test instrument:

- title `IR+Green PPG`
- version `Demo2`
- header includes `no SpO2`
- channels labeled `IR GPIO35` and `GRN GPIO27`
- verdict panel says `CLEANER PPG`
- bottom metric shows experimental `G/IR`

## Data Contract

Keep existing telemetry for now:

```text
AB,t,aSample,aBpm,aIbi,aQuality,aBeats,aAmp,aRange,aLocked,aBeat,bSample,bBpm,bIbi,bQuality,bBeats,bAmp,bRange,bLocked,bBeat,bpmDiff,winner
```

Interpretation on this branch:

- `a*` fields are IR.
- `b*` fields are green.
- `winner` may be `IR`, `GRN`, or `EVEN`.

## Explicit Non-Goals

- Do not calculate or display real SpO2.
- Do not make a public-facing health claim.
- Do not use `GPIO22`.
- Do not auto-connect Web Serial on page load.

## Validation

- Browser Demo mode must show IR/green labels, ratio trend, event markers, and reference markers without hardware.
- Firmware must build with PlatformIO.
- Firmware must flash to the connected CYD.
- Commit local branch changes with changelog and experiment-log notes.
