# GitHub Issue Drafts

Prepared on 2026-03-12 for manual GitHub issue creation.

## Scope Note

The item "There may be additional issues that have not yet been identified" was not converted into a GitHub issue because it is not actionable on its own.

## Draft 1

**Title**: Skirmish vs CPU ends immediately with victory screen

**Description**:

Starting any skirmish match against a CPU opponent loads the map, runs briefly, and then immediately shows the victory screen before returning to the menu.

Campaign and Generals Challenge remain playable, but skirmish is currently not playable on the cross-platform port.

Existing investigation in [ISSUE-002_skirmish_instant_victory.md](ISSUE-002_skirmish_instant_victory.md) points to a likely player or team ownership mismatch during skirmish initialization, causing victory evaluation to treat the opposing side as already defeated.

**Impact**:
- Skirmish mode is effectively blocked
- Critical gameplay regression for Linux builds
- Likely affects macOS as well where the same gameplay initialization path is used

## Draft 2

**Title**: Building and unit shadows render incorrectly on Linux and macOS

**Description**:

Building shadows and unit shadows do not render correctly in the SDL3 + DXVK port.

This does not currently block gameplay, but it noticeably reduces visual fidelity and makes scene readability worse during normal matches.

The issue appears to be limited to the rendering path. Gameplay logic continues to function, so investigation should focus on the shadow rendering pipeline, shadow asset handling, and DXVK translation behavior.

**Impact**:
- Cosmetic but highly visible rendering defect
- Affects normal gameplay presentation
- Medium severity release quality issue

## Draft 3

**Title**: Stealth units and GLA stealth buildings render as fully visible

**Description**:

Stealth units are currently displayed as regular visible units, and the same happens with GLA stealth buildings.

This is more than a cosmetic problem because it exposes information that should remain hidden from the player. Even if stealth gameplay logic is still active internally, the visual presentation breaks the mechanic.

Investigation should focus on the stealth rendering path, object state bits, and material or opacity handling in the cross-platform renderer.

**Impact**:
- Breaks expected stealth presentation
- Changes gameplay readability and hidden-information behavior
- High severity rendering and gameplay issue

## Draft 4

**Title**: Long voice lines and some sound effects do not play correctly with OpenAL backend

**Description**:

The current sound system is partially working, but long voice lines and some sound effects still behave incorrectly.

Short sounds and general audio output are present, but longer spoken lines and selected effects may be missing, cut off, distorted, or otherwise unreliable. This reduces gameplay feedback and makes the audio port incomplete.

Investigation should focus on the Miles-to-OpenAL compatibility layer, especially streaming or buffered playback paths used by longer assets.

**Impact**:
- Audio backend is not feature-complete
- Voice feedback and some effects are unreliable
- Medium severity release blocker for audio parity

## Draft 5

**Title**: Unsigned macOS build is blocked by Gatekeeper on first launch

**Description**:

Because the macOS build is not code-signed, Gatekeeper blocks execution of the GeneralsXZH executable and related libraries.

Users currently have to manually approve the application in Settings > Privacy & Security, and approval may be required multiple times for the main executable and bundled libraries. This is a packaging and distribution problem rather than a gameplay bug, but it creates major friction for testing and adoption.

The long-term fix is proper macOS packaging, code signing, and eventually notarization.

**Impact**:
- New users cannot launch the app without manual OS overrides
- Repeated approval prompts create a poor first-run experience
- High severity platform and distribution issue

## Draft 6

**Title**: Terrain textures fail to load on macOS builds

**Description**:

Terrain textures are still failing to load on macOS builds, making maps effectively unplayable.

Earlier diary notes claimed this problem had been fixed, but that conclusion was incorrect or incomplete. The issue remains present in current testing and should be treated as an active open bug, not as a closed or speculative regression.

Investigation should verify the current macOS deployment flow, including DXVK configuration file deployment, shader pipeline behavior, and whether the observed runtime failure matches the previously diagnosed Metal shader path.

**Impact**:
- Maps become effectively unplayable when terrain is missing
- Critical macOS rendering issue
- Current documentation overstated the level of fix verification

## Draft 7

**Title**: Replace null registry stubs with a cross-platform registry.ini backend

**Description**:

The current cross-platform port replaced much of the Windows registry model with null stubs or limited read-only fallbacks.

That was enough to unblock compilation and some startup paths, but it is not sufficient for mods and engine features that expect registry-backed values to be readable and writable across runs.

The proposed fix is to introduce a cross-platform registry replacement backed by a file named `registry.ini`. The wrapper should preserve the existing registry helper interfaces used throughout the codebase, create the file automatically when missing, and map registry paths and values to deterministic INI sections and keys.

This replacement should be implemented in the backend layer only. Windows builds should continue using the native registry, while Linux and macOS should use the `registry.ini` backend.

**Impact**:
- Required for mods that depend on persisted registry values
- Removes an important class of platform stubs from the runtime path
- High severity platform compatibility issue