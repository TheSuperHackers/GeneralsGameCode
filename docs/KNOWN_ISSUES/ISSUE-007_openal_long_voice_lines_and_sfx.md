# ISSUE-007: Long Voice Lines and Some Sound Effects Do Not Play Correctly

**Status**: OPEN  
**Session Discovered**: Backlog (2026-03-12)  
**Severity**: Medium  
**Component**: Audio  
**Reproducibility**: High  

## Symptom

Audio output is partially functional, but long voice lines and some sound effects do not play correctly.

Short sounds generally work, while longer spoken assets and selected effects may be missing, cut off, distorted, or otherwise unreliable.

## Investigation Summary

The current behavior suggests the OpenAL backend is working for simpler playback cases but remains incomplete for some asset classes.

Likely areas to audit:
- Miles-to-OpenAL compatibility layer
- Streamed or buffered playback paths for longer assets
- Format handling differences between short effects and voice content

## Code Audit Results

Not yet investigated in the main codebase.

## Next Steps (for Future Session)

1. Catalog which voice lines and effect categories fail.
2. Compare short versus long asset playback paths in the audio backend.
3. Review reference notes from the fighter19 and jmarshall audio work.
4. Verify whether the failures are decode-related, buffer-related, or timing-related.

## Workaround

None.

## Impact

- **Gameplay**: Major
- **Stability**: None
- **Determinism**: Not Applicable
- **Release blocker**: Yes

## Reference

- [GITHUB_ISSUE_DRAFTS_2026-03-12.md](GITHUB_ISSUE_DRAFTS_2026-03-12.md)
- [phase0-fighter19-complete-analysis.md](../WORKDIR/support/phase0-fighter19-complete-analysis.md)
- [PHASE_ROADMAP.md](../WORKDIR/planning/PHASE_ROADMAP.md)