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

#include "Compression.h"
#include "strtok_r.h"
#include "Common/AudioEventRTS.h"
#include "Common/CRCDebug.h"
#include "Common/Debug.h"
#include "Common/file.h"
#include "Common/GameAudio.h"
#include "Common/LocalFileSystem.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/RandomValue.h"
#include "Common/Recorder.h"

#include "GameClient/Diplomacy.h"
#include "GameClient/GameText.h"
#include "GameClient/MessageBox.h"
#include "GameNetwork/ConnectionManager.h"
#include "GameNetwork/LANAPICallbacks.h"
#include "GameNetwork/NAT.h"
#include "GameNetwork/NetCommandWrapperList.h"
#include "GameNetwork/networkutil.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/ScriptActions.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/VictoryConditions.h"
#include "GameClient/DisconnectMenu.h"
#include "GameClient/InGameUI.h"

static Bool hasValidTransferFileExtension(const AsciiString& filePath)
{
	static const char* const validExtensions[] = {
		"map",
		"ini",
		"str",
		"wak",
		"tga",
		"txt"
	};

	const char* fileExt = strrchr(filePath.str(), '.');

	if (fileExt == nullptr || fileExt[1] == '\0')
	{
		return false;
	}

	fileExt++;

	for (Int i = 0; i < ARRAY_SIZE(validExtensions); ++i)
	{
		if (stricmp(fileExt, validExtensions[i]) == 0)
		{
			return true;
		}
	}

	return false;
}

/**
 * Le destructor.
 */
ConnectionManager::~ConnectionManager(void)
{
	deleteInstance(m_localUser);
	m_localUser = nullptr;

	// Network will delete transports; we just forget them
	delete m_transport;
	m_transport = nullptr;

	Int i = 0;
	for (; i < MAX_SLOTS; ++i) {
		deleteInstance(m_frameData[i]);
		m_frameData[i] = nullptr;
	}

	for (i = 0; i < NUM_CONNECTIONS; ++i) {
		deleteInstance(m_connections[i]);
		m_connections[i] = nullptr;
	}

	// This is done here since TheDisconnectMenu should only be there if we are in a network game.
	delete TheDisconnectMenu;
	TheDisconnectMenu = nullptr;

	delete m_disconnectManager;
	m_disconnectManager = nullptr;

	deleteInstance(m_pendingCommands);
	m_pendingCommands = nullptr;

	deleteInstance(m_relayedCommands);
	m_relayedCommands = nullptr;

	deleteInstance(m_netCommandWrapperList);
	m_netCommandWrapperList = nullptr;

	s_fileCommandMap.clear();
	s_fileRecipientMaskMap.clear();
	for (i = 0; i < MAX_SLOTS; ++i) {
		s_fileProgressMap[i].clear();
	}
}

/**
 * Le constructor
 */
ConnectionManager::ConnectionManager(void)
{
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		m_frameData[i] = nullptr;
	}
	m_transport = nullptr;
	m_disconnectManager = nullptr;
	m_pendingCommands = nullptr;
	m_relayedCommands = nullptr;
	m_localAddr = 0;
	m_localPort = 0;
	m_netCommandWrapperList = nullptr;
	m_localUser = nullptr;
	m_localUser = newInstance(User);
}

/**
 * Initialize the connection manager and any subsystems.
 */
void ConnectionManager::init()
{
//	if (m_transport == nullptr) {
//		m_transport = new Transport;
//	}
//	m_transport->reset();

	UnsignedInt i = 0;
	for (; i < NUM_CONNECTIONS; ++i) {
		m_connections[i] = nullptr;
	}

	if (m_pendingCommands == nullptr) {
		m_pendingCommands = newInstance(NetCommandList);
		m_pendingCommands->init();
	}
	m_pendingCommands->reset();

	if (m_relayedCommands == nullptr) {
		m_relayedCommands = newInstance(NetCommandList);
		m_relayedCommands->init();
	}
	m_relayedCommands->reset();

	m_localSlot = -1;
#ifdef MEMORYPOOL_DEBUG
	TheMemoryPoolFactory->debugSetInitFillerIndex(m_localSlot);
#endif
	m_packetRouterSlot = 0; /// @todo The LAN/WOL interface should be telling us who the packet router is based on machine specs passed around through game options.
	for (i = 0; i < MAX_SLOTS; ++i) {
		m_packetRouterFallback[i] = -1;
	}

	for (i = 0; i < MAX_SLOTS; ++i) {
		deleteInstance(m_frameData[i]);
		m_frameData[i] = nullptr;
	}

//	m_averageFps = 30;			// since 30 fps is the desired rate, we'll start off at that.
//	m_averageLatency = (Real)0.2; // 200ms seems like a good starting point.

	for (i = 0; i < MAX_SLOTS; ++i) {
		m_fpsAverages[i] = -1;
	}
	for (i = 0; i < MAX_SLOTS; ++i) {
		m_latencyAverages[i] = 0.0; // using zero since all floating point standards should be able to specify 0.0 accurately.
	}
	m_smallestPacketArrivalCushion = -1;

	m_frameMetrics.init();

	TheDisconnectMenu = NEW DisconnectMenu;
	TheDisconnectMenu->init();

	m_disconnectManager = NEW DisconnectManager;
	m_disconnectManager->init();

	TheDisconnectMenu->attachDisconnectManager(m_disconnectManager);
	TheDisconnectMenu->hideScreen();

	m_netCommandWrapperList = newInstance(NetCommandWrapperList);
	m_netCommandWrapperList->init();

	s_fileCommandMap.clear();
	s_fileRecipientMaskMap.clear();
	for (i = 0; i < MAX_SLOTS; ++i) {
		s_fileProgressMap[i].clear();
	}
}

/**
 * Reset the connection manager and any subsystems.
 */
void ConnectionManager::reset()
{
//	if (m_transport == nullptr) {
//		m_transport = new Transport;
//	}
//	m_transport->reset();

	delete m_transport;
	m_transport = nullptr;

	UnsignedInt i = 0;
	for (; i < (UnsignedInt)NUM_CONNECTIONS; ++i) {
		deleteInstance(m_connections[i]);
		m_connections[i] = nullptr;
	}

	for (i=0; i<(UnsignedInt)MAX_SLOTS; ++i)
	{
		deleteInstance(m_frameData[i]);
		m_frameData[i] = nullptr;
	}

	if (m_pendingCommands == nullptr) {
		m_pendingCommands = newInstance(NetCommandList);
		m_pendingCommands->init();
	}
	m_pendingCommands->reset();

	if (m_relayedCommands == nullptr) {
		m_relayedCommands = newInstance(NetCommandList);
		m_relayedCommands->init();
	}
	m_relayedCommands->reset();

	if (m_netCommandWrapperList == nullptr) {
		m_netCommandWrapperList = newInstance(NetCommandWrapperList);
		m_netCommandWrapperList->init();
	}
	m_netCommandWrapperList->reset();

	m_localSlot = -1;
#ifdef MEMORYPOOL_DEBUG
	TheMemoryPoolFactory->debugSetInitFillerIndex(m_localSlot);
#endif
	m_packetRouterSlot = -1;

	for (i = 0; i < TheGlobalData->m_networkFPSHistoryLength; ++i) {
		m_fpsAverages[i] = -1;
	}
	for (i = 0; i < TheGlobalData->m_networkLatencyHistoryLength; ++i) {
		m_latencyAverages[i] = 0.0;
	}

	for (i = 0; i < (UnsignedInt)MAX_SLOTS; ++i) {
		m_packetRouterFallback[i] = -1;
	}

	m_frameMetrics.reset();
}

UnsignedInt ConnectionManager::getPingFrame()
{
	return (m_disconnectManager)?m_disconnectManager->getPingFrame():0;
}

Int ConnectionManager::getPingsSent()
{
	return (m_disconnectManager)?m_disconnectManager->getPingsSent():0;
}

Int ConnectionManager::getPingsReceived()
{
	return (m_disconnectManager)?m_disconnectManager->getPingsReceived():0;
}

Bool ConnectionManager::isPlayerConnected( Int playerID )
{
	return ( playerID == m_localSlot || (m_connections[playerID] && !m_connections[playerID]->isQuitting()) );
}

void ConnectionManager::attachTransport(Transport *transport) {
	delete m_transport;
	m_transport = transport;
}

/**
 * zero out the command counts for the given frames.  Presently this is used for
 * the start of a game since there won't be any commands for the first few frames due to runahead.
 */
void ConnectionManager::zeroFrames(UnsignedInt startingFrame, UnsignedInt numFrames) {
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if (m_frameData[i] != nullptr) {
//			DEBUG_LOG(("Calling zeroFrames on player %d, starting frame %d, numFrames %d", i, startingFrame, numFrames));
			m_frameData[i]->zeroFrames(startingFrame, numFrames);
		}
	}
}

/**
 * Destroy any game messages that are left over due to the run ahead.
 */
void ConnectionManager::destroyGameMessages() {
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		// Need to destroy these game messages because when the game ends, there are
		// still some game messages left over because of the run ahead aspect of
		// network play.
		if (m_frameData[i] != nullptr) {
			m_frameData[i]->destroyGameMessages();
		}
	}
}

/**
 * ConnectionManager::doRelay()
 * Queries the transport for commands that need to be relayed to another client.
 * Get those commands and relay them to the appropriate Connection(s). We make the
 * assumption that a command will only be relayed once.
 */
void ConnectionManager::doRelay() {
	static Int numPackets = 0;
	static Int numCommands = 0;

	NetPacket *packet = nullptr;

	for (Int i = 0; i < MAX_MESSAGES; ++i) {
		if (m_transport->m_inBuffer[i].length != 0) {
			// This transport buffer has yet to be processed.

			// make a NetPacket out of this data so it can be broken up into individual commands.
			packet = newInstance(NetPacket)(&(m_transport->m_inBuffer[i]));

			//DEBUG_LOG(("ConnectionManager::doRelay() - got a packet with %d commands", packet->getNumCommands()));
			//LOGBUFFER( packet->getData(), packet->getLength() );

			// Get the command list from the packet.
			NetCommandList *cmdList = packet->getCommandList();
			NetCommandRef *cmd = cmdList->getFirstMessage();

			// Iterate through the commands in this packet and send them to the proper connections.
			while (cmd != nullptr) {
				//DEBUG_LOG(("ConnectionManager::doRelay() - Looking at a command of type %s",
					//GetNetCommandTypeAsString(cmd->getCommand()->getNetCommandType())));
				if (CommandRequiresAck(cmd->getCommand())) {
					ackCommand(cmd, m_localSlot);
				}
				if (!processNetCommand(cmd)) {
					sendRemoteCommand(cmd);
				}
				cmd = cmd->getNext();

				++numCommands;
			}
			++numPackets;

			// Delete this packet since we won't be needing it anymore.
			deleteInstance(packet);
			packet = nullptr;

			deleteInstance(cmdList);
			cmdList = nullptr;

			// signal that this has been processed.
			m_transport->m_inBuffer[i].length = 0;
		}
	}

	NetCommandList *cmdList = m_netCommandWrapperList->getReadyCommands();
	NetCommandRef *cmd = cmdList->getFirstMessage();
	while (cmd != nullptr) {
		if (CommandRequiresAck(cmd->getCommand())) {
			ackCommand(cmd, m_localSlot);
		}
		if (!processNetCommand(cmd)) {
			sendRemoteCommand(cmd);
		}
		cmd = cmd->getNext();

		++numCommands;
	}
	++numPackets;

	// Delete this packet since we won't be needing it anymore.
	deleteInstance(packet);
	packet = nullptr;

	deleteInstance(cmdList);
	cmdList = nullptr;
}

/**
 * This is where the non-synchronized network commands should be processed.
 * Return TRUE if the command should not be relayed. Return FALSE if it should be relayed.
 */
