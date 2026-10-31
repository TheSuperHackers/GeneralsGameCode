/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////


#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Recorder.h"
#include "GameClient/DisconnectMenu.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/DisconnectManager.h"
#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/networkutil.h"
#include "GameNetwork/GameSpy/PingThread.h"
#include "GameNetwork/GameSpy/GSConfig.h"

#if !RETAIL_COMPATIBLE_NETWORKING
static const UnsignedInt LEAVE_NOTICE_FRAME = 0xFFFFFFFF;

static const time_t LEFT_PLAYER_SCREEN_WAIT_TIME = 200;

// a leave notice of another player only counts once the player itself sent no keepalive for this long
static const time_t LEAVE_NOTICE_QUIET_TIME = 3000;
#endif


DisconnectManager::DisconnectManager()
{
	Int i;
	m_currentPacketRouterIndex = 0;
	m_lastFrame = 0;
	m_lastFrameTime = 0;
	m_lastKeepAliveSendTime = 0;
	for (i = 0; i < MAX_SLOTS; ++i) {
		m_destroySent[i] = FALSE;
	}
	m_haveNotifiedOtherPlayersOfCurrentFrame = FALSE;
	m_timeOfDisconnectScreenOn = 0;

	for( i = 0; i < MAX_SLOTS; ++i) {
		m_packetRouterFallback[i] = 0;
#if RETAIL_COMPATIBLE_NETWORKING
		m_relayFrame[i] = 0;
		m_relayTo[i] = 0;
#else
		m_heldRemovalFrom[i] = -1;
		m_heldRemovalFrame[i] = 0;
#endif
	}

	m_packetRouterTimeout = 0;
	for( i = 0; i < MAX_SLOTS -1; ++i) {
		m_playerTimeouts[i] = 0;
#if !RETAIL_COMPATIBLE_NETWORKING
		m_playerLeft[i] = FALSE;
		m_ownLeaveReceived[i] = FALSE;
#endif
	}
#if !RETAIL_COMPATIBLE_NETWORKING
	m_leftPlayerHold = FALSE;
#endif

	for( i = 0; i < MAX_SLOTS; ++i) {
		for (Int j = 0; j < MAX_SLOTS; ++j) {
			m_playerVotes[i][j].vote = FALSE;
			m_playerVotes[i][j].frame = 0;
		}
	}
}

DisconnectManager::~DisconnectManager() {
}

void DisconnectManager::init() {
	TheDisconnectMenu->hideScreen(); // make sure the screen starts out hidden.
	m_lastFrame = 0;
	m_lastFrameTime = -1;
	m_lastKeepAliveSendTime = -1;
	m_disconnectState = DISCONNECTSTATETYPE_SCREENOFF;
	m_currentPacketRouterIndex = 0;
	m_timeOfDisconnectScreenOn = 0;

	Int i = 0;
	for (; i < MAX_SLOTS; ++i) {
		for (Int j = 0; j < MAX_SLOTS; ++j) {
			m_playerVotes[i][j].vote = FALSE;
			m_playerVotes[i][j].frame = 0;
		}
	}

	for (i = 0; i < MAX_SLOTS; ++i) {
		m_disconnectFrames[i] = 0;
		m_disconnectFramesReceived[i] = FALSE;
#if RETAIL_COMPATIBLE_NETWORKING
		m_relayFrame[i] = 0;
		m_relayTo[i] = 0;
#else
		m_heldRemovalFrom[i] = -1;
		m_heldRemovalFrame[i] = 0;
#endif
		m_destroySent[i] = FALSE;
	}
#if !RETAIL_COMPATIBLE_NETWORKING
	for (i = 0; i < MAX_SLOTS - 1; ++i) {
		m_playerLeft[i] = FALSE;
		m_ownLeaveReceived[i] = FALSE;
	}
	m_leftPlayerHold = FALSE;
#endif

	m_pingFrame = 0;
	m_pingsSent = 0;
	m_pingsReceived = 0;
}

void DisconnectManager::update(ConnectionManager *conMgr) {
	if (m_lastFrameTime == -1) {
		m_lastFrameTime = timeGetTime();
	}

	// The game logic stalls on the frame we are currently waiting for commands on,
	// so we have to check for the current logic frame being one higher than
	// the last one we had the commands ready for.
	if (TheGameLogic->getFrame() == m_lastFrame) {
		time_t curTime = timeGetTime();
		time_t waitTime = TheGlobalData->m_networkDisconnectTime;
#if !RETAIL_COMPATIBLE_NETWORKING
		// the removal of a player that has left runs from the disconnect screen, so a hold brings the screen up at once
		if ((m_leftPlayerHold == TRUE) && (waitTime > LEFT_PLAYER_SCREEN_WAIT_TIME)) {
			waitTime = LEFT_PLAYER_SCREEN_WAIT_TIME;
		}
#endif
		if ((curTime - m_lastFrameTime) > waitTime) {
			if (m_disconnectState == DISCONNECTSTATETYPE_SCREENOFF) {
				turnOnScreen(conMgr);
			}
			sendKeepAlive(conMgr);
		}
	} else {
		nextFrame(TheGameLogic->getFrame(), conMgr);
	}

#if RETAIL_COMPATIBLE_NETWORKING
	sendPendingRelays(conMgr);
#else
	processHeldRemovals(conMgr);
#endif

	if (m_disconnectState != DISCONNECTSTATETYPE_SCREENOFF) {
		updateDisconnectStatus(conMgr);

		// check to see if we need to send pings
		if (m_pingFrame < TheGameLogic->getFrame())
		{
			time_t curTime = timeGetTime();
			if ((curTime - m_lastFrameTime) > 10000) /// @todo: plug in some better measure here
			{
				m_pingFrame = TheGameLogic->getFrame();
				m_pingsSent = 0;
				m_pingsReceived = 0;

				// Send the pings
				if (ThePinger)
				{
					//use next ping server
					static size_t serverIndex = 0;
					serverIndex++;

					AsciiStringList pingServers = TheGameSpyConfig->getPingServers();

					if( serverIndex >= pingServers.size() )
						serverIndex = 0;  //wrap back to first ping server

					std::list<AsciiString>::iterator it = pingServers.begin();
					for( size_t i = 0;  i < serverIndex;  i++ )
						it++;

					PingRequest req;
					req.hostname = it->str();
					req.repetitions = 5;
					req.timeout = 2000;
					m_pingsSent = req.repetitions;
					ThePinger->addRequest(req);
					DEBUG_LOG(("DisconnectManager::update() - requesting %d pings of %d from %s",
						req.repetitions, req.timeout, req.hostname.c_str()));
				}
			}
		}

		// update the ping thread, tracking pings if we are on the same frame
		if (ThePinger)
		{
			PingResponse resp;
			while (ThePinger->getResponse(resp))
			{
				if (m_pingFrame != TheGameLogic->getFrame())
				{
					// wrong frame - we're not pinging yet
					DEBUG_LOG(("DisconnectManager::update() - discarding ping of %d from %s (%d reps)",
						resp.avgPing, resp.hostname.c_str(), resp.repetitions));
				}
				else
				{
					// right frame
					DEBUG_LOG(("DisconnectManager::update() - keeping ping of %d from %s (%d reps)",
						resp.avgPing, resp.hostname.c_str(), resp.repetitions));
					if (resp.avgPing < 2000)
					{
						m_pingsReceived += resp.repetitions;
					}
				}
			}
		}
	}
}

