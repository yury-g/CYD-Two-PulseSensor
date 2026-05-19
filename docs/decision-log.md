# Decision Log

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