Bool ConnectionManager::processNetCommand(NetCommandRef *ref) {
	NetCommandMsg *msg = ref->getCommand();
	NetCommandType cmdType = msg->getNetCommandType();

	// Handle ACK commands first (before connection validation)
	if ((cmdType == NETCOMMANDTYPE_ACKSTAGE1) ||
			(cmdType == NETCOMMANDTYPE_ACKSTAGE2) ||
			(cmdType == NETCOMMANDTYPE_ACKBOTH)) {
		processAck(msg);
		return FALSE;
	}

	// Early validation checks
	if ((m_connections[msg->getPlayerID()] == nullptr) && (msg->getPlayerID() != m_localSlot)) {
		// if this is from a player that is no longer in the game, then ignore them.
		return TRUE;
	}

	// Handle WRAPPER commands (before second connection validation)
	if (cmdType == NETCOMMANDTYPE_WRAPPER) {
		processWrapper(ref); // need to send the NetCommandRef since we have to construct the relay for the wrapped command.
		return FALSE;
	}

	if ((msg->getPlayerID() >= 0) && (msg->getPlayerID() < MAX_SLOTS) && (msg->getPlayerID() != m_localSlot)) {
		if (m_connections[msg->getPlayerID()] == nullptr) {
			return TRUE;
		}
	}

	// Don't allow an out of date command to be sent through.
	// Its unnecessary traffic and it could cause problems.
	//
	// This was a fix for a command count bug where a command would be
	// executed, then a command for that old frame would be added to the
	// FrameData for that frame + 256, and would screw up the command count.
	if (IsCommandSynchronized(cmdType)) {
		if (ref->getCommand()->getExecutionFrame() < TheGameLogic->getFrame()) {
			return TRUE;
		}
	}

	// Handle disconnect commands as a range
	if ((cmdType > NETCOMMANDTYPE_DISCONNECTSTART) && (cmdType < NETCOMMANDTYPE_DISCONNECTEND)) {
		m_disconnectManager->processDisconnectCommand(ref, this);
		return TRUE;
	}

	// Process command by type
	switch (cmdType) {

		case NETCOMMANDTYPE_FRAMEINFO: {
			processFrameInfo((NetFrameCommandMsg *)msg);
			// need to set the relay so we don't send it to ourselves.
			UnsignedByte relay = ref->getRelay();
			relay = relay & (0xff ^ (1 << m_localSlot));
			ref->setRelay(relay);
			return FALSE;
		}

		case NETCOMMANDTYPE_PROGRESS: {
			//DEBUG_LOG(("ConnectionManager::processNetCommand - got a progress net command from player %d", msg->getPlayerID()));
			processProgress((NetProgressCommandMsg *) msg);
			// need to set the relay so we don't send it to ourselves.
			UnsignedByte relay = ref->getRelay();
			relay = relay & (0xff ^ (1 << m_localSlot));
			ref->setRelay(relay);
			return FALSE;
		}

		case NETCOMMANDTYPE_TIMEOUTSTART:
			DEBUG_LOG(("ConnectionManager::processNetCommand - got a TimeOut GameStart net command from player %d", msg->getPlayerID()));
			processTimeOutGameStart(msg);
			return FALSE;

		case NETCOMMANDTYPE_RUNAHEADMETRICS:
			processRunAheadMetrics((NetRunAheadMetricsCommandMsg *)msg);
			return TRUE;

		case NETCOMMANDTYPE_KEEPALIVE:
			return TRUE;

		case NETCOMMANDTYPE_DISCONNECTCHAT:
			processDisconnectChat((NetDisconnectChatCommandMsg *)msg);
			return FALSE;

		case NETCOMMANDTYPE_LOADCOMPLETE:
			DEBUG_LOG(("ConnectionManager::processNetCommand - got a Load Complete net command from player %d", msg->getPlayerID()));
			processLoadComplete(msg);
			return FALSE;

		case NETCOMMANDTYPE_CHAT:
			processChat((NetChatCommandMsg *)msg);
			return FALSE;

		case NETCOMMANDTYPE_FILE:
			processFile((NetFileCommandMsg *)msg);
			return FALSE;

		case NETCOMMANDTYPE_FILEANNOUNCE:
			processFileAnnounce((NetFileAnnounceCommandMsg *)msg);
			return FALSE;

		case NETCOMMANDTYPE_FILEPROGRESS:
			processFileProgress((NetFileProgressCommandMsg *)msg);
			return FALSE;

		case NETCOMMANDTYPE_FRAMERESENDREQUEST:
			processFrameResendRequest((NetFrameResendRequestCommandMsg *)msg);
			return TRUE;

		default:
			return FALSE;
	}
}

void ConnectionManager::processFrameResendRequest(NetFrameResendRequestCommandMsg *msg) {
	// first make sure this is a valid slot
	Int playerID = msg->getPlayerID();
	if ((playerID < 0) || (playerID >= MAX_SLOTS)) {
		return;
	}

	// make sure this player is still in our game.
	if ((m_connections[playerID] == nullptr) || (m_connections[playerID]->isQuitting() == TRUE)) {
		return;
	}

	sendFrameDataToPlayer(playerID, msg->getFrameToResend());
}

/**
 * We have received a wrapper for a command too big to fit in a packet.
 */
void ConnectionManager::processWrapper(NetCommandRef *ref)
{
	NetWrapperCommandMsg *wrapperMsg = (NetWrapperCommandMsg *)(ref->getCommand());
	UnsignedShort commandID = wrapperMsg->getWrappedCommandID();
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::processWrapper() - wrapped commandID is %d, commandID is %d",
		commandID, wrapperMsg->getID()));
	Int origProgress = 0;
	FileCommandMap::iterator fcIt = s_fileCommandMap.find(commandID);
	if (fcIt != s_fileCommandMap.end())
	{
		origProgress = s_fileProgressMap[m_localSlot][commandID];
	}
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::processWrapper() - origProgress[%d] == %d for command %d",
		m_localSlot, origProgress, commandID));

	m_netCommandWrapperList->processWrapper(ref);

	if (fcIt != s_fileCommandMap.end())
	{
		Int newProgress = m_netCommandWrapperList->getPercentComplete(commandID);
		DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::processWrapper() - newProgress[%d] == %d for command %d",
			m_localSlot, newProgress, commandID));
		if (newProgress > origProgress && newProgress < 100)
		{
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::processWrapper() - sending a NetFileProgressCommandMsg"));
			s_fileProgressMap[m_localSlot][commandID] = newProgress;

			Int progressMask = 0xff ^ (1 << m_localSlot);
			NetFileProgressCommandMsg *msg = newInstance(NetFileProgressCommandMsg);
			msg->setPlayerID(m_localSlot);
			msg->setID(0);
			if (DoesCommandRequireACommandID(msg->getNetCommandType()))
			{
				msg->setID(GenerateNextCommandID());
			}
			msg->setFileID(commandID);
			msg->setProgress(newProgress);
			sendLocalCommand(msg, progressMask);
			processFileProgress(msg);
			msg->detach();
		}
	}
}

/**
 * A client has sent us their run ahead metrics, lets store them away for future calculations.
 */
void ConnectionManager::processRunAheadMetrics(NetRunAheadMetricsCommandMsg *msg)
{
	UnsignedInt player = msg->getPlayerID();
	if ((player >= 0) && (player < MAX_SLOTS) && (isPlayerConnected(player))) {
		m_latencyAverages[player] = msg->getAverageLatency();
		m_fpsAverages[player] = msg->getAverageFps();
		//DEBUG_LOG(("ConnectionManager::processRunAheadMetrics - player %d, fps = %d, latency = %f", player, msg->getAverageFps(), msg->getAverageLatency()));
		if (m_fpsAverages[player] > 100) {
			// limit the reported frame rate average to 100.  This is done because if a
			// user alt-tab's out of the game their frame rate climbs to in the neighborhood of
			// 300, that was deemed "ugly" by the powers that be.
			m_fpsAverages[player] = 100;
		}
	}
}

void ConnectionManager::processDisconnectChat(NetDisconnectChatCommandMsg *msg)
{
	UnicodeString unitext;
	UnicodeString name;
	UnsignedByte playerID = msg->getPlayerID();
	if (playerID == m_localSlot) {
		name = m_localUser->GetName();
	} else if (isPlayerConnected(playerID)) {
		name = m_connections[playerID]->getUser()->GetName();
	}
	unitext.format(L"[%ls] %ls", name.str(), msg->getText().str());
//	DEBUG_LOG(("ConnectionManager::processDisconnectChat - got message from player %d, message is %ls", playerID, unitext.str()));
	TheDisconnectMenu->showChat(unitext); // <-- need to implement this
}

void ConnectionManager::processChat(NetChatCommandMsg *msg)
{
	UnicodeString unitext;
	UnicodeString name;
	UnsignedByte playerID = msg->getPlayerID();
	//DEBUG_LOG(("processChat(): playerID = %d", playerID));
	if (playerID == m_localSlot) {
		name = m_localUser->GetName();
		//DEBUG_LOG(("connection is null, using %ls", name.str()));
	} else if (((m_connections[playerID] != nullptr) && (m_connections[playerID]->isQuitting() == FALSE))) {
		name = m_connections[playerID]->getUser()->GetName();
		//DEBUG_LOG(("connection is non-null, using %ls", name.str()));
	}
	unitext.format(L"[%ls] %ls", name.str(), msg->getText().str());
//	DEBUG_LOG(("ConnectionManager::processChat - got message from player %d (mask %8.8X), message is %ls", playerID, msg->getPlayerMask(), unitext.str()));

	AsciiString playerName;
	playerName.format("player%d", msg->getPlayerID());
	const Player *player = ThePlayerList->findPlayerWithNameKey( TheNameKeyGenerator->nameToKey( playerName ) );
	if (!player)
	{
		TheInGameUI->message(L"%ls", unitext.str());
		return;
	}

	Bool fromObserver = !player->isPlayerActive();
	Bool amIObserver = !ThePlayerList->getLocalPlayer()->isPlayerActive();
	Bool canSeeChat = (amIObserver || !fromObserver) && !TheGameInfo->getConstSlot(playerID)->isMuted();

	if ( ((1<<m_localSlot) & msg->getPlayerMask() ) && canSeeChat  )
	{
		RGBColor rgb;
		rgb.setFromInt(player->getPlayerColor());
		TheInGameUI->messageColor(&rgb, L"%ls", unitext.str());

		// feedback for received chat messages in-game
		AudioEventRTS audioEvent("GUICommunicatorIncoming");
		TheAudio->addAudioEvent(&audioEvent);
	}
}