UnsignedInt DisconnectManager::getPingFrame()
{
	return m_pingFrame;
}

Int DisconnectManager::getPingsSent()
{
	return m_pingsSent;
}

Int DisconnectManager::getPingsReceived()
{
	return m_pingsReceived;
}


void DisconnectManager::updateDisconnectStatus(ConnectionManager *conMgr) {
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if (conMgr->isPlayerConnected(i)) {
			Int slot = translatedSlotPosition(i, conMgr->getLocalPlayerID());
			if (slot != -1) {
				time_t curTime = timeGetTime();
				time_t newTime = TheGlobalData->m_networkPlayerTimeoutTime - (curTime - m_playerTimeouts[slot]);
#if !RETAIL_COMPATIBLE_NETWORKING
				if (hasPlayerLeft(slot) == TRUE) {
					newTime = -1;
				}
				if (m_playerLeft[slot] == TRUE) {
					// a player that has left is removed on a frame agreed with everyone, so the frames are synced up right away
					if (m_haveNotifiedOtherPlayersOfCurrentFrame == FALSE) {
						TheNetwork->notifyOthersOfCurrentFrame();
						m_haveNotifiedOtherPlayersOfCurrentFrame = TRUE;
					}
				}
#endif

// if someone is more than 2/3 timed out, lets get our frame numbers sync'd up.  Also if someone is voted out
// lets do the same thing.

				if (m_haveNotifiedOtherPlayersOfCurrentFrame == FALSE) {
					if ((newTime < TheGlobalData->m_networkPlayerTimeoutTime / 3) || (isPlayerVotedOut(slot, conMgr) == TRUE)) {
						TheNetwork->notifyOthersOfCurrentFrame();
						m_haveNotifiedOtherPlayersOfCurrentFrame = TRUE;
					}

					DEBUG_LOG(("DisconnectManager::updateDisconnectStatus - curTime = %d, m_timeOfDisconnectScreenOn = %d, curTime - m_timeOfDisconnectScreenOn = %d", curTime, m_timeOfDisconnectScreenOn, curTime - m_timeOfDisconnectScreenOn));

					if (m_timeOfDisconnectScreenOn != 0) {
						if ((curTime - m_timeOfDisconnectScreenOn) > TheGlobalData->m_networkDisconnectScreenNotifyTime) {
							TheNetwork->notifyOthersOfCurrentFrame();
							m_haveNotifiedOtherPlayersOfCurrentFrame = TRUE;
						}
					}
				}

				if ((newTime < 0) || (isPlayerVotedOut(slot, conMgr) == TRUE)) {
					newTime = 0;
					DEBUG_LOG(("DisconnectManager::updateDisconnectStatus - player %d(translated slot %d) has been voted out or timed out", i, slot));
					if (allOnSameFrame(conMgr) == TRUE) {
						DEBUG_LOG(("DisconnectManager::updateDisconnectStatus - all on same frame"));
						if (isLocalPlayerNextPacketRouter(conMgr) == TRUE) {
							DEBUG_LOG(("DisconnectManager::updateDisconnectStatus - local player is next packet router"));
							DEBUG_LOG(("DisconnectManager::updateDisconnectStatus - about to do the disconnect procedure for player %d", i));
							sendDisconnectCommand(i, conMgr);
							disconnectPlayer(i, conMgr);
							sendPlayerDestruct(i, conMgr);
						} else {
							DEBUG_LOG(("DisconnectManager::updateDisconnectStatus - local player is not the next packet router"));
						}
					} else {
						DEBUG_LOG(("DisconnectManager::updateDisconnectStatus - not all on same frame"));
					}
				}
				TheDisconnectMenu->setPlayerTimeoutTime(slot, newTime);
			}
		}
	}
}

void DisconnectManager::updateWaitForPacketRouter(ConnectionManager *conMgr) {
/*
	time_t curTime = timeGetTime();
	time_t newTime = TheGlobalData->m_networkPlayerTimeoutTime - (curTime - m_packetRouterTimeout);
	if (newTime < 0) {
		newTime = 0;

		// The guy that we were hoping would be the new packet router isn't.  We're screwed, get out of the game.

		DEBUG_LOG(("DisconnectManager::updateWaitForPacketRouter - timed out waiting for new packet router, quitting game"));
		TheNetwork->quitGame();
	}
	TheDisconnectMenu->setPacketRouterTimeoutTime(newTime);
*/
}

void DisconnectManager::processDisconnectCommand(NetCommandRef *ref, ConnectionManager *conMgr) {
	NetCommandMsg *msg = ref->getCommand();
	
	switch (msg->getNetCommandType()) {
		case NETCOMMANDTYPE_DISCONNECTKEEPALIVE:
			processDisconnectKeepAlive(msg, conMgr);
			break;
			
		case NETCOMMANDTYPE_DISCONNECTPLAYER:
			processDisconnectPlayer(msg, conMgr);
			break;
			
		case NETCOMMANDTYPE_PACKETROUTERQUERY:
			processPacketRouterQuery(msg, conMgr);
			break;
			
		case NETCOMMANDTYPE_PACKETROUTERACK:
			processPacketRouterAck(msg, conMgr);
			break;
			
		case NETCOMMANDTYPE_DISCONNECTVOTE:
			processDisconnectVote(msg, conMgr);
			break;
			
		case NETCOMMANDTYPE_DISCONNECTFRAME:
			processDisconnectFrame(msg, conMgr);
			break;
			
		case NETCOMMANDTYPE_DISCONNECTSCREENOFF:
			processDisconnectScreenOff(msg, conMgr);
			break;
			
		default:
			break;
	}
}

