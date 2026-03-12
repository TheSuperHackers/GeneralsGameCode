# Upstream Sync Conflict Resolution Plan — 2026-03-12

**Branch**: `thesuperhackers-sync-03-12-2026`
**Common Ancestor**: `7ce6eeb39b` (tagged `weekly-2026-02-06`, 2026-02-05)
**Upstream commits to merge**: 129
**Our commits not in upstream**: 178
**Files with merge conflicts**: 26

---

## Section 1: Structural Overview

### Upstream's Major Changes Since Divergence

1. **File unification (64 renames)**: Files previously in `GeneralsMD/Code/GameEngine/Include/GameClient/` moved to `Core/GameEngine/Include/GameClient/`. Same for `.cpp` sources under `Source/GameClient/`. Also moved mouse/keyboard device files from `GeneralsMD/Code/GameEngineDevice/` to `Core/GameEngineDevice/`.

2. **Pathfinder unification**: Multiple `unify(pathfinder)` commits merging `PathfindZoneManager`, `PathfindCell`, `Pathfinder::*` functions from Zero Hour into Core. This is gameplay logic unification, no impact on cross-platform layer.

3. **BattlePlanBonuses refactor**: Changed `BattlePlanBonuses*` (heap-allocated) to `BattlePlanBonusesData` (stack-allocated value type) in `Player::removeBattlePlanBonusesForObject()`.

4. **LOCALE_USER_DEFAULT fix**: Changed `LOCALE_SYSTEM_DEFAULT` → `LOCALE_USER_DEFAULT` in date/time formatting in `GameState.cpp`.

5. **CD check removal**: Upstream removed `IsFirstCDPresent`, `CheckForCDAtGameStart`, and associated callbacks from `SkirmishGameOptionsMenu.cpp` and `MainMenu.cpp`.

6. **Override/final macros**: Added compatibility macros for `override` and `final` keywords across the codebase.

7. **Network refactoring**: Various `NetCommandMsg` and `NetPacket` simplifications (auto-merged).

8. **Math CRC utility**: Added `SimulationMathCrc.h` for logic mismatch detection.

---

## Section 2: Conflict Resolution Plan by File

### 2.1 Build System Conflicts (High Risk)

#### `GeneralsMD/Code/Main/CMakeLists.txt` (line 8-13)
**Conflict**: Binary output name for non-Windows platforms.
- **Our side**: `OUTPUT_NAME GeneralsXZH` (cross-platform branding)
- **Upstream**: `OUTPUT_NAME "generalszh${RTS_BUILD_OUTPUT_SUFFIX}"`
**Resolution**: **Keep ours** — `GeneralsXZH` is the GeneralsX branding for the cross-platform binary. Using upstream would break our deploy/run scripts and branding strategy.

#### `Generals/Code/Main/CMakeLists.txt` (line 8-13)
**Same pattern as above.**
**Resolution**: **Keep ours** — `OUTPUT_NAME GeneralsX` for non-Windows.

#### `Core/GameEngineDevice/CMakeLists.txt` (lines 88-102, 196-206)
**Conflict 1** (lines 88-102): Header file list. Our side has Win32 mouse/keyboard headers commented out. Upstream adds them back plus `Win32LocalFile.h`, `Win32LocalFileSystem.h`, `Win32Mouse.h`, and also `Win32BIGFile.h`, `Win32BIGFileSystem.h`.
- **Context**: We added `if(WIN32)` guards later in this file to conditionally include these. Upstream is adding them unconditionally because they only target Windows.
**Resolution**: **Keep our version** — The `if(WIN32)` guard approach below in the file correctly handles this for cross-platform.

**Conflict 2** (lines 196-206): Source file list. Our side has SDL3 mouse/keyboard source commented out. Upstream adds `Win32LocalFile.cpp`, `Win32LocalFileSystem.cpp`, `Win32OSDisplay.cpp`, `Win32DIKeyboard.cpp`, `Win32DIMouse.cpp` (commented), `Win32Mouse.cpp`.
- **Context**: We have `if(WIN32)` guards further down that add these conditionally.
**Resolution**: **Keep our version** — The platform-conditional approach is preferred for cross-platform.

#### `GeneralsMD/Code/GameEngineDevice/CMakeLists.txt` (lines 89-103, 196-207)
**Conflict 1** (includes): Our side has SDL3Device headers (`SDL3GameEngine.h`, `OpenALAudioManager.h`, `SDL3Mouse.h`, `SDL3Keyboard.h`) plus `Win32DIKeyboard.h`, `Win32DIMouse.h`, `Win32Mouse.h`. Upstream removes all these and just removes the Win32 ones.
**Resolution**: **Keep our version** — We need our SDL3/OpenAL device headers.

