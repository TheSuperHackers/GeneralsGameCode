#pragma once

class GameInfo;

#include "Common/AsciiString.h"

#include <vector>

void performRandomAssign(GameInfo *game, const std::vector<Int> &lockedTemplates);
std::vector<Int> buildLockedTemplates();

// If every occupied non-observer slot has team == -1, query the
// balance_teams API (TheGlobalData->m_balanceTeamsUrl) with the lobby's
// player names and assign team 0 / team 1 based on the first response.
// Returns true on success or no-op (teams already set, or fewer than two
// candidates). Returns false on API/network failure; *outError is then
// populated with a player-facing message suitable for chat. Slots are
// left untouched on failure.
bool tryBalanceTeamsViaApi(GameInfo *game, AsciiString *outError);
