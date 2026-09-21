# 01 — Engine seams (what exists today)

> Part of [overview.md](overview.md). Depends on: none. Reference map for 02–06; changes nothing.

All references verified on `main` @ `241f01926`. `Core/` is shared; `GeneralsMD/` is Zero Hour.

## Command path

| Fact | Where |
|---|---|
| `GameMessage`, types enum; network range `MSG_BEGIN_NETWORK_MESSAGES = 1000` … `MSG_END_NETWORK_MESSAGES = 1999` — only this range is sent over the network and recorded | `Core/GameEngine/Include/Common/MessageStream.h:96`, `:498`, `:618` |
| Selection: `MSG_CREATE_SELECTED_GROUP(Bool createNew, objectIDs…)`, `…_NO_SOUND`, `MSG_DESTROY_SELECTED_GROUP`, `MSG_REMOVE_FROM_SELECTED_GROUP` | `MessageStream.h:500-508` |
| Orders used as examples below: `MSG_QUEUE_UNIT_CREATE` `:553`, `MSG_DOZER_CONSTRUCT` `:555`, `MSG_DO_ATTACK_OBJECT` `:568`, `MSG_DO_MOVETO` `:577` | `MessageStream.h` |
| Every `GameMessage` is stamped with the **local** player index at construction | `Core/GameEngine/Source/Common/MessageStream.cpp:57` |
| Translators run in priority order; survivors moved to `TheCommandList` | `MessageStream.cpp:1075-1121` (`propagateMessages`) |
| Translator registration (Window, MetaEvent, HotKey, Place, GUICommand, Selection, LookAt, Command, HintSpy, final dispatcher) | `GeneralsMD/Code/GameEngine/Source/GameClient/GameClient.cpp:297-315` |
| Network takes network-range messages off `TheCommandList` and schedules them at `frame + runAhead` | `Core/GameEngine/Source/GameNetwork/Network.cpp:456-484` |
| On receive, the sender's player index comes from the **packet slot**, not the message → a client can only command its own player | `Core/GameEngine/Source/GameNetwork/NetCommandMsg.cpp:156` |
| Logic dispatch; `getMessagePlayer` | `Core/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp:357`, `:210-213` |
| Selection is **logic state per player** (`Player::m_currentSelection`) | `GeneralsMD/Code/GameEngine/Include/Common/Player.h:828`; `GameLogic::selectObject` `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp:2720-2775` |

**Consequence:** a bot orders specific units by sending `MSG_CREATE_SELECTED_GROUP(true, ids…)` then the order.
Production/building act on a single selected object (`getSingleObjectFromSelection`, `GameLogicDispatch.cpp:217`).
The SDK hides this pairing (see [03](03-commands-and-hardening.md)).

## Frame loop

| Fact | Where |
|---|---|
| 30 logic frames/s (`LOGICFRAMES_PER_SECOND = WWSyncPerSecond = 30`) | `Core/GameEngine/Include/Common/GameCommon.h:68`, `Core/Libraries/Source/WWVegas/WWLib/WWCommon.h:46` |
| `GameEngine::update`: client update → `propagateMessages()` `:910` → `TheNetwork->UPDATE()` `:914` → `TheGameLogic->UPDATE()` `:921` → `TheGameClient->step()` | `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp:896-929` |
| In MP the logic advances only when `isFrameDataReady()`; in skirmish a time accumulator paces it | `GameEngine.cpp:816-893` |
| `GameLogic::update`: scripts → CRC → `TheRecorder->UPDATE()` `:3796` → **`processCommandList` `:3801` (orders for frame N run here, before object updates)** → objects, AI, partition/shroud → victory `:3917` → `m_frame++` `:3935` | `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp:3687` |
| Run-ahead: initial 30, bounds `MIN_RUNAHEAD = 4`, `MAX_FRAMES_AHEAD = 128`, adjusted at runtime | `Network.cpp:330`, `:643-646`; `Core/GameEngine/Include/GameNetwork/NetworkDefs.h:38-39` |

## Validation that exists

| Guard | Where |
|---|---|
| Unknown player index rejected | `GameLogicDispatch.cpp:363-368` |
| Ownership filter on the selected group — **all-or-nothing**: one unowned member drops the whole order | `GameLogicDispatch.cpp:397-400`, `GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIGroup.cpp:295-316`, `:235-240` |
| Explicit owner checks only in some handlers (rally point, cancel unit, cancel construct) | `GameLogicDispatch.cpp:960-972`, `:1829-1830`, `:1956-1958` |
| Network: bad slots / stale frames / non-network types rejected | `Core/GameEngine/Source/GameNetwork/ConnectionManager.cpp:529-560`, `Network.cpp:584-595` |

**Gap (bobtista):** no shroud/visibility check on targets. `onDoAttackObject` only checks the target exists
(`GameLogicDispatch.cpp:1700-1715`); the UI only changes the cursor hint over shrouded objects
(`GeneralsMD/Code/GameEngine/Source/GameClient/InGameUI.cpp:2862-2874`). A crafted message can do what the UI cannot.

## Fog / visibility

