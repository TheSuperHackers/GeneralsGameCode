#include "PreRTS.h"

#include "GameNetwork/RandomAssign.h"
#include "GameNetwork/GameInfo.h"
#include "Common/PlayerTemplate.h"
#include "Common/MultiplayerSettings.h"
#include "Common/GlobalData.h"
#include "Common/StatsUploader.h"
#include "GameClient/MapUtil.h"
#include "GameClient/ChallengeGenerals.h"

#include <stdlib.h>
#include <math.h>
#include <vector>

// Build the list of valid template indices for random assignment.
// Same filtering as populateRandomSideAndColor (GameLogic.cpp).
static void buildValidTemplates(const GameInfo *game, const std::vector<Int> &lockedTemplates, std::vector<Int> &out)
{
	Int count = ThePlayerTemplateStore->getPlayerTemplateCount();
	for (Int c = 0; c < count; ++c)
	{
		const PlayerTemplate *fac = ThePlayerTemplateStore->getNthPlayerTemplate(c);
		if (!fac)
			continue;

		// Must have a starting building (filters out civilian etc)
		if (fac->getStartingBuilding().isEmpty())
			continue;

		// Respect old factions only mode
		if (game->oldFactionsOnly() && !fac->isOldFaction())
			continue;

		// Skip locked generals (list provided by caller from client code)
		Bool isLocked = FALSE;
		for (size_t k = 0; k < lockedTemplates.size(); ++k)
		{
			if (lockedTemplates[k] == c)
			{
				isLocked = TRUE;
				break;
			}
		}
		if (isLocked)
			continue;

		out.push_back(c);
	}
}

// Phase 1: Assign random factions and colors.
// Mirrors populateRandomSideAndColor (GameLogic.cpp:691-781).
// Only touches slots that have PLAYERTEMPLATE_RANDOM / color == -1.
static void assignRandomFactions(GameInfo *game, const std::vector<Int> &validTemplates)
{
	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		GameSlot *slot = game->getSlot(i);
		if (!slot || !slot->isOccupied())
			continue;

		// Assign faction if random
		Int playerTemplateIdx = slot->getPlayerTemplate();
		if (playerTemplateIdx == PLAYERTEMPLATE_RANDOM && !validTemplates.empty())
		{
			playerTemplateIdx = validTemplates[rand() % validTemplates.size()];
			slot->setPlayerTemplate(playerTemplateIdx);
		}

		// Assign color if random (-1)
		Int colorIdx = slot->getColor();
		if (colorIdx < 0 || colorIdx >= TheMultiplayerSettings->getNumColors())
		{
			Int numColors = TheMultiplayerSettings->getNumColors();
			if (numColors > 0)
			{
				colorIdx = -1;
				for (Int attempt = 0; attempt < numColors * 2 && colorIdx == -1; ++attempt)
				{
					Int candidate = rand() % numColors;
					if (!game->isColorTaken(candidate))
						colorIdx = candidate;
				}
				if (colorIdx >= 0)
					slot->setColor(colorIdx);
			}
		}
	}
}

