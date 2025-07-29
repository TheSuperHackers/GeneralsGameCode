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


#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameNetwork/NetCommandMsg.h"
#include "Common/GameState.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"

/**
 * Base constructor
 */
NetCommandMsg::NetCommandMsg() 
{
	//Added By Sadullah Nader
	//Initializations inserted
	m_executionFrame = 0;
	m_id = 0;
	m_playerID = 0;

	//
	m_timestamp = 0;
	m_referenceCount = 1; // start this off as 1.  This means that an "attach" is implied by creating a NetCommandMsg object.
	m_commandType = NETCOMMANDTYPE_UNKNOWN;
}

/**
 * Destructor
 */
NetCommandMsg::~NetCommandMsg() {
}

/**
 * Adds one to the reference count.
 */
void NetCommandMsg::attach() {
	++m_referenceCount;
}

/**
 * Subtracts one from the reference count. If the reference count is 0, the this object is destroyed.
 */
void NetCommandMsg::detach() {
	--m_referenceCount;
	if (m_referenceCount == 0) {
		deleteInstance(this);
		return;
	}
	DEBUG_ASSERTCRASH(m_referenceCount > 0, ("Invalid reference count for NetCommandMsg")); // Just to make sure...
	if (m_referenceCount < 0) {
		deleteInstance(this);
	}
}

/**
 * Returns the value by which this type of message should be sorted.
 */
Int NetCommandMsg::getSortNumber() {
	return m_id;
}

//-------------------------------
// NetGameCommandMsg
//-------------------------------

/**
 * Constructor with no argument, sets everything to default values.
 */
NetGameCommandMsg::NetGameCommandMsg() : NetCommandMsg() {
	//Added By Sadullah Nader
	//Initializations inserted
	m_argSize = 0;
	m_numArgs = 0;
	//

	m_type = (GameMessage::Type)0;
	m_commandType = NETCOMMANDTYPE_GAMECOMMAND;
	m_argList = NULL;
	m_argTail = NULL;
}

/**
 * Constructor with a GameMessage argument. Sets member variables appropriately for this GameMessage.
 * Also copies all the arguments.
 */
NetGameCommandMsg::NetGameCommandMsg(GameMessage *msg) : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_GAMECOMMAND;

	m_type = msg->getType();
	Int count = msg->getArgumentCount();
	for (Int i = 0; i < count; ++i) {
		addArgument(msg->getArgumentDataType(i), *(msg->getArgument(i)));
	}
}

/**
 * Destructor
 */
NetGameCommandMsg::~NetGameCommandMsg() {
	GameMessageArgument *arg = m_argList;
	while (arg != NULL) {
		m_argList = m_argList->m_next;
		deleteInstance(arg);
		arg = m_argList;
	}
}

/**
 * Add an argument to this command.
 */
void NetGameCommandMsg::addArgument(const GameMessageArgumentDataType type, GameMessageArgumentType arg) 
{
	if (m_argTail == NULL) {
		m_argList = newInstance(GameMessageArgument);	
		m_argTail = m_argList;
		m_argList->m_data = arg;
		m_argList->m_type = type;
		m_argList->m_next = NULL;
		return;
	}

	GameMessageArgument *newArg = newInstance(GameMessageArgument);
	newArg->m_data = arg;
	newArg->m_type = type;
	newArg->m_next = NULL;
	m_argTail->m_next = newArg;
	m_argTail = newArg;
}

// here's where we figure out which slot corresponds to which player
static Int indexFromMask(UnsignedInt mask)
{
	Player *player = NULL;
	Int i;

	for( i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		player = ThePlayerList->getNthPlayer( i );
		if( player && player->getPlayerMask() == mask )
			return i;
	}  // end for i

	return -1;
}

/**
 * Construct a new GameMessage object from the data in this object.
 */
