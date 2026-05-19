# Optimistic hotkey-group formation (Option C)

## Problem

Commit `67f50d17b` ("Remove input latency for group creation in multiplayer
games", Aug 24 2025) tried to make Ctrl+N → N feel instant by processing
`MSG_CREATE_TEAM_N` immediately on the sender via `CommandXlat`, while remote
clients still processed it at the network-delivered logic frame. That created
a `RUNAHEAD`-sized window in which `Player::m_squads[N]` differed between the
sender and every remote client.

`Player::m_squads` is **not** itself in `Player::crc`, but it is read by game
logic that *does* feed CRC — most directly by
`ReplaceObjectUpgrade::upgradeImplementation()` calling
`getControllingPlayer()->getSquadNumberForObject(me)`, and indirectly via
`processSelectTeamGameMessage` copying `m_squads[N]` → `m_currentSelection`,
which then drives every subsequent `logicMessageDispatcher` AIGroup creation
in `TheAI->m_groupList`, which **is** in `AI::crc`. So the divergence leaks.

The revert (HEAD already) restores deterministic behavior by removing the
`CommandXlat` fast-path entirely. The cost is that `Player::getHotkeySquad(N)`
in `SelectionXlat`'s `MSG_META_SELECT_TEAM_N` handler (lines 1093–1109,
1118–1133) sees the **old** team contents for up to `RUNAHEAD` frames after
Ctrl+N. With Option A's lowered initial floor the worst case during the first
second is now ~267 ms instead of 1 s, and steady-state on a healthy connection
is ~133 ms. That's borderline acceptable but you can still feel it on
back-to-back `Ctrl+N` → `N`.

Option C keeps the revert (so `Player::m_squads` stays authoritative-only) and
adds a **client-only** layer that the local UI consults during the runahead
window.

## Design summary

Introduce a per-translator pending-hotkey table that lives entirely in
`SelectionTranslator`. It records, for each hotkey slot, the object IDs the
local player *just* asked to bind, the logic frame at which the request was
emitted, and a "confirmed" flag that flips when the network round-trips the
corresponding `MSG_CREATE_TEAM_N` back.

- **Write site:** the existing `MSG_META_CREATE_TEAM0..9` arm of
  `SelectionTranslator::translateGameMessage()` (`SelectionXlat.cpp:1030–1059`).
  Right next to where it builds the outgoing `MSG_CREATE_TEAM_N`, it also
  stamps the pending table with the same object IDs.
- **Read site:** the existing `MSG_META_SELECT_TEAM0..9` arm
  (`SelectionXlat.cpp:1062–1141`). Before falling back to
  `player->getHotkeySquad(group)`, consult the pending table; if the slot has
  a fresh unconfirmed entry, use those IDs for the immediate local selection.
- **Confirmation site:** a new client-only hook fired from
  `GameLogic::logicMessageDispatcher`'s `MSG_CREATE_TEAM` case (the *real*
  network-delivered processing), which tells `TheSelectionTranslator` to
  clear/confirm the pending entry for the local player.

Nothing about this touches `Player::m_squads`, `m_currentSelection`,
`Player::crc`, `AI::m_groupList`, or any other CRC-tracked field. The
authoritative path remains exactly what HEAD does post-revert; we just add a
client-side shadow that the UI prefers during the in-flight window.

## Files touched

```
GeneralsMD/Code/GameEngine/Include/GameClient/SelectionXlat.h        (header)
GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/SelectionXlat.cpp
GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp  (one-line hook call)
```

Per the project's GeneralsMD-only rule, skip the corresponding files under
Generals/.

## Detailed changes

### 1. `SelectionXlat.h` — pending table

Add to `SelectionTranslator`:

