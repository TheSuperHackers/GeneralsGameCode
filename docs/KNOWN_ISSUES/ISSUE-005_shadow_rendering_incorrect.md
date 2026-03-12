# ISSUE-005: Building and Unit Shadows Render Incorrectly

**Status**: OPEN  
**Session Discovered**: Backlog (2026-03-12)  
**Severity**: Medium  
**Component**: Graphics  
**Reproducibility**: High  

## Symptom

Building shadows and unit shadows do not render correctly in the cross-platform port.

The issue is visible during regular gameplay, but it does not currently prevent matches from progressing.

## Investigation Summary

The defect appears isolated to the rendering path rather than gameplay logic.

Likely areas to audit:
- W3D shadow manager behavior in the SDL3 + DXVK path
- Shadow texture or material setup differences versus the legacy renderer
- Platform-specific differences between Linux and macOS graphics backends

## Code Audit Results

Not yet investigated in the main codebase.

## Next Steps (for Future Session)

1. Capture before and after screenshots showing unit and building shadow failures.
2. Audit the shadow submission path starting from the W3D shadow manager.
3. Compare current behavior with the fighter19 reference implementation.
4. Verify whether the defect reproduces identically on Linux and macOS.

## Workaround

None.

## Impact

- **Gameplay**: Minor
- **Stability**: None
- **Determinism**: Not Applicable
- **Release blocker**: No

## Reference

- [GITHUB_ISSUE_DRAFTS_2026-03-12.md](GITHUB_ISSUE_DRAFTS_2026-03-12.md)