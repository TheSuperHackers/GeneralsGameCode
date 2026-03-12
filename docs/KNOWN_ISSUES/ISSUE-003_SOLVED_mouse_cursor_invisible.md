# ISSUE-003: Mouse Cursor Not Visible In-Game

**Status**: OPEN  
**Session Discovered**: 53 (2026-02-21)  
**Severity**: Medium  
**Component**: Platform / Graphics  
**Reproducibility**: 100%  

## Symptom

The mouse cursor is partially invisible:
- **System cursor now visible** ✅ (Session 54 - 2026-02-23) - Generic pointer appears
- **Custom cursor sprites NOT visible** ❌ - Game-specific cursors (attack pointer, move target, etc) are missing

The cursor position works correctly (clicks register), but the visual custom cursor sprite is never drawn.

## Investigation Summary

### Session 54 Progress (2026-02-23)

**Build Status**: ✅ Linux build now compiles successfully with SDL3 + SDL3_image integration
- Fixed libpng16 path conflicts (vcpkg vs system)
- Integrated SDL3_image for ANI cursor file parsing
- Deploy script now copies SDL3, SDL3_image, and GameSpy libraries

**Cursor Status**: ⚠️ Partial progress
- System cursor (generic pointer) now visible (SDL3 default)
- Game-specific custom cursors NOT loading (attack pointer, move cursor, target indicator)
- `SDL3Mouse::initCursorResources()` still stub - not loading ANI files from game data

**Root Cause**: `loadCursorFromFile()` function is implemented and compiled, but `initCursorResources()` is never called to populate `m_cursorResources[]` array with loaded cursor sprites.

### Previous Hypotheses

1. **`SDL3Mouse::initCursorResources()` is a stub** — Our current implementation is empty (deferred to Phase 2). The game relies on `initCursorResources()` to load `.ani` cursor files and set SDL cursors. Without it, no cursor surface is created and SDL falls back to displaying nothing (or the system cursor is hidden by `SDL_HideCursor()` called elsewhere).

2. **`SDL_HideCursor()` called during init without matching `SDL_ShowCursor()`** — A code path may call `SDL_HideCursor()` early and our `SDL_ShowCursor()` in `init()` may not be reached, or may be called before the window is fully created.

3. **SDL3 hardware cursor not set** — `SDL3Mouse::setCursor()` currently creates a `SDL_SYSTEM_CURSOR_DEFAULT` each frame but may fail silently if the window surface or renderer is not ready yet.

4. **Game-side cursor rendering** — The engine may render the cursor via W3D (software sprite drawn over the frame), not via the OS cursor. If W3D cursor rendering is not wired up, the OS cursor is hidden and nothing replaces it.

## Code Audit Results

### SDL3Mouse Implementation Status (Session 54)

✅ **IMPLEMENTED**:
- `SDL3Mouse::SDL3Mouse()` - Constructor with event buffer initialization
- `SDL3Mouse::loadCursorFromFile()` - **COMPLETE** (120+ lines) - Full RIFF/ANI parser with:
  - File buffer allocation and validation
  - RIFF chunk iteration
  - Group ICO parsing (cursor headers)
  - Image data (rate, steps, hotspot) parsing
  - PNG frame loading via `IMG_LoadTyped_IO(libpng16 dynamic)`
  - `SDL_CreateColorCursor()` sprite creation
  - Debug assertions for frame validation
- `SDL3Mouse`` includes - Correct SDL3_image headers + lowercase `file.h` include fixes
- Deploy script - Copies libSDL3.so, libSDL3_image.so, libgamespy.so

❌ **NOT IMPLEMENTED** (Blocking cursor display):
- `SDL3Mouse::initCursorResources()` - **STILL STUB** - Should populate `m_cursorResources[cursor][direction]` array by calling `loadCursorFromFile()` for each cursor type
- `SDL3Mouse::setCursor(MouseCursor)` - **STUB** - Should call `SDL_SetCursor()` with sprite from array
- Cursor enum → filename mapping - Need to map `CURSOR_ARROW`, `CURSOR_ATTACK`, `CURSOR_MOVE` etc to `.ani` files in game data

### Previous Implementation

## Next Steps (for Future Session)

**Phase 1.9 - Activate Cursor Resource Loading**:
1. Implement `SDL3Mouse::initCursorResources()` to:
   - Map `MouseCursor` enum values → cursor filename/path in game data
   - Call `loadCursorFromFile()` for each cursor type (arrow, attack, move, target, etc)
   - Store loaded `AnimatedCursor*` in `m_cursorResources[7][4]` array (Fighter19 pattern: 7 cursor types, 4 directions)
   - Call this function from `SDL3Mouse` constructor or game initialization
   
2. Implement `SDL3Mouse::setCursor(MouseCursor)` to:
   - Validate cursor type
   - Extract current frame from `m_cursorResources[cursor][m_directionFrame]`
   - Call `SDL_SetCursor()` with preloaded sprite
   - Handle animation frame updates per tick

3. Hook cursor setting into game code:
   - Find where game calls `setMouse()` / `setCursorType()`
   - Ensure it calls our `SDL3Mouse::setCursor()` (may already be wired)

4. Test cursor animation:
   - Hover over unit → should show attack pointer
   - Move mouse → should update direction frame
   - Check frame timing matches original game (16ms per frame typical)

## Workaround

The system cursor may be visible if `SDL_CaptureMouse` / `SDL_SetWindowMouseGrab` are not in use. No functional workaround currently.

## Impact

- **Gameplay**: Major (usability severely affected — users cannot see where they are clicking)
- **Stability**: None
- **Determinism**: N/A
- **Release blocker**: Yes

## Reference

- Session 53 discovery (2026-02-21)
- Fighter19 ANI cursor loader: `references/fighter19-dxvk-port/GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/GameClient/SDL3Mouse.cpp` (lines 107–230)
