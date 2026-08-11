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

#if defined(RTS_DEBUG)

#include <limits.h>

#include "GameClient/ClientInstance.h"
#include "GameClient/MapUtil.h"
#include "GameNetwork/LANAPICallbacks.h"
#include "GameNetwork/NetworkAutoStart.h"

namespace
{
enum {
	DefaultStartupTimeoutMilliseconds = 30000,
	ActionRetryMilliseconds = 1000,
	MillisecondsPerSecond = 1000,
	IPv4OctetCount = 4,
	IPv4BitsPerOctet = 8,
	MaxIPv4OctetValue = 255,
};

const UnsignedInt IPv4BroadcastAddress = UINT_MAX;

NetworkAutoStart::Mode s_mode = NetworkAutoStart::MODE_NONE;
NetworkAutoStart::Role s_role = NetworkAutoStart::ROLE_NONE;
Int s_expectedPlayers = 0;
UnsignedInt s_hostAddress = 0;
UnsignedInt s_localAddress = 0;
AsciiString s_playerName;
AsciiString s_mapName;
UnsignedInt s_timeoutMilliseconds = DefaultStartupTimeoutMilliseconds;
UnsignedInt s_startTime = 0;
UnsignedInt s_lastActionTime = 0;
Bool s_hasArguments = false;
Bool s_directConnectOpened = false;
Bool s_actionPending = false;
Bool s_startRequested = false;
Bool s_gameStarted = false;
Bool s_failed = false;

Bool ParseIPv4Address(AsciiString address, UnsignedInt &result)
{
	const char *cursor = address.str();
	result = 0;
	for (Int octet = 0; octet < IPv4OctetCount; ++octet)
	{
		if (*cursor < '0' || *cursor > '9')
		{
			return false;
		}

		UnsignedInt value = 0;
		do
		{
			value = value * 10 + (*cursor - '0');
			if (value > MaxIPv4OctetValue)
			{
				return false;
			}
			++cursor;
		} while (*cursor >= '0' && *cursor <= '9');

		result = (result << IPv4BitsPerOctet) | value;
		if (octet + 1 < IPv4OctetCount)
		{
			if (*cursor != '.')
			{
				return false;
			}
			++cursor;
		}
		else if (*cursor != '\0')
		{
			return false;
		}
	}

	return result != 0 && result != IPv4BroadcastAddress;
}

Bool CanAcceptMap(LANGameInfo *game, LANGameSlot *slot)
{
	if (slot->hasMap())
	{
		return true;
	}

	const MapMetaData *mapData = TheMapCache->findMap(game->getMap());
	if (mapData != nullptr)
	{
		return !mapData->m_isOfficial;
	}

	return WouldMapTransfer(game->getMap());
}
} // namespace

Bool NetworkAutoStart::setMode(AsciiString mode)
{
	s_hasArguments = true;
	if (mode.compareNoCase("direct") == 0)
	{
		s_mode = MODE_DIRECT_CONNECT;
		rts::ClientInstance::setMultiInstance(true);
		rts::ClientInstance::skipPrimaryInstance();
		return true;
	}

	return false;
}

Bool NetworkAutoStart::setHost(Int expectedPlayers)
{
	s_hasArguments = true;
	if (s_role == ROLE_JOIN || expectedPlayers < MIN_EXPECTED_PLAYERS || expectedPlayers > MAX_SLOTS)
	{
		return false;
	}

	s_role = ROLE_HOST;
	s_expectedPlayers = expectedPlayers;
	return true;
}

Bool NetworkAutoStart::setJoin(AsciiString hostAddress)
{
	s_hasArguments = true;
	hostAddress.trim();
	if (s_role == ROLE_HOST || hostAddress.isEmpty())
	{
		return false;
	}

	UnsignedInt resolvedAddress = 0;
	if (!ParseIPv4Address(hostAddress, resolvedAddress))
	{
		return false;
	}

	s_role = ROLE_JOIN;
	s_hostAddress = resolvedAddress;
	return true;
}

Bool NetworkAutoStart::setLocalAddress(AsciiString localAddress)
{
	s_hasArguments = true;
	localAddress.trim();
	if (localAddress.isEmpty())
	{
		return false;
	}

	return ParseIPv4Address(localAddress, s_localAddress);
}

