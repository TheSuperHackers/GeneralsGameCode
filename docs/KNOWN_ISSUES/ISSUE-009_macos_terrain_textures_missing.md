# ISSUE-009: macOS Terrain Textures Missing or Failing to Load

**Status**: OPEN  
**Session Discovered**: Backlog (2026-03-12)  
**Severity**: Critical  
**Component**: Graphics  
**Reproducibility**: High  

## Symptom

Terrain textures are still missing on macOS builds, making maps effectively unplayable.

## Investigation Summary

The current development diary contains February 2026 entries claiming that terrain textures were rendering correctly after DXVK and MoltenVK shader changes. That conclusion was incorrect or incomplete.

At minimum, one of the following is true:
- The earlier validation was not representative of normal runtime conditions
- The fix only addressed one failure mode while the real terrain-loading issue persisted
- Deployment or runtime configuration was assumed correct without complete end-to-end verification

## Code Audit Results

User testing confirms the issue still persists. Root cause remains unresolved.

## Next Steps (for Future Session)

1. Reproduce the issue on the latest macOS build and capture current render and shader logs.
2. Verify whether `dxvk.conf` is deployed next to the runtime binary.
3. Compare the current failure mode against the previously documented Metal shader issue.
4. Identify which part of the terrain pipeline is still failing instead of assuming the prior fix covered the whole problem.

## Workaround

None known.

## Impact

- **Gameplay**: Blocking
- **Stability**: None
- **Determinism**: Not Applicable
- **Release blocker**: Yes

## Reference

- [GITHUB_ISSUE_DRAFTS_2026-03-12.md](GITHUB_ISSUE_DRAFTS_2026-03-12.md)
- [2026-02-DIARY.md](../DEV_BLOG/2026-02-DIARY.md)
- [2026-03-DIARY.md](../DEV_BLOG/2026-03-DIARY.md)