void ConnectionManager::processFile(NetFileCommandMsg *msg)
{
#ifdef DEBUG_LOGGING
	UnicodeString log;
	log.format(L"Saw file transfer: '%hs' of %d bytes from %d", msg->getPortableFilename().str(), msg->getFileLength(), msg->getPlayerID());
	DEBUG_LOG(("%ls", log.str()));
#endif

	AsciiString realFileName = msg->getRealFilename();
	if (realFileName.isEmpty())
	{
		// TheSuperHackers @security slurmlord 18/06/2025 As the file name/path from the NetFileCommandMsg failed to normalize,
		// in other words is bogus and points outside of the approved target directory, avoid an arbitrary file overwrite vulnerability
		// by simply returning and let the transfer time out.
		DEBUG_LOG(("Got a file name transferred that failed to normalize: '%s'!", msg->getPortableFilename().str()));
		return;
	}

	// TheSuperHackers @security bobtista 06/11/2025 Validate file extension to prevent arbitrary file types
	if (!hasValidTransferFileExtension(realFileName))
	{
		DEBUG_LOG(("File '%s' has invalid extension for transfer operations.", realFileName.str()));
		return;
	}

	if (TheFileSystem->doesFileExist(realFileName.str()))
	{
		DEBUG_LOG(("File exists already!"));
		//return;
	}

	UnsignedByte *buf = msg->getFileData();
	Int len = msg->getFileLength();

	// uncompress Targas
#ifdef COMPRESS_TARGAS
	Bool deleteBuf = FALSE;
	if (msg->getFilename().endsWith(".tga") && CompressionManager::isDataCompressed(buf, len))
	{
		Int uncompLen = CompressionManager::getUncompressedSize(buf, len);
		UnsignedByte *uncompBuffer = NEW UnsignedByte[uncompLen];
		Int actualLen = CompressionManager::decompressData(buf, len, uncompBuffer, uncompLen);
		if (actualLen == uncompLen)
		{
			DEBUG_LOG(("Uncompressed Targa after map transfer"));
			deleteBuf = TRUE;
			buf = uncompBuffer;
			len = uncompLen;
		}
		else
		{
			DEBUG_LOG(("Failed to uncompress Targa after map transfer"));
			delete[] uncompBuffer; // failed to decompress, so just use the source
		}
	}
#endif // COMPRESS_TARGAS

	File *fp = TheFileSystem->openFile(realFileName.str(), File::CREATE | File::BINARY | File::WRITE);
	if (fp)
	{
		fp->write(buf, len);
		fp->close();
		fp = nullptr;
		DEBUG_LOG(("Wrote %d bytes to file %s!", len, realFileName.str()));

	}
	else
	{
		DEBUG_LOG(("Cannot open file!"));
	}

	DEBUG_LOG(("ConnectionManager::processFile() - sending a NetFileProgressCommandMsg"));

	Int commandID = msg->getID();
	Int newProgress = 100;

	s_fileProgressMap[m_localSlot][commandID] = newProgress;

	Int progressMask = 0xff ^ (1 << m_localSlot);
	NetFileProgressCommandMsg *progressMsg = newInstance(NetFileProgressCommandMsg);
	progressMsg->setPlayerID(m_localSlot);
	progressMsg->setID(0);
	if (DoesCommandRequireACommandID(progressMsg->getNetCommandType()))
	{
		progressMsg->setID(GenerateNextCommandID());
	}
	progressMsg->setFileID(commandID);
	progressMsg->setProgress(newProgress);
	sendLocalCommand(progressMsg, progressMask);
	processFileProgress(progressMsg);
	progressMsg->detach();

#ifdef COMPRESS_TARGAS
	if (deleteBuf)
	{
		delete[] buf;
		buf = nullptr;
	}
#endif // COMPRESS_TARGAS
}

void ConnectionManager::processFileAnnounce(NetFileAnnounceCommandMsg *msg)
{
	DEBUG_LOG(("ConnectionManager::processFileAnnounce() - expecting '%s' (%s) in command %d", msg->getPortableFilename().str(), msg->getRealFilename().str(), msg->getFileID()));
	s_fileCommandMap[msg->getFileID()] = msg->getRealFilename();
	s_fileRecipientMaskMap[msg->getFileID()] = msg->getPlayerMask();
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		if ( (1<<i) & msg->getPlayerMask() )
		{
			s_fileProgressMap[i][msg->getFileID()] = 0;
		}
		else
		{
			s_fileProgressMap[i][msg->getFileID()] = 100; // they don't need to get it, so they're already done.
		}
	}
}

void ConnectionManager::processFileProgress(NetFileProgressCommandMsg *msg)
{
	DEBUG_LOG(("ConnectionManager::processFileProgress() - command %d is at %d%%",
		msg->getFileID(), msg->getProgress()));
	Int oldProgress = s_fileProgressMap[msg->getPlayerID()][msg->getFileID()];

	s_fileProgressMap[msg->getPlayerID()][msg->getFileID()] = max(oldProgress, msg->getProgress());
}

void ConnectionManager::processProgress( NetProgressCommandMsg *msg )
{
	TheGameLogic->processProgress(msg->getPlayerID(), msg->getPercentage());
}

void ConnectionManager::processLoadComplete( NetCommandMsg *msg )
{
	TheGameLogic->processProgressComplete(msg->getPlayerID());
}

void ConnectionManager::processTimeOutGameStart( NetCommandMsg *msg )
{
	TheGameLogic->timeOutGameStart();
}

/**
 * Another client has sent us the command count for a new frame.
 */
void ConnectionManager::processFrameInfo(NetFrameCommandMsg *msg) {
	//stupid frame info, why don't you process yourself?

	UnsignedInt playerID = msg->getPlayerID();

	if ((playerID >= 0) && (playerID < MAX_SLOTS)) {
		if (m_frameData[playerID] != nullptr) {
//			DEBUG_LOG(("ConnectionManager::processFrameInfo - player %d, frame %d, command count %d, received on frame %d", playerID, msg->getExecutionFrame(), msg->getCommandCount(), TheGameLogic->getFrame()));
			m_frameData[playerID]->setFrameCommandCount(msg->getExecutionFrame(), msg->getCommandCount());
		}
	}
}

/**
 * We just got a stage 1 ack from someone.  So we should remove it from the connection that sent it so
 * it doesn't keep resending it.
 */
void ConnectionManager::processAckStage1(NetCommandMsg *msg) {
#if defined(RTS_DEBUG)
	Bool doDebug = (msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTFRAME) ? TRUE : FALSE;
#endif

	UnsignedByte playerID = msg->getPlayerID();
	NetCommandRef *ref = nullptr;

#if defined(RTS_DEBUG)
	if (doDebug == TRUE) {
		DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::processAck - processing ack for command %d from player %d", ((NetAckStage1CommandMsg *)msg)->getCommandID(), playerID));
	}
#endif

	if ((playerID >= 0) && (playerID < NUM_CONNECTIONS)) {
		if (m_connections[playerID] != nullptr) {
			ref = m_connections[playerID]->processAck(msg);
		}
	} else {
		DEBUG_ASSERTCRASH((playerID >= 0) && (playerID < NUM_CONNECTIONS), ("ConnectionManager::processAck - %d is an invalid player number", playerID));
	}

	if (ref != nullptr) {
		if (ref->getCommand()->getNetCommandType() == NETCOMMANDTYPE_FRAMEINFO) {
			m_frameMetrics.processLatencyResponse(((NetFrameCommandMsg *)(ref->getCommand()))->getExecutionFrame());
		}

		deleteInstance(ref);
		ref = nullptr;
	}
}

/**
 * We just got a stage 2 ack from someone.  So remove it from the pending commands list so it doesn't
 * get sent in the case of a new packet router.
 */
void ConnectionManager::processAckStage2(NetCommandMsg *msg) {
	UnsignedShort commandID = 0;
	UnsignedByte playerID = 0;
	if (msg->getNetCommandType() == NETCOMMANDTYPE_ACKSTAGE2) {
		commandID = ((NetAckStage2CommandMsg *)msg)->getCommandID();
		playerID = ((NetAckStage2CommandMsg *)msg)->getOriginalPlayerID();
	} else if (msg->getNetCommandType() == NETCOMMANDTYPE_ACKBOTH) {
		commandID = ((NetAckBothCommandMsg *)msg)->getCommandID();
		playerID = ((NetAckBothCommandMsg *)msg)->getOriginalPlayerID();
	} else {
		return;
	}

	NetCommandRef *ref = m_pendingCommands->findMessage(commandID, playerID);
	if (ref != nullptr) {
		//DEBUG_LOG(("ConnectionManager::processAckStage2 - removing command %d from the pending commands list.", commandID));
		DEBUG_ASSERTCRASH((m_localSlot == playerID), ("Found a command in the pending commands list that wasn't originated by the local player"));
		m_pendingCommands->removeMessage(ref);
		deleteInstance(ref);
		ref = nullptr;
	} else {
		//DEBUG_LOG(("ConnectionManager::processAckStage2 - Couldn't find command %d from player %d in the pending commands list.", commandID, playerID));
	}

	ref = m_relayedCommands->findMessage(commandID, playerID);
	if (ref != nullptr) {
		//DEBUG_LOG(("ConnectionManager::processAckStage2 - found command ID %d from player %d in the relayed commands list.", commandID, playerID));
		UnsignedByte prevRelay = ref->getRelay();
		UnsignedByte relay = prevRelay & ~(1 << msg->getPlayerID());
		//DEBUG_LOG(("ConnectionManager::processAckStage2 - relay was %d and is now %d", relay, prevRelay));
		if (relay == 0) {
			//DEBUG_LOG(("ConnectionManager::processAckStage2 - relay is 0, removing command from the relayed commands list."));
			m_relayedCommands->removeMessage(ref);
			NetAckStage2CommandMsg *ackmsg = newInstance(NetAckStage2CommandMsg)(ref->getCommand());
			sendLocalCommand(ackmsg, 1 << ackmsg->getOriginalPlayerID());
			deleteInstance(ref);
			ref = nullptr;

			ackmsg->detach();
			ackmsg = nullptr;
		} else {
			ref->setRelay(relay);
		}
	}
}

/**
 * We just got a "both" ack from someone.  So process it as both a stage 1 and stage 2 ack.
 */
void ConnectionManager::processAck(NetCommandMsg *msg) {
	if ((msg->getNetCommandType() == NETCOMMANDTYPE_ACKSTAGE1) || (msg->getNetCommandType() == NETCOMMANDTYPE_ACKBOTH)) {
		processAckStage1(msg);
	}
	if ((msg->getNetCommandType() == NETCOMMANDTYPE_ACKSTAGE2) || (msg->getNetCommandType() == NETCOMMANDTYPE_ACKBOTH)) {
		processAckStage2(msg);
	}
}

/**
 * A player has just left our game. Delete their connection and frame data manager.
 * return codes are:
 * PLAYERLEAVECODE_UNKNOWN - player didn't have a valid slot number.
 * PLAYERLEAVECODE_CLIENT - someone in the game that wasn't us or the packet router.
 * PLAYERLEAVECODE_LOCAL - We are leaving the game, we could also be the packet router.
 * PLAYERLEAVECODE_PACKETROUTER - The packet router left the game.
 *
 * If we are leaving and are also the packet router, it will return the PLAYERLEAVECODE_LOCAL return code.
 */
PlayerLeaveCode ConnectionManager::processPlayerLeave(NetPlayerLeaveCommandMsg *msg) {
	UnsignedByte playerID = msg->getLeavingPlayerID();
	if ((playerID != m_localSlot) && (m_connections[playerID] != nullptr)) {
		DEBUG_LOG(("ConnectionManager::processPlayerLeave() - setQuitting() on player %d on frame %d", playerID, TheGameLogic->getFrame()));
		m_connections[playerID]->setQuitting();
	}
	DEBUG_ASSERTCRASH(m_frameData[playerID]->getIsQuitting() == FALSE, ("Player %d is already quitting", playerID));
	if ((playerID != m_localSlot) && (m_frameData[playerID] != nullptr) && (m_frameData[playerID]->getIsQuitting() == FALSE)) {
		DEBUG_LOG(("ConnectionManager::processPlayerLeave - setQuitFrame on player %d for frame %d", playerID, TheGameLogic->getFrame()+1));
		m_frameData[playerID]->setQuitFrame(TheGameLogic->getFrame() + FRAMES_TO_KEEP + 1);
	}

	if (playerID == m_localSlot)
	{
		// we're leaving, so mark our connections and frame datas to go away.
		for (Int i=0; i<MAX_SLOTS; ++i)
		{
			if (m_connections[i])
			{
				m_connections[i]->clearCommandsExceptFrom(m_localSlot);
				m_connections[i]->setQuitting();
			}
		}
	}

	PlayerLeaveCode code = disconnectPlayer(playerID);
	DEBUG_LOG(("ConnectionManager::processPlayerLeave() - just disconnected player %d with ret code %d", playerID, code));
	if (code == PLAYERLEAVECODE_PACKETROUTER)
		resendPendingCommands();

	PopulateInGameDiplomacyPopup();
	return code;
}

UnsignedInt ConnectionManager::getPacketRouterFallbackSlot(Int packetRouterNumber) {
	if ((packetRouterNumber >= 0) && (packetRouterNumber < MAX_SLOTS)) {
		return m_packetRouterFallback[packetRouterNumber];
	}
	return MAX_SLOTS;
}

