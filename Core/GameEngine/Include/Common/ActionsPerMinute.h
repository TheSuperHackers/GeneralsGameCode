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

#pragma once

#include "Lib/BaseType.h"

class GameMessage;

// TheSuperHackers @feature aibrahim- 13/11/2026 Counts the actions issued by a player over the last 60 seconds
// of logic time. Selecting units often sends a deselect message directly followed by a select message for
// the same player input, so such a pair is counted as one action.
// It is purely informational: it is not saved and not part of the CRC, so it never affects sync.
class ActionsPerMinute
{
public:

	ActionsPerMinute();

	void reset();

	/// Advance the rolling window to the given logic frame.
	void update(UnsignedInt frame);

	/// Count the message as an action if it was issued by the player and does not complete a deselect and select pair.
	void recordMessage(const GameMessage *msg, UnsignedInt frame);

	/// Actions issued over the last 60 seconds.
	UnsignedInt getActionsPerMinute() const { return m_actionsPerMinute; }

	/// Actions issued since the start of the game.
	UnsignedInt getTotalActions() const { return m_totalActions; }

	static Bool isPlayerAction(const GameMessage *msg);
	static Bool isSelectMessage(const GameMessage *msg);

private:

	enum { BucketCount = 60 };

	UnsignedInt m_buckets[BucketCount]; ///< circular buffer of actions per second
	UnsignedInt m_currentSecond;
	UnsignedInt m_lastActionFrame;
	Bool m_lastActionWasDeselect;
	UnsignedInt m_actionsPerMinute;
	UnsignedInt m_totalActions;
};
