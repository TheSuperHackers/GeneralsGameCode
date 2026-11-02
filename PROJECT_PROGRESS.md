# Project Progress — Generals Zero Hour

> **Last updated:** 2026-11-02  
> **Current phase:** Working Debug build confirmed; beginning source-level investigation of the FPS/performance HUD.  
> **How to use:** This is the technical source of truth for resuming work. Keep verified facts separate from observations and hypotheses; update it after meaningful milestones.

## 1. Project objective

Learn professional C++ development by investigating, debugging, building, modifying, and testing *Command & Conquer: Generals Zero Hour*. Favor understanding and hands-on implementation over autonomous code generation.

See [`MENTORING_GUIDELINES.md`](MENTORING_GUIDELINES.md) for how we collaborate.

## 2. Repository and Git setup

| Item | Value |
| --- | --- |
| Upstream repository | https://github.com/TheSuperHackers/GeneralsGameCode |
| Personal fork | https://github.com/ali-zein232/GeneralsGameCode |
| Local clone | `C:\Dev\GeneralsGameCode` |
| `origin` | Personal fork |
| `upstream` | TheSuperHackers repository |
| Local `main` tracks | `origin/main` |
| Last **reported synchronized** commit | `e2a1216b1` |

### Git setup milestone — completed

- Created a personal fork and configured `origin` / `upstream` correctly.
- Compared local `main` to `origin/main` using `git rev-list --left-right --count main...origin/main`; it returned `0 12` at the time (local had no unique commits and was 12 commits behind).
- Brought local `main` forward with a fast-forward-only merge, then changed its upstream tracking to `origin/main`.
- Git subsequently reported `## main...origin/main`.
- **Important:** Those results describe the state at synchronization time, not a fresh verification of today's working tree. Check `git status` before starting a new change.

**Next practice:** Create a focused branch before making substantive project changes; review diffs, test, and commit in small increments.

## 3. Development environment

| Item | Verified/reported configuration |
| --- | --- |
| OS | Windows 10 Pro |
| Hardware | Toshiba Satellite, Intel i5 4th generation, 12 GB RAM, about 200 GB SSD |
| IDE | Visual Studio Community |
| Toolchain | MSVC 19.44, v143, x86 (32-bit) |
| Build generator | Ninja Multi-Config |
| Active Visual Studio configuration | **Build Windows 32bit Debug** |
| Zero Hour CMake target | `z_generals` |
| Original Generals target | `g_generals` |

### Build directories (do not confuse them)

| Purpose | Path |
| --- | --- |
| **Current Visual Studio build tree** | `C:\Dev\GeneralsGameCode\build\win32-debug` |
| Current Zero Hour Debug executable | `C:\Dev\GeneralsGameCode\build\win32-debug\GeneralsMD\Debug\generalszh.exe` |
| Earlier working build tree | `C:\Dev\GeneralsGameCode\build\win32-v143-local` |
| Earlier Zero Hour Debug executable | `C:\Dev\GeneralsGameCode\build\win32-v143-local\GeneralsMD\Debug\generalszh.exe` |

### Known-good build procedure

1. In Visual Studio, use **File → Open → Folder** for `C:\Dev\GeneralsGameCode`.
2. Select **Build Windows 32bit Debug**.
3. In **CMake Targets View**, right-click `z_generals` and choose **Build**.
4. Check the last build lines for the generated executable. The successful build ended with:

   ```text
   [1224/1224] Linking CXX executable GeneralsMD\Debug\generalszh.exe
   ```

**Result:** The complete 1,224-step Debug build compiled and linked successfully. There were warnings (including signed/unsigned comparisons and deprecated arithmetic involving enums/floating-point types), but no errors. The CMake target `z_generals` produces the file `generalszh.exe`.

## 4. Isolated game test — working

| Item | Location / detail |
| --- | --- |
| Steam game ownership | Generals Zero Hour |
| Isolated game test directory | `C:\Dev\ZeroHour-Test` |
| Current game executable | `C:\Dev\ZeroHour-Test\generalszh.exe` |
| Prior executable backup | `C:\Dev\ZeroHour-Test\generalszh-backup.exe` |

**Verified gameplay result:** The newly compiled Debug executable launched successfully, and a skirmish match was played successfully using the isolated test installation.

### Runtime observations requiring investigation

- **Startup assertion:** `ASSERTION FAILURE: Be 1337! Go install Generals!`
  - Clicking **Ignore** allowed the game to continue.
  - A follow-up dialog asked whether to ignore this crash in the future; **No** was selected.
  - **Status:** Cause and significance unverified. Do not classify this as harmless or fixed.