UnsignedInt ConnectionManager::getPacketRouterSlot() {
	return m_packetRouterSlot;
}

Bool ConnectionManager::areAllQueuesEmpty(void) {
	Bool retval = TRUE;
	for (Int i = 0; (i < MAX_SLOTS) && retval; ++i) {
		if (m_connections[i] != nullptr) {
			if (m_connections[i]->isQueueEmpty() == FALSE) {
				//DEBUG_LOG(("ConnectionManager::areAllQueuesEmpty() - m_connections[%d] is not empty", i));
				//m_connections[i]->debugPrintCommands();
				retval = FALSE;
			}
		}
	}

	return retval;
}

Bool ConnectionManager::canILeave() {
	return areAllQueuesEmpty();
}

/**
 * The local player is leaving. Tell the local player as well as the other players
 * to remove this player at the specified frame.
 */
void ConnectionManager::handleLocalPlayerLeaving(UnsignedInt frame) {
	NetPlayerLeaveCommandMsg *msg = newInstance(NetPlayerLeaveCommandMsg);

	msg->setLeavingPlayerID(m_localSlot);
	msg->setExecutionFrame(frame);
	if (DoesCommandRequireACommandID(msg->getNetCommandType())) {
		msg->setID(GenerateNextCommandID());
	}
	msg->setPlayerID(m_localSlot);

	DEBUG_LOG(("ConnectionManager::handleLocalPlayerLeaving - Local player leaving on frame %d", frame));
	DEBUG_ASSERTCRASH(m_packetRouterSlot >= 0, ("ConnectionManager::handleLocalPlayerLeaving, packet router is %d, illegal value.", m_packetRouterSlot));

	sendLocalCommand(msg);

	msg->detach();
}

/**
 * We just got a message that needs to be ack'd, so ack it!
 */
void ConnectionManager::ackCommand(NetCommandRef *ref, UnsignedInt localSlot) {
	NetCommandMsg *msg = ref->getCommand();
	NetCommandMsg *ackmsg;
	UnsignedShort commandID;
	UnsignedByte originalPlayerID;
	UnsignedByte sendRelay = 0;

	// Make send relay a bitmask for the connections that the relay will actually be sent to. This is
	// necessary to determine whether or not we have to wait to send a stage 2 ack.
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if (((m_connections[i] != nullptr) && (m_connections[i]->isQuitting() == FALSE))) {
			sendRelay = sendRelay | (1 << i);
		}
	}

#if defined(RTS_DEBUG)
	Bool doDebug = (msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTFRAME) ? TRUE : FALSE;
#endif

	sendRelay = sendRelay & ref->getRelay();
	if (sendRelay == 0) {
		NetAckBothCommandMsg *bothmsg = newInstance(NetAckBothCommandMsg)(ref->getCommand());
		ackmsg = bothmsg;
		commandID = bothmsg->getCommandID();
		originalPlayerID = bothmsg->getOriginalPlayerID();
#if defined(RTS_DEBUG)
		if (doDebug) {
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::ackCommand - doing ack both for command %d from player %d", bothmsg->getCommandID(), bothmsg->getOriginalPlayerID()));
		}
#endif
	} else {
		NetAckStage1CommandMsg *stage1msg = newInstance(NetAckStage1CommandMsg)(ref->getCommand());
		ackmsg = stage1msg;
		commandID = stage1msg->getCommandID();
		originalPlayerID = stage1msg->getOriginalPlayerID();
#if defined(RTS_DEBUG)
		if (doDebug) {
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::ackCommand - doing ack stage 1 for command %d from player %d", stage1msg->getCommandID(), stage1msg->getOriginalPlayerID()));
		}
#endif
	}

	ackmsg->setPlayerID(localSlot); // Tell the player who this ack is coming from.

	if (CommandRequiresDirectSend(msg) && CommandRequiresAck(msg)) {
		// Send this ack directly back to the sending player, don't go through the packet router.
		if ((msg->getPlayerID() >= 0) && (msg->getPlayerID() < MAX_SLOTS)) {
			if (m_connections[msg->getPlayerID()] != nullptr) {
				m_connections[msg->getPlayerID()]->sendNetCommandMsg(ackmsg, 1 << msg->getPlayerID());
			}
		}
	} else {
		// The local connection may be the packet router, in that case, the connection would be null.  So do something about it!
		if ((m_packetRouterSlot >= 0) && (m_packetRouterSlot < MAX_SLOTS)) {
			if (m_connections[m_packetRouterSlot] != nullptr) {
//				DEBUG_LOG(("ConnectionManager::ackCommand - acking command %d from player %d to packet router.", commandID, m_packetRouterSlot));
				m_connections[m_packetRouterSlot]->sendNetCommandMsg(ackmsg, 1 << m_packetRouterSlot);
			} else if (m_localSlot == m_packetRouterSlot) {
				// we are the packet router, send the ack to the player that sent the command.
				if ((msg->getPlayerID() >= 0) && (msg->getPlayerID() < MAX_SLOTS)) {
					if (m_connections[msg->getPlayerID()] != nullptr) {
//						DEBUG_LOG(("ConnectionManager::ackCommand - acking command %d from player %d directly to player.", commandID, msg->getPlayerID()));
						m_connections[msg->getPlayerID()]->sendNetCommandMsg(ackmsg, 1 << msg->getPlayerID());
					} else {
	//					DEBUG_ASSERTCRASH(m_connections[msg->getPlayerID()] != nullptr, ("Connection to player is null"));
					}
				} else {
					DEBUG_ASSERTCRASH((msg->getPlayerID() >= 0) && (msg->getPlayerID() < MAX_SLOTS), ("Command sent by an invalid player ID."));
				}
			} else {
				DEBUG_ASSERTCRASH(m_connections[m_packetRouterSlot] != nullptr, ("Connection to packet router is null"));
			}
		} else {
			DEBUG_ASSERTCRASH((m_packetRouterSlot >= 0) && (m_packetRouterSlot < MAX_SLOTS), ("I don't know who the packet router is."));
		}
	}

	ackmsg->detach();
}

/**
 * This is where we relay a command from one client to others (including ourselves).
 * This should only be done by the current packet router.
 */
void ConnectionManager::sendRemoteCommand(NetCommandRef *msg) {
	UnsignedByte actualRelay = 0;
	if (msg->getCommand() == nullptr) {
		return;
	}

	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendRemoteCommand - sending net command %d of type %s from player %d, relay is 0x%x",
		msg->getCommand()->getID(), GetNetCommandTypeAsString(msg->getCommand()->getNetCommandType()), msg->getCommand()->getPlayerID(), msg->getRelay()));

	UnsignedByte relay = msg->getRelay();
	if ((relay & (1 << m_localSlot)) && (m_frameData[msg->getCommand()->getPlayerID()] != nullptr)) {
		if (IsCommandSynchronized(msg->getCommand()->getNetCommandType())) {
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendRemoteCommand - adding net command of type %s to player %d for frame %d", GetNetCommandTypeAsString(msg->getCommand()->getNetCommandType()), msg->getCommand()->getPlayerID(), msg->getCommand()->getExecutionFrame()));
			m_frameData[msg->getCommand()->getPlayerID()]->addNetCommandMsg(msg->getCommand());
		}
	}

	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if ((relay & (1 << i)) && ((m_connections[i] != nullptr) && (m_connections[i]->isQuitting() == FALSE))) {
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendRemoteCommand - relaying command %d to player %d", msg->getCommand()->getID(), i));
			m_connections[i]->sendNetCommandMsg(msg->getCommand(), 1 << i);
			actualRelay = actualRelay | (1 << i);
		}
	}

	if ((actualRelay != 0) && (CommandRequiresAck(msg->getCommand()) == TRUE)) {
		NetCommandRef *ref = m_relayedCommands->addMessage(msg->getCommand());
		if (ref != nullptr) {
			ref->setRelay(actualRelay);
			//DEBUG_LOG(("ConnectionManager::sendRemoteCommand - command %d added to relayed commands with relay %d", msg->getCommand()->getID(), ref->getRelay()));
		}
	}

	// Do some metrics to find the minimum packet arrival cushion.
	if (IsCommandSynchronized(msg->getCommand()->getNetCommandType())) {
//		DEBUG_LOG(("ConnectionManager::sendRemoteCommand - about to call allCommandsReady"));
		if (allCommandsReady(msg->getCommand()->getExecutionFrame(), TRUE)) {
			UnsignedInt cushion = msg->getCommand()->getExecutionFrame() - TheGameLogic->getFrame();
			if ((cushion < m_smallestPacketArrivalCushion) || (m_smallestPacketArrivalCushion == -1)) {
				m_smallestPacketArrivalCushion = cushion;
			}
			m_frameMetrics.addCushion(cushion);
//			DEBUG_LOG(("Adding %d to cushion for frame %d", cushion, msg->getCommand()->getExecutionFrame()));
		}
	}
}

/**
 * ConnectionManager::update
 * Update the connections. Tell them to do the receive and send.  Also relay
 * commands to their final destinations as necessary.
 */
void ConnectionManager::update(Bool isInGame) {
//
// 1. do this
// 2. do that
// 3. do something else
// 4. blow something up
// 5. bust some cap
//

	if ((m_localAddr == 0) || (m_localPort == 0)) {
		// we don't have a local address or port yet, this is bad.
		DEBUG_ASSERTCRASH((m_localAddr != 0) && (m_localPort != 0), ("ConnectionManager doesn't have a local address."));
		return;
	}

	m_transport->doRecv();

	if (isInGame) {
		m_disconnectManager->update(this);
	}

	// take the packets from the transport, break them up into commands, and give them to the appropriate connections.
	doRelay();

	// send any necessary keep-alive packets.
	doKeepAlive();

	for (Int i = 0; i < NUM_CONNECTIONS; ++i) {
		if (m_connections[i] != nullptr) {
			/*
			if (m_connections[i]->isQueueEmpty() == FALSE) {
//				DEBUG_LOG(("ConnectionManager::update - calling doSend on connection %d", i));
			}
			*/

			m_connections[i]->doSend();

			if (m_connections[i]->isQuitting() && m_connections[i]->isQueueEmpty())
			{
				DEBUG_LOG(("ConnectionManager::update - deleting connection for slot %d", i));
				deleteInstance(m_connections[i]);
				m_connections[i] = nullptr;
			}
		}

		if ((m_frameData[i] != nullptr) && (m_frameData[i]->getIsQuitting() == TRUE)) {
			if (m_frameData[i]->getQuitFrame() == TheGameLogic->getFrame()) {
				DEBUG_LOG(("ConnectionManager::update - deleting frame data for slot %d on quitting frame %d", i, m_frameData[i]->getQuitFrame()));
				deleteInstance(m_frameData[i]);
				m_frameData[i] = nullptr;
			}
		}
	}

	m_transport->doSend();
}