GameMessage *NetGameCommandMsg::constructGameMessage() 
{
	GameMessage *retval = newInstance(GameMessage)(m_type);

	AsciiString name;
	name.format("player%d", getPlayerID());
	retval->friend_setPlayerIndex( ThePlayerList->findPlayerWithNameKey(TheNameKeyGenerator->nameToKey(name))->getPlayerIndex());
//	retval->friend_setPlayerIndex(indexFromMask(ThePlayerList->findPlayerWithNameKey(TheNameKeyGenerator->nameToKey(name))->getPlayerMask()));

	GameMessageArgument *arg = m_argList;
	while (arg != NULL) {
//		retval->appendGenericArgument(arg->m_data);
		if (arg->m_type == ARGUMENTDATATYPE_INTEGER) {
			retval->appendIntegerArgument(arg->m_data.integer);
		} else if (arg->m_type == ARGUMENTDATATYPE_REAL) {
			retval->appendRealArgument(arg->m_data.real);
		} else if (arg->m_type == ARGUMENTDATATYPE_BOOLEAN) {
			retval->appendBooleanArgument(arg->m_data.boolean);
		} else if (arg->m_type == ARGUMENTDATATYPE_OBJECTID) {
			retval->appendObjectIDArgument(arg->m_data.objectID);
		} else if (arg->m_type == ARGUMENTDATATYPE_DRAWABLEID) {
			retval->appendDrawableIDArgument(arg->m_data.drawableID);
		} else if (arg->m_type == ARGUMENTDATATYPE_TEAMID) {
			retval->appendTeamIDArgument(arg->m_data.teamID);
		} else if (arg->m_type == ARGUMENTDATATYPE_LOCATION) {
			retval->appendLocationArgument(arg->m_data.location);
		} else if (arg->m_type == ARGUMENTDATATYPE_PIXEL) {
			retval->appendPixelArgument(arg->m_data.pixel);
		} else if (arg->m_type == ARGUMENTDATATYPE_PIXELREGION) {
			retval->appendPixelRegionArgument(arg->m_data.pixelRegion);
		} else if (arg->m_type == ARGUMENTDATATYPE_TIMESTAMP) {
			retval->appendTimestampArgument(arg->m_data.timestamp);
		} else if (arg->m_type == ARGUMENTDATATYPE_WIDECHAR) {
			retval->appendWideCharArgument(arg->m_data.wChar);
		}
		arg = arg->m_next;
	}
	return retval;
}

/**
 * Sets the type of game message
 */
void NetGameCommandMsg::setGameMessageType(GameMessage::Type type) {
	m_type = type;
}

//-------------------------
// NetAckBothCommandMsg
//-------------------------
/**
 * Constructor.  Sets the member variables according to the given message.
 */
NetAckBothCommandMsg::NetAckBothCommandMsg(NetCommandMsg *msg) : NetCommandMsg() {
	m_commandID = msg->getID();
	m_commandType = NETCOMMANDTYPE_ACKBOTH;
	m_originalPlayerID = msg->getPlayerID();
}

/**
 * Constructor.  Sets the member variables to default values.
 */
NetAckBothCommandMsg::NetAckBothCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_ACKBOTH;
}

/**
 * Destructor.
 */
NetAckBothCommandMsg::~NetAckBothCommandMsg() {
}

/**
 * Returns the command ID of the command being ack'd.
 */
UnsignedShort NetAckBothCommandMsg::getCommandID() {
	return m_commandID;
}

/**
 * Set the command ID of the command being ack'd.
 */
void NetAckBothCommandMsg::setCommandID(UnsignedShort commandID) {
	m_commandID = commandID;
}

/**
 * Get the player id of the player who originally sent the command.
 */
UnsignedByte NetAckBothCommandMsg::getOriginalPlayerID() {
	return m_originalPlayerID;
}

/**
 * Set the player id of the player who originally sent the command.
 */
void NetAckBothCommandMsg::setOriginalPlayerID(UnsignedByte originalPlayerID) {
	m_originalPlayerID = originalPlayerID;
}

Int NetAckBothCommandMsg::getSortNumber() {
	return m_commandID;
}

//-------------------------
// NetAckStage1CommandMsg
//-------------------------
/**
 * Constructor.  Sets the member variables according to the given message.
 */