```cpp
private:
    // Optimistic client-side hotkey state. Mirrors what the local player just
    // asked to assign to each hotkey slot, so that MSG_META_SELECT_TEAM_N can
    // pick the new units immediately instead of waiting RUNAHEAD frames for
    // the network round-trip into Player::m_squads. NEVER read from game
    // logic or anything that feeds the CRC; UI-only.
    struct PendingHotkey
    {
        std::vector<ObjectID> objects;
        UnsignedInt           issuedFrame;   // TheGameLogic->getFrame() at META_CREATE_TEAM time
        Bool                  confirmed;     // network-delivered CREATE_TEAM has landed
        Bool                  valid;         // set true after the first META_CREATE_TEAM into this slot
    };

    static const Int NUM_HOTKEY_SQUADS_LOCAL = 10; // mirrors NUM_HOTKEY_SQUADS

    PendingHotkey m_pendingHotkeys[NUM_HOTKEY_SQUADS_LOCAL];

public:
    // Called from GameLogic::logicMessageDispatcher when a network-delivered
    // MSG_CREATE_TEAM_N for the local player has been processed. Confirms or
    // clears the pending entry so subsequent selects read from the canonical
    // Player::m_squads instead.
    void notifyHotkeyConfirmed(Int hotkeyNum);
```

Add `<vector>` and `Common/GameType.h` (for `ObjectID`) to the include set if
they aren't already pulled in.

### 2. `SelectionXlat.cpp` — write + read + reset

#### 2a. Constructor / reset

Wherever `m_lastGroupSelTime = 0;` and `m_lastGroupSelGroup = -1;` are set
(constructor and the two existing reset sites at lines ~266, ~722, ~955), also
zero the pending table:

```cpp
for (Int i = 0; i < NUM_HOTKEY_SQUADS_LOCAL; ++i)
{
    m_pendingHotkeys[i].objects.clear();
    m_pendingHotkeys[i].issuedFrame = 0;
    m_pendingHotkeys[i].confirmed   = FALSE;
    m_pendingHotkeys[i].valid       = FALSE;
}
```

VC6 note: declare `Int i;` once at the top of the function and reuse it across
any sibling for-loops in the same scope (the global per-CLAUDE.md VC6 quirk).

#### 2b. Write site — `MSG_META_CREATE_TEAM0..9` (around line 1040)

Extend the existing arm so that right after the outgoing `MSG_CREATE_TEAM_N`
is built, the same object IDs are stamped into the pending slot:

```cpp
case GameMessage::MSG_META_CREATE_TEAM0:
... // unchanged
case GameMessage::MSG_META_CREATE_TEAM9:
{
    Int group = t - GameMessage::MSG_META_CREATE_TEAM0;
    if ( group >= 0 && group < NUM_HOTKEY_SQUADS_LOCAL )
    {
        DEBUG_LOG(("META: create team %d",group));

        // Build the authoritative network message exactly as before.
        GameMessage *newmsg = TheMessageStream->appendMessage(
            (GameMessage::Type)(GameMessage::MSG_CREATE_TEAM0 + group));

        // Snapshot the same set of locally-controlled selected object IDs
        // into the client-only pending table. UI consults this in
        // MSG_META_SELECT_TEAM until the network confirmation arrives.
        PendingHotkey &pending = m_pendingHotkeys[group];
        pending.objects.clear();
        pending.issuedFrame = TheGameLogic ? TheGameLogic->getFrame() : 0;
        pending.confirmed   = FALSE;
        pending.valid       = TRUE;

        Drawable *drawable = TheGameClient->getDrawableList();
        while (drawable != nullptr)
        {
            if (drawable->isSelected()
                && drawable->getObject()
                && drawable->getObject()->isLocallyControlled())
            {
                ObjectID id = drawable->getObject()->getID();
                newmsg->appendObjectIDArgument(id);
                pending.objects.push_back(id);
            }
            drawable = drawable->getNextDrawable();
        }
    }
    disp = DESTROY_MESSAGE;
    break;
}
```

The pending list is built from the exact same iteration as the outgoing
message, so they are guaranteed to contain the same IDs.

#### 2c. Read site — `MSG_META_SELECT_TEAM0..9` (around line 1062)

Replace the inner `Player::getHotkeySquad(group)` reads with a helper that
prefers pending if it's valid and unconfirmed. The structure of the existing
handler (double-click jump, `performSelection`, etc.) stays exactly the same;
only the source of the object list changes.

