# ISSUE-006: Stealth Units and GLA Stealth Buildings Render as Visible

**Status**: OPEN  
**Session Discovered**: Backlog (2026-03-12)  
**Severity**: High  
**Component**: Graphics  
**Reproducibility**: High  

## Symptom

Stealth units appear as regular visible units, and GLA stealth buildings are also rendered as fully visible objects.

## Investigation Summary

This issue affects both presentation and gameplay readability because hidden information is exposed to the player.

Likely areas to audit:
- Stealth update modules and object state propagation
- Friendly and enemy opacity handling in the renderer
- Material or shader selection for stealth objects in the DXVK path

## Code Audit Results

Not yet investigated in the main codebase.

## Next Steps (for Future Session)

1. Confirm whether stealth gameplay logic still behaves correctly while rendering is wrong.
2. Audit stealth state bits from object update through render submission.
3. Compare renderer behavior against upstream and fighter19 reference behavior.
4. Validate whether both units and structures fail for the same underlying reason.

## Workaround

None.

## Impact

- **Gameplay**: Major
- **Stability**: None
- **Determinism**: Not Applicable
- **Release blocker**: Yes

## Reference

- [GITHUB_ISSUE_DRAFTS_2026-03-12.md](GITHUB_ISSUE_DRAFTS_2026-03-12.md)