| Fact | Where |
|---|---|
| `CellShroudStatus {CLEAR, FOGGED, SHROUDED}`, `ObjectShroudStatus {…}` | `Core/GameEngine/Include/Common/GameCommon.h:157`, `:166` |
| `PartitionManager::getShroudStatusForPlayer(idx, x, y)` — **pure read** | `GeneralsMD/Code/GameEngine/Include/GameLogic/PartitionManager.h:1521-1522`, impl `PartitionManager.cpp:3121-3137` |
| `Object::getShroudedStatus(idx) const` → `PartitionData::getShroudedStatus` — **writes caches** (`m_shroudedness`, `m_everSeenByPlayer`, ghost snapshots). Rules: enemy units in fog = shrouded unless immobile and ever seen; mines in fog always shrouded | `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Object.cpp:1856-1866`, `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/PartitionManager.cpp:1616-1722` |
| Ghost objects (remembered buildings) tracked for the local player only unless `m_enablePlayerObserver`; dummy in headless | `GeneralsMD/Code/GameEngine/Include/GameLogic/GhostObject.h:42`, `:102-123` |
| Radar per player (`isRadarHidden`, `isRadarForced`) | `Core/GameEngine/Include/Common/Radar.h:196`, `:202` |

## Headless / replay / CRC

| Fact | Where |
|---|---|
| `-headless`, `-replay`, `-jobs` | `Core/GameEngine/Source/Common/CommandLine.cpp:1170-1182` |
| Headless replay loop calls `TheGameLogic->UPDATE()` directly — **no `GameEngine::update`, no MessageStream, no Network** | `Core/GameEngine/Source/Common/ReplaySimulation.cpp:99` |
| Worker mode spawns child processes (`WorkerProcess`: stdout pipe + job object) | `ReplaySimulation.cpp:130-201`; `Core/GameEngine/Source/Common/WorkerProcess.cpp:77-120` |
| Recording writes every network-range message in `TheCommandList` | `GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp:466-517` |
| Playback **deletes locally injected network-range commands** (`cullBadCommands`) → a bot cannot act during replay playback, by design | `Recorder.cpp:1579-1605` |
| CRC every N frames; covers objects, partition cell shroud, players, AI groups, logic RNG seed | `GameLogic.cpp:3758-3789`, `:4142` |
| CI replay check: `generalszh.exe -jobs 4 -headless -replay *.rep`, exit code checked | `.github/workflows/reusable-check-replays.yml:176`, `:221` |
| Headless dummies: audio, particles, radar, ghost objects, mouse/keyboard, window manager, tactical view, movies, score screen | `GameEngine.cpp:534`, `:555`, `:617`; `GameLogic.cpp:394`; `GameClient.cpp:270`, `:325`, `:343`; `Core/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp:273` |

## Match setup that exists

| Fact | Where |
|---|---|
| Slot states `SLOT_OPEN/CLOSED/EASY_AI/MED_AI/BRUTAL_AI/PLAYER`; observer template | `Core/GameEngine/Include/GameNetwork/GameInfo.h:36-50` |
| `-map` sets `m_mapName`; `-file <x.map>` starts a single-player map via `MSG_NEW_GAME` | `CommandLine.cpp:400-408`; `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp:712-730` |
| Skirmish setup flows through `TheSkirmishGameInfo` | `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/SkirmishGameOptionsMenu.cpp:432` |
| LAN: `RequestGameCreate`, `RequestGameJoin`, `RequestGameJoinDirectConnect` | `Core/GameEngine/Include/GameNetwork/LANAPI.h:75-85` |
| Multi-instance on one machine | `Core/GameEngine/Source/GameClient/ClientInstance.cpp:55` |

## Unsafe getters — an observer must never call

| Call | Why |
|---|---|
| `Object::getShroudedStatus` | Writes partition caches and ghost snapshots |
| `WeaponTemplate::getDelayBetweenShots` | Advances the logic RNG (`GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Weapon.cpp:493-501`) |
| `GameLogicRandomVariable::getValue` | Advances the logic RNG (`Core/GameEngine/Source/Common/RandomValue.cpp:448-460`) |
| `TheAI->createGroup`, `Player::getCurrentSelectionAsAIGroup` | Increments the group ID counter; group list is CRC'd (`GeneralsMD/Code/GameEngine/Include/GameLogic/AI.h:298`) |
| `AIUpdate::getNextMoodTarget`, `getNextWaypoint` | Use logic RNG |

Logic, client and audio RNG streams are separate (`RandomValue.cpp:50-60`). The observer uses **no** RNG.
[09](09-verification.md) adds a test that proves the snapshot writer is pure.

## Placement (xezon on #3328)

- New files → `Core/GameEngine/{Include,Source}/Common/BotBridge/` (name open).
- Hooks into files not yet in `Core` (`GameEngine.cpp`, `GameLogic.cpp`) → ZH (`GeneralsMD/`) first, then the Generals replica, before merge.
- Scripted `AIPlayer` / `AISkirmishPlayer` untouched.

## Platforms (as of 2026-09)

| Platform | Game binary today | Implication |
|---|---|---|
| Windows | Native 32-bit (VC6 and MSVC presets) | Winsock is available (`Core/GameEngine/Source/GameNetwork/Transport.cpp:94`) |
| Linux | Windows `.exe` under Wine (`scripts/docker-build.sh`); native build proposed (#2088) | Loopback TCP crosses the Wine boundary; a native Linux bot talks to a Wine game unchanged |
| macOS | Via Wine/CrossOver only; no native port | Same as Linux |

Engine code stays C++98-compatible for VC6; no new vcpkg dependency (today: `zlib`, `stb`).
