# ISSUE-010: Replace Null Registry Stubs with Cross-Platform registry.ini Backend

**Status**: OPEN  
**Session Discovered**: Backlog (2026-03-12)  
**Severity**: High  
**Component**: Platform  
**Reproducibility**: High  

## Symptom

Cross-platform builds currently replace large parts of the Windows registry model with null stubs or read-only fallbacks.

This is sufficient for basic startup in some cases, but it breaks mod scenarios and game features that expect registry-backed values to persist across runs.

## Investigation Summary

The current codebase has multiple registry access layers with incomplete non-Windows behavior:
- The game-side registry layer in Generals and Zero Hour contains Linux or Unix paths that return failure for most write operations.
- The WWDownload registry layer also uses null stub behavior for non-Windows builds.
- Some read paths have partial environment-variable fallback logic, but that is not a real replacement for persistent read and write semantics.

This leaves the port in an awkward state where the code compiles and some defaults work, but systems that rely on saving configuration or mod metadata still have no durable backend.

## Proposed Direction

Implement a cross-platform registry replacement backed by a file named `registry.ini`.

Requirements for the replacement layer:
- Create `registry.ini` automatically if it does not exist
- Preserve the current public registry helper interfaces used by the game code
- Translate registry path and key lookups into deterministic INI sections and values
- Support both read and write operations instead of returning failure stubs
- Keep platform-specific behavior isolated to the backend layer rather than leaking file logic into gameplay or UI code

Suggested scope for the first implementation:
- Back the existing `GetStringFromRegistry`, `GetUnsignedIntFromRegistry`, `SetStringInRegistry`, and `SetUnsignedIntInRegistry` helpers with `registry.ini`
- Unify or at least align the backend used by both the game-side registry wrapper and the WWDownload registry wrapper
- Continue to use the native Windows registry on real Windows builds

## Code Audit Results

Relevant current behavior was confirmed in these areas:
- [GeneralsMD/Code/GameEngine/Source/Common/System/registry.cpp](GeneralsMD/Code/GameEngine/Source/Common/System/registry.cpp) contains non-Windows helpers that return failure for write operations and only provide partial environment-variable lookup for some reads.
- [Core/Libraries/Source/WWVegas/WWDownload/registry.cpp](Core/Libraries/Source/WWVegas/WWDownload/registry.cpp) contains non-Windows public APIs that log stub calls and return failure.
- [GeneralsMD/Code/CompatLib/Include/socket_compat.h](GeneralsMD/Code/CompatLib/Include/socket_compat.h) defines registry API compatibility stubs that currently model absence rather than replacement.

## Next Steps (for Future Session)

1. Define the on-disk location and format for `registry.ini`.
2. Define deterministic mapping rules from Windows registry roots, subkeys, and value names to INI sections and keys.
3. Implement a shared cross-platform backend for string and integer reads and writes.
4. Preserve native registry usage on Windows while routing non-Windows builds through the INI backend.
5. Test mod scenarios that depend on persisted registry values.

## Workaround

Partial environment-variable fallbacks exist for some keys, but there is no complete persistent workaround for mods that require registry writes.

## Impact

- **Gameplay**: Major for affected mods and settings-dependent features
- **Stability**: None
- **Determinism**: Not Applicable
- **Release blocker**: Yes for cross-platform mod compatibility goals

## Reference

- [GITHUB_ISSUE_DRAFTS_2026-03-12.md](GITHUB_ISSUE_DRAFTS_2026-03-12.md)
- [GeneralsMD/Code/GameEngine/Source/Common/System/registry.cpp](../../GeneralsMD/Code/GameEngine/Source/Common/System/registry.cpp)
- [Core/Libraries/Source/WWVegas/WWDownload/registry.cpp](../../Core/Libraries/Source/WWVegas/WWDownload/registry.cpp)
- [PHASE1_5_STATUS.md](../WORKDIR/reports/PHASE1_5_STATUS.md)