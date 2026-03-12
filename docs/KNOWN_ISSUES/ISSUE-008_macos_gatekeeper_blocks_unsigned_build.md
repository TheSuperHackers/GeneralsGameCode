# ISSUE-008: macOS Gatekeeper Blocks Unsigned Build

**Status**: OPEN  
**Session Discovered**: Backlog (2026-03-12)  
**Severity**: High  
**Component**: Platform  
**Reproducibility**: 100%  

## Symptom

The macOS build is not code-signed, so Gatekeeper blocks the application and related libraries from running on first launch.

Users must manually approve execution under Settings > Privacy & Security, and approval may be required multiple times.

## Investigation Summary

This is a packaging and distribution problem rather than a gameplay or engine-logic defect.

The likely long-term fix requires:
- App bundle hardening
- Proper code signing
- Notarization workflow for distribution builds

## Code Audit Results

Not yet investigated in the packaging pipeline.

## Next Steps (for Future Session)

1. Define expected app bundle layout for macOS distribution.
2. Add signing steps to the macOS packaging workflow.
3. Validate whether bundled libraries are correctly embedded and signable.
4. Document a temporary first-run workaround until packaging is complete.

## Workaround

Allow the executable manually under Settings > Privacy & Security, then retry launch as needed for additional libraries.

## Impact

- **Gameplay**: Blocking for first launch until manually bypassed
- **Stability**: None
- **Determinism**: Not Applicable
- **Release blocker**: Yes

## Reference

- [GITHUB_ISSUE_DRAFTS_2026-03-12.md](GITHUB_ISSUE_DRAFTS_2026-03-12.md)
- [PHASE05_MACOS_PORT.md](../WORKDIR/phases/PHASE05_MACOS_PORT.md)