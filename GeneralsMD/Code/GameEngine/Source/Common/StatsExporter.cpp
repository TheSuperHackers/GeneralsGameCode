/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

// TheSuperHackers @feature bill-rich 10/03/2026 Game stats JSON exporter.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/StatsExporter.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/PlayerTemplate.h"
#include "Common/GlobalData.h"
#include "Common/Energy.h"
#include "Common/ThingTemplate.h"
#include "Common/RandomValue.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/BattlePlanUpdate.h"

#include <stdio.h>

//-----------------------------------------------------------------------------

static void fprintJsonString(FILE *f, const char *s)
{
	fputc('"', f);
	if (s != nullptr)
	{
		for (; *s != '\0'; ++s)
		{
			switch (*s)
			{
				case '"':  fputs("\\\"", f); break;
				case '\\': fputs("\\\\", f); break;
				case '\n': fputs("\\n", f); break;
				case '\r': fputs("\\r", f); break;
				case '\t': fputs("\\t", f); break;
				default:
					if (static_cast<unsigned char>(*s) < 0x20)
						fprintf(f, "\\u%04x", static_cast<unsigned int>(static_cast<unsigned char>(*s)));
					else
						fputc(*s, f);
					break;
			}
		}
	}
	fputc('"', f);
}

//-----------------------------------------------------------------------------

