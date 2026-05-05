/*
**	Command & Conquer Generals Zero Hour(tm)
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/StatsExporter.h"
#include "Common/StatsUploader.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/PlayerTemplate.h"
#include "Common/GlobalData.h"
#include "Common/Energy.h"
#include "Common/ThingTemplate.h"
#include "Common/RandomValue.h"
#include "Common/AcademyStats.h"
#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/BattlePlanUpdate.h"

#include <stdio.h>
#include <string.h>
#include <vector>
#include <zlib.h>

//-----------------------------------------------------------------------------
// vc6-friendly JSON helpers: stream JSON directly to a gzFile via the zlib
// gz* C API (gzopen / gzwrite / gzprintf / gzputs / gzputc / gzclose), all
// of which are present in the bundled zlib 1.1.4 and compile cleanly with
// VC6. No memory buffer; no nlohmann::json.
//-----------------------------------------------------------------------------

static void gzPrintJsonStr(gzFile f, const char *s)
{
	gzputc(f, '"');
	if (s != nullptr)
	{
		for (; *s != '\0'; ++s)
		{
			switch (*s)
			{
				case '"':  gzputs(f, "\\\""); break;
				case '\\': gzputs(f, "\\\\"); break;
				case '\n': gzputs(f, "\\n"); break;
				case '\r': gzputs(f, "\\r"); break;
				case '\t': gzputs(f, "\\t"); break;
				default:
					if (static_cast<unsigned char>(*s) < 0x20)
						gzprintf(f, "\\u%04x", static_cast<unsigned int>(static_cast<unsigned char>(*s)));
					else
						gzputc(f, *s);
					break;
			}
		}
	}
	gzputc(f, '"');
}

// Write a wide string as a JSON string, encoding non-ASCII as UTF-8.
static void gzPrintJsonWideStr(gzFile f, const WideChar *s)
{
	gzputc(f, '"');
	if (s != nullptr)
	{
		for (; *s != L'\0'; ++s)
		{
			unsigned int c = static_cast<unsigned int>(*s);
			if (c == '"')       gzputs(f, "\\\"");
			else if (c == '\\') gzputs(f, "\\\\");
			else if (c == '\n') gzputs(f, "\\n");
			else if (c == '\r') gzputs(f, "\\r");
			else if (c == '\t') gzputs(f, "\\t");
			else if (c < 0x20)  gzprintf(f, "\\u%04x", c);
			else if (c < 0x80)  gzputc(f, static_cast<char>(c));
			else if (c < 0x800)
			{
				gzputc(f, static_cast<char>(0xC0 | (c >> 6)));
				gzputc(f, static_cast<char>(0x80 | (c & 0x3F)));
			}
			else
			{
				gzputc(f, static_cast<char>(0xE0 | (c >> 12)));
				gzputc(f, static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
				gzputc(f, static_cast<char>(0x80 | (c & 0x3F)));
			}
		}
	}
	gzputc(f, '"');
}

//-----------------------------------------------------------------------------

static const char* gameModeToString(GameMode mode)
{
	switch (mode)
	{
		case GAME_SINGLE_PLAYER: return "SinglePlayer";
		case GAME_LAN:           return "LAN";
		case GAME_SKIRMISH:      return "Skirmish";
		case GAME_REPLAY:        return "Replay";
		case GAME_SHELL:         return "Shell";
		case GAME_INTERNET:      return "Internet";
		case GAME_NONE:          return "None";
		default:                 return "Unknown";
	}
}

//-----------------------------------------------------------------------------

static Bool isGamePlayer(Player *player)
{
	if (player == nullptr) return FALSE;
	const PlayerTemplate *pt = player->getPlayerTemplate();
	if (pt == nullptr) return FALSE;
	const char *name = pt->getName().str();
	if (name == nullptr || name[0] == '\0') return FALSE;
	if (strcmp(name, "FactionObserver") == 0) return FALSE;
	if (strcmp(name, "FactionCivilian") == 0) return FALSE;
	return TRUE;
}

bool StatsExporterHasMinHumansForUpload()
{
	if (ThePlayerList == nullptr)
		return false;

	Int humans = 0;
	const Int totalPlayers = ThePlayerList->getPlayerCount();
	Int i;
	for (i = 0; i < totalPlayers; ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (player == nullptr || !isGamePlayer(player))
			continue;
		if (player->getPlayerType() == PLAYER_HUMAN)
			++humans;
	}
	return humans >= 2;
}

//-----------------------------------------------------------------------------

struct PlayerSnapshotData
{
	Int playerIndex;
	UnsignedInt money;
	Int moneyEarned;
	Int moneySpent;
};

struct PlayerStateData
{
	Int energyProduction;
	Int energyConsumption;
	Int rankLevel;
	Int skillPoints;
	Int sciencePurchasePoints;
	Bool hasRadar;
	Bool isDead;
	Int bombardment;
	Int holdTheLine;
	Int searchAndDestroy;
};

struct StateChangeEvent
{
	UnsignedInt frame;
	Int playerIndex;
};

struct EnergyEvent : StateChangeEvent { Int production; Int consumption; };
struct RankEvent : StateChangeEvent { Int rankLevel; };
struct SkillPointsEvent : StateChangeEvent { Int skillPoints; };
struct SciencePointsEvent : StateChangeEvent { Int sciencePurchasePoints; };
struct RadarEvent : StateChangeEvent { Bool hasRadar; };
struct DeathEvent : StateChangeEvent {};
struct BattlePlanEvent : StateChangeEvent { Int bombardment; Int holdTheLine; Int searchAndDestroy; };

struct FrameSnapshotData
{
	UnsignedInt frame;
	Int playerCount;
	PlayerSnapshotData players[MAX_PLAYER_COUNT];
};

struct KillEventData
{
	UnsignedInt frame;
	Int killerPlayerIndex;
	Int victimPlayerIndex;
	Real x;
	Real y;
	char killerTemplateName[64];
	char victimTemplateName[64];
	char damageType[32];
};

struct BuildEventData
{
	UnsignedInt frame;
	Int playerIndex;
	Real x;
	Real y;
	Int cost;
	Int buildTime;
	char templateName[64];
	char producerTemplateName[64];
};

struct CaptureEventData
{
	UnsignedInt frame;
	Int newOwnerPlayerIndex;
	Int oldOwnerPlayerIndex;
	Real x;
	Real y;
	char templateName[64];
};

struct StatsExporterState
{
	Bool exportingActive;
	Bool mappingInitialized;
	Int gamePlayerCount;
	Int originalToNewIndex[MAX_PLAYER_COUNT];
	UnsignedInt lastSnapshotFrame;
	PlayerStateData lastPlayerState[MAX_PLAYER_COUNT];

	std::vector<FrameSnapshotData> snapshots;
	std::vector<KillEventData> killEvents;
	std::vector<BuildEventData> buildEvents;
	std::vector<CaptureEventData> captureEvents;
	std::vector<EnergyEvent> energyEvents;
	std::vector<RankEvent> rankEvents;
	std::vector<SkillPointsEvent> skillPointsEvents;
	std::vector<SciencePointsEvent> sciencePointsEvents;
	std::vector<RadarEvent> radarEvents;
	std::vector<DeathEvent> deathEvents;
	std::vector<BattlePlanEvent> battlePlanEvents;

	void resetData()
	{
		mappingInitialized = FALSE;
		gamePlayerCount = 0;
		lastSnapshotFrame = 0;
		memset(originalToNewIndex, 0, sizeof(originalToNewIndex));
		memset(lastPlayerState, 0, sizeof(lastPlayerState));
		snapshots.clear();
		killEvents.clear();
		buildEvents.clear();
		captureEvents.clear();
		energyEvents.clear();
		rankEvents.clear();
		skillPointsEvents.clear();
		sciencePointsEvents.clear();
		radarEvents.clear();
		deathEvents.clear();
		battlePlanEvents.clear();
	}
};

static StatsExporterState s_state;

//-----------------------------------------------------------------------------

static Int remapPlayerIndex(Int rawIndex)
{
	if (rawIndex >= 0 && rawIndex < MAX_PLAYER_COUNT)
		return s_state.originalToNewIndex[rawIndex];
	return 0;
}

//-----------------------------------------------------------------------------

static void initPlayerMapping()
{
	if (s_state.mappingInitialized)
		return;

	s_state.gamePlayerCount = 0;
	memset(s_state.originalToNewIndex, 0, sizeof(s_state.originalToNewIndex));

	const Int totalPlayers = ThePlayerList->getPlayerCount();
	Int i;
	for (i = 0; i < totalPlayers && i < MAX_PLAYER_COUNT; ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (isGamePlayer(player))
		{
			++s_state.gamePlayerCount;
			s_state.originalToNewIndex[i] = s_state.gamePlayerCount;
		}
	}

	// Only lock in the mapping once we find actual game players.
	// Early calls (before players are fully initialized) will retry.
	if (s_state.gamePlayerCount > 0)
		s_state.mappingInitialized = TRUE;
}

//-----------------------------------------------------------------------------

void StatsExporterCollectSnapshot()
{
	if (!s_state.exportingActive)
		return;
	if (ThePlayerList == nullptr || TheGameLogic == nullptr)
		return;

	UnsignedInt currentFrame = TheGameLogic->getFrame();
	if (!s_state.snapshots.empty() && (currentFrame - s_state.lastSnapshotFrame) < 30)
		return;

	s_state.lastSnapshotFrame = currentFrame;

	initPlayerMapping();

	const Int totalPlayers = ThePlayerList->getPlayerCount();

	FrameSnapshotData snap;
	memset(&snap, 0, sizeof(snap));
	snap.frame = currentFrame;
	snap.playerCount = s_state.gamePlayerCount;

	Int gameIdx = 0;
	Int i;
	for (i = 0; i < totalPlayers && i < MAX_PLAYER_COUNT; ++i)
	{
		if (s_state.originalToNewIndex[i] == 0)
			continue;

		Player *player = ThePlayerList->getNthPlayer(i);
		if (player == nullptr)
			continue;

		PlayerSnapshotData &pd = snap.players[gameIdx];
		ScoreKeeper *sk = player->getScoreKeeper();
		const Energy *energy = player->getEnergy();

		pd.playerIndex = s_state.originalToNewIndex[i];
		pd.money = player->getMoney()->countMoney();
		pd.moneyEarned = sk->getTotalMoneyEarned();
		pd.moneySpent = sk->getTotalMoneySpent();

		// Detect state changes and emit events
		{
			PlayerStateData &last = s_state.lastPlayerState[i];
			Int curVal, curVal2, curVal3;
			Bool curBool;

			curVal = energy->getProduction();
			curVal2 = energy->getConsumption();
			if (curVal != last.energyProduction || curVal2 != last.energyConsumption)
			{
				EnergyEvent eev;
				memset(&eev, 0, sizeof(eev));
				eev.frame = currentFrame;
				eev.playerIndex = i;
				eev.production = curVal;
				eev.consumption = curVal2;
				s_state.energyEvents.push_back(eev);
				last.energyProduction = curVal;
				last.energyConsumption = curVal2;
			}

			curVal = player->getRankLevel();
			if (curVal != last.rankLevel)
			{
				RankEvent rev;
				memset(&rev, 0, sizeof(rev));
				rev.frame = currentFrame;
				rev.playerIndex = i;
				rev.rankLevel = curVal;
				s_state.rankEvents.push_back(rev);
				last.rankLevel = curVal;
			}

			curVal = player->getSkillPoints();
			if (curVal != last.skillPoints)
			{
				SkillPointsEvent sev;
				memset(&sev, 0, sizeof(sev));
				sev.frame = currentFrame;
				sev.playerIndex = i;
				sev.skillPoints = curVal;
				s_state.skillPointsEvents.push_back(sev);
				last.skillPoints = curVal;
			}

			curVal = player->getSciencePurchasePoints();
			if (curVal != last.sciencePurchasePoints)
			{
				SciencePointsEvent spev;
				memset(&spev, 0, sizeof(spev));
				spev.frame = currentFrame;
				spev.playerIndex = i;
				spev.sciencePurchasePoints = curVal;
				s_state.sciencePointsEvents.push_back(spev);
				last.sciencePurchasePoints = curVal;
			}

			curBool = player->hasRadar();
			if (curBool != last.hasRadar)
			{
				RadarEvent raev;
				memset(&raev, 0, sizeof(raev));
				raev.frame = currentFrame;
				raev.playerIndex = i;
				raev.hasRadar = curBool;
				s_state.radarEvents.push_back(raev);
				last.hasRadar = curBool;
			}

			curBool = player->isPlayerDead();
			if (curBool && !last.isDead)
			{
				DeathEvent dev;
				memset(&dev, 0, sizeof(dev));
				dev.frame = currentFrame;
				dev.playerIndex = i;
				s_state.deathEvents.push_back(dev);
				last.isDead = curBool;
			}

			curVal = player->getBattlePlansActiveSpecific(PLANSTATUS_BOMBARDMENT);
			curVal2 = player->getBattlePlansActiveSpecific(PLANSTATUS_HOLDTHELINE);
			curVal3 = player->getBattlePlansActiveSpecific(PLANSTATUS_SEARCHANDDESTROY);
			if (curVal != last.bombardment || curVal2 != last.holdTheLine || curVal3 != last.searchAndDestroy)
			{
				BattlePlanEvent bev;
				memset(&bev, 0, sizeof(bev));
				bev.frame = currentFrame;
				bev.playerIndex = i;
				bev.bombardment = curVal;
				bev.holdTheLine = curVal2;
				bev.searchAndDestroy = curVal3;
				s_state.battlePlanEvents.push_back(bev);
				last.bombardment = curVal;
				last.holdTheLine = curVal2;
				last.searchAndDestroy = curVal3;
			}
		}

		++gameIdx;
	}

	s_state.snapshots.push_back(snap);
}

//-----------------------------------------------------------------------------

void StatsExporterBeginRecording()
{
	s_state.exportingActive = TRUE;
	s_state.resetData();
}

bool StatsExporterIsActive()
{
	return s_state.exportingActive ? true : false;
}

//-----------------------------------------------------------------------------

void StatsExporterRecordKill(const Object *killer, const Object *victim, const DamageInfo *damageInfo)
{
	if (!s_state.exportingActive)
		return;
	if (killer == nullptr || victim == nullptr || TheGameLogic == nullptr)
		return;

	const Player *killerPlayer = killer->getControllingPlayer();
	const Player *victimPlayer = victim->getControllingPlayer();
	if (killerPlayer == nullptr || victimPlayer == nullptr)
		return;

	KillEventData ev;
	memset(&ev, 0, sizeof(ev));
	ev.frame = TheGameLogic->getFrame();

	// Store raw player indices; remapped to game-player indices at export time.
	ev.killerPlayerIndex = killerPlayer->getPlayerIndex();
	ev.victimPlayerIndex = victimPlayer->getPlayerIndex();

	const Coord3D *pos = victim->getPosition();
	if (pos != nullptr)
	{
		ev.x = pos->x;
		ev.y = pos->y;
	}

	strlcpy(ev.killerTemplateName, killer->getTemplate()->getName().str(), ARRAY_SIZE(ev.killerTemplateName));
	strlcpy(ev.victimTemplateName, victim->getTemplate()->getName().str(), ARRAY_SIZE(ev.victimTemplateName));

	if (damageInfo != nullptr && damageInfo->in.m_damageType >= 0 && damageInfo->in.m_damageType < DAMAGE_NUM_TYPES)
	{
		const char *name = DamageTypeFlags::s_bitNameList[damageInfo->in.m_damageType];
		if (name != nullptr)
			strlcpy(ev.damageType, name, ARRAY_SIZE(ev.damageType));
	}

	s_state.killEvents.push_back(ev);
}

//-----------------------------------------------------------------------------

void StatsExporterRecordBuild(const Object *producer, const Object *built)
{
	if (!s_state.exportingActive)
		return;
	if (built == nullptr || TheGameLogic == nullptr)
		return;

	const Player *player = built->getControllingPlayer();
	if (player == nullptr)
		return;

	BuildEventData ev;
	memset(&ev, 0, sizeof(ev));
	ev.frame = TheGameLogic->getFrame();

	// Store raw player index; remapped at export time.
	ev.playerIndex = player->getPlayerIndex();

	const Coord3D *pos = built->getPosition();
	if (pos != nullptr)
	{
		ev.x = pos->x;
		ev.y = pos->y;
	}
	ev.cost = built->getTemplate()->calcCostToBuild(player);
	ev.buildTime = built->getTemplate()->calcTimeToBuild(player);

	strlcpy(ev.templateName, built->getTemplate()->getName().str(), ARRAY_SIZE(ev.templateName));

	if (producer != nullptr)
		strlcpy(ev.producerTemplateName, producer->getTemplate()->getName().str(), ARRAY_SIZE(ev.producerTemplateName));

	s_state.buildEvents.push_back(ev);
}

//-----------------------------------------------------------------------------

void StatsExporterRecordCapture(const Object *captured, const Player *oldOwner, const Player *newOwner)
{
	if (!s_state.exportingActive)
		return;
	if (captured == nullptr || oldOwner == nullptr || newOwner == nullptr || TheGameLogic == nullptr)
		return;
	if (oldOwner == newOwner)
		return;

	CaptureEventData ev;
	memset(&ev, 0, sizeof(ev));
	ev.frame = TheGameLogic->getFrame();

	// Store raw player indices; remapped at export time.
	ev.newOwnerPlayerIndex = newOwner->getPlayerIndex();
	ev.oldOwnerPlayerIndex = oldOwner->getPlayerIndex();

	const Coord3D *pos = captured->getPosition();
	if (pos != nullptr)
	{
		ev.x = pos->x;
		ev.y = pos->y;
	}

	strlcpy(ev.templateName, captured->getTemplate()->getName().str(), ARRAY_SIZE(ev.templateName));

	s_state.captureEvents.push_back(ev);
}

//-----------------------------------------------------------------------------
// JSON emission. Compact (newline-separated) — no pretty indentation, since
// the consumer is downstream tooling, not humans.
//-----------------------------------------------------------------------------

static void writeKillEventsJson(gzFile f)
{
	gzputs(f, "[");
	for (size_t i = 0; i < s_state.killEvents.size(); ++i)
	{
		const KillEventData &ev = s_state.killEvents[i];
		if (i > 0) gzputc(f, ',');
		gzprintf(f, "{\"frame\":%u,\"killerPlayer\":%d,\"victimPlayer\":%d,\"x\":%g,\"y\":%g,",
			ev.frame, remapPlayerIndex(ev.killerPlayerIndex), remapPlayerIndex(ev.victimPlayerIndex),
			ev.x, ev.y);
		gzputs(f, "\"killer\":");   gzPrintJsonStr(f, ev.killerTemplateName);
		gzputs(f, ",\"victim\":");  gzPrintJsonStr(f, ev.victimTemplateName);
		gzputs(f, ",\"damageType\":"); gzPrintJsonStr(f, ev.damageType);
		gzputc(f, '}');
	}
	gzputc(f, ']');
}

static void writeBuildEventsJson(gzFile f)
{
	gzputs(f, "[");
	for (size_t i = 0; i < s_state.buildEvents.size(); ++i)
	{
		const BuildEventData &ev = s_state.buildEvents[i];
		if (i > 0) gzputc(f, ',');
		gzprintf(f, "{\"frame\":%u,\"player\":%d,\"x\":%g,\"y\":%g,\"cost\":%d,\"buildTime\":%d,",
			ev.frame, remapPlayerIndex(ev.playerIndex), ev.x, ev.y, ev.cost, ev.buildTime);
		gzputs(f, "\"object\":");     gzPrintJsonStr(f, ev.templateName);
		gzputs(f, ",\"producer\":");  gzPrintJsonStr(f, ev.producerTemplateName);
		gzputc(f, '}');
	}
	gzputc(f, ']');
}

static void writeCaptureEventsJson(gzFile f)
{
	gzputs(f, "[");
	for (size_t i = 0; i < s_state.captureEvents.size(); ++i)
	{
		const CaptureEventData &ev = s_state.captureEvents[i];
		if (i > 0) gzputc(f, ',');
		gzprintf(f, "{\"frame\":%u,\"newOwner\":%d,\"oldOwner\":%d,\"x\":%g,\"y\":%g,",
			ev.frame, remapPlayerIndex(ev.newOwnerPlayerIndex), remapPlayerIndex(ev.oldOwnerPlayerIndex),
			ev.x, ev.y);
		gzputs(f, "\"object\":"); gzPrintJsonStr(f, ev.templateName);
		gzputc(f, '}');
	}
	gzputc(f, ']');
}

static void writeStateChangeEventsJson(gzFile f)
{
	size_t i;

	gzputs(f, ",\n\"energyEvents\":[");
	for (i = 0; i < s_state.energyEvents.size(); ++i)
	{
		const EnergyEvent &ev = s_state.energyEvents[i];
		if (i > 0) gzputc(f, ',');
		gzprintf(f, "{\"frame\":%u,\"player\":%d,\"production\":%d,\"consumption\":%d}",
			ev.frame, remapPlayerIndex(ev.playerIndex), ev.production, ev.consumption);
	}
	gzputc(f, ']');

	gzputs(f, ",\n\"rankEvents\":[");
	for (i = 0; i < s_state.rankEvents.size(); ++i)
	{
		const RankEvent &ev = s_state.rankEvents[i];
		if (i > 0) gzputc(f, ',');
		gzprintf(f, "{\"frame\":%u,\"player\":%d,\"rankLevel\":%d}",
			ev.frame, remapPlayerIndex(ev.playerIndex), ev.rankLevel);
	}
	gzputc(f, ']');

	gzputs(f, ",\n\"skillPointsEvents\":[");
	for (i = 0; i < s_state.skillPointsEvents.size(); ++i)
	{
		const SkillPointsEvent &ev = s_state.skillPointsEvents[i];
		if (i > 0) gzputc(f, ',');
		gzprintf(f, "{\"frame\":%u,\"player\":%d,\"skillPoints\":%d}",
			ev.frame, remapPlayerIndex(ev.playerIndex), ev.skillPoints);
	}
	gzputc(f, ']');

	gzputs(f, ",\n\"sciencePointsEvents\":[");
	for (i = 0; i < s_state.sciencePointsEvents.size(); ++i)
	{
		const SciencePointsEvent &ev = s_state.sciencePointsEvents[i];
		if (i > 0) gzputc(f, ',');
		gzprintf(f, "{\"frame\":%u,\"player\":%d,\"sciencePurchasePoints\":%d}",
			ev.frame, remapPlayerIndex(ev.playerIndex), ev.sciencePurchasePoints);
	}
	gzputc(f, ']');

	gzputs(f, ",\n\"radarEvents\":[");
	for (i = 0; i < s_state.radarEvents.size(); ++i)
	{
		const RadarEvent &ev = s_state.radarEvents[i];
		if (i > 0) gzputc(f, ',');
		gzprintf(f, "{\"frame\":%u,\"player\":%d,\"hasRadar\":%s}",
			ev.frame, remapPlayerIndex(ev.playerIndex), ev.hasRadar ? "true" : "false");
	}
	gzputc(f, ']');

	gzputs(f, ",\n\"deathEvents\":[");
	for (i = 0; i < s_state.deathEvents.size(); ++i)
	{
		const DeathEvent &ev = s_state.deathEvents[i];
		if (i > 0) gzputc(f, ',');
		gzprintf(f, "{\"frame\":%u,\"player\":%d}",
			ev.frame, remapPlayerIndex(ev.playerIndex));
	}
	gzputc(f, ']');

	gzputs(f, ",\n\"battlePlanEvents\":[");
	for (i = 0; i < s_state.battlePlanEvents.size(); ++i)
	{
		const BattlePlanEvent &ev = s_state.battlePlanEvents[i];
		if (i > 0) gzputc(f, ',');
		gzprintf(f, "{\"frame\":%u,\"player\":%d,\"bombardment\":%d,\"holdTheLine\":%d,\"searchAndDestroy\":%d}",
			ev.frame, remapPlayerIndex(ev.playerIndex), ev.bombardment, ev.holdTheLine, ev.searchAndDestroy);
	}
	gzputc(f, ']');
}

static void writeTimeSeriesJson(gzFile f)
{
	// VC6 leaks for-loop variables into the enclosing scope, so declare
	// the loop counter once and reuse it across the three snapshot loops.
	size_t s;
	gzputs(f, "{\"players\":[");
	for (Int pi = 0; pi < s_state.gamePlayerCount; ++pi)
	{
		if (pi > 0) gzputc(f, ',');
		gzprintf(f, "{\"index\":%d,\"money\":[", pi + 1);
		for (s = 0; s < s_state.snapshots.size(); ++s)
		{
			if (s > 0) gzputc(f, ',');
			gzprintf(f, "%u", s_state.snapshots[s].players[pi].money);
		}
		gzputs(f, "],\"moneyEarned\":[");
		for (s = 0; s < s_state.snapshots.size(); ++s)
		{
			if (s > 0) gzputc(f, ',');
			gzprintf(f, "%d", s_state.snapshots[s].players[pi].moneyEarned);
		}
		gzputs(f, "],\"moneySpent\":[");
		for (s = 0; s < s_state.snapshots.size(); ++s)
		{
			if (s > 0) gzputc(f, ',');
			gzprintf(f, "%d", s_state.snapshots[s].players[pi].moneySpent);
		}
		gzputs(f, "]}");
	}
	gzputs(f, "]}");
}

static void writePlayerJson(gzFile f, Player *player, Int gameIndex)
{
	ScoreKeeper *sk = player->getScoreKeeper();
	const PlayerTemplate *pt = player->getPlayerTemplate();
	const AcademyStats *academy = player->getAcademyStats();

	gzprintf(f, "{\"index\":%d,\"displayName\":", gameIndex);
	gzPrintJsonWideStr(f, player->getPlayerDisplayName().str());

	gzputs(f, ",\"faction\":");
	gzPrintJsonStr(f, pt != nullptr ? pt->getName().str() : "");

	gzputs(f, ",\"side\":");
	gzPrintJsonStr(f, player->getSide().str());

	gzputs(f, ",\"baseSide\":");
	gzPrintJsonStr(f, player->getBaseSide().str());

	gzputs(f, ",\"type\":");
	gzPrintJsonStr(f, player->getPlayerType() == PLAYER_HUMAN ? "Human" : "Computer");

	char colorBuf[8];
	snprintf(colorBuf, sizeof(colorBuf), "#%06X", static_cast<unsigned int>(player->getPlayerColor()) & 0x00FFFFFFu);
	colorBuf[sizeof(colorBuf) - 1] = '\0';
	gzputs(f, ",\"color\":");
	gzPrintJsonStr(f, colorBuf);

	gzprintf(f, ",\"money\":%u,\"moneyEarned\":%d,\"moneySpent\":%d,\"score\":%d",
		player->getMoney()->countMoney(),
		sk->getTotalMoneyEarned(),
		sk->getTotalMoneySpent(),
		sk->calculateScore());

	if (academy != nullptr)
	{
		gzprintf(f,
			",\"academy\":{"
			"\"supplyCentersBuilt\":%u,\"peonsBuilt\":%u,\"structuresCaptured\":%u,"
			"\"generalsPointsSpent\":%u,\"specialPowersUsed\":%u,\"structuresGarrisoned\":%u,"
			"\"upgradesPurchased\":%u,\"gatherersBuilt\":%u,\"heroesBuilt\":%u,"
			"\"controlGroupsUsed\":%u,\"secondaryIncomeUnitsBuilt\":%u,\"clearedGarrisonedBuildings\":%u,"
			"\"salvageCollected\":%u,\"guardAbilityUsedCount\":%u,\"doubleClickAttackMoveOrdersGiven\":%u,"
			"\"minesCleared\":%u,\"vehiclesDisguised\":%u,\"firestormsCreated\":%u}",
			academy->getSupplyCentersBuilt(), academy->getPeonsBuilt(), academy->getStructuresCaptured(),
			academy->getGeneralsPointsSpent(), academy->getSpecialPowersUsed(), academy->getStructuresGarrisoned(),
			academy->getUpgradesPurchased(), academy->getGatherersBuilt(), academy->getHeroesBuilt(),
			academy->getControlGroupsUsed(), academy->getSecondaryIncomeUnitsBuilt(), academy->getClearedGarrisonedBuildings(),
			academy->getSalvageCollected(), academy->getGuardAbilityUsedCount(), academy->getDoubleClickAttackMoveOrdersGiven(),
			academy->getMinesCleared(), academy->getVehiclesDisguised(), academy->getFirestormsCreated());
	}

	gzputc(f, '}');
}

//-----------------------------------------------------------------------------

void ExportGameStatsJSON(const AsciiString& replayDir, const AsciiString& replayFileName)
{
	// No-op if collection wasn't started for this game (e.g. non-host LAN
	// peers, replay viewing). Lets callers like Recorder::stopRecording
	// invoke this unconditionally.
	if (!s_state.exportingActive)
		return;

	if (ThePlayerList == nullptr || TheGameLogic == nullptr || TheGlobalData == nullptr)
	{
		s_state.resetData();
		s_state.exportingActive = FALSE;
		return;
	}

	// Strip any directory components from the replay filename
	const char *replayBase = replayFileName.str();
	const char *lastSlash = strrchr(replayBase, '/');
	const char *lastBackslash = strrchr(replayBase, '\\');
	if (lastBackslash != nullptr && (lastSlash == nullptr || lastBackslash > lastSlash))
		lastSlash = lastBackslash;
	if (lastSlash != nullptr)
		replayBase = lastSlash + 1;

	// Build stats file path: replace .rep extension with .gamestats.json.gz
	char baseName[_MAX_PATH + 1];
	strlcpy(baseName, replayBase, ARRAY_SIZE(baseName));
	char *dot = strrchr(baseName, '.');
	if (dot != nullptr) *dot = '\0';

	AsciiString statsPath;
	statsPath.format("%s%s.gamestats.json.gz", replayDir.str(), baseName);

	initPlayerMapping();

	const Int playerCount = ThePlayerList->getPlayerCount();

	gzFile f = gzopen(statsPath.str(), "wb9");
	if (f == nullptr)
	{
		printf("[stats] ERROR: Failed to open %s for writing\n", statsPath.str());
		fflush(stdout);
		s_state.resetData();
		s_state.exportingActive = FALSE;
		return;
	}

	gzputs(f, "{\n\"version\":1,\n\"game\":{");
	gzputs(f, "\"map\":");  gzPrintJsonStr(f, TheGlobalData->m_mapName.str());
	gzputs(f, ",\"mode\":"); gzPrintJsonStr(f, gameModeToString(TheGameLogic->getGameMode()));
	gzprintf(f, ",\"frameCount\":%u,\"seed\":%u,",
		TheGameLogic->getFrame(), GetGameLogicRandomSeed());
	gzputs(f, "\"replayFile\":"); gzPrintJsonStr(f, replayFileName.str());
	gzprintf(f, ",\"playerCount\":%d,\"snapshotInterval\":30},\n",
		s_state.gamePlayerCount);

	// Players array
	gzputs(f, "\"players\":[");
	{
		Bool first = TRUE;
		for (Int i = 0; i < playerCount; ++i)
		{
			Player *player = ThePlayerList->getNthPlayer(i);
			if (player == nullptr || !isGamePlayer(player))
				continue;
			if (!first)
				gzputc(f, ',');
			first = FALSE;
			writePlayerJson(f, player, s_state.originalToNewIndex[i]);
		}
	}
	gzputs(f, "],\n");

	gzputs(f, "\"buildEvents\":"); writeBuildEventsJson(f);
	gzputs(f, ",\n\"killEvents\":"); writeKillEventsJson(f);
	gzputs(f, ",\n\"captureEvents\":"); writeCaptureEventsJson(f);
	writeStateChangeEventsJson(f);
	gzputs(f, ",\n\"timeSeries\":"); writeTimeSeriesJson(f);

	gzputs(f, "\n}\n");

	gzclose(f);

	printf("[stats] Wrote gzipped JSON to %s\n", statsPath.str());
	fflush(stdout);

	// Upload gzipped file to server if URL configured. Skip uploads for
	// games with fewer than two human players: solo skirmishes / campaign
	// missions / replay watches don't represent humans-vs-humans data and
	// would just pollute the cncstats dataset.
	if (!TheGlobalData->m_statsUrl.isEmpty() && StatsExporterHasMinHumansForUpload())
	{
		FILE *rf = fopen(statsPath.str(), "rb");
		if (rf != nullptr)
		{
			fseek(rf, 0, SEEK_END);
			long size = ftell(rf);
			fseek(rf, 0, SEEK_SET);
			if (size > 0)
			{
				void *fileData = malloc(static_cast<size_t>(size));
				if (fileData != nullptr)
				{
					if (fread(fileData, 1, static_cast<size_t>(size), rf) == static_cast<size_t>(size))
					{
						printf("[stats] Uploading %ld bytes to %s\n", size, TheGlobalData->m_statsUrl.str());
						fflush(stdout);
						UploadStatsToServer(TheGlobalData->m_statsUrl, fileData, static_cast<unsigned int>(size), GetGameLogicRandomSeed());
					}
					free(fileData);
				}
			}
			fclose(rf);
		}
		else
		{
			printf("[stats] ERROR: Failed to read %s for upload\n", statsPath.str());
			fflush(stdout);
		}
	}

	s_state.resetData();
	s_state.exportingActive = FALSE;
}