NetAckStage1CommandMsg::NetAckStage1CommandMsg(NetCommandMsg *msg) : NetCommandMsg() {
	m_commandID = msg->getID();
	m_commandType = NETCOMMANDTYPE_ACKSTAGE1;
	m_originalPlayerID = msg->getPlayerID();
}

/**
 * Constructor.  Sets the member variables to default values.
 */
NetAckStage1CommandMsg::NetAckStage1CommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_ACKSTAGE1;
}

/**
 * Destructor.
 */
NetAckStage1CommandMsg::~NetAckStage1CommandMsg() {
}

/**
 * Returns the command ID of the command being ack'd.
 */
UnsignedShort NetAckStage1CommandMsg::getCommandID() {
	return m_commandID;
}

/**
 * Set the command ID of the command being ack'd.
 */
void NetAckStage1CommandMsg::setCommandID(UnsignedShort commandID) {
	m_commandID = commandID;
}

/**
 * Get the player id of the player who originally sent the command.
 */
UnsignedByte NetAckStage1CommandMsg::getOriginalPlayerID() {
	return m_originalPlayerID;
}

/**
 * Set the player id of the player who originally sent the command.
 */
void NetAckStage1CommandMsg::setOriginalPlayerID(UnsignedByte originalPlayerID) {
	m_originalPlayerID = originalPlayerID;
}

Int NetAckStage1CommandMsg::getSortNumber() {
	return m_commandID;
}

//-------------------------
// NetAckStage2CommandMsg
//-------------------------
/**
 * Constructor.  Sets the member variables according to the given message.
 */
NetAckStage2CommandMsg::NetAckStage2CommandMsg(NetCommandMsg *msg) : NetCommandMsg() {
	m_commandID = msg->getID();
	m_commandType = NETCOMMANDTYPE_ACKSTAGE2;
	m_originalPlayerID = msg->getPlayerID();
}

/**
 * Constructor.  Sets the member variables to default values.
 */
NetAckStage2CommandMsg::NetAckStage2CommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_ACKSTAGE2;
}

/**
 * Destructor.
 */
NetAckStage2CommandMsg::~NetAckStage2CommandMsg() {
}

/**
 * Returns the command ID of the command being ack'd.
 */
UnsignedShort NetAckStage2CommandMsg::getCommandID() {
	return m_commandID;
}

/**
 * Set the command ID of the command being ack'd.
 */
void NetAckStage2CommandMsg::setCommandID(UnsignedShort commandID) {
	m_commandID = commandID;
}

/**
 * Get the player id of the player who originally sent the command.
 */
UnsignedByte NetAckStage2CommandMsg::getOriginalPlayerID() {
	return m_originalPlayerID;
}

/**
 * Set the player id of the player who originally sent the command.
 */
void NetAckStage2CommandMsg::setOriginalPlayerID(UnsignedByte originalPlayerID) {
	m_originalPlayerID = originalPlayerID;
}

Int NetAckStage2CommandMsg::getSortNumber() {
	return m_commandID;
}

//-------------------------
// NetFrameCommandMsg
//-------------------------
/**
 * Constructor.
 */
NetFrameCommandMsg::NetFrameCommandMsg() : NetCommandMsg() {
	m_commandCount = 0;
	m_commandType = NETCOMMANDTYPE_FRAMEINFO;
}

/**
 * Destructor
 */
NetFrameCommandMsg::~NetFrameCommandMsg() {
}

/**
 * Set the command count of this frame.
 */
void NetFrameCommandMsg::setCommandCount(UnsignedShort commandCount) {
	m_commandCount = commandCount;
}

/**
 * Return the command count of this frame.
 */
UnsignedShort NetFrameCommandMsg::getCommandCount() {
	return m_commandCount;
}

//-------------------------
// NetPlayerLeaveCommandMsg
//-------------------------
/**
 * Constructor
 */
NetPlayerLeaveCommandMsg::NetPlayerLeaveCommandMsg() : NetCommandMsg() {
	m_leavingPlayerID = 0;
	m_commandType = NETCOMMANDTYPE_PLAYERLEAVE;
}

/**
 * Destructor
 */
NetPlayerLeaveCommandMsg::~NetPlayerLeaveCommandMsg() {
}

/**
 * Get the id of the player leaving the game.
 */