void DisconnectManager::processDisconnectKeepAlive(NetCommandMsg *msg, ConnectionManager *conMgr) {
	NetDisconnectKeepAliveCommandMsg *cmdMsg = (NetDisconnectKeepAliveCommandMsg *)msg;
	Int slot = translatedSlotPosition(cmdMsg->getPlayerID(), conMgr->getLocalPlayerID());
	if (slot != -1) {
		resetPlayerTimeout(slot);
#if !RETAIL_COMPATIBLE_NETWORKING
		// a player that is still around undoes a leave notice of another player, its own leave is final
		if (m_ownLeaveReceived[slot] == FALSE) {
			m_playerLeft[slot] = FALSE;
		}
#endif
	}
}

void DisconnectManager::processDisconnectPlayer(NetCommandMsg *msg, ConnectionManager *conMgr) {
	NetDisconnectPlayerCommandMsg *cmdMsg = (NetDisconnectPlayerCommandMsg *)msg;
	DEBUG_LOG(("DisconnectManager::processDisconnectPlayer - Got disconnect player command from player %d.  Disconnecting player %d on frame %d", msg->getPlayerID(), cmdMsg->getDisconnectSlot(), cmdMsg->getDisconnectFrame()));
	const Int slot = cmdMsg->getDisconnectSlot();
	if ((slot < MAX_SLOTS) && (slot != (Int)conMgr->getLocalPlayerID()) && (conMgr->isPlayerConnected(slot) == FALSE)) {
		DEBUG_LOG(("DisconnectManager::processDisconnectPlayer - player %d is already disconnected", slot));
		return;
	}
	// a disconnect player command is always sent directly, so it has to come from the address of the player it claims to be from
	if (conMgr->getCurrentSenderSlot() != (Int)msg->getPlayerID()) {
		DEBUG_LOG(("DisconnectManager::processDisconnectPlayer - command of player %d did not come from its address, ignored", msg->getPlayerID()));
		return;
	}

#if !RETAIL_COMPATIBLE_NETWORKING
	// TheSuperHackers @bugfix arcticdolphin 27/10/2026 A player that leaves is removed by the acting packet router on one frame for everyone instead of on the frame each player got the leave on.
	if ((slot < MAX_SLOTS) && ((msg->getPlayerID() == (UnsignedInt)slot) || (cmdMsg->getDisconnectFrame() == LEAVE_NOTICE_FRAME))) {
		Int transSlot = translatedSlotPosition(slot, conMgr->getLocalPlayerID());
		if (transSlot != -1) {
			m_playerLeft[transSlot] = TRUE;
			if (msg->getPlayerID() == (UnsignedInt)slot) {
				m_ownLeaveReceived[transSlot] = TRUE;
			}
		}

		// everyone that got the leave from the leaver tells the others, so all of them pick the same acting packet router
		if (msg->getPlayerID() == (UnsignedInt)slot) {
			NetDisconnectPlayerCommandMsg *noticeMsg = newInstance(NetDisconnectPlayerCommandMsg);
			noticeMsg->setDisconnectSlot(slot);
			noticeMsg->setDisconnectFrame(LEAVE_NOTICE_FRAME);
			noticeMsg->setPlayerID(conMgr->getLocalPlayerID());
			if (DoesCommandRequireACommandID(noticeMsg->getNetCommandType())) {
				noticeMsg->setID(GenerateNextCommandID());
			}
			conMgr->sendLocalCommandDirect(noticeMsg, (UnsignedByte)(0xff & ~(1 << conMgr->getLocalPlayerID()) & ~(1 << slot)));
			noticeMsg->detach();
		}
		return;
	}

	// a router being removed may have timed out on the sender first, so the next router in line can remove it
	if ((Int)msg->getPlayerID() != getActingPacketRouterSlot(conMgr, slot)) {
		DEBUG_LOG(("DisconnectManager::processDisconnectPlayer - player %d is not the acting packet router, removal of player %d held", msg->getPlayerID(), slot));
		// the removal of a router ahead of the sender can still be on its way, so this one waits for it
		if (slot < MAX_SLOTS) {
			m_heldRemovalFrom[slot] = (Int)msg->getPlayerID();
			m_heldRemovalFrame[slot] = cmdMsg->getDisconnectFrame();
		}
		return;
	}
#endif

	DEBUG_ASSERTCRASH(TheGameLogic->getFrame() == cmdMsg->getDisconnectFrame(), ("disconnecting player on the wrong frame!!!"));
	disconnectPlayer(slot, conMgr);

#if RETAIL_COMPATIBLE_NETWORKING
	// TheSuperHackers @bugfix arcticdolphin 12/10/2026 A player that quits from the disconnect screen sends no destroy command, so every player that stays sends it.
	if (msg->getPlayerID() == (UnsignedInt)slot) {
		sendPlayerDestruct(slot, conMgr);
	}

	// a leave that some players did not get would wait for a disconnect command that only the packet router sends, so the next router passes it on
	if (msg->getPlayerID() == (UnsignedInt)slot && isLocalPlayerNextPacketRouter(conMgr) == TRUE && TheGameLogic->getFrame() == cmdMsg->getDisconnectFrame()) {
		m_relayFrame[slot] = cmdMsg->getDisconnectFrame();
		m_relayTo[slot] = 0;
		for (Int i = 0; i < MAX_SLOTS; ++i) {
			if ((i != slot) && (i != (Int)conMgr->getLocalPlayerID()) && (conMgr->isPlayerConnected(i) == TRUE)) {
				m_relayTo[slot] |= (UnsignedByte)(1 << i);
			}
		}
	}
#endif
}