**Conflict 2** (sources at end): We have nothing (empty). Upstream adds Win32 device sources: `Win32GameEngine.cpp`, `Win32OSDisplay.cpp`, etc.
- **Context**: We handle these with `if(WIN32)` guards below.
**Resolution**: **Keep our version** (keep the empty block in the GAMEENGINEDEVICE_SRC list; the `if(WIN32)` block below handles platform-specific files).

#### `Generals/Code/GameEngine/CMakeLists.txt` (line 136-140)
**Conflict**: `ClientInstance.h` included vs commented.
- **Our side**: `Include/GameClient/ClientInstance.h` (included)
- **Upstream**: `#    Include/GameClient/ClientInstance.h` (commented out — because file moved to `Core/GameEngine/`)
**Resolution**: **Take upstream's version** — The file was moved to `Core/GameEngine/` and is now included from there; it should be commented out here to avoid duplicate includes.

---

### 2.2 GitHub Workflows Conflict (Low Risk)

#### `.github/workflows/old/validate-pull-request.yml` (line 10-29)
**Conflict**: Our side has the `pull_request_target` trigger commented out, only keeping `workflow_dispatch`. Upstream has the live trigger.
- **Context**: This file is in our `old/` folder (archived). Upstream doesn't have this concept; their version is the active one. The merge created it as a conflict between our archived version and their active version.
**Resolution**: **Keep our version** — This workflow is intentionally archived/disabled in our repo. The `workflow_dispatch` gives us manual execution ability without automatic triggering.

---

### 2.3 Audio Conflicts (Medium Risk)

#### `Core/GameEngine/Source/Common/Audio/GameAudio.cpp` (line 235-241)
**Conflict**: Our side has a comment block explaining the CD check removal. Upstream simply deletes the code with no comment.
- **Our code**: An explanatory comment about why the CD check is removed.
- **Upstream**: No comment; code simply absent.
**Resolution**: **Take upstream's version** — The comment is verbose; upstream's removed code is cleaner. The rationale is already documented in our git history.

---

### 2.4 Filesystem Conflicts (High Risk — CD Removal)

#### `Core/GameEngine/Source/Common/System/FileSystem.cpp` (line 332-361)
**Conflict**: Our side has stub implementations of `areMusicFilesOnCD()`, `loadMusicFilesFromCD()`, and `unloadMusicFilesFromCD()`. Upstream removes these entirely (no stubs).
- **Risk**: If these functions are declared in a header and called from elsewhere, removing them causes linker errors.
- **Check needed**: Verify if these are declared in `FileSystem.h`.
**Resolution – conditionally determined**:
  - If declared in header with no callers: **Take upstream** (remove stubs)
  - If declared in header with active callers: **Keep our stubs**

After checking: These functions appear to be runtime stubs for platform compatibility in our port. Upstream removes them because they don't need cross-platform stubs. We need to verify if the header declares them. **Preliminary resolution: Keep our stubs** (safe for cross-platform) unless confirmed they're not referenced.

---

### 2.5 Core GameEngine Conflicts (Medium Risk)

#### `GeneralsMD/Code/GameEngine/Include/Common/GameEngine.h` (line 109-121)
**Conflict**: `setQuitting(Bool)` inline implementation.
- **Our side**: Added `fprintf(stderr, "DEBUG: GameEngine::setQuitting(TRUE)...")` debug logging
- **Upstream**: Simple `{ m_quitting = quitting; }` without debug logging
**Resolution**: **Take upstream's version** — The debug `fprintf` was a temporary diagnostic. Remove it. Use clean `{ m_quitting = quitting; }` and `getQuitting()` instead of `getQuitting(void)`.

#### `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp` (line 925-928)
**Conflict**: Extra blank line vs. `}` closing brace.
- **Our side**: Empty line (whitespace-only change we accidentally introduced)
- **Upstream**: A real structural change — closes the `VERIFY_CRC` block earlier and moves `canUpdate` logic outside of it
**Resolution**: **Take upstream's structural change** — Moving `canUpdate` outside the CRC block is a valid logical improvement. Our blank line was accidental.

#### `Generals/Code/GameEngine/Source/Common/GameEngine.cpp` (same pattern)
**Resolution**: **Take upstream's structural change** (same reasoning as above).

---

### 2.6 Registry Conflicts (Medium Risk — Platform Guards)