Bool NetworkAutoStart::setPlayerName(AsciiString playerName)
{
	s_hasArguments = true;
	playerName.trim();
	if (playerName.isEmpty())
	{
		return false;
	}

	s_playerName = playerName;
	return true;
}

Bool NetworkAutoStart::setMapName(AsciiString mapName)
{
	s_hasArguments = true;
	mapName.trim();
	if (mapName.isEmpty())
	{
		return false;
	}

	s_mapName = mapName;
	return true;
}

Bool NetworkAutoStart::setTimeoutSeconds(Int seconds)
{
	s_hasArguments = true;
	if (seconds < 1 || (UnsignedInt)seconds > UINT_MAX / MillisecondsPerSecond)
	{
		return false;
	}

	s_timeoutMilliseconds = (UnsignedInt)seconds * MillisecondsPerSecond;
	return true;
}

Bool NetworkAutoStart::hasArguments()
{
	return s_hasArguments;
}

Bool NetworkAutoStart::isEnabled()
{
	return !s_failed && s_mode != MODE_NONE && s_role != ROLE_NONE;
}

Bool NetworkAutoStart::validateConfiguration()
{
	if (s_failed)
	{
		return false;
	}

	if (s_mode == MODE_NONE)
	{
		fail("-autoNetworkMode direct is required");
		return false;
	}

	if (s_role == ROLE_NONE)
	{
		fail("either -autoNetworkHost or -autoNetworkJoin is required");
		return false;
	}

	return true;
}

Bool NetworkAutoStart::shouldOpenDirectConnect()
{
	if (!s_hasArguments || s_directConnectOpened || !validateConfiguration())
	{
		return false;
	}

	return s_mode == MODE_DIRECT_CONNECT;
}

void NetworkAutoStart::markDirectConnectOpened()
{
	s_directConnectOpened = true;
	if (s_startTime == 0)
	{
		s_startTime = timeGetTime();
	}
}

AsciiString NetworkAutoStart::getMapName()
{
	return s_mapName;
}

UnsignedInt NetworkAutoStart::getLocalAddress()
{
	return s_localAddress;
}

UnicodeString NetworkAutoStart::getPlayerName()
{
	UnicodeString name;
	if (s_playerName.isNotEmpty())
	{
		name.translate(s_playerName);
	}
	else
	{
		name.format(L"AutoNet%02u", rts::ClientInstance::getInstanceId());
	}
	name.truncateTo(g_lanPlayerNameLength);
	return name;
}

Bool NetworkAutoStart::checkTimeout()
{
	if (s_failed || s_gameStarted)
	{
		return true;
	}

	const UnsignedInt now = timeGetTime();
	if (s_startTime != 0 && now - s_startTime >= s_timeoutMilliseconds)
	{
		fail("network match startup timed out");
		return true;
	}

	return false;
}

void NetworkAutoStart::fail(const char *message)
{
	if (s_failed)
	{
		return;
	}

	s_failed = true;
	s_actionPending = false;
	DEBUG_LOG(("NetworkAutoStart failed: %s", message));
	printf("NetworkAutoStart failed: %s\n", message);
}

void NetworkAutoStart::updateDirectConnect()
{
	if (!isEnabled() || s_mode != MODE_DIRECT_CONNECT || checkTimeout() || TheLAN == nullptr)
	{
		return;
	}

	const UnsignedInt now = timeGetTime();
	if (s_actionPending || (s_lastActionTime != 0 && now - s_lastActionTime < ActionRetryMilliseconds))
	{
		return;
	}

	TheLAN->RequestSetName(getPlayerName());
	s_lastActionTime = now;
	s_actionPending = true;

	if (s_role == ROLE_HOST)
	{
		DEBUG_LOG(("NetworkAutoStart creating Direct Connect game for %d players", s_expectedPlayers));
		TheLAN->RequestGameCreate(UnicodeString::TheEmptyString, true);
	}
	else
	{
		DEBUG_LOG(("NetworkAutoStart joining Direct Connect host 0x%08X", s_hostAddress));
		TheLAN->RequestGameJoinDirectConnect(s_hostAddress);
	}
}