#if RETAIL_COMPATIBLE_NETWORKING
void DisconnectManager::sendPendingRelays(ConnectionManager *conMgr) {
	for (Int slot = 0; slot < MAX_SLOTS; ++slot) {
		if (m_relayTo[slot] == 0) {
			continue;
		}
		if (isLocalPlayerNextPacketRouter(conMgr) == FALSE) {
			m_relayTo[slot] = 0;
			continue;
		}
		UnsignedByte sendTo = 0;
		for (Int i = 0; i < MAX_SLOTS; ++i) {
			const UnsignedByte bit = (UnsignedByte)(1 << i);
			if ((m_relayTo[slot] & bit) == 0) {
				continue;
			}
			if (conMgr->isPlayerConnected(i) == FALSE) {
				m_relayTo[slot] &= (UnsignedByte)~bit;
			}
			// A leave is applied on the frame its receiver is on, so it only goes to a player that reported being stalled on the frame of the leave.
			else if ((m_disconnectFramesReceived[i] == TRUE) && (m_disconnectFrames[i] == m_relayFrame[slot])) {
				sendTo |= bit;
				m_relayTo[slot] &= (UnsignedByte)~bit;
			}
		}
		if (sendTo == 0) {
			continue;
		}
		DEBUG_LOG(("DisconnectManager::sendPendingRelays - passing on the leave of player %d on frame %d to mask %X", slot, m_relayFrame[slot], sendTo));
		NetDisconnectPlayerCommandMsg *relayMsg = newInstance(NetDisconnectPlayerCommandMsg);
		relayMsg->setDisconnectSlot(slot);
		relayMsg->setDisconnectFrame(m_relayFrame[slot]);
		relayMsg->setPlayerID(conMgr->getLocalPlayerID());
		if (DoesCommandRequireACommandID(relayMsg->getNetCommandType())) {
			relayMsg->setID(GenerateNextCommandID());
		}
		conMgr->sendLocalCommand(relayMsg, sendTo);
		relayMsg->detach();
	}
}
#endif

void DisconnectManager::processPacketRouterQuery(NetCommandMsg *msg, ConnectionManager *conMgr) {
	NetPacketRouterQueryCommandMsg *cmdMsg = (NetPacketRouterQueryCommandMsg *)msg;
	DEBUG_LOG(("DisconnectManager::processPacketRouterQuery - got a packet router query command from player %d", msg->getPlayerID()));

	if (conMgr->getPacketRouterSlot() == conMgr->getLocalPlayerID()) {
		NetPacketRouterAckCommandMsg *ackmsg = newInstance(NetPacketRouterAckCommandMsg);
		ackmsg->setPlayerID(conMgr->getLocalPlayerID());
		if (DoesCommandRequireACommandID(ackmsg->getNetCommandType()) == TRUE) {
			ackmsg->setID(GenerateNextCommandID());
		}
		DEBUG_LOG(("DisconnectManager::processPacketRouterQuery - We are the new packet router, responding with an packet router ack. Local player is %d", ackmsg->getPlayerID()));
		conMgr->sendLocalCommandDirect(ackmsg, 1 << cmdMsg->getPlayerID());
		ackmsg->detach();
	} else {
		DEBUG_LOG(("DisconnectManager::processPacketRouterQuery - We are NOT the new packet router, these are not the droids you're looking for."));
	}
}

void DisconnectManager::processPacketRouterAck(NetCommandMsg *msg, ConnectionManager *conMgr) {
	NetPacketRouterAckCommandMsg *cmdMsg = (NetPacketRouterAckCommandMsg *)msg;
	DEBUG_LOG(("DisconnectManager::processPacketRouterAck - got packet router ack command from player %d", msg->getPlayerID()));

	if (conMgr->getPacketRouterSlot() == cmdMsg->getPlayerID()) {
		DEBUG_LOG(("DisconnectManager::processPacketRouterAck - packet router command is from who it should be."));
		resetPacketRouterTimeout();
		Int currentPacketRouterSlot = conMgr->getPacketRouterSlot();
		Int currentPacketRouterIndex = 0;
		while ((currentPacketRouterSlot != conMgr->getPacketRouterFallbackSlot(currentPacketRouterIndex)) && (currentPacketRouterIndex < MAX_SLOTS)) {
			++currentPacketRouterIndex;
		}
		DEBUG_ASSERTCRASH((currentPacketRouterIndex < MAX_SLOTS), ("Invalid packet router index"));

		DEBUG_LOG(("DisconnectManager::processPacketRouterAck - New packet router confirmed, resending pending commands"));
		conMgr->resendPendingCommands();
		m_currentPacketRouterIndex = currentPacketRouterIndex;
		DEBUG_LOG(("DisconnectManager::processPacketRouterAck - Setting disconnect state to screen on."));
		m_disconnectState = DISCONNECTSTATETYPE_SCREENON; ///< set it to screen on so that the next call to AllCommandsReady can set up everything for the next frame properly.
	}
}

void DisconnectManager::processDisconnectVote(NetCommandMsg *msg, ConnectionManager *conMgr) {
	NetDisconnectVoteCommandMsg *cmdMsg = (NetDisconnectVoteCommandMsg *)msg;
	DEBUG_LOG(("DisconnectManager::processDisconnectVote - Got a disconnect vote for player %d command from player %d", cmdMsg->getSlot(), cmdMsg->getPlayerID()));
	Int transSlot = translatedSlotPosition(msg->getPlayerID(), conMgr->getLocalPlayerID());

	if (isPlayerInGame(transSlot, conMgr) == FALSE) {
		// if they've been timed out, voted out, disconnected, don't count their vote.
		return;
	}

	applyDisconnectVote(cmdMsg->getSlot(), cmdMsg->getVoteFrame(), cmdMsg->getPlayerID(), conMgr);
}