#### `Generals/Code/GameEngine/Source/Common/System/registry.cpp` (line 218-226)
#### `GeneralsMD/Code/GameEngine/Source/Common/System/registry.cpp` (line 343-350)
**Conflict**: Our side has `#endif // _UNIX` before `GetRegistryLanguage(void)`. Upstream has `GetRegistryLanguage()` with empty parens.
- **Our side**: `#endif // _UNIX` closes the Linux stub `#ifdef _UNIX` block we added earlier. The `(void)` parameter is C-style.
- **Upstream**: No Unix guard (they don't do Linux), uses `()`.
**Resolution**: **Use both** — Keep `#endif // _UNIX` (it's needed to close the `#ifdef _UNIX` block started at line 44 that wraps the Win32 API stubs), AND adopt upstream's `()` empty parens style instead of `(void)`. Final:
```cpp
#endif // _UNIX

AsciiString GetRegistryLanguage()
```

---

### 2.7 Player RTS Logic Conflicts (Medium Risk)

#### `Generals/Code/GameEngine/Source/Common/RTS/Player.cpp` (line 3109-3128)
#### `GeneralsMD/Code/GameEngine/Source/Common/RTS/Player.cpp` (line 3600-3619)
**Conflict**: `BattlePlanBonuses*` (heap) vs `BattlePlanBonusesData` (stack).
- **Our side**: Heap-allocated `BattlePlanBonuses*`, uses `std::max` (cross-platform).
- **Upstream**: Stack-allocated `BattlePlanBonusesData bonus`, uses `__max` (MSVC-only). Also adds two missing fields: `m_validKindOf` and `m_invalidKindOf`.
- **Risk**: Heap-allocated version is a memory leak risk (unless `bonus` is deleted before returning). Stack-allocated upstream version is cleaner and avoids the leak.
- **Cross-platform issue**: `__max` is MSVC-specific; we must use `std::max`.
**Resolution**: **Take upstream's structural change** but replace `__max` with `std::max`. This fixes the memory leak AND includes the two missing fields. Final code uses `BattlePlanBonusesData bonus` on the stack with `std::max`.

---

### 2.8 SaveGame Conflicts (Low Risk)

#### `GeneralsMD/Code/GameEngine/Source/Common/System/SaveGame/GameState.cpp` (3 conflict regions)
**Conflict 1** (line 209-217): Comment style. Our big section divider vs upstream's concise comment.
**Resolution**: **Take upstream** — Their comment is more informative (explains WHY: "use user default locale").

**Conflicts 2 & 3** (lines 272-284 and 292-306): `LOCALE_SYSTEM_DEFAULT` → `LOCALE_USER_DEFAULT`.
**Resolution**: **Take upstream's value** for both occurrences — This is a real bugfix that improves locale handling on Windows with non-system regional settings. Our `#else` block (Linux path) is unaffected.

---

### 2.9 GUI/Menu Conflicts (High Risk — CD Removal)

#### `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/MainMenu.cpp` (line 301-324)
**Conflict**: Our side has `cancelStartBecauseOfNoCD()` and `checkCDCallback()` functions. Upstream removes them.
- **No external callers confirmed** — verified by grep.
**Resolution**: **Take upstream** — Remove the CD callback functions. The `doGameStart()` signature also changes to `()` from `void`. Upstream's approach is cleaner.

#### `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/SkirmishGameOptionsMenu.cpp` (line 464-507)
#### `Generals/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/SkirmishGameOptionsMenu.cpp` (line 453-496)
**Conflict**: Our side has `cancelStartBecauseOfNoCD`, `IsFirstCDPresent`, `checkCDCallback`, and `CheckForCDAtGameStart`. Upstream removes all.
- **No external callers confirmed** — `CheckForCDAtGameStart` is only referenced within these files.
**Resolution**: **Take upstream** — Remove all CD check functions.

---

### 2.10 Graphics/Renderer Conflicts (High Risk — Cross-Platform)

#### `GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameClient.h` (line 136-150)
**Conflict**: `createKeyboard()` factory.
- **Our side**: Platform-switching — returns `SDL3Keyboard` on non-Windows, `DirectInputKeyboard` on Windows.
- **Upstream**: Only `DirectInputKeyboard` (Windows-only).
**Resolution**: **Keep ours** — The cross-platform keyboard factory is a core part of our SDL3 input layer. Removing it would break Linux/macOS keyboard input.

#### `Core/GameEngineDevice/Include/W3DDevice/GameClient/W3DShaderManager.h` (line 83-89)
**Conflict**: `getChipset(void)` vs `getChipset()`, plus `Int64` vs `__int64` in `getCurrentDriverVersion`.
- **Our side**: Uses `Int64` (portable), `(void)` C-style params.
- **Upstream**: Uses `__int64` (MSVC-only), `()` C++ style.
**Resolution**: **Hybrid** — Take upstream's `()` style but keep `Int64` (our portable type). Upstream also removes `getCurrentVendor` and `getCurrentDriverVersion` inline getter declarations in this region — need to verify carefully.

#### `Core/GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWaterTracks.cpp` (2 conflict regions)
**Conflict 1** (declaration): Our side: `#ifdef _WIN32 void TestWaterUpdate(void); #endif`. Upstream: `void TestWaterUpdate();` (no guard).
**Conflict 2** (implementation): Our side: `#ifdef _WIN32 void TestWaterUpdate(void) { ... }`. Upstream: `void TestWaterUpdate() { ... }` (no guard).
- **Context**: `TestWaterUpdate` uses keyboard input (Windows-specific). Also references `ApplicationHWnd` which is Windows-specific.
**Resolution**: **Keep our `#ifdef _WIN32` guards** but adopt `()` style. The function is Windows-specific and shouldn't be compiled on Linux/macOS.

#### `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp` (line 3042-3057)
**Conflict**: Our side has: `#else static void CreateBMPFile(...) {} #endif #ifdef _WIN32 void W3DDisplay::takeScreenShot(void)`. Upstream: just `void W3DDisplay::takeScreenShot()`.
- **Context**: We split `takeScreenShot` into `#ifdef _WIN32` (real implementation) and `#else` (Linux stub). Upstream has one implementation for Windows only.
**Resolution**: **Keep our structure** — The `#ifdef _WIN32` / `#else` guards with the stub at line ~3195 provide a correct cross-platform implementation. Keep our version for this conflict region.

---

### 2.11 Library Conflicts (Low-Medium Risk)

#### `Core/Libraries/Source/WWVegas/WW3D2/render2dsentence.cpp` (line 1188-1193)
**Conflict**: `FontCharsClass::FontCharsClass(void)` vs `FontCharsClass::FontCharsClass()`. Our side adds `#ifdef _WIN32` after the `:` to guard GDI member initializers.
**Resolution**: **Hybrid** — Take upstream's `()` style, keep our `#ifdef _WIN32` to guard GDI initializers.

#### `Core/Libraries/Source/WWVegas/WWDownload/ftp.h` (line 25-30)
**Conflict**: Our side: `#ifdef _WIN32 #include <winsock.h> #else #include "windows_compat.h" #endif`. Upstream: `#include <cstddef>` before winsock.
- **Context**: Upstream added `<cstddef>` for type definitions. Our version has the platform-guard pattern for socket compatibility.
**Resolution**: **Hybrid** — Add upstream's `#include <cstddef>` before our `#ifdef _WIN32` block. Final:
```cpp
#include <cstddef>
#ifdef _WIN32
#include <winsock.h>
#else
#include "windows_compat.h"
#endif
```

#### `Core/Libraries/Source/profile/profile_funclevel.h` (line 211-217)
**Conflict**: `uintptr_t GetId(void)` vs `unsigned GetId()`.
- **Our side**: Changed to `uintptr_t` for 64-bit safety (pointer precision).
- **Upstream**: Keeps `unsigned` (32-bit only, MSVC-specific assumption).
**Resolution**: **Keep our `uintptr_t`** — This is an important 64-bit fix. Also adopt `()` style.

---

## Section 3: Risk Assessment

### High-Risk Files
1. `GeneralsMD/Code/GameEngineDevice/CMakeLists.txt` — SDL3/OpenAL device entries must survive
2. `Core/GameEngineDevice/CMakeLists.txt` — Platform-conditional file lists must remain correct
3. `GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameClient.h` — Input factory wiring
4. `Core/GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWaterTracks.cpp` — Windows guards

### Medium-Risk Files
5. `Generals/Code/GameEngine/Source/Common/RTS/Player.cpp` — `__max` → `std::max` conversion
6. `GeneralsMD/Code/GameEngine/Source/Common/RTS/Player.cpp` — Same
7. `Core/GameEngine/Source/Common/System/FileSystem.cpp` — CD stub removal
8. Registry files — `#endif // _UNIX` guard must be preserved

### Low-Risk Files
- Workflow YAML, comment-only conflicts, locale changes

---

## Section 4: Post-Merge Validation Checklist

- [ ] `cmake --preset macos-vulkan` configures without errors
- [ ] Build completes: `./scripts/build/macos/build-macos-zh.sh --build-only`
- [ ] No undefined symbol linker errors for removed CD functions
- [ ] SDL3 input devices compile and wire correctly
- [ ] `GeneralsXZH` binary name preserved on macOS
- [ ] Linux Docker configure + build succeeds