void NetworkAutoStart::updateGameOptions()
{
	if (!isEnabled() || checkTimeout() || TheLAN == nullptr || TheLAN->GetMyGame() == nullptr)
	{
		return;
	}

	LANGameInfo *game = TheLAN->GetMyGame();
	if (s_role == ROLE_JOIN)
	{
		const Int localSlot = game->getLocalSlotNum();
		if (localSlot < 0)
		{
			return;
		}

		LANGameSlot *slot = game->getLANSlot(localSlot);
		const UnsignedInt now = timeGetTime();
		if (slot != nullptr && !slot->isAccepted() &&
				(s_lastActionTime == 0 || now - s_lastActionTime >= ActionRetryMilliseconds))
		{
			TheLAN->RequestHasMap();
			if (!CanAcceptMap(game, slot))
			{
				fail("required map is unavailable and cannot be transferred");
				return;
			}

			TheLAN->RequestAccept();
			s_lastActionTime = now;
		}
		return;
	}

	if (s_startRequested)
	{
		return;
	}

	const MapMetaData *mapData = TheMapCache->findMap(game->getMap());
	if (mapData == nullptr)
	{
		fail("selected map was not found");
		return;
	}
	if (mapData->m_numPlayers < s_expectedPlayers)
	{
		fail("selected map has fewer slots than -autoNetworkHost requires");
		return;
	}

	Int humanPlayers = 0;
	for (Int humanIndex = 0; humanIndex < MAX_SLOTS; ++humanIndex)
	{
		LANGameSlot *slot = game->getLANSlot(humanIndex);
		if (slot != nullptr && slot->isHuman())
		{
			++humanPlayers;
		}
	}

	if (humanPlayers > s_expectedPlayers)
	{
		fail("more players joined than -autoNetworkHost expects");
		return;
	}
	if (humanPlayers != s_expectedPlayers)
	{
		return;
	}

	LANGameSlot *hostSlot = game->getLANSlot(0);
	if (hostSlot == nullptr)
	{
		fail("host slot is unavailable");
		return;
	}
	hostSlot->setAccept();
	for (Int acceptedIndex = 0; acceptedIndex < MAX_SLOTS; ++acceptedIndex)
	{
		LANGameSlot *slot = game->getLANSlot(acceptedIndex);
		if (slot != nullptr && slot->isHuman() && !slot->isAccepted())
		{
			return;
		}
	}

	const UnsignedInt now = timeGetTime();
	if (s_lastActionTime == 0 || now - s_lastActionTime >= ActionRetryMilliseconds)
	{
		DEBUG_LOG(("NetworkAutoStart starting Direct Connect game with %d players", humanPlayers));
		s_lastActionTime = now;
		s_startRequested = true;
		StartLANGame();
	}
}

void NetworkAutoStart::onGameCreate(LANAPIInterface::ReturnType result)
{
	if (!isEnabled())
	{
		return;
	}

	if (result == LANAPIInterface::RET_OK)
	{
		return;
	}

	s_actionPending = false;
	fail("could not create Direct Connect game");
}

void NetworkAutoStart::onGameJoin(LANAPIInterface::ReturnType result)
{
	if (!isEnabled())
	{
		return;
	}

	if (result == LANAPIInterface::RET_OK)
	{
		return;
	}

	s_actionPending = false;
	if (result == LANAPIInterface::RET_TIMEOUT || result == LANAPIInterface::RET_GAME_GONE)
	{
		DEBUG_LOG(("NetworkAutoStart will retry Direct Connect join after result %d", result));
		return;
	}

	fail("Direct Connect join was rejected");
}

void NetworkAutoStart::onLocalAddressSet(Bool result)
{
	if (isEnabled() && !result)
	{
		fail("could not bind the Direct Connect local address");
	}
}

void NetworkAutoStart::onGameStartFailure()
{
	if (isEnabled())
	{
		fail("required map could not be transferred");
	}
}

void NetworkAutoStart::onGameStart()
{
	if (!isEnabled())
	{
		return;
	}

	s_gameStarted = true;
	DEBUG_LOG(("NetworkAutoStart entered the network game"));
	printf("NetworkAutoStart entered the network game\n");
}

#endif