void DisconnectManager::processDisconnectFrame(NetCommandMsg *msg, ConnectionManager *conMgr) {
	NetDisconnectFrameCommandMsg *cmdMsg = (NetDisconnectFrameCommandMsg *)msg;
	const UnsignedInt playerID = cmdMsg->getPlayerID();
	if (playerID >= MAX_SLOTS) {
		return;
	}

	if (m_disconnectFrames[playerID] >= cmdMsg->getDisconnectFrame()) {
		// this message isn't valid, we have a disconnect frame that is later than this already.
		return;
	}

	if (m_disconnectFramesReceived[playerID] == TRUE) {
		DEBUG_LOG(("DisconnectManager::processDisconnectFrame - Got two disconnect frames without an intervening disconnect screen off command from player %d. Frames are %d and %d", playerID, m_disconnectFrames[playerID], cmdMsg->getDisconnectFrame()));
	}

	DEBUG_LOG(("DisconnectManager::processDisconnectFrame - about to call resetPlayersVotes for player %d", playerID));
	resetPlayersVotes(playerID, cmdMsg->getDisconnectFrame()-1, conMgr);

	m_disconnectFrames[playerID] = cmdMsg->getDisconnectFrame();
	m_disconnectFramesReceived[playerID] = TRUE;
	DEBUG_LOG(("DisconnectManager::processDisconnectFrame - Got a disconnect frame for player %d, frame = %d, local player is %d, local disconnect frame = %d, command id = %d", cmdMsg->getPlayerID(), cmdMsg->getDisconnectFrame(), conMgr->getLocalPlayerID(), m_disconnectFrames[conMgr->getLocalPlayerID()], cmdMsg->getID()));

	if (playerID == conMgr->getLocalPlayerID()) {
		DEBUG_LOG(("DisconnectManager::processDisconnectFrame - player %d is the local player", playerID));
		// we just got the message from the local player, check to see if we need to send
		// commands to anyone we already have heard from.
		for (Int i = 0; i < MAX_SLOTS; ++i) {
			if (i != playerID) {
				Int transSlot = translatedSlotPosition(i, conMgr->getLocalPlayerID());
				if (isPlayerInGame(transSlot, conMgr) == TRUE) {
					if ((m_disconnectFrames[i] < m_disconnectFrames[playerID]) && (m_disconnectFramesReceived[i] == TRUE)) {
						DEBUG_LOG(("DisconnectManager::processDisconnectFrame - I have more frames than player %d, my frame = %d, their frame = %d", i, m_disconnectFrames[conMgr->getLocalPlayerID()], m_disconnectFrames[i]));
						conMgr->sendFrameDataToPlayer(i, m_disconnectFrames[i]);
					}
				}
			}
		}
	} else if ((m_disconnectFrames[playerID] < m_disconnectFrames[conMgr->getLocalPlayerID()]) && (m_disconnectFramesReceived[playerID] == TRUE)) {
		DEBUG_LOG(("DisconnectManager::processDisconnectFrame - I have more frames than player %d, my frame = %d, their frame = %d", playerID, m_disconnectFrames[conMgr->getLocalPlayerID()], m_disconnectFrames[playerID]));
		conMgr->sendFrameDataToPlayer(playerID, m_disconnectFrames[playerID]);
	}
}

void DisconnectManager::processDisconnectScreenOff(NetCommandMsg *msg, ConnectionManager *conMgr) {
	NetDisconnectScreenOffCommandMsg *cmdMsg = (NetDisconnectScreenOffCommandMsg *)msg;
	UnsignedInt playerID = cmdMsg->getPlayerID();

	DEBUG_LOG(("DisconnectManager::processDisconnectScreenOff - got a screen off command from player %d for frame %d", cmdMsg->getPlayerID(), cmdMsg->getNewFrame()));

	if (playerID >= MAX_SLOTS) {
		return;
	}

	UnsignedInt newFrame = cmdMsg->getNewFrame();
	if (newFrame >= m_disconnectFrames[playerID]) {
		DEBUG_LOG(("DisconnectManager::processDisconnectScreenOff - resetting the disconnect screen status for player %d", playerID));
		m_disconnectFramesReceived[playerID] = FALSE;
		m_disconnectFrames[playerID] = newFrame; // just in case we get packets out of order and the disconnect screen off message gets here before the disconnect frame message.

		DEBUG_LOG(("DisconnectManager::processDisconnectScreenOff - about to call resetPlayersVotes for player %d", playerID));
		resetPlayersVotes(playerID, cmdMsg->getNewFrame(), conMgr);
	}
}

void DisconnectManager::applyDisconnectVote(Int slot, UnsignedInt frame, Int fromSlot, ConnectionManager *conMgr) {
	m_playerVotes[slot][fromSlot].vote = TRUE;
	m_playerVotes[slot][fromSlot].frame = frame;
	Int numVotes = countVotesForPlayer(slot);
	DEBUG_LOG(("DisconnectManager::applyDisconnectVote - added a vote to disconnect slot %d, from slot %d, for frame %d, current votes are %d", slot, fromSlot, frame, numVotes));
	Int transSlot = translatedSlotPosition(slot, conMgr->getLocalPlayerID());
	if (transSlot != -1) {
		TheDisconnectMenu->updateVotes(transSlot, numVotes);
	}
}

void DisconnectManager::nextFrame(UnsignedInt frame, ConnectionManager *conMgr) {
	m_lastFrame = frame;
	m_lastFrameTime = timeGetTime();
	resetPlayerTimeouts(conMgr);
}

void DisconnectManager::allCommandsReady(UnsignedInt frame, ConnectionManager *conMgr, Bool waitForPacketRouter) {
#if !RETAIL_COMPATIBLE_NETWORKING
		// a player that has left is removed on the highest frame anyone reached, so nobody runs past it while that player is still connected
		m_leftPlayerHold = (isLeftPlayerConnected(conMgr) == TRUE) && (frame >= getMaxDisconnectFrame(conMgr));
		if (m_leftPlayerHold == TRUE) {
			return;
		}
#endif
		if (m_disconnectState != DISCONNECTSTATETYPE_SCREENOFF) {
			DEBUG_LOG(("DisconnectManager::allCommandsReady - setting screen state to off."));

			TheDisconnectMenu->hideScreen();
			m_disconnectState = DISCONNECTSTATETYPE_SCREENOFF;
			TheNetwork->notifyOthersOfNewFrame(frame);

			// reset the votes since we're moving to a new frame.
			for (Int i = 0; i < MAX_SLOTS; ++i) {
				m_playerVotes[i][conMgr->getLocalPlayerID()].vote = FALSE;
			}

			DEBUG_LOG(("DisconnectManager::allCommandsReady - resetting m_timeOfDisconnectScreenOn"));
			m_timeOfDisconnectScreenOn = 0;
		}
}

Bool DisconnectManager::allowedToContinue() {
#if !RETAIL_COMPATIBLE_NETWORKING
	if (m_leftPlayerHold == TRUE) {
		return FALSE;
	}
#endif
	if (m_disconnectState != DISCONNECTSTATETYPE_SCREENOFF) {
		return FALSE;
	}
	return TRUE;
}