UnsignedByte NetPlayerLeaveCommandMsg::getLeavingPlayerID() {
	return m_leavingPlayerID;
}

/**
 * Set the id of the player leaving the game.
 */
void NetPlayerLeaveCommandMsg::setLeavingPlayerID(UnsignedByte id) {
	m_leavingPlayerID = id;
}

//-------------------------
// NetRunAheadMetricsCommandMsg
//-------------------------
/**
 * Constructor
 */
NetRunAheadMetricsCommandMsg::NetRunAheadMetricsCommandMsg() : NetCommandMsg() {
	m_averageLatency = 0.0;
	m_averageFps = 0;
	m_commandType = NETCOMMANDTYPE_RUNAHEADMETRICS;
}

/**
 * Destructor
 */
NetRunAheadMetricsCommandMsg::~NetRunAheadMetricsCommandMsg() {
}

/**
 * set the average latency
 */
void NetRunAheadMetricsCommandMsg::setAverageLatency(Real avgLat) {
	m_averageLatency = avgLat;
}

/**
 * get the average latency
 */
Real NetRunAheadMetricsCommandMsg::getAverageLatency() {
	return m_averageLatency;
}

/**
 * set the average fps
 */
void NetRunAheadMetricsCommandMsg::setAverageFps(Int fps) {
	m_averageFps = fps;
}

/**
 * get the average fps
 */
Int NetRunAheadMetricsCommandMsg::getAverageFps() {
	return m_averageFps;
}

//-------------------------
// NetRunAheadCommandMsg
//-------------------------
NetRunAheadCommandMsg::NetRunAheadCommandMsg() : NetCommandMsg() {
	m_runAhead = min(max(20, MIN_RUNAHEAD), MAX_FRAMES_AHEAD/2);
	m_frameRate = 30;
	m_commandType = NETCOMMANDTYPE_RUNAHEAD;
}

NetRunAheadCommandMsg::~NetRunAheadCommandMsg() {
}

UnsignedShort NetRunAheadCommandMsg::getRunAhead() {
	return m_runAhead;
}

void NetRunAheadCommandMsg::setRunAhead(UnsignedShort runAhead) {
	m_runAhead = runAhead;
}

UnsignedByte NetRunAheadCommandMsg::getFrameRate() {
	return m_frameRate;
}

void NetRunAheadCommandMsg::setFrameRate(UnsignedByte frameRate) {
	m_frameRate = frameRate;
}

//-------------------------
// NetDestroyPlayerCommandMsg
//-------------------------
/**
 * Constructor
 */
NetDestroyPlayerCommandMsg::NetDestroyPlayerCommandMsg() : NetCommandMsg()
{
	m_playerIndex = 0;
	m_commandType = NETCOMMANDTYPE_DESTROYPLAYER;
}

/**
 * Destructor
 */
NetDestroyPlayerCommandMsg::~NetDestroyPlayerCommandMsg()
{
}

/**
 * set the CRC
 */
void NetDestroyPlayerCommandMsg::setPlayerIndex( UnsignedInt playerIndex )
{
	m_playerIndex = playerIndex;
}

/**
 * get the average CRC
 */
UnsignedInt NetDestroyPlayerCommandMsg::getPlayerIndex( void )
{
	return m_playerIndex;
}

//-------------------------
// NetKeepAliveCommandMsg
//-------------------------
/**
 * Constructor
 */
NetKeepAliveCommandMsg::NetKeepAliveCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_KEEPALIVE;
}

NetKeepAliveCommandMsg::~NetKeepAliveCommandMsg() {
}

//-------------------------
// NetDisconnectKeepAliveCommandMsg
//-------------------------
/**
 * Constructor
 */
NetDisconnectKeepAliveCommandMsg::NetDisconnectKeepAliveCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_DISCONNECTKEEPALIVE;
}

NetDisconnectKeepAliveCommandMsg::~NetDisconnectKeepAliveCommandMsg() {
}

//-------------------------
// NetDisconnectPlayerCommandMsg
//-------------------------
/**
 * Constructor
 */
