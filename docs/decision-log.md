# Decision Log

## 2026-05-19 13:31:33 EDT — PulseSensorPlayground A/B Refocus

Status: built and flashed 2026-05-19 13:36:21 EDT; awaiting hardware screen feedback.

Decision: refocus the one-screen dashboard around the question: which sensor works better with PulseSensorPlayground for plausible BPM, IBI, and `sawStartOfBeat()` detection. Keep the raw visual comparator, but shrink it into two top "raw truth strip" waveforms so it supports the detector comparison instead of dominating the screen.

Why: the next useful hardware experiment is not just which pin wiggles more. It is which sensor placement/sensor unit produces stable, believable Playground beat detection first and most consistently. The display should let a beginner see the answer quickly while still giving a technical observer enough raw context to trust or question the detector.

Rendered visual spec:

```text
docs/screenshots/playground-ab-20260519-133133-EDT.svg
```

Firmware behavior:

- Uses `PulseSensorPlayground(2)` with `GPIO35` as sensor A and `GPIO27` as sensor B.
- Keeps 10-bit ADC scale and threshold `550` to match the existing one-sensor Playground dashboard.
- Shows compact raw waveform strips for both sensors.
- Shows Playground BPM, IBI, beat flashes, beat counts, and pickup quality bars.
- `BETTER PICKUP` compares qualified beat streak, beat recency, amplitude, live raw range, and IBI stability.
- Keeps `GPIO22` out of the dashboard path.

## 2026-05-19 13:25:31 EDT — Light Waveform Color Pass

Status: built locally, not yet hardware-flashed.

Decision: change the overlaid raw traces from the stronger cyan/amber pair to a softer light-blue/light-yellow pair, and draw near-overlap points in light green.

Why: with both live waveforms on one shared-scale graph, matching movement should read as a third visual state rather than whichever trace was drawn last. The light green overlap highlight makes moments where the signals align easier to see without adding another panel or more labels.

Rendered visual spec:

```text
docs/screenshots/comparator-dashboard-colors-20260519-132531-EDT.svg
```

Firmware behavior:

- `GPIO35` channel A is light blue.
- `GPIO27` channel B is light yellow.
- When the two live waveform y-positions are within two pixels, the sketch paints that overlap light green.

## 2026-05-19 13:19:51 EDT — One-Screen Dashboard Raw Comparator Pass

Status: built and flashed 2026-05-19 13:24:01 EDT; awaiting hardware screen feedback.

Decision: adapt the raw two-PulseSensor comparator back toward the successful original one-screen dashboard shape: one large live graph with both raw traces overlaid on a shared scale, plus three calm bottom panels for `A SCORE`, `B SCORE`, and `REL %`.

Why: the stacked science-style comparator was clear, but the original CYD dashboard's strongest IRL pattern was a single large graph and compact bottom metrics. Overlaying `GPIO35` and `GPIO27` on the same graph makes their relationship easier to see while preserving the minimal, low-flicker communication style.

Rendered visual spec:

```text
docs/screenshots/comparator-dashboard-20260519-131951-EDT.svg
```

Firmware behavior:

- `GPIO35` remains channel A.
- `GPIO27` remains channel B.
- Both traces draw in the same graph using a shared rolling raw scale.
- Bottom panels continue to show rolling non-railed score, raw value, range, current `BEST`, and rolling relationship percentage.
- Text and panel updates remain throttled to avoid the fast flicker seen in earlier dashboard experiments.

## 2026-05-19 13:09:20 EDT — Science-Style Comparator Visual Pass

Status: success candidate, built and flashed after SVG review.

Decision: keep the two-PulseSensor MVP as a raw signal comparator rather than jumping directly into two-channel beat detection. The screen now uses a science textbook style: two labeled traces, live raw/range overlays, score boxes, and a relationship/correlation box.

Why: the hardware question is still signal quality and relationship between `GPIO35` and `GPIO27`. Raw traces plus rolling range and correlation make that easier for a third party to understand than a premature BPM UI.

Rendered visual spec:

```text
docs/screenshots/comparator-textbook-20260519-130920-EDT.svg
```

Firmware behavior:

- `GPIO35` is channel A, cyan, top trace.
- `GPIO27` is channel B, amber, bottom trace.
- `A SCORE` and `B SCORE` use rolling non-railed signal range.
- `BEST` marks the stronger current signal.
- `RELATION` reports rolling correlation from `-100%` to `+100%`.

Prior success:

- `two-pulse-comparator-mvp-20260519-130451-EDT` worked well on hardware.

Known dead end:

- `GPIO22` produced usable raw scanner signal, but caused a reset/screen on-off loop in a dashboard build. Keep it out of this two-sensor dashboard until there is a specific reason to revisit it.

## 2026-05-19 13:14:49 EDT — Flicker Reduction Pass

Status: hardware-requested fix after the textbook visual build showed distracting fast flicker on the CYD.

Decision: keep live graph drawing at about 40 Hz, but stop repainting all text and metric panels every graph frame. Text and summary values now update about every 250 ms, and the bottom metric frames are drawn once as static UI.

Why: the CYD TFT shows visible shimmer when large text/panel regions are repeatedly erased and redrawn. Raw traces can update quickly, but labels, scores, and relationship text should be calmer.