void DisconnectManager::sendKeepAlive(ConnectionManager *conMgr) {
	time_t curTime = timeGetTime();

	if (((curTime - m_lastKeepAliveSendTime) > 500) || (m_lastKeepAliveSendTime == -1)) {
		NetDisconnectKeepAliveCommandMsg *msg = newInstance(NetDisconnectKeepAliveCommandMsg);
		msg->setPlayerID(conMgr->getLocalPlayerID());
		if (DoesCommandRequireACommandID(msg->getNetCommandType()) == TRUE) {
			msg->setID(GenerateNextCommandID());
		}
		conMgr->sendLocalCommandDirect(msg, 0xff ^ (1 << msg->getPlayerID()));
		msg->detach();

		m_lastKeepAliveSendTime = curTime;
	}
}

void DisconnectManager::populateDisconnectScreen(ConnectionManager *conMgr) {
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		UnicodeString name = conMgr->getPlayerName(i);
		Int slot = translatedSlotPosition(i, conMgr->getLocalPlayerID());
		if (slot != -1) {
			TheDisconnectMenu->setPlayerName(slot, name);

			Int numVotes = countVotesForPlayer(i);
			TheDisconnectMenu->updateVotes(slot, numVotes);
		}
	}
}

Int DisconnectManager::translatedSlotPosition(Int slot, Int localSlot) {
	if (slot < localSlot) {
		return slot;
	}

	if (slot == localSlot) {
		return -1;
	}

	return (slot - 1);
}

Int DisconnectManager::untranslatedSlotPosition(Int slot, Int localSlot) {
	if (slot == -1) {
		return localSlot;
	}

	if (slot < localSlot) {
		return slot;
	}

	return (slot + 1);
}

void DisconnectManager::resetPlayerTimeouts(ConnectionManager *conMgr) {
	// reset the player timeouts.
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		Int slot = translatedSlotPosition(i, conMgr->getLocalPlayerID());
		if (slot != -1) {
			resetPlayerTimeout(slot);
		}
	}
}

void DisconnectManager::resetPlayerTimeout(Int slot) {
	m_playerTimeouts[slot] = timeGetTime();
}

void DisconnectManager::resetPacketRouterTimeout() {
	m_packetRouterTimeout = timeGetTime();
}

void DisconnectManager::turnOnScreen(ConnectionManager *conMgr) {
	TheDisconnectMenu->showScreen();
	DEBUG_LOG(("DisconnectManager::turnOnScreen - turning on screen on frame %d", TheGameLogic->getFrame()));
	m_disconnectState = DISCONNECTSTATETYPE_SCREENON;
	m_lastKeepAliveSendTime = -1;
	populateDisconnectScreen(conMgr);
	resetPlayerTimeouts(conMgr);
	TheDisconnectMenu->hidePacketRouterTimeout();

	m_haveNotifiedOtherPlayersOfCurrentFrame = FALSE;

	m_timeOfDisconnectScreenOn = timeGetTime();
	DEBUG_LOG(("DisconnectManager::turnOnScreen - turned on screen at time %d", m_timeOfDisconnectScreenOn));
}

void DisconnectManager::disconnectPlayer(Int slot, ConnectionManager *conMgr) {
	DEBUG_LOG(("DisconnectManager::disconnectPlayer - Disconnecting slot number %d on frame %d", slot, TheGameLogic->getFrame()));
	DEBUG_ASSERTCRASH((slot >= 0) && (slot < MAX_SLOTS), ("Attempting to disconnect an invalid slot number"));
	if ((slot < 0) || (slot >= (MAX_SLOTS))) {
		return;
	}

	if (TheGameInfo)
	{
		GameSlot *gSlot = TheGameInfo->getSlot( slot );
		if (gSlot)
		{
			gSlot->markAsDisconnected();
		}
	}

	Int transSlot = translatedSlotPosition(slot, conMgr->getLocalPlayerID());

	if (transSlot != -1) {
		// Ignore any disconnect commands that tell us to disconnect ourselves.

		// Get the disconnecting player off the disconnect window.
		UnicodeString uname = conMgr->getPlayerName(slot);
		TheRecorder->logPlayerDisconnect(uname, slot);
		TheDisconnectMenu->removePlayer(transSlot, uname);

		PlayerLeaveCode retcode = conMgr->disconnectPlayer(slot);
		DEBUG_ASSERTCRASH((retcode != PLAYERLEAVECODE_UNKNOWN), ("Invalid player leave code"));

		if (retcode == PLAYERLEAVECODE_PACKETROUTER) {
			DEBUG_LOG(("DisconnectManager::disconnectPlayer - disconnecting player was packet router."));

			conMgr->resendPendingCommands();
		}
	}
}

void DisconnectManager::sendDisconnectCommand(Int slot, ConnectionManager *conMgr) {
	DEBUG_LOG(("DisconnectManager::sendDisconnectCommand - Sending disconnect command for slot number %d", slot));
	DEBUG_ASSERTCRASH((slot >= 0) && (slot < MAX_SLOTS), ("Attempting to send a disconnect command for an invalid slot number"));
	if ((slot < 0) || (slot >= (MAX_SLOTS))) {
		return;
	}

	UnsignedInt disconnectFrame = getMaxDisconnectFrame(conMgr);

	// Need to do the NetDisconnectPlayerCommandMsg creation and sending here.
	NetDisconnectPlayerCommandMsg *msg = newInstance(NetDisconnectPlayerCommandMsg);
	msg->setDisconnectSlot(slot);
	msg->setDisconnectFrame(disconnectFrame);
	msg->setPlayerID(conMgr->getLocalPlayerID());
	if (DoesCommandRequireACommandID(msg->getNetCommandType())) {
		msg->setID(GenerateNextCommandID());
	}

	conMgr->sendLocalCommand(msg);

	DEBUG_LOG(("DisconnectManager::sendDisconnectCommand - Sending disconnect command for slot number %d for frame %d", slot, disconnectFrame));

	msg->detach();
}

void DisconnectManager::sendVoteCommand(Int slot, ConnectionManager *conMgr) {
	NetDisconnectVoteCommandMsg *msg = newInstance(NetDisconnectVoteCommandMsg);

	msg->setPlayerID(conMgr->getLocalPlayerID());
	msg->setSlot(slot);
	msg->setVoteFrame(TheGameLogic->getFrame());
	if (DoesCommandRequireACommandID(msg->getNetCommandType()) == TRUE) {
		msg->setID(GenerateNextCommandID());
	}

	conMgr->sendLocalCommandDirect(msg, 0xff & ~(1 << conMgr->getLocalPlayerID()));

	msg->detach();
}