void ConnectionManager::updateRunAhead(Int oldRunAhead, Int frameRate, Bool didSelfSlug, Int nextExecutionFrame) {
	static time_t lasttimesent = 0;
	time_t curTime = timeGetTime();

	if ((lasttimesent == 0) || ((curTime - lasttimesent) > TheGlobalData->m_networkRunAheadMetricsTime)) {
		if (m_localSlot == m_packetRouterSlot) {
			// We are the packet router, time to compute a new run ahead for this game.
			m_latencyAverages[m_localSlot] = m_frameMetrics.getAverageLatency();

			// since we are now using the display frame rate rather than the logic frame rate to get our average FPS,
			// it doesn't make sense to send the desired logic frame rate if we "slugged" ourself.
//			if (didSelfSlug) {
//				m_fpsAverages[m_localSlot] = frameRate;
//			} else {
				m_fpsAverages[m_localSlot] = m_frameMetrics.getAverageFPS();
//			}
			if (didSelfSlug) {
				//DEBUG_LOG(("ConnectionManager::updateRunAhead - local player run ahead metrics, fps = %d, actual fps = %d, latency = %f, didSelfSlug = true", m_fpsAverages[m_localSlot], m_frameMetrics.getAverageFPS(), m_latencyAverages[m_localSlot]));
			} else {
				//DEBUG_LOG(("ConnectionManager::updateRunAhead - local player run ahead metrics, fps = %d, latency = %f, didSelfSlug = false", m_fpsAverages[m_localSlot], m_latencyAverages[m_localSlot]));
			}
			Int minFps;
			Int minFpsPlayer;
			getMinimumFps(minFps, minFpsPlayer);
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::updateRunAhead - max latency = %f, min fps = %d, min fps player = %d old FPS = %d", getMaximumLatency(), minFps, minFpsPlayer, frameRate));
			if ((minFps >= ((frameRate * 9) / 10)) && (minFps < frameRate)) {
				// if the minimum fps is within 10% of the desired framerate, then keep the current minimum fps.
				minFps = frameRate;
			}

			// TheSuperHackers @info this clamps the logic time scale fps in network games
			minFps = clamp<Int>(MIN_LOGIC_FRAMES, minFps, TheGlobalData->m_framesPerSecondLimit);
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::updateRunAhead - minFps after adjustment is %d", minFps));

			// TheSuperHackers @bugfix Mauller 21/08/2025 calculate the runahead so it always follows the latency
			// The runahead should always be rounded up to the next integer value to prevent variations in latency from causing stutter
			// The network slack pushes the runahead up to the next value when the latency is within the slack percentage of the current runahead
			const Real runAheadSlackScale = 1.0f + ( (Real)TheGlobalData->m_networkRunAheadSlack / 100.0f );
			Int newRunAhead = ceilf( getMaximumLatency() * runAheadSlackScale * (Real)minFps );

			// TheSuperHackers @info if the runahead goes below 3 logic frames it can start to introduce stutter
			// We also limit the upper range of the runahead to prevent it getting out of hand
			newRunAhead = clamp<Int>(MIN_RUNAHEAD, newRunAhead, MAX_FRAMES_AHEAD / 2);

			NetRunAheadCommandMsg *msg = newInstance(NetRunAheadCommandMsg);
			msg->setPlayerID(m_localSlot);
			if (DoesCommandRequireACommandID(msg->getNetCommandType())) {
				msg->setID(GenerateNextCommandID());
			}

			// needs to be set to the greater of getExecutionFrame and TheGameLogic->getFrame() + oldRunAhead
			// This prevents the case of...
			// run ahead starts at 30
			// run ahead changes to 10 at frame 31 (the command was created on frame 1)
			// run ahead changes to 10 at frame 56 (the command was created on frame 46)
			// notice that 56 is within the previous run ahead of 30 which has triggered
			// the frame command count being set for frames 1 through 60 since run ahead
			// didn't change for the first time till frame 31.  This creates an extra command
			// for frame 56 that isn't accounted for in the frame command count that is sent
			// out in the NetFrameCommandMsg.  sheesh.
			if (nextExecutionFrame > (TheGameLogic->getFrame() + oldRunAhead)) {
				msg->setExecutionFrame(nextExecutionFrame);
			} else {
				msg->setExecutionFrame(TheGameLogic->getFrame() + oldRunAhead);
			}

			msg->setRunAhead(newRunAhead);
			msg->setFrameRate(minFps);
			//DEBUG_LOG(("ConnectionManager::updateRunAhead - new run ahead = %d, new frame rate = %d, execution frame %d", newRunAhead, minFps, msg->getExecutionFrame()));
			sendLocalCommand(msg, 0xff ^ (1 << minFpsPlayer)); // Send the packet to everyone but the lowest FPS player.

			NetRunAheadCommandMsg *msg2 = newInstance(NetRunAheadCommandMsg);
			msg2->setPlayerID(m_localSlot);
			if (DoesCommandRequireACommandID(msg2->getNetCommandType())) {
				/*
				 * Ok there needs to be a big friggin comment about this change...
				 * What happens is that the two run ahead commands get sent to different players
				 * using different command ID's.  So player 1 has the run ahead command as command x
				 * and player 2 has the command as command x+1.  This is all good except when it comes
				 * to players being disconnected.  With the new disconnect scheme player 1 could potentially
				 * send his run ahead command to player 2 (or the other way around) to let player 2 catch
				 * up to him.  So if player 2 has his run ahead command as x+1 and now he gets player 1's
				 * command list with the run ahead command listed as command x, he won't see them as being
				 * the same command and will now think he has two different run ahead commands for that frame
				 * and thus his command list will have an extra command and he will never be able to recover.
				 * So to fix this we have to use the same command ID for both run ahead commands.  That way
				 * when the commands are copied places for the disconnect screen they will be seen as the
				 * same command, and all will be good.
				 */
//				msg2->setID(GenerateNextCommandID());
				msg2->setID(msg->getID());
			}
			if (nextExecutionFrame > (TheGameLogic->getFrame() + oldRunAhead)) {
				msg2->setExecutionFrame(nextExecutionFrame);
			} else {
				msg2->setExecutionFrame(TheGameLogic->getFrame() + oldRunAhead);
			}

			// Let the player with the slowest FPS run a little faster than the other computers...
			// just in case they are able to.  Then we might be able to run the game faster which would be good.
			Int newMinFps = (minFps * 11) / 10;
			if (newMinFps == minFps) {
				newMinFps = minFps + 1;
			}
			if (newMinFps > 30) {
				newMinFps = 30; // Cap FPS to 30.
			}
			msg2->setRunAhead(newRunAhead);
			msg2->setFrameRate(newMinFps);

			sendLocalCommand(msg2, 1 << minFpsPlayer);

			msg->detach();
			msg2->detach();
		} else {
			// We are not the packet router, send our metrics info to the packet router.
			NetRunAheadMetricsCommandMsg *msg = newInstance(NetRunAheadMetricsCommandMsg);
			msg->setPlayerID(m_localSlot);
			if (DoesCommandRequireACommandID(msg->getNetCommandType())) {
				msg->setID(GenerateNextCommandID());
			}
			msg->setAverageLatency(m_frameMetrics.getAverageLatency());

			// see above for explanation.
//			if (didSelfSlug) {
//				msg->setAverageFps(frameRate);
//			} else {
				msg->setAverageFps(m_frameMetrics.getAverageFPS());
//			}
			if (didSelfSlug) {
				//DEBUG_LOG(("ConnectionManager::updateRunAhead - average latency = %f, average fps = %d, actual fps = %d, didSelfSlug = true", m_frameMetrics.getAverageLatency(), m_frameMetrics.getAverageFPS(), m_frameMetrics.getAverageFPS()));
			} else {
				//DEBUG_LOG(("ConnectionManager::updateRunAhead - average latency = %f, average fps = %d, didSelfSlug = false", m_frameMetrics.getAverageLatency(), m_frameMetrics.getAverageFPS()));
			}
			m_connections[m_packetRouterSlot]->sendNetCommandMsg(msg, 1 << m_packetRouterSlot);
			msg->detach();
		}
		lasttimesent = curTime;
	}
}

Real ConnectionManager::getMaximumLatency() {
	Real maxLatency = 0.0f;

	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if (isPlayerConnected(i) && m_latencyAverages[i] > maxLatency) {
				maxLatency = m_latencyAverages[i];
		}
	}

	return maxLatency;
}

void ConnectionManager::getMinimumFps(Int &minFps, Int &minFpsPlayer) {
	minFps = -1;
	minFpsPlayer = -1;
//	DEBUG_LOG_RAW(("ConnectionManager::getMinimumFps -"));
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if ((m_connections[i] != nullptr) || (i == m_localSlot)) {
//			DEBUG_LOG_RAW((" %d: %d,", i, m_fpsAverages[i]));
			if (m_fpsAverages[i] != -1) {
				if ((minFps == -1) || (m_fpsAverages[i] < minFps)) {
					minFps = m_fpsAverages[i];
					minFpsPlayer = i;
				}
			}
		}
	}
//	DEBUG_LOG_RAW(("\n"));
}

UnsignedInt ConnectionManager::getMinimumCushion() {
	return m_frameMetrics.getMinimumCushion();
}

/**
 * The commands for the given frame are all ready, time to send out our command count for that frame.
 */
void ConnectionManager::processFrameTick(UnsignedInt frame) {
	if ((m_frameData[m_localSlot] == nullptr) || (m_frameData[m_localSlot]->getIsQuitting() == TRUE)) {
		// if the local frame data stuff is null, we must be leaving the game.
		return;
	}
	UnsignedShort commandCount = m_frameData[m_localSlot]->getCommandCount(frame);
	NetFrameCommandMsg *msg = newInstance(NetFrameCommandMsg);
	msg->setExecutionFrame(frame);
	msg->setCommandCount(commandCount);
	if (DoesCommandRequireACommandID(msg->getNetCommandType())) {
		msg->setID(GenerateNextCommandID());
	}
	msg->setPlayerID(m_localSlot);

	m_frameMetrics.doPerFrameMetrics(frame);

	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::processFrameTick - sending frame info for frame %d, ID %d, command count %d", frame, msg->getID(), commandCount));

	sendLocalCommand(msg, 0xff & ~(1 << m_localSlot));

	msg->detach();
}

/**
 * Set the local address.
 */
void ConnectionManager::setLocalAddress(UnsignedInt ip, UnsignedInt port) {
	DEBUG_LOG(("ConnectionManager::setLocalAddress() - local address is %X:%d", ip, port));
	m_localAddr = ip;
	m_localPort = port;
}

/**
 * Initialize the transport object
 */
void ConnectionManager::initTransport() {
	DEBUG_ASSERTCRASH((m_transport == nullptr), ("m_transport already exists when trying to init it."));
	DEBUG_LOG(("ConnectionManager::initTransport - Initializing Transport"));

	delete m_transport;
	m_transport = new Transport;
	m_transport->reset();
	m_transport->init(m_localAddr, m_localPort);
}

/**
 * This is where the commands from the local client are sent to the other clients in
 * the game.  This is also where the local commands are put into the frame data for
 * future execution.
 */
void ConnectionManager::sendLocalGameMessage(GameMessage *msg, UnsignedInt frame) {
	UnsignedShort currentID = 0;
	if (DoesCommandRequireACommandID(NETCOMMANDTYPE_GAMECOMMAND)) {
		currentID = GenerateNextCommandID();
	}

	NetCommandMsg *netmsg = newInstance(NetGameCommandMsg)(msg);
	netmsg->setExecutionFrame(frame);
	netmsg->setPlayerID(m_localSlot);
	netmsg->setID(currentID);

	sendLocalCommand(netmsg);

	netmsg->detach();
}

/**
 * This is a NetCommandMsg that originated on the local computer. Send this to everyone specified
 * in the relay field.  Commands sent in this way go through the packet router.
 */