- **Menu label:** Red `Debug: Load Map` text appears on the main menu.
  - **Status:** Observed in Debug build; source and expectedness not yet verified.

## 5. FPS/performance HUD investigation — in progress

### Observed behavior

- A **previous working executable** displayed a performance/debug HUD in the upper-left corner, with an example resembling `60 / 46 ... 02:50:34`.
- The interpretation of **every counter has not been established**.
- Keyboard shortcuts `Ctrl++` and `Ctrl+-` were observed changing the maximum rendering FPS among settings including 30, 60, 144, 240, 480, and uncapped.
- Do **not** assume the previous executable's HUD is identical to the newly built Debug executable's HUD without testing.

### Source locations confirmed by read-only GitHub inspection

| Source | Finding |
| --- | --- |
| `Core/GameEngine/Source/Common/FramePacer.cpp` | `FramePacer::getUpdateFps()` returns `1.0f / m_updateTime`. `FramePacer::update()` gets its update duration from `m_frameRateLimit.wait(maxFps)`. |
| `Core/GameEngine/Include/Common/FramePacer.h` | Declares the update FPS/time API and FPS-limiting and logic-time-scale operations. |
| `Core/GameEngine/Source/Common/FrameRateLimit.cpp` | Implements timing-based limiting with a high-resolution Windows counter; contains render FPS presets. |
| `Core/GameEngine/Source/GameClient/System/DebugDisplay.cpp` | Implements formatted debug-text positioning via `DebugDisplay::printf`. |
| `Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DDebugDisplay.cpp` | Implements on-screen debug-text drawing via `W3DDebugDisplay::drawText`. |

**Interpretation:** These establish promising measurement and display mechanisms, **not yet proof** that either mechanism produces the observed upper-left HUD. The connection must be traced.

### Immediate next step — Visual Studio

1. Open `Core/GameEngine/Source/Common/FramePacer.cpp`.
2. Find `FramePacer::getUpdateFps()`.
3. Use **Find All References** (`Shift + F12` on `getUpdateFps`).
4. Examine the callers; identify any involved in HUD rendering or frame statistics. Share the results for discussion **before editing code**.

**Why:** Following references from a known measurement function is a controlled way to discover how an unfamiliar codebase propagates FPS data.

### Open questions

- Which function draws the upper-left performance HUD, and under what conditions?
- What does each counter represent, and how is it calculated?
- Does `FramePacer::getUpdateFps()` feed that HUD, or is another FPS measurement used?
- How are the `Ctrl++` / `Ctrl+-` shortcuts handled?
- How does the frame pacing system interact with the broader game/render/logic loops?

## 6. Planned learning milestones

- [x] Clone source and configure Visual Studio's Debug build.
- [x] Produce a working `z_generals` Zero Hour executable.
- [x] Validate skirmish gameplay in a separate test directory.
- [x] Create and synchronize a personal GitHub fork with clear `origin`/`upstream` roles.
- [ ] Trace the performance HUD counters from calculation to rendering.
- [ ] Set and use a relevant breakpoint in Visual Studio, and inspect the call stack/variables.
- [ ] Understand the relevant parts of the update/render/logic loop.
- [ ] Create a focused feature branch and implement a small understood C++ change.
- [ ] Rebuild, test, review the diff, and commit the change with a clear message.
- [ ] Investigate the startup `Be 1337!` assertion and `Debug: Load Map` label separately.

## 7. Progress log

| Date | Milestone | Evidence / result | Next action |
| --- | --- | --- | --- |
| By 2026-11-02 | Fork and Git synchronization | `origin` and `upstream` configured; synchronized commit reported as `e2a1216b1` | Use feature branches for new work |
| By 2026-11-02 | Debug build established | 1,224 build steps, final `generalszh.exe` link succeeded | Learn debugger workflow |
| By 2026-11-02 | Debug executable game-tested | Game launched and skirmish worked in `ZeroHour-Test` | Investigate assertion separately |
| 2026-11-02 | FPS HUD investigation started | Read-only inspection located `FramePacer`, `FrameRateLimit`, and debug display code | Find callers of `getUpdateFps()` |

## 8. How to update this file

After meaningful work, update only what changed:

1. **Current phase / immediate next step:** State exactly where to resume.
2. **Verified findings:** Include file/function names and what the source, logs, or tests actually establish.
3. **Open questions / failures:** State uncertainty and preserve the exact error message where useful.
4. **Progress log:** Add a concise date, milestone, evidence, and next action.
5. **Build/Git status:** Refresh only after verification—do not silently carry forward assumptions about current commit, branch, working tree, or game behavior.

For substantial changes, review the Markdown diff before committing this document. Keep detailed debugging transcripts in issues, commits, or separate notes rather than letting this file become a full chat history.