static void fprintJsonWideString(FILE *f, const WideChar *s)
{
	fputc('"', f);
	if (s != nullptr)
	{
		for (; *s != L'\0'; ++s)
		{
			unsigned int c = static_cast<unsigned int>(*s);
			if (c == '"')
				fputs("\\\"", f);
			else if (c == '\\')
				fputs("\\\\", f);
			else if (c < 0x20)
				fprintf(f, "\\u%04x", c);
			else if (c < 0x80)
				fputc(static_cast<char>(c), f);
			else
				fprintf(f, "\\u%04x", c);
		}
	}
	fputc('"', f);
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

static void writeObjectCountMap(FILE *f, const ScoreKeeper::ObjectCountMap &map, const char *indent)
{
	fprintf(f, "{\n");
	Bool first = TRUE;
	for (ScoreKeeper::ObjectCountMap::const_iterator it = map.begin(); it != map.end(); ++it)
	{
		if (!first) fprintf(f, ",\n");
		first = FALSE;
		const ThingTemplate *tmpl = it->first;
		fprintf(f, "%s  ", indent);
		if (tmpl != nullptr)
			fprintJsonString(f, tmpl->getName().str());
		else
			fprintJsonString(f, "unknown");
		fprintf(f, ": %d", it->second);
	}
	if (!map.empty()) fprintf(f, "\n%s", indent);
	fprintf(f, "}");
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

//-----------------------------------------------------------------------------

struct PlayerSnapshotData
{
	Int playerIndex;
	UnsignedInt money;
	Int moneyEarned;
	Int moneySpent;
	Int energyProduction;
	Int energyConsumption;
	Int unitsBuilt;
	Int unitsLost;
	Int buildingsBuilt;
	Int buildingsLost;
	Int techBuildingsCaptured;
	Int factionBuildingsCaptured;
	Int rankLevel;
	Int skillPoints;
	Int sciencePurchasePoints;
	Int score;
	Int unitsKilled[MAX_PLAYER_COUNT];
	Int buildingsKilled[MAX_PLAYER_COUNT];
};

struct FrameSnapshotData
{
	UnsignedInt frame;
	Int playerCount;
	PlayerSnapshotData players[MAX_PLAYER_COUNT];
};

static std::vector<FrameSnapshotData> s_snapshots;
static UnsignedInt s_lastSnapshotFrame = 0;
static Int s_gamePlayerCount = 0;
static Int s_originalToNewIndex[MAX_PLAYER_COUNT];
static Bool s_mappingInitialized = FALSE;

//-----------------------------------------------------------------------------

static void initPlayerMapping()
{
	if (s_mappingInitialized)
		return;

	s_gamePlayerCount = 0;
	memset(s_originalToNewIndex, 0, sizeof(s_originalToNewIndex));

	const Int totalPlayers = ThePlayerList->getPlayerCount();
	Int i;
	for (i = 0; i < totalPlayers && i < MAX_PLAYER_COUNT; ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (isGamePlayer(player))
		{
			++s_gamePlayerCount;
			s_originalToNewIndex[i] = s_gamePlayerCount;
		}
	}
	s_mappingInitialized = TRUE;
}

//-----------------------------------------------------------------------------

void StatsExporterCollectSnapshot()
{
	if (ThePlayerList == nullptr || TheGameLogic == nullptr)
		return;

	UnsignedInt currentFrame = TheGameLogic->getFrame();
	if (!s_snapshots.empty() && (currentFrame - s_lastSnapshotFrame) < 30)
		return;

	s_lastSnapshotFrame = currentFrame;

	initPlayerMapping();

	const Int totalPlayers = ThePlayerList->getPlayerCount();

	FrameSnapshotData snap;
	memset(&snap, 0, sizeof(snap));
	snap.frame = currentFrame;
	snap.playerCount = s_gamePlayerCount;

	Int gameIdx = 0;
	Int i;
	for (i = 0; i < totalPlayers && i < MAX_PLAYER_COUNT; ++i)
	{
		if (s_originalToNewIndex[i] == 0)
			continue;

		Player *player = ThePlayerList->getNthPlayer(i);
		if (player == nullptr)
			continue;

		PlayerSnapshotData &pd = snap.players[gameIdx];
		ScoreKeeper *sk = player->getScoreKeeper();
		const Energy *energy = player->getEnergy();

		pd.playerIndex = s_originalToNewIndex[i];
		pd.money = player->getMoney()->countMoney();
		pd.moneyEarned = sk->getTotalMoneyEarned();
		pd.moneySpent = sk->getTotalMoneySpent();
		pd.energyProduction = energy->getProduction();
		pd.energyConsumption = energy->getConsumption();
		pd.unitsBuilt = sk->getTotalUnitsBuilt();
		pd.unitsLost = sk->getTotalUnitsLost();
		pd.buildingsBuilt = sk->getTotalBuildingsBuilt();
		pd.buildingsLost = sk->getTotalBuildingsLost();
		pd.techBuildingsCaptured = sk->getTotalTechBuildingsCaptured();
		pd.factionBuildingsCaptured = sk->getTotalFactionBuildingsCaptured();
		pd.rankLevel = player->getRankLevel();
		pd.skillPoints = player->getSkillPoints();
		pd.sciencePurchasePoints = player->getSciencePurchasePoints();
		pd.score = sk->calculateScore();

		Int j;
		for (j = 0; j < MAX_PLAYER_COUNT; ++j)
		{
			pd.unitsKilled[j] = sk->getUnitsDestroyedByPlayer(j);
			pd.buildingsKilled[j] = sk->getBuildingsDestroyedByPlayer(j);
		}

		++gameIdx;
	}

	s_snapshots.push_back(snap);
}

//-----------------------------------------------------------------------------

void StatsExporterClearSnapshots()
{
	s_snapshots.clear();
	s_lastSnapshotFrame = 0;
	s_gamePlayerCount = 0;
	s_mappingInitialized = FALSE;
	memset(s_originalToNewIndex, 0, sizeof(s_originalToNewIndex));
}

//-----------------------------------------------------------------------------

static void writeTimeSeries(FILE *f)
{
	fprintf(f, "  \"timeSeries\": {\n");

	// Frames array
	fprintf(f, "    \"frames\": [");
	size_t s;
	for (s = 0; s < s_snapshots.size(); ++s)
	{
		if (s > 0) fputc(',', f);
		fprintf(f, "%u", s_snapshots[s].frame);
	}
	fprintf(f, "],\n");

	// Players array
	fprintf(f, "    \"players\": [\n");

	Int pi;
	for (pi = 0; pi < s_gamePlayerCount; ++pi)
	{
		if (pi > 0) fprintf(f, ",\n");
		fprintf(f, "      {\n");

		fprintf(f, "        \"index\": %d,\n", pi + 1);

		// money (UnsignedInt)
		fprintf(f, "        \"money\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%u", s_snapshots[s].players[pi].money);
		}
		fprintf(f, "],\n");

		// moneyEarned
		fprintf(f, "        \"moneyEarned\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].moneyEarned);
		}
		fprintf(f, "],\n");

		// moneySpent
		fprintf(f, "        \"moneySpent\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].moneySpent);
		}
		fprintf(f, "],\n");

		// energyProduction
		fprintf(f, "        \"energyProduction\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].energyProduction);
		}
		fprintf(f, "],\n");

		// energyConsumption
		fprintf(f, "        \"energyConsumption\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].energyConsumption);
		}
		fprintf(f, "],\n");

		// unitsBuilt
		fprintf(f, "        \"unitsBuilt\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].unitsBuilt);
		}
		fprintf(f, "],\n");

		// unitsLost
		fprintf(f, "        \"unitsLost\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].unitsLost);
		}
		fprintf(f, "],\n");

		// buildingsBuilt
		fprintf(f, "        \"buildingsBuilt\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].buildingsBuilt);
		}
		fprintf(f, "],\n");

		// buildingsLost
		fprintf(f, "        \"buildingsLost\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].buildingsLost);
		}
		fprintf(f, "],\n");

		// techBuildingsCaptured
		fprintf(f, "        \"techBuildingsCaptured\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].techBuildingsCaptured);
		}
		fprintf(f, "],\n");

		// factionBuildingsCaptured
		fprintf(f, "        \"factionBuildingsCaptured\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].factionBuildingsCaptured);
		}
		fprintf(f, "],\n");

		// rankLevel
		fprintf(f, "        \"rankLevel\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].rankLevel);
		}
		fprintf(f, "],\n");

		// skillPoints
		fprintf(f, "        \"skillPoints\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].skillPoints);
		}
		fprintf(f, "],\n");

		// sciencePurchasePoints
		fprintf(f, "        \"sciencePurchasePoints\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].sciencePurchasePoints);
		}
		fprintf(f, "],\n");

		// score
		fprintf(f, "        \"score\": [");
		for (s = 0; s < s_snapshots.size(); ++s)
		{
			if (s > 0) fputc(',', f);
			fprintf(f, "%d", s_snapshots[s].players[pi].score);
		}
		fprintf(f, "],\n");

		// unitsKilled - sparse per-opponent
		fprintf(f, "        \"unitsKilled\": {");
		{
			Bool firstOpp = TRUE;
			Int j;
			for (j = 0; j < MAX_PLAYER_COUNT; ++j)
			{
				Int remapped = s_originalToNewIndex[j];
				if (remapped == 0) continue;

				// Check if any snapshot has non-zero value
				Bool hasNonZero = FALSE;
				for (s = 0; s < s_snapshots.size(); ++s)
				{
					if (s_snapshots[s].players[pi].unitsKilled[j] != 0)
					{
						hasNonZero = TRUE;
						break;
					}
				}
				if (!hasNonZero) continue;

				if (!firstOpp) fprintf(f, ",");
				firstOpp = FALSE;
				fprintf(f, "\n          \"%d\": [", remapped);
				for (s = 0; s < s_snapshots.size(); ++s)
				{
					if (s > 0) fputc(',', f);
					fprintf(f, "%d", s_snapshots[s].players[pi].unitsKilled[j]);
				}
				fprintf(f, "]");
			}
			if (!firstOpp) fprintf(f, "\n        ");
		}
		fprintf(f, "},\n");

		// buildingsKilled - sparse per-opponent
		fprintf(f, "        \"buildingsKilled\": {");
		{
			Bool firstOpp = TRUE;
			Int j;
			for (j = 0; j < MAX_PLAYER_COUNT; ++j)
			{
				Int remapped = s_originalToNewIndex[j];
				if (remapped == 0) continue;

				Bool hasNonZero = FALSE;
				for (s = 0; s < s_snapshots.size(); ++s)
				{
					if (s_snapshots[s].players[pi].buildingsKilled[j] != 0)
					{
						hasNonZero = TRUE;
						break;
					}
				}
				if (!hasNonZero) continue;

				if (!firstOpp) fprintf(f, ",");
				firstOpp = FALSE;
				fprintf(f, "\n          \"%d\": [", remapped);
				for (s = 0; s < s_snapshots.size(); ++s)
				{
					if (s > 0) fputc(',', f);
					fprintf(f, "%d", s_snapshots[s].players[pi].buildingsKilled[j]);
				}
				fprintf(f, "]");
			}
			if (!firstOpp) fprintf(f, "\n        ");
		}
		fprintf(f, "}\n");

		fprintf(f, "      }");
	}

	fprintf(f, "\n    ]\n");
	fprintf(f, "  }\n");
}

//-----------------------------------------------------------------------------

void ExportGameStatsJSON(const AsciiString& replayDir, const AsciiString& replayFileName)
{
	if (ThePlayerList == nullptr || TheGameLogic == nullptr || TheGlobalData == nullptr)
		return;

	// Build stats file path: replace .rep extension with .gamestats.json
	char baseName[_MAX_PATH + 1];
	strlcpy(baseName, replayFileName.str(), ARRAY_SIZE(baseName));
	char *dot = strrchr(baseName, '.');
	if (dot != nullptr) *dot = '\0';

	AsciiString statsPath;
	statsPath.format("%s%s.gamestats.json", replayDir.str(), baseName);

	FILE *f = fopen(statsPath.str(), "w");
	if (f == nullptr)
		return;

	initPlayerMapping();

	const Int playerCount = ThePlayerList->getPlayerCount();

	fprintf(f, "{\n");
	fprintf(f, "  \"version\": 3,\n");

	// Game info
	fprintf(f, "  \"game\": {\n");
	fprintf(f, "    \"map\": "); fprintJsonString(f, TheGlobalData->m_mapName.str()); fprintf(f, ",\n");
	fprintf(f, "    \"mode\": \"%s\",\n", gameModeToString(TheGameLogic->getGameMode()));
	fprintf(f, "    \"frameCount\": %u,\n", TheGameLogic->getFrame());
	fprintf(f, "    \"seed\": %u,\n", GetGameLogicRandomSeed());
	fprintf(f, "    \"replayFile\": "); fprintJsonString(f, replayFileName.str()); fprintf(f, ",\n");
	fprintf(f, "    \"playerCount\": %d\n", s_gamePlayerCount);
	fprintf(f, "  },\n");

	// Players array
	fprintf(f, "  \"players\": [\n");
	Bool firstPlayer = TRUE;
	Int i;
	for (i = 0; i < playerCount; ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (player == nullptr || !isGamePlayer(player))
			continue;

		if (!firstPlayer) fprintf(f, ",\n");
		firstPlayer = FALSE;

		ScoreKeeper *sk = player->getScoreKeeper();
		const Energy *energy = player->getEnergy();
		const PlayerTemplate *pt = player->getPlayerTemplate();
		const AcademyStats *academy = player->getAcademyStats();

		fprintf(f, "    {\n");

		// Basic info
		fprintf(f, "      \"index\": %d,\n", s_originalToNewIndex[i]);
		fprintf(f, "      \"displayName\": "); fprintJsonWideString(f, player->getPlayerDisplayName().str()); fprintf(f, ",\n");
		if (pt != nullptr)
		{
			fprintf(f, "      \"faction\": "); fprintJsonString(f, pt->getName().str()); fprintf(f, ",\n");
		}
		fprintf(f, "      \"side\": "); fprintJsonString(f, player->getSide().str()); fprintf(f, ",\n");
		fprintf(f, "      \"baseSide\": "); fprintJsonString(f, player->getBaseSide().str()); fprintf(f, ",\n");
		fprintf(f, "      \"type\": \"%s\",\n", player->getPlayerType() == PLAYER_HUMAN ? "Human" : "Computer");
		fprintf(f, "      \"color\": \"#%06X\",\n", static_cast<unsigned int>(player->getPlayerColor()) & 0x00FFFFFFu);
		fprintf(f, "      \"isDead\": %s,\n", player->isPlayerDead() ? "true" : "false");

		// Economy
		fprintf(f, "      \"money\": %u,\n", player->getMoney()->countMoney());
		fprintf(f, "      \"moneyEarned\": %d,\n", sk->getTotalMoneyEarned());
		fprintf(f, "      \"moneySpent\": %d,\n", sk->getTotalMoneySpent());

		// Energy
		fprintf(f, "      \"energyProduction\": %d,\n", energy->getProduction());
		fprintf(f, "      \"energyConsumption\": %d,\n", energy->getConsumption());

		// Rank
		fprintf(f, "      \"rankLevel\": %d,\n", player->getRankLevel());
		fprintf(f, "      \"skillPoints\": %d,\n", player->getSkillPoints());
		fprintf(f, "      \"sciencePurchasePoints\": %d,\n", player->getSciencePurchasePoints());

		// Units/Buildings summary
		fprintf(f, "      \"unitsBuilt\": %d,\n", sk->getTotalUnitsBuilt());
		fprintf(f, "      \"unitsLost\": %d,\n", sk->getTotalUnitsLost());
		fprintf(f, "      \"buildingsBuilt\": %d,\n", sk->getTotalBuildingsBuilt());
		fprintf(f, "      \"buildingsLost\": %d,\n", sk->getTotalBuildingsLost());
		fprintf(f, "      \"techBuildingsCaptured\": %d,\n", sk->getTotalTechBuildingsCaptured());
		fprintf(f, "      \"factionBuildingsCaptured\": %d,\n", sk->getTotalFactionBuildingsCaptured());

		// Radar & Battle plans
		fprintf(f, "      \"hasRadar\": %s,\n", player->hasRadar() ? "true" : "false");
		fprintf(f, "      \"battlePlans\": {\n");
		fprintf(f, "        \"bombardment\": %d,\n", player->getBattlePlansActiveSpecific(PLANSTATUS_BOMBARDMENT));
		fprintf(f, "        \"holdTheLine\": %d,\n", player->getBattlePlansActiveSpecific(PLANSTATUS_HOLDTHELINE));
		fprintf(f, "        \"searchAndDestroy\": %d\n", player->getBattlePlansActiveSpecific(PLANSTATUS_SEARCHANDDESTROY));
		fprintf(f, "      },\n");

		// Score
		fprintf(f, "      \"score\": %d,\n", sk->calculateScore());

		// Per-player destroy counts (sparse objects with remapped keys)
		Int j;
		fprintf(f, "      \"unitsDestroyedPerPlayer\": {");
		{
			Bool firstKill = TRUE;
			for (j = 0; j < MAX_PLAYER_COUNT; ++j)
			{
				if (s_originalToNewIndex[j] == 0) continue;
				Int count = sk->getUnitsDestroyedByPlayer(j);
				if (count == 0) continue;
				if (!firstKill) fprintf(f, ",");
				firstKill = FALSE;
				fprintf(f, " \"%d\": %d", s_originalToNewIndex[j], count);
			}
			if (!firstKill) fprintf(f, " ");
		}
		fprintf(f, "},\n");

		fprintf(f, "      \"buildingsDestroyedPerPlayer\": {");
		{
			Bool firstKill = TRUE;
			for (j = 0; j < MAX_PLAYER_COUNT; ++j)
			{
				if (s_originalToNewIndex[j] == 0) continue;
				Int count = sk->getBuildingsDestroyedByPlayer(j);
				if (count == 0) continue;
				if (!firstKill) fprintf(f, ",");
				firstKill = FALSE;
				fprintf(f, " \"%d\": %d", s_originalToNewIndex[j], count);
			}
			if (!firstKill) fprintf(f, " ");
		}
		fprintf(f, "},\n");

		// Per-object-type maps
		fprintf(f, "      \"objectsBuilt\": "); writeObjectCountMap(f, sk->getObjectsBuilt(), "      "); fprintf(f, ",\n");
		fprintf(f, "      \"objectsLost\": "); writeObjectCountMap(f, sk->getObjectsLost(), "      "); fprintf(f, ",\n");
		fprintf(f, "      \"objectsCaptured\": "); writeObjectCountMap(f, sk->getObjectsCaptured(), "      "); fprintf(f, ",\n");

		// Per-player per-object-type destroyed (sparse object with remapped keys)
		fprintf(f, "      \"objectsDestroyedPerPlayer\": {");
		{
			const ScoreKeeper::ObjectCountMap *destroyedArr = sk->getObjectsDestroyedArray();
			Bool firstOpp = TRUE;
			for (j = 0; j < MAX_PLAYER_COUNT; ++j)
			{
				if (s_originalToNewIndex[j] == 0) continue;
				if (destroyedArr[j].empty()) continue;
				if (!firstOpp) fprintf(f, ",");
				firstOpp = FALSE;
				fprintf(f, "\n        \"%d\": ", s_originalToNewIndex[j]);
				writeObjectCountMap(f, destroyedArr[j], "        ");
			}
			if (!firstOpp) fprintf(f, "\n      ");
		}
		fprintf(f, "},\n");

		// AcademyStats (Zero Hour only)
		fprintf(f, "      \"academy\": {\n");
		fprintf(f, "        \"supplyCentersBuilt\": %u,\n", academy->getSupplyCentersBuilt());
		fprintf(f, "        \"peonsBuilt\": %u,\n", academy->getPeonsBuilt());
		fprintf(f, "        \"structuresCaptured\": %u,\n", academy->getStructuresCaptured());
		fprintf(f, "        \"generalsPointsSpent\": %u,\n", academy->getGeneralsPointsSpent());
		fprintf(f, "        \"specialPowersUsed\": %u,\n", academy->getSpecialPowersUsed());
		fprintf(f, "        \"structuresGarrisoned\": %u,\n", academy->getStructuresGarrisoned());
		fprintf(f, "        \"upgradesPurchased\": %u,\n", academy->getUpgradesPurchased());
		fprintf(f, "        \"gatherersBuilt\": %u,\n", academy->getGatherersBuilt());
		fprintf(f, "        \"heroesBuilt\": %u,\n", academy->getHeroesBuilt());
		fprintf(f, "        \"controlGroupsUsed\": %u,\n", academy->getControlGroupsUsed());
		fprintf(f, "        \"secondaryIncomeUnitsBuilt\": %u,\n", academy->getSecondaryIncomeUnitsBuilt());
		fprintf(f, "        \"clearedGarrisonedBuildings\": %u,\n", academy->getClearedGarrisonedBuildings());
		fprintf(f, "        \"salvageCollected\": %u,\n", academy->getSalvageCollected());
		fprintf(f, "        \"guardAbilityUsedCount\": %u,\n", academy->getGuardAbilityUsedCount());
		fprintf(f, "        \"doubleClickAttackMoveOrdersGiven\": %u,\n", academy->getDoubleClickAttackMoveOrdersGiven());
		fprintf(f, "        \"minesCleared\": %u,\n", academy->getMinesCleared());
		fprintf(f, "        \"vehiclesDisguised\": %u,\n", academy->getVehiclesDisguised());
		fprintf(f, "        \"firestormsCreated\": %u\n", academy->getFirestormsCreated());
		fprintf(f, "      }\n");

		fprintf(f, "    }");
	}
	fprintf(f, "\n  ],\n");

	writeTimeSeries(f);

	fprintf(f, "}\n");

	fclose(f);

	StatsExporterClearSnapshots();
}