void DisconnectManager::voteForPlayerDisconnect(Int slot, ConnectionManager *conMgr) {
	Int transSlot = untranslatedSlotPosition(slot, conMgr->getLocalPlayerID());

	if (m_playerVotes[transSlot][conMgr->getLocalPlayerID()].vote == FALSE) {
		m_playerVotes[transSlot][conMgr->getLocalPlayerID()].vote = TRUE;

		sendVoteCommand(transSlot, conMgr);

		// we use the game logic frame cause we might not have sent out our own disconnect frame yet.
		applyDisconnectVote(transSlot, TheGameLogic->getFrame(), conMgr->getLocalPlayerID(), conMgr);
	}
}

void DisconnectManager::recalculatePacketRouterIndex(ConnectionManager *conMgr) {
	Int currentPacketRouterSlot = conMgr->getPacketRouterSlot();
	m_currentPacketRouterIndex = 0;
	while ((currentPacketRouterSlot != conMgr->getPacketRouterFallbackSlot(m_currentPacketRouterIndex)) && (m_currentPacketRouterIndex < MAX_SLOTS)) {
		++m_currentPacketRouterIndex;
	}
	DEBUG_ASSERTCRASH((m_currentPacketRouterIndex < MAX_SLOTS), ("Invalid packet router index"));
}

Bool DisconnectManager::allOnSameFrame(ConnectionManager *conMgr) {
	Bool retval = TRUE;
	for (Int i = 0; (i < MAX_SLOTS) && (retval == TRUE); ++i) {
		Int transSlot = translatedSlotPosition(i, conMgr->getLocalPlayerID());
		if (transSlot == -1) {
			continue;
		}
		if ((conMgr->isPlayerConnected(i) == TRUE) && (isPlayerInGame(transSlot, conMgr) == TRUE)) {
			// ok, i is someone who is in the game and hasn't timed out yet or been voted out.
			if (m_disconnectFramesReceived[i] == FALSE) {
				// we don't know what frame they are on yet.
				retval = FALSE;
			}
			if ((m_disconnectFramesReceived[i] == TRUE) && (m_disconnectFrames[conMgr->getLocalPlayerID()] != m_disconnectFrames[i])) {
				// We know their frame, but they aren't on the same frame as us.
				retval = FALSE;
			}
		}
	}
	return retval;
}

#if !RETAIL_COMPATIBLE_NETWORKING
Int DisconnectManager::getActingPacketRouterSlot(ConnectionManager *conMgr, Int removedSlot) {
	UnsignedInt localSlot = conMgr->getLocalPlayerID();
	UnsignedInt packetRouterSlot = conMgr->getPacketRouterSlot();
	Int transSlot = translatedSlotPosition(packetRouterSlot, localSlot);
	while ((transSlot != -1) && ((isPlayerInGame(transSlot, conMgr) == FALSE) || ((Int)packetRouterSlot == removedSlot))) {
		packetRouterSlot = conMgr->getNextPacketRouterSlot(packetRouterSlot);
		if (packetRouterSlot >= MAX_SLOTS) {
			return -1;
		}
		transSlot = translatedSlotPosition(packetRouterSlot, localSlot);
	}
	return (Int)packetRouterSlot;
}

Bool DisconnectManager::hasPlayerLeft(Int slot) {
	if (m_ownLeaveReceived[slot] == TRUE) {
		return TRUE;
	}
	// a false notice can't remove a player that is still around, it keeps sending keepalives
	return (m_playerLeft[slot] == TRUE) && ((timeGetTime() - m_playerTimeouts[slot]) > LEAVE_NOTICE_QUIET_TIME);
}

Bool DisconnectManager::isLeftPlayerConnected(ConnectionManager *conMgr) {
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		Int transSlot = translatedSlotPosition(i, conMgr->getLocalPlayerID());
		if ((transSlot != -1) && (m_playerLeft[transSlot] == TRUE) && (conMgr->isPlayerConnected(i) == TRUE)) {
			return TRUE;
		}
	}
	return FALSE;
}

void DisconnectManager::processHeldRemovals(ConnectionManager *conMgr) {
	for (Int slot = 0; slot < MAX_SLOTS; ++slot) {
		const Int from = m_heldRemovalFrom[slot];
		if (from == -1) {
			continue;
		}
		if ((conMgr->isPlayerConnected(slot) == FALSE) || (conMgr->isPlayerConnected(from) == FALSE)) {
			m_heldRemovalFrom[slot] = -1;
			continue;
		}
		if ((from == getActingPacketRouterSlot(conMgr, slot)) && (TheGameLogic->getFrame() == m_heldRemovalFrame[slot])) {
			DEBUG_LOG(("DisconnectManager::processHeldRemovals - player %d is the acting packet router now, removing player %d", from, slot));
			m_heldRemovalFrom[slot] = -1;
			disconnectPlayer(slot, conMgr);
		}
	}
}

#endif

Bool DisconnectManager::isLocalPlayerNextPacketRouter(ConnectionManager *conMgr) {
	UnsignedInt localSlot = conMgr->getLocalPlayerID();
	UnsignedInt packetRouterSlot = conMgr->getPacketRouterSlot();
	Int transSlot = translatedSlotPosition(packetRouterSlot, localSlot);

	// stop when we have found a packet router that is connected
	while ((transSlot != -1) && (isPlayerInGame(transSlot, conMgr) == FALSE)) {
		packetRouterSlot = conMgr->getNextPacketRouterSlot(packetRouterSlot);
		if ((packetRouterSlot >= MAX_SLOTS) || (packetRouterSlot < 0)) {
			// don't know who the next packet router is going to be,
			// so this game is not going to go anywhere anymore.
			DEBUG_CRASH(("no more players left to be the packet router, this shouldn't happen."));
			return FALSE;
		}
		transSlot = translatedSlotPosition(packetRouterSlot, localSlot);
	}

	if (packetRouterSlot == localSlot) {
		return TRUE;
	}

	return FALSE;
}

