# ISSUE-011: Replay Menu Loading Screen Crashes with SIGSEGV

**Status**: INVESTIGATING  
**Session Discovered**: Backlog (2026-03-12)  
**Severity**: High  
**Component**: Gameplay / Platform  
**Reproducibility**: High  

## Symptom

Opening the replay menu causes an immediate crash during layout initialization. The game receives `SIGSEGV` while processing replay list population after `ReplayMenu.wnd` is created.

## Investigation Summary

Current stack trace indicates a null/invalid string source is passed into `AsciiString::set()` from replay list population flow.

Observed execution path:
- `Shell::update()` pushes `Menus/ReplayMenu.wnd`
- `TheWindowManager->winCreateLayout("Menus/ReplayMenu.wnd")` returns a valid layout pointer
- `ReplayMenuInit()` calls replay list population
- `PopulateReplayFileListbox()` reaches `AsciiString::set()` with `s=0x1`
- Crash occurs inside `__strlen_avx2`

Primary working hypothesis:
- Replay entry parsing or listbox item text extraction is producing an invalid pointer sentinel (`0x1`) that is later treated as a valid C string.

## Code Audit Results

Confirmed from GDB backtrace:
- `Core/GameEngine/Source/Common/System/AsciiString.cpp:239`
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/ReplayMenu.cpp:423`
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Shell/Shell.cpp:709`

Debug trace before crash confirms layout creation succeeds and failure happens during replay menu initialization, not layout file loading.

## Next Steps (for Future Session)

1. Audit `PopulateReplayFileListbox()` inputs and all string sources used before `AsciiString::set()`.
2. Add temporary defensive assertions/logging around replay filename/title extraction to capture first invalid pointer creation site.
3. Verify replay directory scanning results and parser behavior for empty/malformed replay metadata.
4. Check for 64-bit pointer truncation or legacy sentinel misuse in replay UI code paths.
5. Re-run with AddressSanitizer in a testing preset (if available) to identify upstream memory corruption earlier in the call chain.

## Workaround

Avoid opening the replay menu. Replay loading from GUI is currently unstable.

## Impact

- **Gameplay**: Major (Replay feature path blocked)
- **Stability**: Always Crashes (when repro path is hit)
- **Determinism**: Not Applicable
- **Release blocker**: Yes

## Reference

- `Core/GameEngine/Source/Common/System/AsciiString.cpp:239`
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/ReplayMenu.cpp:423`
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Shell/Shell.cpp:709`
- `docs/KNOWN_ISSUES/GITHUB_ISSUE_DRAFTS_2026-03-12.md`
