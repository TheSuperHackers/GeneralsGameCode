# ISSUE-001: Shell Map Units Immortal Under Bombardment

**Status**: OPEN — Investigation in Progress  
**Session Discovered**: 51 (2026-02-20)  
**Severity**: Medium (cosmetic — intro doesn't crash, just accumulates units)  
**Component**: Gameplay  
**Reproducibility**: 100% — Run shell map with units visible  

## Symptom

Shell map intro displays GLA terrorists and USA soldiers/vehicles being attacked by army units. Explosions render correctly, but:
- Units take no damage and remain at full health
- Camera moves and units animate (scripts work)
- Water renders (scripts work)
- No crashes — game loop continues indefinitely
- Units accumulate in one location due to pathfinding bottleneck

## Investigation Summary (Session 51)

**Root Cause NOT ABI-Related:** Traced entire damage pipeline:
- Health variables: `Real` (float, 4 bytes fixed width on all platforms)
- Damage parameters: `Int` (32-bit, fixed width)
- INI parsing: `offsetof()` calculated per-platform, so no struct padding issues
- `Armor.h`: No `long`, no `wchar_t`, no ABI-sensitive types in damage flow

### Investigated but Inconclusive

1. **HighlanderBody module** — Code exists and works as designed:
   - Limits damage so unit survives with ≥1 HP: `m_amount = min(m_amount, getHealth() - 1)`
   - Unless damage type is `DAMAGE_UNRESISTABLE`
   - **Unknown if shell map units use this module** (difficult to inspect INI/maps without BIG extractor)

2. **inflictDamage=FALSE flag** — Pipeline supports this:
   - Explosions fire (`fireWeaponTemplate` called)
   - VFX rendered
   - Damage NOT applied to objects
   - Likely explanation for "explosions but no effect" but unconfirmed

3. **Script waypoint completion killer** — Scripts check `NAMED_REACHED_WAYPOINTS_END` / `TEAM_REACHED_WAYPOINTS_END`:
   - Now triggers correctly because waypoints load (fixed in Session 51's DataChunk WideChar bug)
   - Logic kills units when waypoint path ends
   - **Possible**: Script tries to kill before units take damage via bombardment

4. **Weapon damage evaluation logic** — `WeaponBonus::getField(DAMAGE)` returns multiplier:
   - Default: 1.0f
   - Unconfirmed if shell map weapons have 0.0f bonus (would explain no damage)

## Code Audit Results

**Files checked for health/damage path:**
- `ActiveBody.cpp`: Health decrement logic — sound
- `Armor.cpp`: Damage modifiers — sound (defaults to 1.0f if template missing)
- `Weapon.cpp:dealDamageInternal()`: Calls `attemptDamage()` — sound
- `DataChunk.cpp`: Fixed WideChar bug Session 51 — all scripts/objects now load
- `ScriptActions.cpp`: Damage application path — sound

**No blocking bugs found in 64-bit ABI on damage path itself.**

## Next Steps (for Future Session)

1. **Extract shell map or INI data** — Determine which units use `HighlanderBody` module
   - Need BIG file extractor or access to uncompressed INI files
   - Check `ObjectTemplate` definitions for GLA/USA units in intro

2. **Inspect shell map script conditions** — Verify waypoint kill logic
   - Does script kill units before weapon damage?
   - Are there `TEAM_REACHED_WAYPOINTS_END` → `TEAM_KILL` chains?

3. **Add diagnostic logging** — Temporary instrumentation:
   - Log `WeaponBonus::getField(DAMAGE)` values during shell map
   - Log `inflictDamage` flag values when weapons fire
   - Log when `HighlanderBody::attemptDamage()` limits damage below expected amount

4. **Test DAMAGE_UNRESISTABLE** — If HighlanderBody is the culprit:
   - Verify that UNRESISTABLE damage bypasses the 1-HP limit
   - Check if any shell map weapons use UNRESISTABLE type

5. **Review shell map INI** — After extracting:
   - Check all unit MaxHealth values (should be >0)
   - Verify armor templates are assigned correctly
   - Confirm weapon Primary/SecondaryDamage ≠ 0

## Workaround

None identified — this is a data/script issue, not a code bug blocking gameplay.

## Impact

- **Gameplay**: None — this is the intro/shell map only
- **Stability**: None — game loop continues
- **Determinism**: Not applicable (intro sequence)
- **Release blocker**: No (can ship with cosmetic issue, address later)

## Reference

- **Fixed related bug**: Session 51 — WideChar binary serialization in `DataChunk::readUnicodeString()` and `LanguageFilter::readWord()` — restored map scripts, camera movement, unit animation
- **Dev diary**: [docs/DEV_BLOG/2026-02-DIARY.md](../DEV_BLOG/2026-02-DIARY.md) — Session 51 entry
- **Lessons**: [docs/WORKDIR/lessons/2026-02-LESSONS.md](../WORKDIR/lessons/2026-02-LESSONS.md) — LESSON-51 (WideChar/UTF-16LE ABI)
