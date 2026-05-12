# Slide Animation Overshoot Snap

## What it is

`ProcessAnimateWindowSlideFromRight`, `SlideFromLeft`, and their
`Fast` variants now call `win->winSetPosition(endPos)` immediately
before returning from `updateAnimateWindow`, snapping the window
exactly to its rest position once the slide-in finishes.

Single commit (PR #8 — `95669c641`), <10 lines added.

## Why

The slide-in animators decay velocity geometrically and exit the
animation loop when "close enough" to the end position. At 800x600
retail resolution the peak velocity was -40 px/frame and the
overshoot was sub-pixel, so the visual difference between "close
enough" and "exact rest position" was invisible.

At 1920x1080 (and higher), the peak velocity is proportionally
larger, the overshoot becomes visible, and the window settles a
few pixels short of its design position. The Generals Powers
shortcut bar (`GenPowersShortcutBar*.wnd`) is the most obvious
victim because it docks flush to a UI edge — even a 4-pixel
overshoot leaves a visible gap.

The corresponding `SlideFromTop` / `SlideFromBottom` and
`*Fast` forward paths already called `winSetPosition(endPos)` on
exit; this just makes the side-slide animators match.

## Code surface

One file, `Core/GameEngine/Source/GameClient/GUI/ProcessAnimateWindow.cpp`,
three function endings. Pure additive — no behavior change at low
resolutions where the slide-in was already settling within a pixel
of `endPos`.

## Concerns

- **Mismatch / replay / save.** None — pure render-side UI fix.
- **Asymmetry with retail.** At higher resolutions Zulu's slide-in
  animation snaps to rest exactly, where retail leaves a small
  visible offset. A player switching back to a vanilla binary
  would see the gap return; nobody has reported this as a
  regression.
- **Symmetric paths not touched.** The `SlideToRight` /
  `SlideToLeft` slide-**out** animators don't have this fix because
  the end position is off-screen, so overshoot is invisible.