NetDisconnectPlayerCommandMsg::NetDisconnectPlayerCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_DISCONNECTPLAYER;
	m_disconnectSlot = 0;
}

/**
 * Destructor
 */
NetDisconnectPlayerCommandMsg::~NetDisconnectPlayerCommandMsg() {
}

/**
 * Returns the disconnecting slot number
 */
UnsignedByte NetDisconnectPlayerCommandMsg::getDisconnectSlot() {
	return m_disconnectSlot;
}

/**
 * Sets the disconnecting slot number
 */
void NetDisconnectPlayerCommandMsg::setDisconnectSlot(UnsignedByte slot) {
	m_disconnectSlot = slot;
}

/**
 * Sets the disconnect frame
 */
void NetDisconnectPlayerCommandMsg::setDisconnectFrame(UnsignedInt frame) {
	m_disconnectFrame = frame;
}

/**
 * returns the disconnect frame
 */
UnsignedInt NetDisconnectPlayerCommandMsg::getDisconnectFrame() {
	return m_disconnectFrame;
}

//-------------------------
// NetPacketRouterQueryCommandMsg
//-------------------------
/**
 * Constructor
 */
NetPacketRouterQueryCommandMsg::NetPacketRouterQueryCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_PACKETROUTERQUERY;
}

/**
 * Destructor
 */
NetPacketRouterQueryCommandMsg::~NetPacketRouterQueryCommandMsg() {
}

//-------------------------
// NetPacketRouterAckCommandMsg
//-------------------------
/**
 * Constructor
 */
NetPacketRouterAckCommandMsg::NetPacketRouterAckCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_PACKETROUTERACK;
}

/**
 * Destructor
 */
NetPacketRouterAckCommandMsg::~NetPacketRouterAckCommandMsg() {
}

//-------------------------
// NetDisconnectChatCommandMsg
//-------------------------
/**
 * Constructor
 */
NetDisconnectChatCommandMsg::NetDisconnectChatCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_DISCONNECTCHAT;
}

/**
 * Destructor
 */
NetDisconnectChatCommandMsg::~NetDisconnectChatCommandMsg() {
}

/**
 * Set the chat text for this message.
 */
void NetDisconnectChatCommandMsg::setText(UnicodeString text) {
	m_text = text;
}

/**
 * Get the chat text for this message.
 */
UnicodeString NetDisconnectChatCommandMsg::getText() {
	return m_text;
}

//-------------------------
// NetChatCommandMsg
//-------------------------
/**
 * Constructor
 */
NetChatCommandMsg::NetChatCommandMsg() : NetCommandMsg()
{
	m_commandType = NETCOMMANDTYPE_CHAT;
	//added by Sadullah Nader
	//Initializations inserted
	m_playerMask = 0;
	//
}

/**
 * Destructor
 */
NetChatCommandMsg::~NetChatCommandMsg()
{
}

/**
 * Set the chat text for this message.
 */
void NetChatCommandMsg::setText(UnicodeString text)
{
	m_text = text;
}

/**
 * Get the chat text for this message.
 */
UnicodeString NetChatCommandMsg::getText()
{
	return m_text;
}

/**
 * Get the bitmask of chat recipients from this message.
 */
Int NetChatCommandMsg::getPlayerMask()
{
	return m_playerMask;
}

/**
 * Set a bitmask of chat recipients in this message.
 */
void NetChatCommandMsg::setPlayerMask( Int playerMask )
{
	m_playerMask = playerMask;
}

//-------------------------
// NetDisconnectVoteCommandMsg
//-------------------------
/**
 * Constructor
 */
NetDisconnectVoteCommandMsg::NetDisconnectVoteCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_DISCONNECTVOTE;
	m_slot = 0;
}

/**
 * Destructor
 */
NetDisconnectVoteCommandMsg::~NetDisconnectVoteCommandMsg() {
}

/**
 * Set the slot that is being voted for.
 */
void NetDisconnectVoteCommandMsg::setSlot(UnsignedByte slot) {
	m_slot = slot;
}

/**
 * Get the slot that is being voted for.
 */
UnsignedByte NetDisconnectVoteCommandMsg::getSlot() {
	return m_slot;
}