```cpp
// Helper local to this file (or a private member).
static void getHotkeyObjectsForLocalSelect(
    SelectionTranslator *self, Int group, Player *localPlayer,
    VecObjectPtr &outObjs)
{
    outObjs.clear();
    if (group < 0 || group >= NUM_HOTKEY_SQUADS_LOCAL || localPlayer == nullptr)
        return;

    const SelectionTranslator::PendingHotkey &pending =
        self->getPendingHotkey(group); // small inline accessor

    // Use pending if we have one that the network has not yet confirmed.
    // After confirmation we let m_squads be authoritative again, because the
    // network-delivered CREATE_TEAM may have filtered out units that died,
    // changed ownership, or otherwise didn't make it.
    if (pending.valid && !pending.confirmed && !pending.objects.empty())
    {
        for (std::vector<ObjectID>::const_iterator it = pending.objects.begin();
             it != pending.objects.end(); ++it)
        {
            Object *obj = TheGameLogic->findObjectByID(*it);
            if (obj != nullptr
                && obj->getControllingPlayer() == localPlayer
                && !obj->isEffectivelyDead())
            {
                outObjs.push_back(obj);
            }
        }
        return;
    }

    Squad *squad = localPlayer->getHotkeySquad(group);
    if (squad == nullptr)
        return;

    outObjs = squad->getLiveObjects();
}
```

Then in the `MSG_META_SELECT_TEAM_N` arm, replace both the double-tap jump
block (lines ~1093–1109) and the main `performSelection` block (lines
~1118–1133) so they call this helper instead of touching
`player->getHotkeySquad(group)` directly. The voice cue and
`m_lastGroupSelTime` / `m_lastGroupSelGroup` bookkeeping stay unchanged.

The outgoing `MSG_SELECT_TEAM_N` at line 1117 still goes through unchanged;
that one is what eventually populates `Player::m_currentSelection`
deterministically on every client.

#### 2d. Confirmation handler

```cpp
void SelectionTranslator::notifyHotkeyConfirmed(Int hotkeyNum)
{
    if (hotkeyNum < 0 || hotkeyNum >= NUM_HOTKEY_SQUADS_LOCAL)
        return;
    PendingHotkey &p = m_pendingHotkeys[hotkeyNum];
    p.confirmed = TRUE;
    // We deliberately leave .objects populated. Reads filter on .confirmed so
    // the next select reads from m_squads; this keeps the data around for
    // diagnostics without changing behavior.
}
```

### 3. `GameLogicDispatch.cpp` — confirmation hook

In the `MSG_CREATE_TEAM0..9` case (the one I just restored to call
`processCreateTeamGameMessage` for every player), add a single call after
the squad update, only when this is the local player's own assignment:

```cpp
case GameMessage::MSG_CREATE_TEAM0:
...
case GameMessage::MSG_CREATE_TEAM9:
{
    msgPlayer->processCreateTeamGameMessage(
        msg->getType() - GameMessage::MSG_CREATE_TEAM0, msg);

    // Client-only: tell the selection translator the optimistic pending
    // entry has been ratified by the network, so subsequent selects can
    // read from the canonical Player::m_squads. Safe to call on every
    // client; only the local player has a pending entry.
    if (msgPlayer->isLocalPlayer() && TheSelectionTranslator != nullptr)
    {
        TheSelectionTranslator->notifyHotkeyConfirmed(
            msg->getType() - GameMessage::MSG_CREATE_TEAM0);
    }

    break;
}
```

This is the only line that crosses the logic→client boundary. It is
deterministic-safe because `notifyHotkeyConfirmed` only touches the client
translator's private fields; it does not feed the message stream, the
random seed, `Object`/`Player`/`AI` state, or anything xfer'd into the CRC.

## Edge cases

