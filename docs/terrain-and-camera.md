# Terrain and Camera

Two independent renderer fixes that became necessary once Zulu
shipped with `MaxCameraHeight = 450` (matching Generals Online)
instead of the retail `MaxCameraHeight = 300`.

## `VERTEX_BUFFER_TILE_LENGTH` bump (PR #7 — `3cc418dc1`)

The terrain renderer keeps a sliding sub-window of vertex buffers
sized by `VERTEX_BUFFER_TILE_LENGTH`. At retail height, the
window covered the visible terrain. `MaxCameraHeight = 450`
widens the on-screen terrain extent past that window, so cells
beyond the window pop in and out as the camera moves.

Bumping `VERTEX_BUFFER_TILE_LENGTH` from 36 to 48 yields a 193-cell
(~1930 world-unit) sliding window, covering the full visible
terrain at the new max height.

Single-line change in `W3DTerrainBuffer.cpp` (constant). Memory
overhead is roughly the square of the bump because the buffer is
tiled in both axes; current target hardware handles it without
issue.

## `farZ` camera-plane scaling fix (PR #17 — `40171fa5b`)

Stock retail's far clip plane in `W3DView::updateCameraClipPlanes`
is a fixed constant (1200). At `MaxCameraHeight = 450` the far
plane clips terrain near the top of the screen, producing a black
horizontal band that grows as the camera rises.

The fix scales `farZ` to `4 * m_maxCameraHeight` with a 1200 floor:

- At `H = 300` (retail), `4 * 300 = 1200` so retail-height
  behavior is preserved.
- At `H = 450`, `4 * 450 = 1800` so the far plane sits past the
  visible terrain again.
- Floor of 1200 prevents shrinking `farZ` below retail if someone
  drops `MaxCameraHeight` below 300.

Single-function change in
`Core/GameEngine/Source/GameRenderer/W3DView.cpp`. Independent of
the earlier vertex-buffer fix: that one widens the data the
renderer has, this one widens the slice the camera is willing to
draw.

## Concerns

- **Mismatch risk.** None — both changes are render-only.
  `farZ` and the vertex buffer never feed into sim state.
- **Replay / save compatibility.** None — same reasoning.
- **Memory.** The `VERTEX_BUFFER_TILE_LENGTH` bump increases
  terrain vertex-buffer memory by roughly `(48 / 36)^2 ≈ 1.78x`.
  Not a concern on any target hardware Zulu runs on, but worth
  knowing if memory pressure ever shows up here.
- **Different `MaxCameraHeight` per side.** Multiplayer is fine
  because `MaxCameraHeight` comes from `GameData.ini` which is
  loaded identically on every client. If a future mod swaps
  `GameData.ini` per faction or per map, the per-client `farZ`
  would diverge — but only visually; sim still matches.
