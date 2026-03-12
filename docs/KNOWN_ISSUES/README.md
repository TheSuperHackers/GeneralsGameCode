# Known Issues

This directory contains documentation for all known issues, bugs, and limitations discovered during development.

## File Naming Convention

Use the format: `ISSUE-XXX_slug_description.md`

Where:
- `XXX` = Zero-padded issue number (001, 002, 003, etc.)
- `slug_description` = Brief lowercase, underscore-separated description of the issue

**Examples:**
- `ISSUE-001_shell_map_unit_immortality.md`
- `ISSUE-002_audio_crackling_on_startup.md`
- `ISSUE-003_replay_desync_multiplayer.md`

## Issue Document Structure

Each issue file should contain:

```markdown
# ISSUE-XXX: [Full Title]

**Status**: [OPEN|INVESTIGATING|BLOCKED|RESOLVED|WONTFIX]
**Session Discovered**: XX (YYYY-MM-DD)
**Severity**: [Critical|High|Medium|Low]
**Component**: [Graphics|Audio|Gameplay|Platform|Build|Other]
**Reproducibility**: [100%|High|Intermittent|Rare]

## Symptom

(Clear description of observable behavior)

## Investigation Summary

(Root cause analysis, potential hypotheses, findings)

## Code Audit Results

(What was checked, what was found)

## Next Steps (for Future Session)

(Actionable investigation items)

## Workaround

(If any exists)

## Impact

- **Gameplay**: [None|Minor|Major|Blocking]
- **Stability**: [None|Intermittent Crashes|Always Crashes]
- **Determinism**: [Not Applicable|Critical|Non-Critical]
- **Release blocker**: [Yes|No]

## Reference

(Links to related code, dev diary entries, etc.)
```

## Status Definitions

- **OPEN** — Confirmed issue, awaiting investigation or fix
- **INVESTIGATING** — Currently being researched; uncertain root cause or multiple hypotheses
- **BLOCKED** — Waiting for external feedback, data access, or prerequisite fixes
- **RESOLVED** — Fixed; waiting for verification or release
- **WONTFIX** — Intentionally deferred; documented rationale for non-resolution

## Severity Levels

- **Critical** — Game-breaking, prevents progress, crashes required
- **High** — Major feature impaired, significant gameplay impact
- **Medium** — Observable but workaroundable, cosmetic or edge-case impact
- **Low** — Cosmetic only, no gameplay impact, deferred indefinitely

## Component Categories

- **Graphics** — Rendering, DXVK, DirectX, Vulkan, visual artifacts
- **Audio** — Miles Sound System, OpenAL, music, voice, sound effects
- **Gameplay** — Game logic, physics, unit behavior, campaigns, multiplayer
- **Platform** — Linux/Windows compatibility, build system, POSIX/Win32 layer
- **Build** — CMake, compilation, linker errors, toolchain setup
- **Other** — Miscellaneous

---

**Generated**: 2026-03-12  
**Phase**: Known issues backlog and GitHub issue drafting

## Issue Index

| ID | Title | Severity | Status | Component |
|----|-------|----------|--------|-----------|
| [ISSUE-001](ISSUE-001_SOLVED_shell_map_unit_immortality.md) | Shell Map Units Immortal Under Bombardment | Medium | OPEN | Gameplay |
| [ISSUE-002](ISSUE-002_skirmish_instant_victory.md) | Skirmish vs CPU Ends Immediately with Victory Screen | Critical | INVESTIGATING | Gameplay |
| [ISSUE-003](ISSUE-003_SOLVED_mouse_cursor_invisible.md) | Mouse Cursor Not Visible In-Game | Medium | OPEN | Platform / Graphics |
| [ISSUE-004](ISSUE-004_SOLVED_generals_challenge_segfault.md) | Generals Challenge Mode Crashes with Segfault | High | RESOLVED | Platform / Video |
| [ISSUE-005](ISSUE-005_shadow_rendering_incorrect.md) | Building and Unit Shadows Render Incorrectly | Medium | OPEN | Graphics |
| [ISSUE-006](ISSUE-006_stealth_units_visible.md) | Stealth Units and GLA Stealth Buildings Render as Visible | High | OPEN | Graphics |
| [ISSUE-007](ISSUE-007_openal_long_voice_lines_and_sfx.md) | Long Voice Lines and Some Sound Effects Do Not Play Correctly | Medium | OPEN | Audio |
| [ISSUE-008](ISSUE-008_macos_gatekeeper_blocks_unsigned_build.md) | macOS Gatekeeper Blocks Unsigned Build | High | OPEN | Platform |
| [ISSUE-009](ISSUE-009_macos_terrain_textures_missing.md) | macOS Terrain Textures Missing or Failing to Load | Critical | OPEN | Graphics |
| [ISSUE-010](ISSUE-010_registry_ini_replacement.md) | Replace Null Registry Stubs with Cross-Platform registry.ini Backend | High | OPEN | Platform |
| [ISSUE-011](ISSUE-011_replay_menu_loading_screen_segfault.md) | Replay Menu Loading Screen Crashes with SIGSEGV | High | INVESTIGATING | Gameplay / Platform |