void ConnectionManager::sendLocalCommand(NetCommandMsg *msg, UnsignedByte relay /* = 0xff by default*/) {
	if (CommandRequiresDirectSend(msg) || (m_packetRouterSlot < 0) || (m_packetRouterSlot >= MAX_SLOTS) || (m_connections[m_packetRouterSlot] == nullptr)) {
		sendLocalCommandDirect(msg, relay);
		return;
	}
	msg->attach();

	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendLocalCommand - sending net command %d of type %s", msg->getID(),
		GetNetCommandTypeAsString(msg->getNetCommandType())));

	if (relay & (1 << m_localSlot)) {
		DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendLocalCommand - adding net command of type %s to player %d for frame %d", GetNetCommandTypeAsString(msg->getNetCommandType()), msg->getPlayerID(), msg->getExecutionFrame()));
		m_frameData[m_localSlot]->addNetCommandMsg(msg);
	}

	// Send the packet to everyone else
	if (m_localSlot == m_packetRouterSlot) {
		// I am the packet router, I need to send this packet to everyone individually.
		for (Int i = 0; i < MAX_SLOTS; ++i) {
			// Send it to all open connections.
			if (((m_connections[i] != nullptr) && (m_connections[i]->isQuitting() == FALSE)) && (relay & (1 << i))) {
				// Set the relay mask to only go to this player so he knows not to relay it to anyone else.
				UnsignedByte temprelay = 1 << i;
				m_connections[i]->sendNetCommandMsg(msg, temprelay); // This will create a new copy of netmsg for this connection.
			}
		}
	} else {
		// Send the command to everyone else via the packet router.
		UnsignedByte temprelay = relay & ~(1 << m_localSlot);	// Tell the packet router to relay the message to everyone but myself.
														// Hopefully the packet router is smart enough to not send it
														// to slots that are not in the game.

		m_connections[m_packetRouterSlot]->sendNetCommandMsg(msg, temprelay); // This will create a new copy of netmsg for this connection.

		if (CommandRequiresAck(msg)) {
			NetCommandRef *ref = m_pendingCommands->addMessage(msg);
			//DEBUG_LOG(("ConnectionManager::sendLocalCommand - added command %d to pending commands list.", msg->getID()));
			if (ref != nullptr) {
				ref->setRelay(temprelay);
			}
		}
	}

	msg->detach(); // detach from the command msg.
}

/**
 * This is a NetCommandMsg that originated on the local computer.  Send this to everyone specified
 * in the relay field.  Commands sent in this way do not go through the packet router.
 */
void ConnectionManager::sendLocalCommandDirect(NetCommandMsg *msg, UnsignedByte relay) {
	msg->attach();

	if (((relay & (1 << m_localSlot)) != 0) && (m_frameData[m_localSlot] != nullptr)) {
		if (IsCommandSynchronized(msg->getNetCommandType()) == TRUE) {
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendLocalCommandDirect - adding net command of type %s to player %d for frame %d", GetNetCommandTypeAsString(msg->getNetCommandType()), msg->getPlayerID(), msg->getExecutionFrame()));
			m_frameData[m_localSlot]->addNetCommandMsg(msg);
		}
	}

	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if ((relay & (1 << i)) != 0) {
			if ((m_connections[i] != nullptr) && (m_connections[i]->isQuitting() == FALSE)) {
				UnsignedByte temprelay = 1 << i;
				m_connections[i]->sendNetCommandMsg(msg, temprelay);
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendLocalCommandDirect - Sending direct command %d of type %s to player %d", msg->getID(), GetNetCommandTypeAsString(msg->getNetCommandType()), i));
			}
		}
	}

	msg->detach();
}

Int commandsReadyDebugSpewage = 0;

/**
 * Returns true if all the commands for the given frame are ready to be executed.
 */
Bool ConnectionManager::allCommandsReady(UnsignedInt frame, Bool justTesting /* = FALSE */) {
	Bool retval = TRUE;
	FrameDataReturnType frameRetVal = FRAMEDATA_NOTREADY;
//	retval = FALSE;  // ****for testing purposes only!!!!!!****
	Int i = 0;
	for (; (i < MAX_SLOTS) && retval; ++i) {
		if ((m_frameData[i] != nullptr) && (m_frameData[i]->getIsQuitting() == FALSE)) {
/*
			if (!(m_frameData[i]->allCommandsReady(frame, (frame != commandsReadyDebugSpewage) && (justTesting == FALSE)))) {
				if ((frame != commandsReadyDebugSpewage) && (justTesting == FALSE)) {
					DEBUG_LOG(("ConnectionManager::allCommandsReady, frame %d player %d not ready.", frame, i));
					commandsReadyDebugSpewage = frame;
				}
				retval = FALSE;
			} else {
//				DEBUG_LOG(("ConnectionManager::allCommandsReady, frame %d player %d is ready.", frame, i));
			}
*/

			frameRetVal = m_frameData[i]->allCommandsReady(frame, (frame != commandsReadyDebugSpewage) && (justTesting == FALSE));
			if (frameRetVal == FRAMEDATA_NOTREADY) {
				retval = FALSE;
			} else if (frameRetVal == FRAMEDATA_RESEND) {
				requestFrameDataResend(i, frame);
				retval = FALSE;
			}
		}
	}

	if (frameRetVal == FRAMEDATA_RESEND) {
		// this frame's data is really screwed up, we need to clean it out so it can be resent to us.
		for (i = 0; i < MAX_SLOTS; ++i) {
			if ((m_frameData[i] != nullptr) && (i != m_localSlot)) {
				m_frameData[i]->resetFrame(frame, FALSE);
			}
		}
	}

	if ((retval == TRUE) && (justTesting == FALSE)) {
		m_disconnectManager->allCommandsReady(TheGameLogic->getFrame(), this);
		retval = m_disconnectManager->allowedToContinue(); // allow the disconnect manager to keep us on this frame
																											// in case we are waiting for a new packet router or something.
	}

	return retval;
}

void ConnectionManager::handleAllCommandsReady(void)
{
	m_disconnectManager->allCommandsReady(TheGameLogic->getFrame(), this, FALSE);
}


/**
 * Only call this after making sure that all the commands are there for this frame.
 * After calling this the commands for this frame will be removed from the connection.
 *
 * BGC - To account for the case where the host disconnects without sending the
 *       same commands to all players, we now have to keep around the last 'run ahead'
 *       frames so we can potentially send those commands to the other players in the
 *       game so they can catch up.
 */
NetCommandList *ConnectionManager::getFrameCommandList(UnsignedInt frame)
{
	NetCommandList *retlist = newInstance(NetCommandList);
	retlist->init();

	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if (m_frameData[i] != nullptr) {
			retlist->appendList(m_frameData[i]->getFrameCommandList(frame));
			if (frame > FRAMES_TO_KEEP) {
				m_frameData[i]->resetFrame(frame - FRAMES_TO_KEEP);	// After getting the commands for that frame from this
													// FrameDataManager object, we need to tell it that we're
													// done with the messages for that frame.
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("getFrameCommandList - called reset frame on player %d for frame %d", i, frame - FRAMES_TO_KEEP));
			}
		}
	}

	return retlist; // retlist deallocated by calling function.
}

void ConnectionManager::setFrameGrouping(time_t frameGrouping) {
	// Since we are the packet router, we should send more packets per second since we
	// may become the latency bottleneck for sending packets from one player to the next.
	// This is probably ok since the packet router should have the fastest connection of all
	// the players in the game.
	if (m_localSlot == m_packetRouterSlot) {
		frameGrouping = frameGrouping / 2;
	}
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if (m_connections[i] != nullptr) {
			m_connections[i]->setFrameGrouping(frameGrouping);
		}
	}
}

/*
void ConnectionManager::determineRouterFallbackPlan() {
	memset(m_packetRouterFallback, 0, sizeof(m_packetRouterFallback));
	Int curnum = 1;
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if (m_connections[i] != nullptr) {
			m_packetRouterFallback[i] = curnum;
			if (curnum == 1) {
				m_packetRouterSlot = i;
			}
			++curnum;
		}
	}
}
*/

void ConnectionManager::doKeepAlive() {
	static Int nextIndex = 0;
	static time_t startTime = 0;

	time_t curTime = timeGetTime();

	if (startTime == 0) {
		startTime = curTime;
		return;
	}

	time_t numSeconds = (curTime - startTime) / 1000;

	while ((nextIndex <= numSeconds) && (nextIndex < MAX_SLOTS)) {
//		DEBUG_LOG(("ConnectionManager::doKeepAlive - trying to send keep alive message to player %d", nextIndex));
		if (m_connections[nextIndex] != nullptr) {
			NetKeepAliveCommandMsg *msg = newInstance(NetKeepAliveCommandMsg);
			msg->setPlayerID(m_localSlot);
			if (DoesCommandRequireACommandID(msg->getNetCommandType()) == TRUE) {
				msg->setID(GenerateNextCommandID());
			}
//			DEBUG_LOG(("ConnectionManager::doKeepAlive - sending keep alive message to player %d", nextIndex));
			sendLocalCommandDirect(msg, 1 << nextIndex);
			msg->detach();
		}
		++nextIndex;
	}
	if (nextIndex == MAX_SLOTS) {
		nextIndex = 0;
		startTime = curTime;
	}
}

PlayerLeaveCode ConnectionManager::disconnectPlayer(Int slot) {
	// Need to do the deletion of the slot's connection and frame data here.
	PlayerLeaveCode retval = PLAYERLEAVECODE_CLIENT;
	DEBUG_LOG(("ConnectionManager::disconnectPlayer - disconnecting slot %d on frame %d", slot, TheGameLogic->getFrame()));

	if ((slot < 0) || (slot >= MAX_SLOTS)) {
		return PLAYERLEAVECODE_UNKNOWN;
	}

	if (TheGameInfo)
	{
		GameSlot *gSlot = TheGameInfo->getSlot( slot );
		if (gSlot && !gSlot->lastFrameInGame())
		{
			DEBUG_LOG(("ConnectionManager::disconnectPlayer(%d) - slot is last in the game on frame %d",
				slot, TheGameLogic->getFrame()));
			gSlot->setLastFrameInGame(TheGameLogic->getFrame());
		}
	}

	UnicodeString unicodeName;
	unicodeName = getPlayerName(slot);
	if (!unicodeName.isEmpty() && m_connections[slot]) {
		TheInGameUI->message("Network:PlayerLeftGame", unicodeName.str());

		// People are boneheads. Also play a sound
		static AudioEventRTS leftGameSound("GUIMessageReceived");
		TheAudio->addAudioEvent(&leftGameSound);
	}

	if ((m_frameData[slot] != nullptr) && (m_frameData[slot]->getIsQuitting() == FALSE)) {
		DEBUG_LOG(("ConnectionManager::disconnectPlayer - deleting player %d frame data", slot));
		deleteInstance(m_frameData[slot]);
		m_frameData[slot] = nullptr;
	}

	if (m_connections[slot] != nullptr && !m_connections[slot]->isQuitting()) {
		DEBUG_LOG(("ConnectionManager::disconnectPlayer - deleting player %d connection", slot));
		deleteInstance(m_connections[slot]);
		m_connections[slot] = nullptr;
	}

//	if (playerID == m_localSlot) {
//		TheMessageStream->appendMessage(GameMessage::MSG_CLEAR_GAME_DATA);
//	}

	if (slot == m_packetRouterSlot) {
		Int index = 0;
		while ((index < (MAX_SLOTS-1)) && (m_packetRouterFallback[index] != m_packetRouterSlot)) {
			++index;
		}
		++index;
		m_packetRouterSlot = m_packetRouterFallback[index];
		DEBUG_LOG(("Packet router left.  New packet router is slot %d", m_packetRouterSlot));
		retval = PLAYERLEAVECODE_PACKETROUTER;
	}
	if (m_localSlot == slot) {
		DEBUG_LOG(("Disconnecting self"));
		retval = PLAYERLEAVECODE_LOCAL;
	}

	// Take the player out of the fallback plan
	Int fallbackindex = 0;
	while ((fallbackindex < MAX_SLOTS) && (m_packetRouterFallback[fallbackindex] != slot)) {
		++fallbackindex;
	}

	for (Int i = fallbackindex; i < MAX_SLOTS-1; ++i) {
		m_packetRouterFallback[i] = m_packetRouterFallback[i+1];
	}
	m_packetRouterFallback[MAX_SLOTS-1] = -1;

	return retval;
}

