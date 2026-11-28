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

#include "PreRTS.h"

#include "Common/ActionsPerMinute.h"
#include "Common/GameCommon.h"
#include "Common/MessageStream.h"

// ------------------------------------------------------------------------------------------------
ActionsPerMinute::ActionsPerMinute()
{
	reset();
}

// ------------------------------------------------------------------------------------------------
void ActionsPerMinute::reset()
{
	std::fill(m_buckets, m_buckets + ARRAY_SIZE(m_buckets), 0u);
	m_currentSecond = 0u;
	m_lastActionFrame = ~0u;
	m_lastActionWasDeselect = FALSE;
	m_actionsPerMinute = 0u;
	m_totalActions = 0u;
}

// ------------------------------------------------------------------------------------------------
void ActionsPerMinute::update(UnsignedInt frame)
{
	const UnsignedInt second = frame / LOGICFRAMES_PER_SECOND;
	if (second <= m_currentSecond)
		return;

	if (second - m_currentSecond >= BucketCount)
	{
		std::fill(m_buckets, m_buckets + ARRAY_SIZE(m_buckets), 0u);
		m_actionsPerMinute = 0u;
	}
	else
	{
		// Drop every second that has slid out of the window.
		for (UnsignedInt s = m_currentSecond + 1; s <= second; ++s)
		{
			UnsignedInt &bucket = m_buckets[s % BucketCount];
			m_actionsPerMinute -= bucket;
			bucket = 0u;
		}
	}

	m_currentSecond = second;
}

// ------------------------------------------------------------------------------------------------
void ActionsPerMinute::recordMessage(const GameMessage *msg, UnsignedInt frame)
{
	if (!isPlayerAction(msg))
		return;

	// The select message that directly follows a deselect message belongs to the same player input.
	if (m_lastActionWasDeselect && frame == m_lastActionFrame && isSelectMessage(msg))
	{
		m_lastActionWasDeselect = FALSE;
		return;
	}

	m_lastActionFrame = frame;
	m_lastActionWasDeselect = (msg->getType() == GameMessage::MSG_DESTROY_SELECTED_GROUP);

	update(frame);

	++m_buckets[m_currentSecond % BucketCount];
	++m_actionsPerMinute;
	++m_totalActions;
}

// ------------------------------------------------------------------------------------------------
Bool ActionsPerMinute::isPlayerAction(const GameMessage *msg)
{
	const GameMessage::Type type = msg->getType();

	if (type <= GameMessage::MSG_BEGIN_NETWORK_MESSAGES || type >= GameMessage::MSG_BEGIN_DEBUG_NETWORK_MESSAGES)
		return FALSE;

	switch (type)
	{
		// Sent automatically by the game, not by the player.
		case GameMessage::MSG_LOGIC_CRC:
		case GameMessage::MSG_SET_REPLAY_CAMERA:
		case GameMessage::MSG_ENABLE_RETALIATION_MODE:
		case GameMessage::MSG_CLEAR_INGAME_POPUP_MESSAGE:
		case GameMessage::MSG_SELF_DESTRUCT:
			return FALSE;

		default:
			return TRUE;
	}
}

// ------------------------------------------------------------------------------------------------
Bool ActionsPerMinute::isSelectMessage(const GameMessage *msg)
{
	const GameMessage::Type type = msg->getType();

	if (type == GameMessage::MSG_CREATE_SELECTED_GROUP || type == GameMessage::MSG_CREATE_SELECTED_GROUP_NO_SOUND)
		return TRUE;

	if (type >= GameMessage::MSG_SELECT_TEAM0 && type <= GameMessage::MSG_SELECT_TEAM9)
		return TRUE;

	if (type >= GameMessage::MSG_ADD_TEAM0 && type <= GameMessage::MSG_ADD_TEAM9)
		return TRUE;

	return FALSE;
}