// Phase 2: Assign start positions using distance-based placement.
// Mirrors populateRandomStartPosition (GameLogic.cpp:787-1053).
// Different teams are placed far apart, teammates close together.
// Only touches slots that have startPos == -1.
static void assignRandomPositions(GameInfo *game)
{
	Int i;
	Int numPlayers = MAX_SLOTS;
	const MapMetaData *md = TheMapCache ? TheMapCache->findMap(game->getMap()) : NULL;
	if (md)
		numPlayers = md->m_numPlayers;

	if (numPlayers <= 0)
		return;

	// Build distance matrix between all start positions using map waypoints
	static const WaypointMap s_emptyWaypoints;
	const WaypointMap &waypoints = md ? md->m_waypoints : s_emptyWaypoints;
	Real startSpotDistance[MAX_SLOTS][MAX_SLOTS];
	for (i = 0; i < MAX_SLOTS; ++i)
	{
		for (Int j = 0; j < MAX_SLOTS; ++j)
		{
			if (i != j && i < numPlayers && j < numPlayers)
			{
				AsciiString w1, w2;
				w1.format("Player_%d_Start", i + 1);
				w2.format("Player_%d_Start", j + 1);
				WaypointMap::const_iterator c1 = waypoints.find(w1);
				WaypointMap::const_iterator c2 = waypoints.find(w2);
				if (c1 == waypoints.end() || c2 == waypoints.end())
				{
					startSpotDistance[i][j] = 1000000.0f;
				}
				else
				{
					Coord3D p1 = c1->second;
					Coord3D p2 = c2->second;
					startSpotDistance[i][j] = sqrt(sqr(p1.x - p2.x) + sqr(p1.y - p2.y));
				}
			}
			else
			{
				startSpotDistance[i][j] = 0.0f;
			}
		}
	}

	// Track which positions are already taken (deliberately assigned)
	Bool taken[MAX_SLOTS];
	for (i = 0; i < MAX_SLOTS; ++i)
		taken[i] = (i < numPlayers) ? FALSE : TRUE;

	Bool hasStartSpotBeenPicked = FALSE;
	for (i = 0; i < MAX_SLOTS; ++i)
	{
		GameSlot *slot = game->getSlot(i);
		if (!slot || !slot->isOccupied() || slot->getPlayerTemplate() == PLAYERTEMPLATE_OBSERVER)
			continue;

		Int posIdx = slot->getStartPos();
		if (posIdx >= 0 && posIdx < numPlayers)
		{
			hasStartSpotBeenPicked = TRUE;
			taken[posIdx] = TRUE;
		}
	}

	// Track first position per team for teammate clustering
	Int teamPosIdx[MAX_SLOTS];
	for (i = 0; i < MAX_SLOTS; ++i)
		teamPosIdx[i] = -1;

	// Seed teamPosIdx from already-assigned slots
	for (i = 0; i < MAX_SLOTS; ++i)
	{
		const GameSlot *slot = game->getConstSlot(i);
		if (!slot || !slot->isOccupied() || slot->getPlayerTemplate() == PLAYERTEMPLATE_OBSERVER)
			continue;
		Int posIdx = slot->getStartPos();
		if (posIdx >= 0 && posIdx < numPlayers)
		{
			Int team = slot->getTeamNumber();
			if (team >= 0 && teamPosIdx[team] == -1)
				teamPosIdx[team] = posIdx;
		}
	}

	// Assign positions for non-observer slots that don't have one yet
	for (i = 0; i < MAX_SLOTS; ++i)
	{
		GameSlot *slot = game->getSlot(i);
		if (!slot || !slot->isOccupied() || slot->getPlayerTemplate() == PLAYERTEMPLATE_OBSERVER)
			continue;

		Int posIdx = slot->getStartPos();
		if (posIdx >= 0 && posIdx < numPlayers)
			continue; // already assigned

		Int team = slot->getTeamNumber();

		if (!hasStartSpotBeenPicked)
		{
			// First player: pick randomly
			posIdx = -1;
			for (Int attempt = 0; attempt < numPlayers * 2 && posIdx == -1; ++attempt)
			{
				Int candidate = rand() % numPlayers;
				if (!taken[candidate])
					posIdx = candidate;
			}
			if (posIdx < 0)
				continue;
			hasStartSpotBeenPicked = TRUE;
			slot->setStartPos(posIdx);
			taken[posIdx] = TRUE;
			if (team >= 0)
				teamPosIdx[team] = posIdx;
		}
		else if (team < 0 || teamPosIdx[team] == -1)
		{
			// New team or no team: pick position farthest from all taken positions
			Real farthestDistance = 0.0f;
			Int farthestIndex = -1;
			for (posIdx = 0; posIdx < numPlayers; ++posIdx)
			{
				if (taken[posIdx])
					continue;

				Real dist = 0.0f;
				for (Int n = 0; n < numPlayers; ++n)
				{
					if (taken[n] && n != posIdx)
						dist += startSpotDistance[posIdx][n];
				}
				if (farthestIndex < 0 || dist > farthestDistance)
				{
					farthestDistance = dist;
					farthestIndex = posIdx;
				}
			}

			if (farthestIndex >= 0)
			{
				slot->setStartPos(farthestIndex);
				taken[farthestIndex] = TRUE;
				if (team >= 0)
					teamPosIdx[team] = farthestIndex;
			}
		}
		else
		{
			// Teammate: pick position closest to team's existing position
			Real closestDist = FLT_MAX;
			Int closestIdx = -1;
			for (Int n = 0; n < numPlayers; ++n)
			{
				if (!taken[n] && startSpotDistance[teamPosIdx[team]][n] < closestDist)
				{
					closestDist = startSpotDistance[teamPosIdx[team]][n];
					closestIdx = n;
				}
			}
			if (closestIdx >= 0)
			{
				slot->setStartPos(closestIdx);
				taken[closestIdx] = TRUE;
			}
		}
	}

	// Assign observer slots to an existing player's position
	Int numPlayersInGame = 0;
	for (i = 0; i < MAX_SLOTS; ++i)
	{
		const GameSlot *slot = game->getConstSlot(i);
		if (slot->isOccupied() && slot->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
			++numPlayersInGame;
	}
	for (i = 0; i < MAX_SLOTS; ++i)
	{
		GameSlot *slot = game->getSlot(i);
		if (!slot || !slot->isOccupied() || slot->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
			continue;

		Int posIdx = -1;
		if (numPlayersInGame == 0)
		{
			posIdx = 0;
		}
		else
		{
			// Pick a random position that IS taken by a real player
			for (Int attempt = 0; attempt < numPlayers * 2 && posIdx == -1; ++attempt)
			{
				Int candidate = rand() % numPlayers;
				if (game->isStartPositionTaken(candidate))
					posIdx = candidate;
			}
		}
		if (posIdx >= 0)
			slot->setStartPos(posIdx);
	}
}

void performRandomAssign(GameInfo *game, const std::vector<Int> &lockedTemplates)
{
	if (!game)
		return;

	srand(timeGetTime());

	// Phase 1: Assign factions and colors for random slots
	std::vector<Int> validTemplates;
	buildValidTemplates(game, lockedTemplates, validTemplates);
	assignRandomFactions(game, validTemplates);

	// Phase 2: Assign start positions using distance-based placement
	assignRandomPositions(game);

	// Reset accepted state since we changed settings
	game->resetAccepted();
}

std::vector<Int> buildLockedTemplates()
{
	std::vector<Int> lockedTemplates;
	Int templateCount = ThePlayerTemplateStore->getPlayerTemplateCount();
	for (Int t = 0; t < templateCount; ++t)
	{
		const PlayerTemplate *fac = ThePlayerTemplateStore->getNthPlayerTemplate(t);
		if (fac)
		{
			const GeneralPersona *general = TheChallengeGenerals->getGeneralByTemplateName(fac->getName());
			if (general && !general->isStartingEnabled())
				lockedTemplates.push_back(t);
		}
	}
	return lockedTemplates;
}

// Translate a slot's display name into the form expected by the
// balance_teams API. AI slot names ("Easy Army", "Hard Army", ...) are
// joined to a single token ("EasyArmy", "HardArmy") so the server's name
// table doesn't have to care about spacing. Human names pass through.
static AsciiString slotNameForApi(const GameSlot *slot)
{
	AsciiString out;
	out.translate(slot->getName());
	if (slot->isAI())
	{
		AsciiString joined;
		for (const char *p = out.str(); *p != '\0'; ++p)
		{
			if (*p != ' ')
				joined.concat(*p);
		}
		out = joined;
	}
	return out;
}

// Hand-maintained aliases for slot-display-name <-> server-canonical-name
// pairs that prefix matching can't bridge (e.g. a numeric in-game handle vs.
// a spelled-out canonical). Either form may appear on either side of the
// comparison.
struct BalanceNameAlias { const char *a; const char *b; };
static const BalanceNameAlias g_balanceNameAliases[] = {
	{ "131", "OneThree111" },
};
static const size_t g_balanceNameAliasCount =
	sizeof(g_balanceNameAliases) / sizeof(g_balanceNameAliases[0]);

static Bool aliasMatch(const AsciiString &x, const AsciiString &y)
{
	for (size_t k = 0; k < g_balanceNameAliasCount; ++k)
	{
		const BalanceNameAlias &al = g_balanceNameAliases[k];
		if ((x.compareNoCase(al.a) == 0 && y.compareNoCase(al.b) == 0) ||
		    (x.compareNoCase(al.b) == 0 && y.compareNoCase(al.a) == 0))
		{
			return TRUE;
		}
	}
	return FALSE;
}

// Case-insensitive prefix compare of two char arrays for `n` chars.
static Bool prefixMatchNoCase(const char *a, const char *b, Int n)
{
	for (Int i = 0; i < n; ++i)
	{
		char ca = a[i];
		char cb = b[i];
		if (ca == 0 || cb == 0) return FALSE;
		if (ca >= 'A' && ca <= 'Z') ca = (char)(ca + ('a' - 'A'));
		if (cb >= 'A' && cb <= 'Z') cb = (char)(cb + ('a' - 'A'));
		if (ca != cb) return FALSE;
	}
	return TRUE;
}

// The balance-teams server canonicalizes the names it receives ("Pan" comes
// back as "Pancake", "Wild" as "WildCard"), so a slot's display name will
// often differ from the form the server returns. Treat exact-equal-no-case as
// the strongest signal, with case-insensitive prefix-match-in-either-direction
// as the fallback. The 2-character minimum guards against single-letter
// collisions.
static Bool slotMatchesCanonical(const AsciiString &slotName, const AsciiString &canonical)
{
	if (slotName.compareNoCase(canonical) == 0) return TRUE;
	if (aliasMatch(slotName, canonical)) return TRUE;
	Int slen = slotName.getLength();
	Int clen = canonical.getLength();
	if (slen < 2 || clen < 2) return FALSE;
	Int shortLen = (slen < clen) ? slen : clen;
	return prefixMatchNoCase(slotName.str(), canonical.str(), shortLen);
}

struct BalanceSlotPlan { Int slotIdx; Int team; };

// True if every occupied non-observer slot has m_teamNumber == -1.
static Bool allTeamsUnset(const GameInfo *game)
{
	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		const GameSlot *slot = game->getConstSlot(i);
		if (!slot || !slot->isOccupied())
			continue;
		if (slot->getPlayerTemplate() == PLAYERTEMPLATE_OBSERVER)
			continue;
		if (slot->getTeamNumber() != -1)
			return FALSE;
	}
	return TRUE;
}

bool tryBalanceTeamsViaApi(GameInfo *game, AsciiString *outError)
{
	if (!game)
		return true;

	if (!allTeamsUnset(game))
		return true;

	// Collect names from occupied non-observer slots in slot order.
	std::vector<AsciiString> names;
	Int i;
	for (i = 0; i < MAX_SLOTS; ++i)
	{
		const GameSlot *slot = game->getConstSlot(i);
		if (!slot || !slot->isOccupied())
			continue;
		if (slot->getPlayerTemplate() == PLAYERTEMPLATE_OBSERVER)
			continue;

		AsciiString asciiName = slotNameForApi(slot);
		if (asciiName.isEmpty())
			continue;
		names.push_back(asciiName);
	}

	if (names.size() < 2)
		return true; // nothing to balance

	BalanceTeamsResult res = BalanceTeamsFromServer(TheGlobalData->m_balanceTeamsUrl, names);
	if (!res.success)
	{
		if (outError)
			*outError = res.errorMessage;
		return false;
	}

	// Decide a target team per slot:
	//   - matches a name in res.team1                   -> team 0
	//   - matches a recognized name not in res.team1    -> team 1
	//   - matches no canonical name (server didn't recognize) -> defer
	// Deferred slots are then distributed to whichever team has fewer members,
	// so a 4-player lobby with one unrecognized name still ends up 2v2 instead
	// of dumping everything unmatched onto team 1.
	std::vector<BalanceSlotPlan> plans;
	plans.reserve(MAX_SLOTS);

	for (i = 0; i < MAX_SLOTS; ++i)
	{
		GameSlot *slot = game->getSlot(i);
		if (!slot || !slot->isOccupied())
			continue;
		if (slot->getPlayerTemplate() == PLAYERTEMPLATE_OBSERVER)
			continue;

		AsciiString asciiName = slotNameForApi(slot);

		Int team = -1;
		size_t j;
		for (j = 0; j < res.team1.size() && team == -1; ++j)
		{
			if (slotMatchesCanonical(asciiName, res.team1[j]))
				team = 0;
		}
		if (team == -1)
		{
			for (j = 0; j < res.allKnown.size() && team == -1; ++j)
			{
				Bool isInTeam1 = FALSE;
				size_t a;
				for (a = 0; a < res.team1.size(); ++a)
				{
					if (res.allKnown[j].compareNoCase(res.team1[a]) == 0)
					{
						isInTeam1 = TRUE;
						break;
					}
				}
				if (isInTeam1) continue;
				if (slotMatchesCanonical(asciiName, res.allKnown[j]))
					team = 1;
			}
		}

		BalanceSlotPlan plan;
		plan.slotIdx = i;
		plan.team = team;
		plans.push_back(plan);
	}

	Int t0count = 0, t1count = 0;
	size_t pi;
	for (pi = 0; pi < plans.size(); ++pi)
	{
		if (plans[pi].team == 0) ++t0count;
		else if (plans[pi].team == 1) ++t1count;
	}
	for (pi = 0; pi < plans.size(); ++pi)
	{
		if (plans[pi].team != -1) continue;
		if (t0count <= t1count)
		{
			plans[pi].team = 0;
			++t0count;
		}
		else
		{
			plans[pi].team = 1;
			++t1count;
		}
	}

	// Ensure the host (slot 0) ends up on team 0 (UI "Team 1"). If the host
	// landed on team 1 because of how the server split names, flip both teams
	// so the host's side is always Team 1.
	Int hostTeam = -1;
	for (pi = 0; pi < plans.size(); ++pi)
	{
		if (plans[pi].slotIdx == 0)
		{
			hostTeam = plans[pi].team;
			break;
		}
	}
	if (hostTeam == 1)
	{
		for (pi = 0; pi < plans.size(); ++pi)
		{
			if (plans[pi].team == 0)
				plans[pi].team = 1;
			else if (plans[pi].team == 1)
				plans[pi].team = 0;
		}
	}

	for (pi = 0; pi < plans.size(); ++pi)
	{
		GameSlot *slot = game->getSlot(plans[pi].slotIdx);
		if (!slot) continue;
		slot->setTeamNumber(plans[pi].team);
	}

	return true;
}