void ConnectionManager::quitGame() {
	// Need to do the NetDisconnectPlayerCommandMsg creation and sending here.
	NetDisconnectPlayerCommandMsg *disconnectMsg = newInstance(NetDisconnectPlayerCommandMsg);
	disconnectMsg->setDisconnectSlot(m_localSlot);
	disconnectMsg->setDisconnectFrame(TheGameLogic->getFrame());
	disconnectMsg->setPlayerID(m_localSlot);
	if (DoesCommandRequireACommandID(disconnectMsg->getNetCommandType())) {
		disconnectMsg->setID(GenerateNextCommandID());
	}
	//DEBUG_LOG(("ConnectionManager::disconnectLocalPlayer - about to send disconnect command"));
	sendLocalCommandDirect(disconnectMsg, 0xff ^ (1 << m_localSlot));

	//DEBUG_LOG(("ConnectionManager::disconnectLocalPlayer - about to flush connections"));
	flushConnections(); // need to do this so our packet actually gets sent before the connections are deleted.
	//DEBUG_LOG(("ConnectionManager::disconnectLocalPlayer - done flushing connections"));

	disconnectMsg->detach();

#if RTS_GENERALS
	// if we get here, we hit Quit on the disconnect screen.  Mark everyone as having disconnected from us
	// so the online stats can give us appropriate feedback.
	if (TheGameInfo)
	{
		for (Int i = 0; i < MAX_SLOTS; ++i)
		{
			GameSlot *gSlot = TheGameInfo->getSlot( i );
			if (gSlot && !gSlot->lastFrameInGame())
			{
				gSlot->markAsDisconnected();
			}
		}
	}
#endif

	disconnectLocalPlayer();
}

void ConnectionManager::disconnectLocalPlayer() {
	// kill the frame data and the connections for all the other players.
	DEBUG_LOG(("ConnectionManager::disconnectLocalPlayer()"));
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if (i != m_localSlot) {
			disconnectPlayer(i);
		}
	}
}

/**
 * Takes all the commands that are ready to send and sends them right now.
 */
void ConnectionManager::flushConnections() {
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if (m_connections[i] != nullptr) {
//			DEBUG_LOG(("ConnectionManager::flushConnections - flushing connection to player %d", i));
			/*
			if (m_connections[i]->isQueueEmpty()) {
//				DEBUG_LOG(("ConnectionManager::flushConnections - connection queue empty"));
			}
			*/
			m_connections[i]->doSend();
		}
	}

	if (m_transport != nullptr) {
		m_transport->doSend();
	}
}

void ConnectionManager::resendPendingCommands() {
	//DEBUG_LOG(("ConnectionManager::resendPendingCommands()"));
	if (m_pendingCommands == nullptr) {
		return;
	}

	NetCommandRef *ref = m_pendingCommands->getFirstMessage();
	while (ref != nullptr) {
		//DEBUG_LOG(("ConnectionManager::resendPendingCommands - resending command %d", ref->getCommand()->getID()));
		sendLocalCommand(ref->getCommand(), ref->getRelay());
		ref = ref->getNext();
	}
}

UnsignedInt ConnectionManager::getLocalPlayerID() {
	return m_localSlot;
}

UnicodeString ConnectionManager::getPlayerName(Int playerNum) {
	UnicodeString retval;
	if( playerNum == m_localSlot ) {
		retval = m_localUser->GetName();
	}	else if (((m_connections[playerNum] != nullptr) && (m_connections[playerNum]->isQuitting() == FALSE))) {
		retval = m_connections[playerNum]->getUser()->GetName();
	}
	return retval;
}

/**
 * Take a user list and make connections and frame data manager objects for each of the players.
 * For now, this is also how we'll determine the packet router fallback plan.
 */
void ConnectionManager::parseUserList(const GameInfo *game)
{
	if (!game)
		return;

	Int i;
	Int numUsers = 0;
	m_localSlot = -1;
	DEBUG_LOG(("Local slot is %d", game->getLocalSlotNum()));
	for (i=0; i<MAX_SLOTS; ++i)
	{
		const GameSlot *slot = game->getConstSlot(i);	// badness, but since we cast right back to const, we should be ok
		if (slot->isHuman())
		{
			if (game->getLocalSlotNum() == i)
			{
				m_localSlot = i;
   			m_localUser->setName(slot->getName());
			}

			if (m_localSlot != i)
			{
				m_connections[i] = newInstance(Connection)();
				m_connections[i]->init();
				m_connections[i]->attachTransport(m_transport);
//				UnsignedShort port = (TheNAT)?TheNAT->getSlotPort(i):8088;
				UnsignedShort port = slot->getPort();
				m_connections[i]->setUser(newInstance(User)(slot->getName(), slot->getIP(), port));
				m_frameData[i] = newInstance(FrameDataManager)(FALSE);
				DEBUG_LOG(("Remote user is at %X:%d", slot->getIP(), slot->getPort()));
			}
			else
			{
				DEBUG_LOG(("Local user is %d (%X:%d)", m_localSlot, slot->getIP(), slot->getPort()));
				m_frameData[i] = newInstance(FrameDataManager)(TRUE);
			}
			m_frameData[i]->init();
			m_frameData[i]->reset();

			m_packetRouterFallback[numUsers] = i;

			++numUsers;

		}
	}
#ifdef MEMORYPOOL_DEBUG
	TheMemoryPoolFactory->debugSetInitFillerIndex(m_localSlot);
#endif

	/*
	if ( numUsers < 2 || m_localSlot == -1 )
	{
		DEBUG_CRASH(("FAILED parseUserList - network game won't work as expected"));
		return;
	}

	char * list = strdup(buf);
	char *listPtr = list;
	if (!list)
		return;

	User users[MAX_SLOTS];
	int localUser = -1;
	int i;

	for (i=0; i<MAX_SLOTS; i++)
	{
		users[i].setName("");
	}

	char *userStr, *nameStr, *addrStr, *portStr;
	AsciiString addrAsciiStr;

	char *listPos;

	DEBUG_LOG(("ConnectionManager::parseUserList - looking for local user at %d.%d.%d.%d:%d",
		PRINTF_IP_AS_4_INTS(m_localAddr),
		m_localPort));

	int numUsers = 0;
	while ( (userStr=strtok_r(listPtr, ",", &listPos)) != nullptr )
	{
		listPtr = nullptr;
		char *pos = nullptr;

		nameStr = strtok_r(userStr, "@", &pos);
		addrStr = strtok_r(nullptr, "@:", &pos);
		portStr = strtok_r(nullptr, ": ", &pos);

		if (!portStr || numUsers >= MAX_SLOTS)
		{
			DEBUG_LOG(("ConnectionManager::parseUserList - (numUsers = %d) FAILED parseUserList with list [%s]", numUsers, buf));
			return;
		}

		addrAsciiStr = addrStr;
		UnsignedInt addr = ResolveIP(addrAsciiStr);
		UnsignedInt port = atoi(portStr);

//		if ((m_localAddr != addr) || (m_localPort != port)) {
		if (loginName.compare(nameStr) != 0) {
			m_connections[numUsers] = newInstance(Connection)();
			m_connections[numUsers]->init();
			m_connections[numUsers]->attachTransport(m_transport);
			m_connections[numUsers]->setUser(newInstance(User)(nameStr, addr, port));

			m_frameData[numUsers] = newInstance(FrameDataManager)(FALSE);

			DEBUG_LOG(("ConnectionManager::parseUserList - User %d is %s", numUsers, nameStr));
		} else {
			m_localSlot = numUsers;
			m_localUser.setName(nameStr);

			DEBUG_LOG(("ConnectionManager::parseUserList - User %d is %s", numUsers, nameStr));
			DEBUG_LOG(("Local user is %d", m_localSlot));

			m_frameData[numUsers] = newInstance(FrameDataManager)(TRUE);
		}
		m_frameData[numUsers]->init();
		m_frameData[numUsers]->reset();

		m_packetRouterFallback[numUsers] = numUsers;

		numUsers++;
	}

	if (numUsers < 2 || m_localSlot == -1)
	{
		DEBUG_LOG(("ConnectionManager::parseUserList - FAILED (local user = %d, num players = %d) with list [%s]", m_localSlot, numUsers, buf));
		return;
	}

	free(list); // from the strdup above.
	list = nullptr;
	*/
}

/**
 * Return the number of incoming bytes per second averaged over 30 sec.
 */
Real ConnectionManager::getIncomingBytesPerSecond( void )
{
	if (m_transport)
		return m_transport->getIncomingBytesPerSecond();
	else
	  return 0.0;
}

/**
 * Return the number of incoming packets per second averaged over the last 30 sec.
 */
Real ConnectionManager::getIncomingPacketsPerSecond( void )
{
	if (m_transport)
		return m_transport->getIncomingPacketsPerSecond();
	else
	  return 0.0;
}

/**
 * Return the number of outgoing bytes per second averaged over the last 30 sec.
 */
Real ConnectionManager::getOutgoingBytesPerSecond( void )
{
	if (m_transport)
		return m_transport->getOutgoingBytesPerSecond();
	else
	  return 0.0;
}

/**
 * Return the number of outgoing packets per second averaged over the last 30 sec.
 */
Real ConnectionManager::getOutgoingPacketsPerSecond( void )
{
	if (m_transport) {
		return m_transport->getOutgoingPacketsPerSecond();
	} else {
	  return 0.0;
	}
}

/**
 * Return the number of bytes not from generals clients received per second averaged over the last 30 sec.
 */
Real ConnectionManager::getUnknownBytesPerSecond( void )
{
	if (m_transport)
		return m_transport->getUnknownBytesPerSecond();
	else
	  return 0.0;
}

/**
 * Return the number ov packets not from generals clients received per second averaged over the last 30 sec.
 */
Real ConnectionManager::getUnknownPacketsPerSecond( void )
{
	if (m_transport)
		return m_transport->getUnknownPacketsPerSecond();
	else
	  return 0.0;
}

/**
 * Return the smallest packet arrival cushion since this was last called.
 */
UnsignedInt ConnectionManager::getPacketArrivalCushion() {
	UnsignedInt retval = m_smallestPacketArrivalCushion;
	m_smallestPacketArrivalCushion = -1;
	return retval;
}

void ConnectionManager::sendChat(UnicodeString text, Int playerMask, UnsignedInt executionFrame)
{
	NetChatCommandMsg *msg = newInstance(NetChatCommandMsg);
	msg->setText(text);
	msg->setPlayerMask(playerMask);
	msg->setPlayerID(m_localSlot);
	msg->setID(0);
	msg->setExecutionFrame(executionFrame);
	if (DoesCommandRequireACommandID(msg->getNetCommandType()))
	{
		msg->setID(GenerateNextCommandID());
	}
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("Chat message has ID of %d, mask of %8.8X, text of %ls", msg->getID(), msg->getPlayerMask(), msg->getText().str()));

	sendLocalCommand(msg, 0xff ^ (1 << m_localSlot));
	processChat(msg);

	msg->detach();
}

void ConnectionManager::sendDisconnectChat(UnicodeString text) {
	NetDisconnectChatCommandMsg *msg = newInstance(NetDisconnectChatCommandMsg);
	msg->setPlayerID(m_localSlot);
	if (DoesCommandRequireACommandID(msg->getNetCommandType())) {
		msg->setID(GenerateNextCommandID());
	}
	msg->setText(text);

	sendLocalCommandDirect(msg, 0xff ^ (1 << m_localSlot));
	processDisconnectChat(msg);
}

UnsignedShort ConnectionManager::sendFileAnnounce(AsciiString path, UnsignedByte playerMask)
{
	File *theFile = TheLocalFileSystem->openFile(path.str());
	if (!theFile || !theFile->size())
	{
		UnicodeString log;
		log.format(L"Not sending file '%hs' to %X", path.str(), playerMask);
		DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("%ls", log.str()));
		if (TheLAN)
			TheLAN->OnChat(L"sendFile", 0, log, LANAPI::LANCHAT_SYSTEM);
		return 0;
	}

	theFile->close();

	Int announceMask = 0xff ^ (1 << m_localSlot);
	NetFileAnnounceCommandMsg *announceMsg = newInstance(NetFileAnnounceCommandMsg);
	announceMsg->setPlayerID(m_localSlot);
	if (DoesCommandRequireACommandID(announceMsg->getNetCommandType()) == TRUE) {
		announceMsg->setID(GenerateNextCommandID());
	}
	announceMsg->setRealFilename(path);
	announceMsg->setPlayerMask(playerMask);
	UnsignedShort fileID = GenerateNextCommandID();
	announceMsg->setFileID(fileID);
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendFileAnnounce() - creating announce message with ID of %d from %d to mask %X for '%s' going to %X as command %d",
		announceMsg->getID(), announceMsg->getPlayerID(), announceMask, announceMsg->getRealFilename().str(),
		announceMsg->getPlayerMask(), announceMsg->getFileID()));

	processFileAnnounce(announceMsg); // set up things for the host

	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("Sending file announce to %X", announceMask));
	sendLocalCommand(announceMsg, announceMask);
	announceMsg->detach();

	return fileID;
}