/**
 * Get the vote frame.
 */
UnsignedInt NetDisconnectVoteCommandMsg::getVoteFrame() {
	return m_voteFrame;
}

/**
 * Set the vote frame.
 */
void NetDisconnectVoteCommandMsg::setVoteFrame(UnsignedInt voteFrame) {
	m_voteFrame = voteFrame;
}

//-------------------------
// NetProgressCommandMsg
//-------------------------
NetProgressCommandMsg::NetProgressCommandMsg( void ) : NetCommandMsg()
{
	m_commandType = NETCOMMANDTYPE_PROGRESS;
	m_percent = 0;
}

NetProgressCommandMsg::~NetProgressCommandMsg( void ) {}
		
UnsignedByte NetProgressCommandMsg::getPercentage()
{
	return m_percent;
}

void NetProgressCommandMsg::setPercentage( UnsignedByte percent )
{
	m_percent = percent;
}

//-------------------------
// NetWrapperCommandMsg
//-------------------------
NetWrapperCommandMsg::NetWrapperCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_WRAPPER;
	m_numChunks = 0;
	m_data = NULL;
	m_totalDataLength = 0;
	m_chunkNumber = 0;
	m_dataLength = 0;
	m_dataOffset = 0;
	m_wrappedCommandID = 0;
}

NetWrapperCommandMsg::~NetWrapperCommandMsg() {
	if (m_data != NULL) {
		delete m_data;
		m_data = NULL;
	}
}

UnsignedByte * NetWrapperCommandMsg::getData() {
	return m_data;
}

void NetWrapperCommandMsg::setData(UnsignedByte *data, UnsignedInt dataLength) 
{
	if (m_data != NULL) {
		delete m_data;
		m_data = NULL;
	}

	m_data = NEW UnsignedByte[dataLength];	// pool[]ify
	memcpy(m_data, data, dataLength);
	m_dataLength = dataLength;
}

UnsignedInt NetWrapperCommandMsg::getDataLength() {
	return m_dataLength;
}

UnsignedInt NetWrapperCommandMsg::getDataOffset() {
	return m_dataOffset;
}

void NetWrapperCommandMsg::setDataOffset(UnsignedInt offset) {
	m_dataOffset = offset;
}

UnsignedInt NetWrapperCommandMsg::getChunkNumber() {
	return m_chunkNumber;
}

void NetWrapperCommandMsg::setChunkNumber(UnsignedInt chunkNumber) {
	m_chunkNumber = chunkNumber;
}

UnsignedInt NetWrapperCommandMsg::getNumChunks() {
	return m_numChunks;
}

void NetWrapperCommandMsg::setNumChunks(UnsignedInt numChunks) {
	m_numChunks = numChunks;
}

UnsignedInt NetWrapperCommandMsg::getTotalDataLength() {
	return m_totalDataLength;
}

void NetWrapperCommandMsg::setTotalDataLength(UnsignedInt totalDataLength) {
	m_totalDataLength = totalDataLength;
}

UnsignedShort NetWrapperCommandMsg::getWrappedCommandID() {
	return m_wrappedCommandID;
}

void NetWrapperCommandMsg::setWrappedCommandID(UnsignedShort wrappedCommandID) {
	m_wrappedCommandID = wrappedCommandID;
}

//-------------------------
// NetFileCommandMsg
//-------------------------
NetFileCommandMsg::NetFileCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_FILE;
	m_data = NULL;
	m_portableFilename.clear();
	m_dataLength = 0;
}

NetFileCommandMsg::~NetFileCommandMsg() {
	if (m_data != NULL) {
		delete[] m_data;
		m_data = NULL;
	}
}

AsciiString NetFileCommandMsg::getRealFilename() 
{
	return TheGameState->portableMapPathToRealMapPath(m_portableFilename);
}

void NetFileCommandMsg::setRealFilename(AsciiString filename) 
{
	m_portableFilename = TheGameState->realMapPathToPortableMapPath(filename);
}

UnsignedInt NetFileCommandMsg::getFileLength() {
	return m_dataLength;
}