| case | behavior |
|---|---|
| Unit in pending dies before confirmation | `getHotkeyObjectsForLocalSelect` filters by `isEffectivelyDead()` |
| Unit changes ownership before confirmation | Filtered by `getControllingPlayer() == localPlayer` |
| Player presses `Ctrl+N` twice before first confirms | Second press overwrites pending and resets `confirmed = FALSE` (the first network-delivered CREATE_TEAM still arrives and sets `confirmed = TRUE`; that's fine because the read path checks `!confirmed` and the *current* pending matches the second press) |
| Player presses `Ctrl+N` then `N` repeatedly in same frame | Both presses see the same pending entry; selection deterministic locally |
| Player presses `N` for a slot that's never been used | `pending.valid == FALSE`; falls through to `Player::getHotkeySquad`, which returns an empty `Squad`; behavior matches HEAD |
| Network delivers an old CREATE_TEAM after a newer pending | The handler still sets `confirmed = TRUE` on the slot; next select falls back to `m_squads` which the engine has now updated to the most recent network-authoritative state. Race window is one extra select returning stale-network units instead of optimistic units — UX bug at worst, never desync |
| Replay playback | `MSG_META_*` messages are filtered out before they ever reach `SelectionTranslator` in playback mode (`getInputEnabled()` gates it earlier in `translateGameMessage`); pending table stays at zero. Replays read directly from `Player::m_squads` via the existing path |
| Observer / spectator | Same as replay — `META_*` originates from local input, observers have none |
| Save/load mid-game | Pending is client-only, transient; deliberately not in `xfer`. After a load, all slots reset to invalid and the next `META_CREATE_TEAM` repopulates |
| Resume-from-replay catchup | `TheSelectionTranslator` survives across catchup; pending stays cleared because no local META input fires during catchup |

## Why this is determinism-safe

Three checks to keep in mind when reviewing:

1. `m_pendingHotkeys` is read **only** by `SelectionXlat`'s
   `MSG_META_SELECT_TEAM_N` branch, which is a client-only translator that
   never runs inside `processCommandList` and never appends to `m_objList`
   or `TheAI->m_groupList`.
2. The only logic-side hook (`notifyHotkeyConfirmed`) writes one `Bool` on a
   client-only struct; it does not call any code path that reaches a
   `xferSnapshot`, `getCRC`, `getGameLogicRandomValue*`, or
   `appendMessage`.
3. The outgoing `MSG_CREATE_TEAM` / `MSG_SELECT_TEAM` payloads are unchanged,
   so the recorded `.rep` is byte-identical to pre-Option-C with everything
   else equal. The `MSG_LOGIC_CRC` traffic is unaffected.

If anyone later wants to surface the pending state for HUD rendering ("team
1 forming…" badge), do it by adding a `const` accessor on
`SelectionTranslator` and reading from `Drawable::draw()`. Do **not** move
the pending fields into `InGameUI` if you want them to be xfer'd, and do
**not** plumb them into `Player::m_squads` even temporarily.

## Test plan

- **Determinism regression:** play one of the matches that previously
  desynced (4v4, `exkalibur_zh`) with all eight players on the patched
  build. No CRC mismatch over 20+ min should reproduce.
- **Snappiness:** press `Ctrl+1` then immediately `1` (within 100 ms). The
  newly-created team should be selected without the 133–267 ms gap that
  exists post-revert without Option C.
- **Stale-pending:** create team 1, then kill one of its units before
  pressing `1`. Selection should include the live units only.
- **Double-tap jump:** `Ctrl+1`, then double-tap `1` quickly. Camera should
  jump to the team and select it, same as HEAD.
- **Network LAN observer:** run an observer client at the same time; the
  recorded `.rep` from the host and observer must be byte-identical (the
  pending table is not networked or recorded).
- **VC6 build:** confirm the for-loop variable declarations follow the
  per-CLAUDE.md VC6 scoping rule (one declaration per scope).

## Effort estimate

- `SelectionXlat.h`: ~25 lines (struct + members + accessor + prototype)
- `SelectionXlat.cpp`: ~80 lines (helper + constructor reset + write-site
  population + two read-site swaps + confirmation impl)
- `GameLogicDispatch.cpp`: ~5 lines (hook call)

Plus ~50 lines of doc comments and tests. Realistically one focused session.