void ConnectionManager::sendFile(AsciiString path, UnsignedByte playerMask, UnsignedShort commandID)
{
	File *theFile = TheLocalFileSystem->openFile(path.str());
	if (!theFile || !theFile->size())
	{
		UnicodeString log;
		log.format(L"Not sending file '%hs' to %X", path.str(), playerMask);
		DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("%ls", log.str()));
		if (TheLAN)
			TheLAN->OnChat(L"sendFile", 0, log, LANAPI::LANCHAT_SYSTEM);
		return;
	}

	Int len = theFile->size();
	char *buf = theFile->readEntireAndClose();

	// compress Targas
#ifdef COMPRESS_TARGAS
	char *compressedBuf = nullptr;
	Int compressedLen = path.endsWith(".tga")?CompressionManager::getMaxCompressedSize(len, CompressionManager::getPreferredCompression()):0;
	Int compressedSize = 0;
	if (compressedLen)
		compressedSize = CompressionManager::compressData(CompressionManager::getPreferredCompression(),
		buf, len, compressedBuf, compressedLen);

	if (!compressedSize)
	{
		delete[] compressedBuf;
		compressedBuf = nullptr;
	}
#endif // COMPRESS_TARGAS

	NetFileCommandMsg *fileMsg = newInstance(NetFileCommandMsg);
	fileMsg->setPlayerID(m_localSlot);
	fileMsg->setID(commandID);
	fileMsg->setRealFilename(path);
#ifdef COMPRESS_TARGAS
	if (compressedBuf)
	{
		DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("Compressed '%s' from %d to %d (%g%%) before transfer", path.str(), len, compressedSize,
			(Real)compressedSize/(Real)len*100.0f));
		fileMsg->setFileData((unsigned char *)compressedBuf, compressedSize);
	}
	else
#endif // COMPRESS_TARGAS
	{
		fileMsg->setFileData((unsigned char *)buf, len);
	}

	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendFile() - creating file message with ID of %d for '%s' going to %X from %d, size of %d",
		fileMsg->getID(), fileMsg->getRealFilename().str(), playerMask, fileMsg->getPlayerID(), fileMsg->getFileLength()));

	delete[] buf;
	buf = nullptr;
#ifdef COMPRESS_TARGAS
	delete[] compressedBuf;
	compressedBuf = nullptr;
#endif // COMPRESS_TARGAS

	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("Sending file: '%s', len %d, to %X", path.str(), len, playerMask));

	sendLocalCommand(fileMsg, playerMask);

	fileMsg->detach();
}

Int ConnectionManager::getFileTransferProgress(Int playerID, AsciiString path)
{
	FileCommandMap::iterator commandIt = s_fileCommandMap.begin();
	while (commandIt != s_fileCommandMap.end())
	{
		//DEBUG_LOG(("ConnectionManager::getFileTransferProgress(%s): looking at existing transfer of '%s'",
		//	path.str(), commandIt->second.str()));
		if (commandIt->second == path)
		{
			return s_fileProgressMap[playerID][commandIt->first];
		}
		++commandIt;
	}
	//DEBUG_LOG(("Falling back to 0, since we couldn't find the map"));
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::getFileTransferProgress: path %s not found",path.str()));
	return 0;
}


void ConnectionManager::voteForPlayerDisconnect(Int slot) {
	if (m_disconnectManager != nullptr) {
		m_disconnectManager->voteForPlayerDisconnect(slot, this);
	}
}

Int ConnectionManager::getNumPlayers()
{
	Int retval = 0;
	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		if (isPlayerConnected(i)) {
			++retval;
		}
	}

	return retval;
}

void ConnectionManager::updateLoadProgress( Int progress )
{
	NetProgressCommandMsg *msg = newInstance(NetProgressCommandMsg);
	msg->setPercentage( progress );
	msg->setPlayerID( m_localSlot );
	if (DoesCommandRequireACommandID(msg->getNetCommandType()) == TRUE) {
		msg->setID(GenerateNextCommandID());
	}
	processProgress(msg);
	sendLocalCommand(msg, 0xff ^ (1 << m_localSlot));

	msg->detach();
}

void ConnectionManager::loadProgressComplete()
{
	NetLoadCompleteCommandMsg *msg = newInstance(NetLoadCompleteCommandMsg);
	msg->setPlayerID( m_localSlot );
	if (DoesCommandRequireACommandID(msg->getNetCommandType()) == TRUE) {
		msg->setID(GenerateNextCommandID());
	}
	processLoadComplete(msg);
	sendLocalCommand(msg, 0xff ^ (1 << m_localSlot));

	msg->detach();
}

void ConnectionManager::sendTimeOutGameStart()
{
	NetTimeOutGameStartCommandMsg *msg = newInstance(NetTimeOutGameStartCommandMsg);
	msg->setPlayerID( m_localSlot );
	if (DoesCommandRequireACommandID(msg->getNetCommandType()) == TRUE) {
		msg->setID(GenerateNextCommandID());
	}
	processTimeOutGameStart(msg);
	sendLocalCommand(msg, 0xff ^ (1 << m_localSlot));

	msg->detach();
}

Bool ConnectionManager::isPacketRouter( void )
{
	return m_localSlot == m_packetRouterSlot;
}

Int ConnectionManager::getAverageFPS( void )
{
	return m_frameMetrics.getAverageFPS();
}

Int ConnectionManager::getSlotAverageFPS(Int slot) {
	if ((slot < 0) || (slot >= MAX_SLOTS)) {
		return -1;
	}
	if ((m_packetRouterSlot != m_localSlot) && (slot == m_localSlot)) {
		// our framerate data isn't valid for other players unless we are the
		// packet router, so don't fake someone out.
		return -1;
	}
	return m_fpsAverages[slot];
}

#if defined(RTS_DEBUG)
void ConnectionManager::debugPrintConnectionCommands() {
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::debugPrintConnectionCommands - begin commands"));
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if (m_connections[i] != nullptr) {
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::debugPrintConnectionCommands - commands for connection %d", i));
			m_connections[i]->debugPrintCommands();
		}
	}
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::debugPrintConnectionCommands - end commands"));
}
#endif

void ConnectionManager::notifyOthersOfCurrentFrame(Int frame) {
	NetDisconnectFrameCommandMsg *msg = newInstance(NetDisconnectFrameCommandMsg);

	msg->setPlayerID(m_localSlot);
	msg->setDisconnectFrame(frame);
	if (DoesCommandRequireACommandID(msg->getNetCommandType())) {
		msg->setID(GenerateNextCommandID());
	}

	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::notifyOthersOfCurrentFrame - sending disconnect frame of %d, command ID = %d", frame, msg->getID()));
	sendLocalCommandDirect(msg, 0xff ^ (1 << m_localSlot));
	NetCommandRef *ref = NEW_NETCOMMANDREF(msg);
	ref->setRelay(1 << m_localSlot);
	m_disconnectManager->processDisconnectCommand(ref, this);
	deleteInstance(ref);

	msg->detach();

	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::notifyOthersOfCurrentFrame - start screen on debug stuff"));
#if defined(RTS_DEBUG)
	debugPrintConnectionCommands();
#endif
}

void ConnectionManager::notifyOthersOfNewFrame(UnsignedInt frame) {
	NetDisconnectScreenOffCommandMsg *msg = newInstance(NetDisconnectScreenOffCommandMsg);

	msg->setPlayerID(m_localSlot);
	msg->setNewFrame(frame);
	if (DoesCommandRequireACommandID(msg->getNetCommandType())) {
		msg->setID(GenerateNextCommandID());
	}

	sendLocalCommandDirect(msg, 0xff ^ (1 << m_localSlot));
	NetCommandRef *ref = NEW_NETCOMMANDREF(msg);
	ref->setRelay(1 << m_localSlot);
	m_disconnectManager->processDisconnectCommand(ref, this);
	deleteInstance(ref);

	msg->detach();
}

void ConnectionManager::sendFrameDataToPlayer(UnsignedInt playerID, UnsignedInt startingFrame) {
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendFrameDataToPlayer - sending frame data to player %d starting with frame %d", playerID, startingFrame));
	for (UnsignedInt frame = startingFrame; frame < TheGameLogic->getFrame(); ++frame) {
		sendSingleFrameToPlayer(playerID, frame);
	}
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendFrameDataToPlayer - done sending commands to player %d", playerID));
}

void ConnectionManager::sendSingleFrameToPlayer(UnsignedInt playerID, UnsignedInt frame) {
	if ((TheGameLogic->getFrame() - FRAMES_TO_KEEP) > frame) {
		DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendSingleFrameToPlayer - player %d requested frame %d when we are on frame %d, this is too far in the past.", playerID, frame, TheGameLogic->getFrame()));
		return;
	}

	UnsignedByte relay = 1 << playerID;

	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendFrameDataToPlayer - sending data for frame %d", frame));
	for (Int i = 0; i < MAX_SLOTS; ++i) {
		if ((m_frameData[i] != nullptr) && (i != playerID)) { // no need to send his own commands to him.
			NetCommandList *list = m_frameData[i]->getFrameCommandList(frame);
			if (list != nullptr) {
				NetCommandRef *ref = list->getFirstMessage();
				while (ref != nullptr) {
					DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendFrameDataToPlayer - sending command %d from player %d to player %d using relay 0x%x", ref->getCommand()->getID(), i, playerID, relay));
					sendLocalCommandDirect(ref->getCommand(), relay);
					ref = ref->getNext();
				}
			}
			UnsignedInt frameCommandCount = m_frameData[i]->getFrameCommandCount(frame);
			NetFrameCommandMsg *msg = newInstance(NetFrameCommandMsg);
			msg->setExecutionFrame(frame);
			msg->setCommandCount(frameCommandCount);
			if (DoesCommandRequireACommandID(msg->getNetCommandType())) {
				msg->setID(GenerateNextCommandID());
			}
			msg->setPlayerID(i);
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("ConnectionManager::sendFrameDataToPlayer - sending frame info from player %d to player %d for frame %d with command count %d and ID %d and relay %d", i, playerID, msg->getExecutionFrame(), msg->getCommandCount(), msg->getID(), relay));
			sendLocalCommandDirect(msg, relay);
			msg->detach();
		}
	}
}

UnsignedInt ConnectionManager::getNextPacketRouterSlot(UnsignedInt playerID) {
	Int index = 0;
	while ((index < (MAX_SLOTS-1)) && (m_packetRouterFallback[index] != playerID)) {
		++index;
	}
	++index;
	return m_packetRouterFallback[index];
}

void ConnectionManager::requestFrameDataResend(Int playerID, UnsignedInt frame) {
	NetFrameResendRequestCommandMsg *msg = newInstance(NetFrameResendRequestCommandMsg);
	msg->setPlayerID(m_localSlot);
	msg->setFrameToResend(frame);
	if (DoesCommandRequireACommandID(msg->getNetCommandType())) {
		msg->setID(GenerateNextCommandID());
	}

	if (isPlayerConnected(playerID) == FALSE) {
		playerID = 0;
		while ((playerID < MAX_SLOTS) && (isPlayerConnected(playerID) == FALSE)) {
			++playerID;
		}
	}

	if (playerID < MAX_SLOTS) {
		sendLocalCommandDirect(msg, 1 << playerID);
	}

	msg->detach();
}