UnsignedByte * NetFileCommandMsg::getFileData() {
	return m_data;
}

void NetFileCommandMsg::setFileData(UnsignedByte *data, UnsignedInt dataLength) 
{
	m_dataLength = dataLength;
	m_data = NEW UnsignedByte[dataLength];	// pool[]ify
	memcpy(m_data, data, dataLength);
}

//-------------------------
// NetFileAnnounceCommandMsg
//-------------------------
NetFileAnnounceCommandMsg::NetFileAnnounceCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_FILEANNOUNCE;
	m_portableFilename.clear();
	m_fileID = 0;
	m_playerMask = 0;
}

NetFileAnnounceCommandMsg::~NetFileAnnounceCommandMsg() {
}

AsciiString NetFileAnnounceCommandMsg::getRealFilename() 
{
	return TheGameState->portableMapPathToRealMapPath(m_portableFilename);
}

void NetFileAnnounceCommandMsg::setRealFilename(AsciiString filename) 
{
	m_portableFilename = TheGameState->realMapPathToPortableMapPath(filename);
}

UnsignedShort NetFileAnnounceCommandMsg::getFileID() {
	return m_fileID;
}

void NetFileAnnounceCommandMsg::setFileID(UnsignedShort fileID) {
	m_fileID = fileID;
}

UnsignedByte NetFileAnnounceCommandMsg::getPlayerMask(void) {
	return m_playerMask;
}

void NetFileAnnounceCommandMsg::setPlayerMask(UnsignedByte playerMask) {
	m_playerMask = playerMask;
}


//-------------------------
// NetFileProgressCommandMsg
//-------------------------
NetFileProgressCommandMsg::NetFileProgressCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_FILEPROGRESS;
	m_fileID = 0;
	m_progress = 0;
}

NetFileProgressCommandMsg::~NetFileProgressCommandMsg() {
}

UnsignedShort NetFileProgressCommandMsg::getFileID() {
	return m_fileID;
}

void NetFileProgressCommandMsg::setFileID(UnsignedShort val) {
	m_fileID = val;
}

Int NetFileProgressCommandMsg::getProgress() {
	return m_progress;
}

void NetFileProgressCommandMsg::setProgress(Int val) {
	m_progress = val;
}

//-------------------------
// NetDisconnectFrameCommandMsg
//-------------------------
NetDisconnectFrameCommandMsg::NetDisconnectFrameCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_DISCONNECTFRAME;
	m_disconnectFrame = 0;
}

NetDisconnectFrameCommandMsg::~NetDisconnectFrameCommandMsg() {
}

UnsignedInt NetDisconnectFrameCommandMsg::getDisconnectFrame() {
	return m_disconnectFrame;
}

void NetDisconnectFrameCommandMsg::setDisconnectFrame(UnsignedInt disconnectFrame) {
	m_disconnectFrame = disconnectFrame;
}

//-------------------------
// NetDisconnectScreenOffCommandMsg
//-------------------------
NetDisconnectScreenOffCommandMsg::NetDisconnectScreenOffCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_DISCONNECTSCREENOFF;
	m_newFrame = 0;
}

NetDisconnectScreenOffCommandMsg::~NetDisconnectScreenOffCommandMsg() {
}

UnsignedInt NetDisconnectScreenOffCommandMsg::getNewFrame() {
	return m_newFrame;
}

void NetDisconnectScreenOffCommandMsg::setNewFrame(UnsignedInt newFrame) {
	m_newFrame = newFrame;
}

//-------------------------
// NetFrameResendRequestCommandMsg
//-------------------------
NetFrameResendRequestCommandMsg::NetFrameResendRequestCommandMsg() : NetCommandMsg() {
	m_commandType = NETCOMMANDTYPE_FRAMERESENDREQUEST;
	m_frameToResend = 0;
}

NetFrameResendRequestCommandMsg::~NetFrameResendRequestCommandMsg() {
}

UnsignedInt NetFrameResendRequestCommandMsg::getFrameToResend() {
	return m_frameToResend;
}

void NetFrameResendRequestCommandMsg::setFrameToResend(UnsignedInt frame) {
	m_frameToResend = frame;
}
