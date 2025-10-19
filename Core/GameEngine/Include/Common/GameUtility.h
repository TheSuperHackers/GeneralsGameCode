/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#pragma once

#include "Lib/BaseType.h"

// For miscellaneous game utility functions.

class Player;
typedef Int PlayerIndex;

namespace rts
{

bool localPlayerIsObserving();
bool localPlayerHasRadar();
Player* getObservedOrLocalPlayer(); ///< Get the current observed or local player. Is never null.
Player* getObservedOrLocalPlayer_Safe(); ///< Get the current observed or local player. Is never null, except when the application does not have players.
PlayerIndex getObservedOrLocalPlayerIndex_Safe(); ///< Get the current observed or local player index. Returns 0 when the application does not have players.

void changeLocalPlayer(Player* player); //< Change local player during game
void changeObservedPlayer(Player* player); ///< Change observed player during game

} // namespace rts