Bool DisconnectManager::hasPlayerTimedOut(Int slot) {
	if (slot == -1) {
		return FALSE;
	}

#if !RETAIL_COMPATIBLE_NETWORKING
	if (hasPlayerLeft(slot) == TRUE) {
		return TRUE;
	}
#endif

	time_t newTime = TheGlobalData->m_networkPlayerTimeoutTime - (timeGetTime() - m_playerTimeouts[slot]);
	if (newTime <= 0) {
		return TRUE;
	}

	return FALSE;
}

// this function assumes that we are the packet router. (or at least that
// we will be after everyone is getting disconnected)
void DisconnectManager::sendPlayerDestruct(Int slot, ConnectionManager *conMgr) {
	if ((slot < 0) || (slot >= MAX_SLOTS) || m_destroySent[slot]) {
		return;
	}
	m_destroySent[slot] = TRUE;

	UnsignedShort currentID = 0;
	if (DoesCommandRequireACommandID(NETCOMMANDTYPE_DESTROYPLAYER))
	{
		currentID = GenerateNextCommandID();
	}

	DEBUG_LOG(("Queueing DestroyPlayer %d for frame %d on frame %d as command %d",
		slot, TheNetwork->getExecutionFrame()+1, TheGameLogic->getFrame(), currentID));

	NetDestroyPlayerCommandMsg *netmsg = newInstance(NetDestroyPlayerCommandMsg);
	netmsg->setExecutionFrame(TheNetwork->getExecutionFrame()+1);
	netmsg->setPlayerID(conMgr->getLocalPlayerID());
	netmsg->setID(currentID);
	netmsg->setPlayerIndex(slot);
	conMgr->sendLocalCommand(netmsg);
	netmsg->detach();
}

// the 'slot' variable is supposed to be a translated slot position. (translated slot meaning
// that it is the player's position in the disconnect menu)
Bool DisconnectManager::isPlayerVotedOut(Int slot, ConnectionManager *conMgr) {
	if (slot == -1) {
		// we can't vote out ourselves.
		return FALSE;
	}
	Int transSlot = untranslatedSlotPosition(slot, conMgr->getLocalPlayerID());
	Int numVotes = countVotesForPlayer(transSlot);
	Int numVoters = conMgr->getNumPlayers() - 1;
#if !RETAIL_COMPATIBLE_NETWORKING
	// a player that has left is on its way out, it is not needed as a voter and its vote does not count
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		Int leftSlot = translatedSlotPosition(i, conMgr->getLocalPlayerID());
		if ((i != transSlot) && (leftSlot != -1) && (hasPlayerLeft(leftSlot) == TRUE) && (conMgr->isPlayerConnected(i) == TRUE)) {
			--numVoters;
			if ((m_playerVotes[transSlot][i].vote == TRUE) && (m_playerVotes[transSlot][i].frame == TheGameLogic->getFrame())) {
				--numVotes;
			}
		}
	}
	if (numVoters < 1) {
		numVoters = 1;
	}
#endif
	if (numVotes >= numVoters) {
		return TRUE;
	}
	return FALSE;
}

UnsignedInt DisconnectManager::getMaxDisconnectFrame(ConnectionManager *conMgr) {
	UnsignedInt retval = 0;
	for (Int i = 0; i < MAX_SLOTS; ++i) {
#if !RETAIL_COMPATIBLE_NETWORKING
		// a player that has left reports the frame it stopped on, which the others may never reach
		Int transSlot = translatedSlotPosition(i, conMgr->getLocalPlayerID());
		if ((transSlot != -1) && (m_playerLeft[transSlot] == TRUE)) {
			continue;
		}
#endif
		if (m_disconnectFrames[i] > retval) {
			retval = m_disconnectFrames[i];
		}
	}
	return retval;
}

Bool DisconnectManager::isPlayerInGame(Int slot, ConnectionManager *conMgr) {
	Int transSlot = untranslatedSlotPosition(slot, conMgr->getLocalPlayerID());
	DEBUG_ASSERTCRASH((transSlot >= 0) && (transSlot < MAX_SLOTS), ("invalid slot number"));
	if (((transSlot < 0) || (transSlot >= MAX_SLOTS)) || conMgr->isPlayerConnected(transSlot) == FALSE) {
		return FALSE;
	}

	if (isPlayerVotedOut(slot, conMgr) == TRUE) {
		return FALSE;
	}

	if (hasPlayerTimedOut(slot) == TRUE) {
		return FALSE;
	}

	return TRUE;
}

void DisconnectManager::playerHasAdvancedAFrame(Int slot, UnsignedInt frame) {
	// if they have advanced beyond the frame they had been previously disconnecting on.
	if (frame >= m_disconnectFrames[slot]) {
		m_disconnectFrames[slot] = frame; // just in case we get a disconnect frame command after this is called.
		m_disconnectFramesReceived[slot] = FALSE;
	}
}

Int DisconnectManager::countVotesForPlayer(Int slot) {
	if ((slot < 0) || (slot >= MAX_SLOTS)) {
		return 0;
	}

	Int retval = 0;
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		// using TheGameLogic->getFrame() cause we might not have sent our disconnect frame yet.
		if ((m_playerVotes[slot][i].vote == TRUE) && (m_playerVotes[slot][i].frame == TheGameLogic->getFrame())) {
			++retval;
		}
	}

	return retval;
}

void DisconnectManager::resetPlayersVotes(Int playerID, UnsignedInt frame, ConnectionManager *conMgr) {
	DEBUG_LOG(("DisconnectManager::resetPlayersVotes - resetting player %d's votes on frame %d", playerID, frame));

	// we need to reset this player's votes that happened before or on the given frame.
	for(Int i = 0; i < MAX_SLOTS; ++i) {
		if (m_playerVotes[i][playerID].frame <= frame) {
			DEBUG_LOG(("DisconnectManager::resetPlayersVotes - resetting player %d's vote for player %d from frame %d on frame %d", playerID, i, m_playerVotes[i][playerID].frame, frame));
			m_playerVotes[i][playerID].vote = FALSE;
		}
	}

	Int numVotes = countVotesForPlayer(playerID);
	DEBUG_LOG(("DisconnectManager::resetPlayersVotes - after adjusting votes, player %d has %d votes", playerID, numVotes));
	Int transSlot = translatedSlotPosition(playerID, conMgr->getLocalPlayerID());
	if (transSlot != -1) {
		TheDisconnectMenu->updateVotes(transSlot, numVotes);
	}
}
