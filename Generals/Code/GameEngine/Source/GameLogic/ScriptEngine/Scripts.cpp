/*
**	Command & Conquer Generals(tm)
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

// FILE: Scripts.cpp /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   Generals
//
// File name: Scripts.cpp
//
// Created:   John Ahlquist, Nov 2001
//
// Desc:      Contains the information describing scripts.
//
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Lib/BaseType.h"

#define DEFINE_BUILDABLE_STATUS_NAMES
#define DEFINE_OBJECT_STATUS_NAMES
#define DEFINE_SCIENCE_AVAILABILITY_NAMES

#include "Common/BorderColors.h"
#include "Common/DataChunk.h"
#include "Common/GameState.h"
#include "Common/KindOf.h"
#include "Common/Radar.h"
#include "Common/ThingTemplate.h"
#include "Common/Player.h"
#include "Common/Xfer.h"

#include "GameClient/ShellHooks.h"

#include "GameLogic/AI.h"
#include "GameLogic/Object.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/SidesList.h"


#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif


static Script *s_mtScript = NULL;
static ScriptGroup *s_mtGroup = NULL;

//
// These strings must be in the same order as they are in their definitions 
// (See SHELL_SCRIPT_HOOK_* )
//
const char *TheShellHookNames[]=
{
	"ShellMainMenuCampaignPushed", //SHELL_SCRIPT_HOOK_MAIN_MENU_CAMPAIGN_SELECTED,
	"ShellMainMenuCampaignHighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_CAMPAIGN_HIGHLIGHTED,
	"ShellMainMenuCampaignUnhighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_CAMPAIGN_UNHIGHLIGHTED,

	"ShellMainMenuSkirmishPushed", //SHELL_SCRIPT_HOOK_MAIN_MENU_SKIRMISH_SELECTED,
	"ShellMainMenuSkirmishHighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_SKIRMISH_HIGHLIGHTED,
	"ShellMainMenuSkirmishUnhighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_SKIRMISH_UNHIGHLIGHTED,

	"ShellMainMenuOptionsPushed", //SHELL_SCRIPT_HOOK_MAIN_MENU_OPTIONS_SELECTED,
	"ShellMainMenuOptionsHighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_OPTIONS_HIGHLIGHTED,
	"ShellMainMenuOptionsUnhighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_OPTIONS_UNHIGHLIGHTED,

	"ShellMainMenuOnlinePushed", //SHELL_SCRIPT_HOOK_MAIN_MENU_ONLINE_SELECTED,
	"ShellMainMenuOnlineHighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_ONLINE_HIGHLIGHTED,
	"ShellMainMenuOnlineUnhighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_ONLINE_UNHIGHLIGHTED,

	"ShellMainMenuNetworkPushed", //SHELL_SCRIPT_HOOK_MAIN_MENU_NETWORK_SELECTED,
	"ShellMainMenuNetworkHighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_NETWORK_HIGHLIGHTED,
	"ShellMainMenuNetworkUnhighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_NETWORK_UNHIGHLIGHTED,

	"ShellMainMenuExitPushed", //SHELL_SCRIPT_HOOK_MAIN_MENU_EXIT_SELECTED,
	"ShellMainMenuExitHighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_EXIT_HIGHLIGHTED,
	"ShellMainMenuExitUnhighlighted", //SHELL_SCRIPT_HOOK_MAIN_MENU_EXIT_UNHIGHLIGHTED,

	"ShellGeneralsOnlineLogin", //SHELL_SCRIPT_HOOK_GENERALS_ONLINE_LOGIN,
	"ShellGeneralsOnlineLogout", //SHELL_SCRIPT_HOOK_GENERALS_ONLINE_LOGOUT,
	"ShellGeneralsOnlineEnteredFromGame", //SHELL_SCRIPT_HOOK_GENERALS_ONLINE_ENTERED_FROM_GAME,

	"ShellOptionsOpened", //SHELL_SCRIPT_HOOK_OPTIONS_OPENED,
	"ShellOptionsClosed", //SHELL_SCRIPT_HOOK_OPTIONS_CLOSED,

	"ShellSkirmishOpened", //SHELL_SCRIPT_HOOK_SKIRMISH_OPENED,
	"ShellSkirmishClosed", //SHELL_SCRIPT_HOOK_SKIRMISH_CLOSED,
	"ShellSkirmishEnteredFromGame", //SHELL_SCRIPT_HOOK_SKIRMISH_ENTERED_FROM_GAME,

	"ShellLANOpened", //SHELL_SCRIPT_HOOK_LAN_OPENED,
	"ShellLANClosed", //SHELL_SCRIPT_HOOK_LAN_CLOSED,
	"ShellLANEnteredFromGame", //SHELL_SCRIPT_HOOK_LAN_ENTERED_FROM_GAME,
};
void SignalUIInteraction(Int interaction)
{
	if (TheScriptEngine)
		TheScriptEngine->signalUIInteract(TheShellHookNames[interaction]);
}


// Changing the order or meaning of either of these will require you to update the maps
// in a meaningful way. If there are new entries, add them to the end, rather than the middle.
const char *Surfaces[] = { "Ground", "Air", "Ground or Air", };
const char *ShakeIntensities[] = { "Subtle", "Normal", "Strong", "Severe", "Cine_Extreme", "Cine_Insane" };

enum { K_SCRIPT_LIST_DATA_VERSION_1 = 1,
			K_SCRIPT_GROUP_DATA_VERSION_1 = 1,
			K_SCRIPT_GROUP_DATA_VERSION_2 = 2,
			K_SCRIPT_DATA_VERSION_1 = 1,
			K_SCRIPT_DATA_VERSION_2 = 2,
			K_SCRIPT_OR_CONDITION_DATA_VERSION_1=1, 
			K_SCRIPT_ACTION_VERSION_1 = 1,
			K_SCRIPT_CONDITION_VERSION_1 = 1,
			K_SCRIPT_CONDITION_VERSION_2 = 2,
			K_SCRIPT_CONDITION_VERSION_3 = 3,
			K_SCRIPTS_DATA_VERSION_1,
			end_of_the_enumeration
};

static Condition::ConditionType ParameterChangesVer2[] = 
{ 
	// Seven Changed from version 1.
	Condition::TEAM_INSIDE_AREA_PARTIALLY, 
	Condition::TEAM_INSIDE_AREA_ENTIRELY, 
	Condition::TEAM_OUTSIDE_AREA_ENTIRELY, 
	Condition::TEAM_ENTERED_AREA_PARTIALLY, 
	Condition::TEAM_ENTERED_AREA_ENTIRELY, 
	Condition::TEAM_EXITED_AREA_ENTIRELY, 
	Condition::TEAM_EXITED_AREA_PARTIALLY, 
	(Condition::ConditionType) -1,
};

enum { AT_END = 0x00FFFFFF };

//-------------------------------------------------------------------------------------------------
// ******************************** class  ScriptList *********************************************
//-------------------------------------------------------------------------------------------------
// Statics ///////////////////////////////////////////////////////////////////////////////////////
ScriptList *ScriptList::s_readLists[MAX_PLAYER_COUNT] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
Int					ScriptList::s_numInReadList = 0;

Int ScriptList::m_curId = 0;

/**
 ScriptList::updateDefaults -  checks for empty script lists, and adds some default stuff
 so you don't get a totally blank screen in the editor.
*/
void ScriptList::updateDefaults(void) 
{
	Int i;
	for (i=0; i<TheSidesList->getNumSides(); i++) 
	{
		ScriptList* pList = TheSidesList->getSideInfo(i)->getScriptList();
		if (pList == NULL) {
			pList = newInstance(ScriptList);	
			TheSidesList->getSideInfo(i)->setScriptList(pList);
		}
	}
}

/**
  Deletes any script lists attached to sides.  Used for editor cleanup.
*/
void ScriptList::reset(void) 
{
	Int i;
	if (TheSidesList == NULL) return; /// @todo - move this code into sides list.
	for (i=0; i<TheSidesList->getNumSides(); i++) 
	{
		ScriptList* pList = TheSidesList->getSideInfo(i)->getScriptList();
		TheSidesList->getSideInfo(i)->setScriptList(NULL);
		pList->deleteInstance();
	}
}



/**
  Ctor.
*/
ScriptList::ScriptList(void) :
m_firstGroup(NULL),
m_firstScript(NULL)
{
}

/**
  Dtor.  Deletes any script lists or group lists.  Note that dtors for groups and script lists
	delete the whole list, so don't need to traverse here.
*/
ScriptList::~ScriptList(void) 
{
	if (m_firstGroup) {
		m_firstGroup->deleteInstance();
		m_firstGroup = NULL;
	}
	if (m_firstScript) {
		m_firstScript->deleteInstance();
		m_firstScript = NULL;
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ScriptList::crc( Xfer *xfer )
{

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ScriptList::xfer( Xfer *xfer )
{
	UnsignedShort countVerify;

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// count of scripts here
	Script *script;
	UnsignedShort scriptCount = 0;
	for( script = getScript(); script; script = script->getNext() )
		scriptCount++;
	countVerify = scriptCount; 
	xfer->xferUnsignedShort( &scriptCount );
	if( countVerify != scriptCount )
	{

		DEBUG_CRASH(( "ScriptList::xfer - Script list count has changed, attempting to recover."));
		// throw SC_INVALID_DATA; try to recover. jba.

	}  // end if

	// all script data here
	for( script = getScript(); script; script = script->getNext() )	{
		xfer->xferSnapshot( script );
		scriptCount--;
		if (scriptCount==0) break;
	}
	if (scriptCount>0) {
		DEBUG_CRASH(("Stripping out extra scripts - Bad..."));
		if (s_mtScript==NULL) s_mtScript = newInstance(Script);	// Yes it leaks, but this is unusual recovery only. jba.
		while (scriptCount) {
			xfer->xferSnapshot(s_mtScript);
			scriptCount--;
		}
	}

	// count of script groups
	ScriptGroup *scriptGroup;
	UnsignedShort scriptGroupCount = 0;
	for( scriptGroup = getScriptGroup(); scriptGroup; scriptGroup = scriptGroup->getNext() )
		scriptGroupCount++;
	countVerify = scriptGroupCount;
	xfer->xferUnsignedShort( &scriptGroupCount );
	if( countVerify != scriptGroupCount )
	{

		DEBUG_CRASH(( "ScriptList::xfer - Script group count has changed, attempting to recover."));

	}  // end if

	// all script group data
	for( scriptGroup = getScriptGroup(); scriptGroup; scriptGroup = scriptGroup->getNext() ) {
		xfer->xferSnapshot( scriptGroup );
		scriptGroupCount--;
		if (scriptGroupCount==0) break;
	}
	if (scriptGroupCount>0) {
		DEBUG_CRASH(("Stripping out extra groups. - Bad..."));
		if (s_mtGroup == NULL) s_mtGroup = newInstance(ScriptGroup);	// Yes it leaks, but this is only for recovery.
		while (scriptGroupCount) {
			xfer->xferSnapshot(s_mtGroup);
			scriptGroupCount--;
		}
	}

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ScriptList::loadPostProcess( void )
{

}

/**
  ScriptList::duplicate - Creates a full, "deep" copy of scriptlist. 
*/
ScriptList *ScriptList::duplicate(void) const 
{
	ScriptList *pNew = newInstance(ScriptList);

	{
		const ScriptGroup *src = this->m_firstGroup;
		ScriptGroup *dst = NULL;
		while (src)
		{
			ScriptGroup *tmp = src->duplicate();

			if (dst)
				dst->setNextGroup(tmp);
			else
				pNew->m_firstGroup = tmp;

			src = src->getNext();
			dst = tmp;
		}
	}

	{
		const Script *src = this->m_firstScript;
		Script *dst = NULL;
		while (src)
		{
			Script *tmp = src->duplicate();

			if (dst)
				dst->setNextScript(tmp);
			else
				pNew->m_firstScript = tmp;

			src = src->getNext();
			dst = tmp;
		}
	}

	return pNew;
}

/**
  ScriptList::duplicateAndQualify - Creates a full, "deep" copy of scriptlist,
	adding the qualifier to names. 
*/
ScriptList *ScriptList::duplicateAndQualify(const AsciiString& qualifier, 
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const 
{
	ScriptList *pNew = newInstance(ScriptList);

	{
		const ScriptGroup *src = this->m_firstGroup;
		ScriptGroup *dst = NULL;
		while (src)
		{
			ScriptGroup *tmp = src->duplicateAndQualify( qualifier, playerTemplateName, newPlayerName);

			if (dst)
				dst->setNextGroup(tmp);
			else
				pNew->m_firstGroup = tmp;

			src = src->getNext();
			dst = tmp;
		}
	}

	{
		const Script *src = this->m_firstScript;
		Script *dst = NULL;
		while (src)
		{
			Script *tmp = src->duplicateAndQualify(qualifier, playerTemplateName, newPlayerName);

			if (dst)
				dst->setNextScript(tmp);
			else
				pNew->m_firstScript = tmp;

			src = src->getNext();
			dst = tmp;
		}
	}

	return pNew;
}

/**
  ScriptList::discard - Deletes a script list, but not any children. 
*/
void ScriptList::discard(void)  
{
	m_firstGroup = NULL;
	m_firstScript = NULL;
	this->deleteInstance();
}

/**
  Add a script group to the current list of groups.  Offset to position ndx.
*/
void ScriptList::addGroup(ScriptGroup *pGrp, Int ndx)
{
	ScriptGroup *pPrev = NULL;
	ScriptGroup *pCur = m_firstGroup;
	DEBUG_ASSERTCRASH(pGrp->getNext()==NULL, ("Adding already linked group."));
	while (ndx && pCur) {
		pPrev = pCur;
		pCur = pCur->getNext();
		ndx--;
	}
	if (pPrev) {
		// Weave into prev link.
		pGrp->setNextGroup(pPrev->getNext());
		pPrev->setNextGroup(pGrp);
	} else {
		// Goes at head of list.
		pGrp->setNextGroup(m_firstGroup);
		m_firstGroup = pGrp;
	}
}

/**
  Add a script to the current list of scripts.  Offset to position ndx.
*/
void ScriptList::addScript(Script *pScr, Int ndx)
{
	Script *pPrev = NULL;
	Script *pCur = m_firstScript;
	DEBUG_ASSERTCRASH(pScr->getNext()==NULL, ("Adding already linked group."));
	while (ndx && pCur) {
		pPrev = pCur;
		pCur = pCur->getNext();
		ndx--;
	}
	if (pPrev) {
		pScr->setNextScript(pPrev->getNext());
		pPrev->setNextScript(pScr);
	} else {
		pScr->setNextScript(m_firstScript);
		m_firstScript = pScr;
	}
}

/**
  Delete a script from the current list of scripts.
*/
void ScriptList::deleteScript(Script *pScr)
{
	Script *pPrev = NULL;
	Script *pCur = m_firstScript;
	while (pCur != pScr) {
		pPrev = pCur;
		pCur = pCur->getNext();
	}
	DEBUG_ASSERTCRASH(pCur, ("Couldn't find script."));
	if (pCur==NULL) return;

	if (pPrev) {
		// unlink from previous script.
		pPrev->setNextScript(pCur->getNext());
	} else {
		// Unlink from head of list.
		m_firstScript = pCur->getNext();
	}
	// Clear the link & delete.
	pCur->setNextScript(NULL);
	pCur->deleteInstance();
}

/**
  Delete a group from the current list of groups.
*/
void ScriptList::deleteGroup(ScriptGroup *pGrp)
{
	ScriptGroup *pPrev = NULL;
	ScriptGroup *pCur = m_firstGroup;
	while (pCur != pGrp) {
		pPrev = pCur;
		pCur = pCur->getNext();
	}
	DEBUG_ASSERTCRASH(pCur, ("Couldn't find group."));
	if (pCur==NULL) return;
	if (pPrev) {
		// unlink from previous group.
		pPrev->setNextGroup(pCur->getNext());
	} else {
		// Unlink from head of list.
		m_firstGroup = pCur->getNext();
	}
	// Clear the link & delete.
	pCur->setNextGroup(NULL);
	pCur->deleteInstance();
}

/**
* ScriptList::ParseScriptsDataChunk - read a Scripts chunk.
* Format is the newer CHUNKY format.
*	See ScriptList::ScriptList for the writer.
*	Input: DataChunkInput 
*		
*/
Bool ScriptList::ParseScriptsDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Int i;
	file.registerParser( AsciiString("ScriptList"), info->label, ScriptList::ParseScriptListDataChunk );
	DEBUG_ASSERTCRASH(s_numInReadList==0, ("Leftover scripts floating aroung."));
	for (i=0; i<s_numInReadList; i++) {
		if (s_readLists[i]) {
			s_readLists[i]->deleteInstance();
			s_readLists[i] = NULL;
		}
	}
	TScriptListReadInfo readInfo;
	for (i=0; i<MAX_PLAYER_COUNT; i++) {
		readInfo.readLists[i] = 0;
	}
	readInfo.numLists = 0;
	if (file.parse(&readInfo)) {
		DEBUG_ASSERTCRASH(readInfo.numLists<MAX_PLAYER_COUNT, ("Read too many, overrun buffer."));
		s_numInReadList = readInfo.numLists;
		for (i=0; i<s_numInReadList; i++) {
			s_readLists[i] = readInfo.readLists[i];
		}
		return true;
	}
	return false;
}

/**
* ScriptList::getReadScripts - Gets the scripts read in from a file by .
* ScriptList::ParseScriptsDataChunk.
*		
*/
Int ScriptList::getReadScripts(ScriptList *scriptLists[MAX_PLAYER_COUNT])
{		 
	Int i;
	Int count = s_numInReadList;
	s_numInReadList = 0;
	for (i=0; i<count; i++) {
		scriptLists[i] = s_readLists[i];
		s_readLists[i] = NULL;
	}
	return count;
}								 

/**
* ScriptList::WriteScriptsDataChunk - Writes a Scripts chunk.
* Format is the newer CHUNKY format.
*	See ScriptEngine::ParseScriptsDataChunk for the reader.
*	Input: DataChunkInput 
*		
*/
void ScriptList::WriteScriptsDataChunk(DataChunkOutput &chunkWriter, ScriptList *scriptLists[], Int numLists )
{
	/**********SCRIPTS DATA ***********************/
	chunkWriter.openDataChunk("PlayerScriptsList", K_SCRIPTS_DATA_VERSION_1);	
		Int i;
		for (i=0; i<numLists; i++) {
			chunkWriter.openDataChunk("ScriptList", K_SCRIPT_LIST_DATA_VERSION_1);
			if (scriptLists[i]) scriptLists[i]->WriteScriptListDataChunk(chunkWriter);
			chunkWriter.closeDataChunk();
		}
	chunkWriter.closeDataChunk();
	
}



/**
* ScriptList::WriteScriptListDataChunk - Writes a Scripts chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
void ScriptList::WriteScriptListDataChunk(DataChunkOutput &chunkWriter)
{
	/**********SCRIPTS DATA ***********************/
		if (m_firstScript) m_firstScript->WriteScriptDataChunk(chunkWriter, m_firstScript);
		if (m_firstGroup) m_firstGroup->WriteGroupDataChunk(chunkWriter, m_firstGroup);
}


/**
* ScriptList::ParseScriptListDataChunk - read a Scripts chunk.
* Format is the newer CHUNKY format.
*	See ScriptList::WriteScriptListDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool ScriptList::ParseScriptListDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	TScriptListReadInfo *pInfo = (TScriptListReadInfo*)userData;
	DEBUG_ASSERTCRASH(pInfo->numLists < MAX_PLAYER_COUNT, ("Too many."));
	if (pInfo->numLists >= MAX_PLAYER_COUNT) return false;
	pInfo->readLists[pInfo->numLists] = newInstance(ScriptList);
	Int cur = pInfo->numLists;
	pInfo->numLists++;
	file.registerParser( AsciiString("Script"), info->label, Script::ParseScriptFromListDataChunk );
	file.registerParser( AsciiString("ScriptGroup"), info->label, ScriptGroup::ParseGroupDataChunk );
	return file.parse(pInfo->readLists[cur]);
	
}


//-------------------------------------------------------------------------------------------------
// ******************************** class  ScriptGroup *********************************************
//-------------------------------------------------------------------------------------------------

/**
  Ctor - gives it a default name.
*/
ScriptGroup::ScriptGroup(void) :
m_firstScript(NULL),
m_hasWarnings(false),
m_isGroupActive(true),
m_isGroupSubroutine(false),
//Added By Sadullah Nader
//Initializations inserted
m_nextGroup(NULL)
//
{
	m_groupName.format("Script Group %d", ScriptList::getNextID());
}

/**
  Dtor - The script list deletes the rest of the list, but we have to loop & delete
	sll the script groups in out list.
*/
ScriptGroup::~ScriptGroup(void) 
{
	if (m_firstScript) {
		// Delete the first script.  m_firstScript deletes the entire list.
		m_firstScript->deleteInstance();
		m_firstScript = NULL;
	}
	if (m_nextGroup) {
		// Delete all the subsequent groups in our list.
		ScriptGroup *cur = m_nextGroup;
		ScriptGroup *next;
		while (cur) {
			next = cur->getNext();
			cur->setNextGroup(NULL); // prevents recursion. 
			cur->deleteInstance();
			cur = next; 
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ScriptGroup::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ScriptGroup::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// count of scripts here
	UnsignedShort scriptCount = 0;
	Script *script;
	for( script = getScript(); script; script = script->getNext() )
		scriptCount++;
	UnsignedShort countVerify = scriptCount;
	xfer->xferUnsignedShort( &scriptCount );
	if( countVerify != scriptCount )
	{

		DEBUG_CRASH(( "ScriptGroup::xfer - Script list count has changed, attempting to recover."));
		// throw SC_INVALID_DATA; try to recover. jba.

	}  // end if

	// xfer script data
	for( script = getScript(); script; script = script->getNext() )	{
		xfer->xferSnapshot( script );
		scriptCount--;
		if (scriptCount==0) break;
	}
	if (scriptCount>0) {
		DEBUG_CRASH(("Stripping out extra scripts - Bad..."));
		if (s_mtScript==NULL) s_mtScript = newInstance(Script);	// Yes it leaks, but this is unusual recovery only. jba.
		while (scriptCount) {
			xfer->xferSnapshot(s_mtScript);
			scriptCount--;
		}
	}

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ScriptGroup::loadPostProcess( void )
{

}  // end loadPostProcess

/**
  ScriptGroup::duplicate - Creates a full, "deep" copy of ScriptGroup.
	m_nextGroup is NULL on the copy.
*/
ScriptGroup *ScriptGroup::duplicate(void) const 
{
	ScriptGroup *pNew = newInstance(ScriptGroup);	

	{
		Script *src = this->m_firstScript;
		Script *dst = NULL;
		while (src)
		{
			Script *tmp = src->duplicate();

			if (dst)
				dst->setNextScript(tmp);
			else
				pNew->m_firstScript = tmp;

			src = src->getNext();
			dst = tmp;
		}
	}

	pNew->m_groupName = this->m_groupName;
	pNew->m_isGroupActive = this->m_isGroupActive;
	pNew->m_isGroupSubroutine = this->m_isGroupSubroutine;
	pNew->m_nextGroup = NULL;

	return pNew;
}

/**
  ScriptGroup::duplicateAndQualify - Creates a full, "deep" copy of ScriptGroup, 
	adding qualifier to names.
	m_nextGroup is NULL on the copy.
*/
ScriptGroup *ScriptGroup::duplicateAndQualify(const AsciiString& qualifier, 
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const 
{
	ScriptGroup *pNew = newInstance(ScriptGroup);

	{
		Script *src = this->m_firstScript;
		Script *dst = NULL;
		while (src)
		{
			Script *tmp = src->duplicateAndQualify(qualifier, playerTemplateName, newPlayerName);

			if (dst)
				dst->setNextScript(tmp);
			else
				pNew->m_firstScript = tmp;

			src = src->getNext();
			dst = tmp;
		}
	}

	pNew->m_groupName = this->m_groupName;
	pNew->m_groupName.concat(qualifier);
	pNew->m_isGroupActive = this->m_isGroupActive;
	pNew->m_isGroupSubroutine = this->m_isGroupSubroutine;
	pNew->m_nextGroup = NULL;

	return pNew;
}

/**
  Delete a script from the current list of scripts.
*/
void ScriptGroup::deleteScript(Script *pScr)
{
	Script *pPrev = NULL;
	Script *pCur = m_firstScript;
	while (pScr != pCur) {
		pPrev = pCur;
		pCur = pCur->getNext();
	}
	DEBUG_ASSERTCRASH(pCur, ("Couldn't find script."));
	if (pCur==NULL) return;
	if (pPrev) {
		pPrev->setNextScript(pCur->getNext());
	} else {
		m_firstScript = pCur->getNext();
	}
	// Clear link & delete.
	pCur->setNextScript(NULL);
	pCur->deleteInstance();
}

/**
  Add a script to the current list of scripts.  Offset to position ndx.
*/
void ScriptGroup::addScript(Script *pScr, Int ndx)
{
	Script *pPrev = NULL;
	Script *pCur = m_firstScript;
	DEBUG_ASSERTCRASH(pScr->getNext()==NULL, ("Adding already linked group."));
	while (ndx && pCur) {
		pPrev = pCur;
		pCur = pCur->getNext();
		ndx--;
	}
	if (pPrev) {
		// link to pPrev
		pScr->setNextScript(pPrev->getNext());
		pPrev->setNextScript(pScr);
	} else {
		// add to head of list.
		pScr->setNextScript(m_firstScript);
		m_firstScript = pScr;
	}
}

/**
* ScriptGroup::WriteGroupDataChunk - Writes a Scripts chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
void ScriptGroup::WriteGroupDataChunk(DataChunkOutput &chunkWriter, ScriptGroup *pGroup)
{

	/**********SCRIPT GROUP DATA ***********************/
	while (pGroup) {
		chunkWriter.openDataChunk("ScriptGroup", K_SCRIPT_GROUP_DATA_VERSION_2);
			chunkWriter.writeAsciiString(pGroup->m_groupName);
			chunkWriter.writeByte(pGroup->m_isGroupActive);
			chunkWriter.writeByte(pGroup->m_isGroupSubroutine);
			if (pGroup->m_firstScript) Script::WriteScriptDataChunk(chunkWriter, pGroup->m_firstScript);
		chunkWriter.closeDataChunk();
		pGroup = pGroup->getNext();
	}
	
}

/**
* ScriptGroup::ParseGroupDataChunk - read a Group chunk.
* Format is the newer CHUNKY format.
*	See ScriptList::WriteScriptListDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool ScriptGroup::ParseGroupDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	ScriptList *pList = (ScriptList *)userData;
	ScriptGroup *pGroup = newInstance(ScriptGroup);

	pGroup->m_groupName = file.readAsciiString();
	pGroup->m_isGroupActive = file.readByte();
	if (info->version == K_SCRIPT_GROUP_DATA_VERSION_2) {
		pGroup->m_isGroupSubroutine= file.readByte();
	}
	pList->addGroup(pGroup, AT_END);
	file.registerParser( AsciiString("Script"), info->label, Script::ParseScriptFromGroupDataChunk );
	return file.parse(pGroup);
	
}

//-------------------------------------------------------------------------------------------------
// ******************************** class  Script *********************************************
//-------------------------------------------------------------------------------------------------
/**
  Ctor - initializes members.
*/
Script::Script(void) :
m_isActive(true),
m_isOneShot(true),
m_easy(true),
m_normal(true),
m_hard(true),	
m_delayEvaluationSeconds(0),
m_conditionTime(0),
m_conditionExecutedCount(0),
m_frameToEvaluateAt(0),
m_isSubroutine(false),
m_hasWarnings(false),
m_nextScript(NULL),
m_condition(NULL),
m_action(NULL),
//Added By Sadullah Nader
//Initializations inserted
m_actionFalse(NULL),
m_curTime(0.0f)
//
{
}

/**
  Dtor - The condition and action deletes the rest of the list, but we have to loop & delete
	all the scripts in out list.
*/
Script::~Script(void) 
{
	if (m_nextScript) {
		Script *cur = m_nextScript;
		Script *next;
		while (cur) {
			next = cur->getNext();
			cur->setNextScript(NULL); // prevents recursion. 
			cur->deleteInstance();
			cur = next; 
		}
	}
	if (m_condition) {
		m_condition->deleteInstance();
	}
	if (m_action) {
		m_action->deleteInstance();
	}

	if (m_actionFalse) {
		m_actionFalse->deleteInstance();
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void Script::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void Script::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// active
	Bool active = isActive();
	xfer->xferBool( &active );
	setActive( active );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void Script::loadPostProcess( void )
{

}  // end loadPostProcess

/**
  Script::duplicate - Creates a full, "deep" copy of script. Condition list and action
  list is duplicated as well.  Note - just the script, doesn't
	duplicate a list of scripts.  m_nextScript is NULL on the copy.
*/
Script *Script::duplicate(void) const 
{
	Script *pNew = newInstance(Script);	
	if (pNew->m_condition) {
		pNew->m_condition->deleteInstance();
	}
	if (pNew->m_action) {
		pNew->m_action->deleteInstance();
	}
	pNew->m_scriptName = m_scriptName;
	pNew->m_comment = m_comment;
	pNew->m_conditionComment = m_conditionComment;
	pNew->m_actionComment = m_actionComment;
	pNew->m_isActive = m_isActive;
	pNew->m_isOneShot = m_isOneShot;
	pNew->m_isSubroutine = m_isSubroutine;
	pNew->m_easy = m_easy;
	pNew->m_normal = m_normal;
	pNew->m_hard = m_hard;
	pNew->m_delayEvaluationSeconds = m_delayEvaluationSeconds;
	if (m_condition) {
		pNew->m_condition = m_condition->duplicate();
	}
	if (m_action) {
		pNew->m_action = m_action->duplicate();
	}
	if (m_actionFalse) {
		pNew->m_actionFalse = m_actionFalse->duplicate();
	}
	return pNew;
}

/**
  Script::duplicate - Creates a full, "deep" copy of script, with qualifier 
  added to names. Condition list and action
  list is duplicated as well.  Note - just the script, doesn't
	duplicate a list of scripts.  m_nextScript is NULL on the copy.
*/
Script *Script::duplicateAndQualify(const AsciiString& qualifier, 
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const 
{
	Script *pNew = newInstance(Script);
	if (pNew->m_condition) {
		pNew->m_condition->deleteInstance();
	}
	if (pNew->m_action) {
		pNew->m_action->deleteInstance();
	}
	pNew->m_scriptName = m_scriptName;
	pNew->m_scriptName.concat(qualifier);
	pNew->m_comment = m_comment;
	pNew->m_conditionComment = m_conditionComment;
	pNew->m_actionComment = m_actionComment;
	pNew->m_isActive = m_isActive;
	pNew->m_isOneShot = m_isOneShot;
	pNew->m_isSubroutine = m_isSubroutine;
	pNew->m_easy = m_easy;
	pNew->m_normal = m_normal;
	pNew->m_hard = m_hard;
	pNew->m_delayEvaluationSeconds = m_delayEvaluationSeconds;
	if (m_condition) {
		pNew->m_condition = m_condition->duplicateAndQualify(qualifier, playerTemplateName, newPlayerName);
	}
	if (m_action) {
		pNew->m_action = m_action->duplicateAndQualify(qualifier, playerTemplateName, newPlayerName);
	}
	if (m_actionFalse) {
		pNew->m_actionFalse = m_actionFalse->duplicateAndQualify(qualifier, playerTemplateName, newPlayerName);
	}
	return pNew;
}

/**
  Script::updateFrom - Copies all the data from pSrc into this.  Any data in this 
	is deleted (conditions, actions).  Note that this guts pSrc, and removes it's conditions
	and actions.  Intended for use in an edit dialog, where pSrc is a copy edited, and if cancelled
	discarded, and if not cancelled, updated into the real script, then discarded.
*/
void Script::updateFrom(Script *pSrc) 
{
	this->m_scriptName = pSrc->m_scriptName;
	this->m_comment = pSrc->m_comment;
	this->m_conditionComment = pSrc->m_conditionComment;
	this->m_actionComment = pSrc->m_actionComment;
	this->m_isActive = pSrc->m_isActive;
	this->m_isSubroutine = pSrc->m_isSubroutine;
	this->m_delayEvaluationSeconds = pSrc->m_delayEvaluationSeconds;
	this->m_isOneShot = pSrc->m_isOneShot;
	this->m_easy = pSrc->m_easy;
	this->m_normal = pSrc->m_normal;
	this->m_hard = pSrc->m_hard;
	if (this->m_condition) {
		this->m_condition->deleteInstance();
	}
	this->m_condition = pSrc->m_condition;
	pSrc->m_condition = NULL;
	if (this->m_action) {
		this->m_action->deleteInstance();
	}
	this->m_action = pSrc->m_action;
	pSrc->m_action = NULL;
	if (this->m_actionFalse) {
		this->m_actionFalse->deleteInstance();
	}
	this->m_actionFalse = pSrc->m_actionFalse;
	pSrc->m_actionFalse = NULL;
}

/**
  Script::deleteOrCondition - delete pCond from the or condition list.
*/
void Script::deleteOrCondition(OrCondition *pCond)
{
	OrCondition *pPrev = NULL;
	OrCondition *pCur = m_condition;
	while (pCond != pCur) {
		pPrev = pCur;
		pCur = pCur->getNextOrCondition();
	}
	DEBUG_ASSERTCRASH(pCur, ("Couldn't find condition."));
	if (pCur==NULL) return;
	if (pPrev) {
		pPrev->setNextOrCondition(pCur->getNextOrCondition());
	} else {
		m_condition = pCur->getNextOrCondition();
	}
	pCur->setNextOrCondition(NULL);
	pCur->deleteInstance();
}


/**
  Script::deleteAction - delete pAct from the action list.
*/
void Script::deleteAction(ScriptAction *pAct)
{
	ScriptAction *pPrev = NULL;
	ScriptAction *pCur = m_action;
	while (pAct != pCur) {
		pPrev = pCur;
		pCur = pCur->getNext();
	}
	DEBUG_ASSERTCRASH(pCur, ("Couldn't find action."));
	if (pCur==NULL) return;
	if (pPrev) {
		pPrev->setNextAction(pCur->getNext());
	} else {
		m_action = pCur->getNext();
	}
	pCur->setNextAction(NULL);
	pCur->deleteInstance();
}


/**
  Script::deleteFalseAction - delete pAct from the false action list.
*/
void Script::deleteFalseAction(ScriptAction *pAct)
{
	ScriptAction *pPrev = NULL;
	ScriptAction *pCur = m_actionFalse;
	while (pAct != pCur) {
		pPrev = pCur;
		pCur = pCur->getNext();
	}
	DEBUG_ASSERTCRASH(pCur, ("Couldn't find action."));
	if (pCur==NULL) return;
	if (pPrev) {
		pPrev->setNextAction(pCur->getNext());
	} else {
		m_actionFalse = pCur->getNext();
	}
	pCur->setNextAction(NULL);
	pCur->deleteInstance();
}


/**
  Script::getUiText - Creates the string to display in the scripts dialog box.
*/
AsciiString Script::getUiText(void) 
{
	AsciiString uiText("*** IF ***\r\n");
	OrCondition *pOr = m_condition;
	Int count=0;

	while (pOr) {
		Condition *pCond = pOr->getFirstAndCondition();
		if (count>0) uiText.concat("  *** OR ***\r\n");
		count = 0;
		while (pCond) {
			if (count>0) {
				uiText.concat("    *AND* ");
			} else {
				uiText.concat("    ");
			}
			uiText.concat(pCond->getUiText());
			uiText.concat("\r\n");
			pCond = pCond->getNext();
			count++;
		}
		pOr = pOr->getNextOrCondition();
	}
	uiText.concat("*** THEN ***\r\n");
	ScriptAction *pAction = m_action;
	while (pAction) {
		uiText.concat("  ");
		uiText.concat(pAction->getUiText());
		uiText.concat("\r\n");
		pAction = pAction->getNext();
	}
	pAction = m_actionFalse;
	if (pAction) {
		uiText.concat("*** ELSE ***\r\n");
		while (pAction) {
			uiText.concat("  ");
			uiText.concat(pAction->getUiText());
			uiText.concat("\r\n");
			pAction = pAction->getNext();
		}
	}
	return uiText;
}

/**
* Script::WriteScriptDataChunk - Writes a Scripts chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
void Script::WriteScriptDataChunk(DataChunkOutput &chunkWriter, Script *pScript)
{
	/**********SCRIPT  DATA ***********************/
	while (pScript) {
		chunkWriter.openDataChunk("Script", K_SCRIPT_DATA_VERSION_2);
			chunkWriter.writeAsciiString(pScript->m_scriptName);
			chunkWriter.writeAsciiString(pScript->m_comment);
			chunkWriter.writeAsciiString(pScript->m_conditionComment);
			chunkWriter.writeAsciiString(pScript->m_actionComment);

			chunkWriter.writeByte(pScript->m_isActive);
			chunkWriter.writeByte(pScript->m_isOneShot);
			chunkWriter.writeByte(pScript->m_easy);
			chunkWriter.writeByte(pScript->m_normal);
			chunkWriter.writeByte(pScript->m_hard);	 
			chunkWriter.writeByte(pScript->m_isSubroutine);	
			chunkWriter.writeInt(pScript->m_delayEvaluationSeconds);	
			if (pScript->m_condition) OrCondition::WriteOrConditionDataChunk(chunkWriter, pScript->m_condition);
			if (pScript->m_action) ScriptAction::WriteActionDataChunk(chunkWriter, pScript->m_action);
			if (pScript->m_actionFalse) ScriptAction::WriteActionFalseDataChunk(chunkWriter, pScript->m_actionFalse);
		chunkWriter.closeDataChunk();
		pScript = pScript->getNext();
	}	
}
/**
* Script::ParseScript - read a script chunk.
* Format is the newer CHUNKY format.
*	See ScriptList::WriteScriptDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Script *Script::ParseScript(DataChunkInput &file, unsigned short version)
{
	Script *pScript = newInstance(Script);

	pScript->m_scriptName = file.readAsciiString();
	pScript->m_comment = file.readAsciiString();
	pScript->m_conditionComment = file.readAsciiString();
	pScript->m_actionComment = file.readAsciiString();

	pScript->m_isActive = file.readByte();
	pScript->m_isOneShot = file.readByte();
	pScript->m_easy = file.readByte();
	pScript->m_normal = file.readByte();
	pScript->m_hard = file.readByte();
	pScript->m_isSubroutine = file.readByte();
	if (version>=K_SCRIPT_DATA_VERSION_2) {
		pScript->m_delayEvaluationSeconds = file.readInt();
	}
	file.registerParser( AsciiString("OrCondition"), AsciiString("Script"), OrCondition::ParseOrConditionDataChunk );
	file.registerParser( AsciiString("ScriptAction"),  AsciiString("Script"), ScriptAction::ParseActionDataChunk );
	file.registerParser( AsciiString("ScriptActionFalse"),  AsciiString("Script"), ScriptAction::ParseActionFalseDataChunk );
	if (! file.parse(pScript) ) 
	{
		return NULL;
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));	
	return pScript;
}

/**
* Script::ParseScriptFromListDataChunk - read a script chunk in a script list.
* Format is the newer CHUNKY format.
*	See ScriptList::WriteScriptListDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool Script::ParseScriptFromListDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	ScriptList *pList = (ScriptList *)userData;
	Script *pScript = ParseScript(file, info->version);
	pList->addScript(pScript, AT_END);
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));	
	return true;
}

/**
* Script::ParseScriptFromGroupDataChunk - read a script chunk in a script group.
* Format is the newer CHUNKY format.
*	See ScriptList::WriteScriptListDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool Script::ParseScriptFromGroupDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	ScriptGroup *pGroup = (ScriptGroup *)userData;
	Script *pScript = ParseScript(file, info->version);
	pGroup->addScript(pScript, AT_END);
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));	
	return true;
}


/**
* Script::findPreviousOrCondition - find the OrCondition that immediately proceeds curOr.
*	Input: OrCondition 
*		
*/
OrCondition *Script::findPreviousOrCondition( OrCondition *curOr )
{
	OrCondition *myConditions = getOrCondition();
	if ( myConditions == curOr ) {
		return NULL;
	}
	
	while (myConditions) {
		if (myConditions->getNextOrCondition() == curOr) {
			return myConditions;
		}
		myConditions = myConditions->getNextOrCondition();
	}

	DEBUG_CRASH(("Tried to find an OrCondition that doesn't seem to exist (jkmcd)"));
	return NULL;
}

//-------------------------------------------------------------------------------------------------
// ******************************** class  OrCondition *********************************************
//-------------------------------------------------------------------------------------------------
OrCondition::~OrCondition(void) 
{
	if (m_firstAnd) {
		m_firstAnd->deleteInstance();
		m_firstAnd = NULL;
	}
	if (m_nextOr) {
		OrCondition *cur = m_nextOr;
		OrCondition *next;
		while (cur) {
			next = cur->getNextOrCondition();
			cur->setNextOrCondition(NULL); // prevents recursion. 
			cur->deleteInstance();
			cur = next; 
		}
	}
}

OrCondition *OrCondition::duplicate(void) const 
{
	OrCondition *pNew = newInstance(OrCondition);	
	if (m_firstAnd) {
		pNew->m_firstAnd = m_firstAnd->duplicate();
	}
	OrCondition *pLink = m_nextOr;
	OrCondition *pCur = pNew;
	while (pLink) {
		pCur->m_nextOr = newInstance(OrCondition);
		pCur = pCur->m_nextOr;
		if (pLink->m_firstAnd) {
			pCur->m_firstAnd = pLink->m_firstAnd->duplicate();
		}
		pLink = pLink->m_nextOr;
	}
	return pNew;
}

OrCondition *OrCondition::duplicateAndQualify(const AsciiString& qualifier, 
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const 
{
	OrCondition *pNew = newInstance(OrCondition);
	if (m_firstAnd) {
		pNew->m_firstAnd = m_firstAnd->duplicateAndQualify(qualifier, playerTemplateName, newPlayerName);
	}
	OrCondition *pLink = m_nextOr;
	OrCondition *pCur = pNew;
	while (pLink) {
		pCur->m_nextOr = newInstance(OrCondition);
		pCur = pCur->m_nextOr;
		if (pLink->m_firstAnd) {
			pCur->m_firstAnd = pLink->m_firstAnd->duplicateAndQualify(qualifier, playerTemplateName, newPlayerName);
		}
		pLink = pLink->m_nextOr;
	}
	return pNew;
}

Condition *OrCondition::removeCondition(Condition *pCond)
{
	Condition *pPrev = NULL;
	Condition *pCur = m_firstAnd;
	while (pCond != pCur) {
		pPrev = pCur;
		pCur = pCur->getNext();
	}

	DEBUG_ASSERTCRASH(pCur, ("Couldn't find condition."));
	if (pCur==NULL) 
		return NULL;
	if (pPrev) {
		pPrev->setNextCondition(pCur->getNext());
	} else {
		m_firstAnd = pCur->getNext();
	}

	pCur->setNextCondition(NULL);
	return pCur;
}

void OrCondition::deleteCondition(Condition *pCond)
{
	Condition *pCur = removeCondition(pCond);
	DEBUG_ASSERTCRASH(pCur, ("Couldn't find condition."));
	if (pCur==NULL) 
		return;
	pCur->deleteInstance();
}


/**
* OrCondition::WriteOrConditionDataChunk - Writes a Or condition chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
void OrCondition::WriteOrConditionDataChunk(DataChunkOutput &chunkWriter, OrCondition	*pOrCondition)
{
	/**********OR CONDITION DATA ***********************/
	while (pOrCondition) {
		chunkWriter.openDataChunk("OrCondition", K_SCRIPT_OR_CONDITION_DATA_VERSION_1);
		if (pOrCondition->m_firstAnd) Condition::WriteConditionDataChunk(chunkWriter, pOrCondition->m_firstAnd);
		chunkWriter.closeDataChunk();
		pOrCondition = pOrCondition->getNextOrCondition();
	}
	
}

/**
* OrCondition::ParseOrConditionDataChunk - read a Or condition chunk.
* Format is the newer CHUNKY format.
*	See OrCondition::WriteOrConditionDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool OrCondition::ParseOrConditionDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Script *pScript = (Script *)userData;
	OrCondition *pOrCondition = newInstance(OrCondition);
	OrCondition *pFirst = pScript->getOrCondition();
	while (pFirst && pFirst->getNextOrCondition()) {
		pFirst = pFirst->getNextOrCondition();
	}
	if (pFirst) {
		pFirst->setNextOrCondition(pOrCondition);
	} else {
		pScript->setOrCondition(pOrCondition);
	}
	file.registerParser( AsciiString("Condition"), info->label, Condition::ParseConditionDataChunk );
	return file.parse(pOrCondition);
	
}

/**
* OrCondition::findPreviousCondition - find the condition that immediately proceeds curCond.
* Format is the newer CHUNKY format.
*	See OrCondition::WriteOrConditionDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Condition *OrCondition::findPreviousCondition( Condition *curCond )
{
	Condition *myConditions = getFirstAndCondition();
	if (myConditions == curCond) {
		return NULL;
	}

	while (myConditions) {
		if (myConditions->getNext() == curCond) {
			return myConditions;
		}
		myConditions = myConditions->getNext();
	}

	DEBUG_CRASH(("Searched for non-existent And Condition. (jkmcd)"));
	return NULL;
}


//-------------------------------------------------------------------------------------------------
// ******************************** class  Condition *********************************************
//-------------------------------------------------------------------------------------------------
Condition::Condition():
m_conditionType(CONDITION_FALSE),
m_hasWarnings(false),
m_customData(0),
m_numParms(0),
m_nextAndCondition(NULL)
{
	Int i;
	for (i = 0; i < MAX_PARMS; i++) 
	{
		m_parms[i] = NULL;
	}
}

Condition::Condition(enum ConditionType type):
m_conditionType(type),
m_numParms(0)
{
	Int i;
	for (i=0; i<MAX_PARMS; i++) {
		m_parms[i] = NULL;
	}
	setConditionType(type);
}

void Condition::setConditionType(enum ConditionType type)
{
	Int i;
	for (i=0; i<m_numParms; i++) {
		if (m_parms[i]) 
			m_parms[i]->deleteInstance();
		m_parms[i] = NULL;
	}
	m_conditionType = type;
	const ConditionTemplate *pTemplate = TheScriptEngine->getConditionTemplate(m_conditionType);
	m_numParms = pTemplate->getNumParameters();
	for (i=0; i<m_numParms; i++) {
		m_parms[i] = newInstance(Parameter)(pTemplate->getParameterType(i));	
	}
}

Condition *Condition::duplicate(void) const 
{
	Condition *pNew = newInstance(Condition)(m_conditionType);	
	Int i;
	for (i=0; i<m_numParms && i<pNew->m_numParms; i++) {
		*pNew->m_parms[i] = *m_parms[i];
	}
	Condition *pLink = m_nextAndCondition;
	Condition *pCur = pNew;
	while (pLink) {
		pCur->m_nextAndCondition = newInstance(Condition)(pLink->getConditionType());
		pCur = pCur->m_nextAndCondition;
		for (i=0; i<pLink->m_numParms; i++) {
			*pCur->m_parms[i] = *pLink->m_parms[i];
		}
		pLink = pLink->m_nextAndCondition;
	}
	return pNew;
}

Condition *Condition::duplicateAndQualify(const AsciiString& qualifier, 
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const 
{
	Condition *pNew = newInstance(Condition)(m_conditionType);
	Int i;
	for (i=0; i<m_numParms && i<pNew->m_numParms; i++) {
		*pNew->m_parms[i] = *m_parms[i];
		pNew->m_parms[i]->qualify(qualifier, playerTemplateName, newPlayerName);
	}
	Condition *pLink = m_nextAndCondition;
	Condition *pCur = pNew;
	while (pLink) {
		pCur->m_nextAndCondition = newInstance(Condition)(pLink->getConditionType());
		pCur = pCur->m_nextAndCondition;
		for (i=0; i<pLink->m_numParms; i++) {
			*pCur->m_parms[i] = *pLink->m_parms[i];
			pCur->m_parms[i]->qualify(qualifier, playerTemplateName, newPlayerName);
		}
		pLink = pLink->m_nextAndCondition;
	}
	return pNew;
}

Condition::~Condition(void) 
{
	Int i;
	for (i=0; i<m_numParms; i++) {
		m_parms[i]->deleteInstance();
		m_parms[i] = NULL;
	}
	if (m_nextAndCondition) {
		Condition *cur = m_nextAndCondition;
		Condition *next;
		while (cur) {
			next = cur->getNext();
			cur->setNextCondition(NULL); // prevents recursion. 
			cur->deleteInstance();
			cur = next; 
		}
	}
}



Int Condition::getUiStrings(AsciiString strings[MAX_PARMS])
{
	const ConditionTemplate *pTemplate = TheScriptEngine->getConditionTemplate(m_conditionType);
	return pTemplate->getUiStrings(strings);
}

AsciiString Condition::getUiText(void)
{
	AsciiString uiText;
	AsciiString strings[MAX_PARMS];
	Int numStrings = getUiStrings(strings);
	Int i;

	if (m_hasWarnings) {
		uiText = "[???]";
	}

	for (i=0; i<MAX_PARMS; i++) {
		if (i<numStrings) {
			uiText.concat(strings[i]);
		}
		if (i<m_numParms) {
			uiText.concat(m_parms[i]->getUiText());
		}
	}

	return uiText;
}


/**
* Condition::WriteConditionDataChunk - Writes a condition chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
void Condition::WriteConditionDataChunk(DataChunkOutput &chunkWriter, Condition	*pCondition)
{
	/**********ACTION  DATA ***********************/
	while (pCondition) {
		chunkWriter.openDataChunk("Condition", K_SCRIPT_CONDITION_VERSION_3);
			chunkWriter.writeInt(pCondition->m_conditionType);
			chunkWriter.writeInt(pCondition->m_numParms);
			Int i;
			for (i=0; i<pCondition->m_numParms; i++) {
				pCondition->m_parms[i]->WriteParameter(chunkWriter);
			}
		chunkWriter.closeDataChunk();
		pCondition = pCondition->getNext();
	}	
}
/**
* Condition::ParseConditionDataChunk - read a condition.
* Format is the newer CHUNKY format.
*	See Condition::WriteActionDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool Condition::ParseConditionDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Condition	*pCondition = newInstance(Condition);
	OrCondition *pOr = (OrCondition *)userData;
	pCondition->m_conditionType = (enum ConditionType)file.readInt();
	pCondition->m_numParms =file.readInt();
	Int i;
	for (i=0; i<pCondition->m_numParms; i++) 
	{
		pCondition->m_parms[i] = Parameter::ReadParameter(file);
	}
	
	if (file.getChunkVersion() < K_SCRIPT_CONDITION_VERSION_2) {
		for (int j = 0; ParameterChangesVer2[j] != -1; ++j) {
			if (pCondition->m_conditionType == ParameterChangesVer2[j]) {
				pCondition->m_parms[pCondition->m_numParms] = newInstance(Parameter)(Parameter::SURFACES_ALLOWED, 3);
				pCondition->m_numParms = 3;
			}
		}
	}
	// heal old files.
	switch (pCondition->getConditionType()) 
	{
		case SKIRMISH_SPECIAL_POWER_READY:
			if (pCondition->m_numParms == 1)
			{
				pCondition->m_numParms = 2;
				pCondition->m_parms[1] = pCondition->m_parms[0];
				pCondition->m_parms[0] = newInstance(Parameter)(Parameter::SIDE, 0);
				pCondition->m_parms[0]->friend_setString(THIS_PLAYER);
			}
			break;
	}
	Condition *pLast = pOr->getFirstAndCondition();
	while (pLast && pLast->getNext()) {
		pLast = pLast->getNext();
	}
	if (pLast) {
		pLast->setNextCondition(pCondition);
	} else {
		pOr->setFirstAndCondition(pCondition);
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));	
	return true;
}


//-------------------------------------------------------------------------------------------------
// ******************************** class  Template *********************************************
//-------------------------------------------------------------------------------------------------
Template::Template() :
m_numUiStrings(0),
m_numParameters(0),
m_name("(placeholder)")
{
}

Int Template::getUiStrings(AsciiString strings[MAX_PARMS]) const
{
	Int i;
	for (i=0; i<m_numUiStrings; i++) {
		strings[i] = m_uiStrings[i];
	}
	return m_numUiStrings;
}

//-------------------------------------------------------------------------------------------------
// ******************************** class Parameter ***********************************************
//-------------------------------------------------------------------------------------------------
enum Parameter::ParameterType Template::getParameterType(Int ndx) const 
{
	if (ndx >= 0 && ndx < m_numParameters) {
		return m_parameters[ndx];
	}
	DEBUG_CRASH(("Index out of range."));
	return Parameter::INT;
}

void Parameter::getCoord3D(Coord3D *pLoc) const
{
	DEBUG_ASSERTCRASH(m_paramType==COORD3D, ("Wrong parameter type."));
	pLoc->x = pLoc->y = pLoc->z = 0;
	if (m_paramType==COORD3D) {
		*pLoc = m_coord;
	}
}

void Parameter::setCoord3D(const Coord3D *pLoc)
{
	DEBUG_ASSERTCRASH(m_paramType==COORD3D, ("Wrong parameter type."));
	if (m_paramType==COORD3D) {
		m_coord= *pLoc ;
	}
}

void Parameter::qualify(const AsciiString& qualifier, 
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) 
{
	AsciiString tmpString;
	switch (m_paramType) {
		case SIDE:
			tmpString = m_string;
			tmpString.concat(qualifier);
			if (tmpString==playerTemplateName) {
				m_string = newPlayerName;
			}
			break;
		case TEAM:	
			if (m_string == THIS_TEAM) {
				break;
			}
			/// otherwise drop down & qualify.
		case SCRIPT:
		case COUNTER:
		case FLAG:
		case SCRIPT_SUBROUTINE: m_string.concat(qualifier); break;
		default: break;
	}
}

AsciiString Parameter::getUiText(void) const
{
	AsciiString uiText;
	AsciiString uiString = m_string;
	if (uiString.isEmpty()) {
		uiString = "???";
	}

	Coord3D pos;
	switch (m_paramType) 
	{
		default:
			DEBUG_CRASH(("Unknown parameter type."));
			break;
		case SOUND:
			uiText.format("Sound '%s'", uiString.str());
			break;
		case SCRIPT:
			uiText.format("Script '%s'", uiString.str());
			break;
		case TEAM_STATE:
			uiText.format("'%s'", uiString.str());
			break;
		case SCRIPT_SUBROUTINE:
			uiText.format("Subroutine '%s'", uiString.str());
			break;
		case ATTACK_PRIORITY_SET:
			uiText.format("Attack priority set '%s'", uiString.str());
			break;
		case WAYPOINT:
			uiText.format("Waypoint '%s'", uiString.str());
			break;
		case WAYPOINT_PATH:
			uiText.format("Waypoint Path '%s'", uiString.str());
			break;
		case TRIGGER_AREA:
			uiText.format(" area '%s'", uiString.str());
			break;
		case COMMAND_BUTTON:
			uiText.format("Command button: '%s'", uiString.str());
			break;
		case FONT_NAME:
			uiText.format("Font: '%s'", uiString.str());
			break;
		case LOCALIZED_TEXT:
			uiText.format("Localized String: '%s'", uiString.str());
			break;
		case TEXT_STRING:
			uiText.format("String: '%s'", uiString.str());
			break;
		case TEAM:
			uiText.format("Team '%s'", uiString.str());
			break;
		case UNIT:
			uiText.format("Unit '%s'", uiString.str());
			break;
		case BRIDGE:
			uiText.format("Bridge '%s'", uiString.str());
			break;
		case ANGLE:
			uiText.format("%.2f degrees", m_real*180/PI);
			break;
		case COORD3D:
			getCoord3D(&pos);
			uiText.format("(%.2f,%.2f,%.2f)", pos.x,pos.y,pos.z);
			break;	
		case OBJECT_TYPE:
			uiText.format("'%s'", uiString.str());
			break;
		case KIND_OF_PARAM:
			if (m_int >= KINDOF_FIRST && m_int < KINDOF_COUNT )
				uiText.format("Kind is '%s'", KindOfMaskType::getNameFromSingleBit(m_int));
			else 
				uiText.format("Kind is '???'");
			break;
		case SIDE:
			uiText.format("Player '%s'", uiString.str());
			break;
		case COUNTER:
			uiText.format("'%s'", uiString.str());
			break;
		case INT:
			uiText.format(" %d ", m_int);
			break;
		case BOOLEAN:
			uiText.concat(m_int?"TRUE":"FALSE");
			break;
					 
		case REAL:
			uiText.format("%.2f", m_real);
			break;

		case FLAG:
			uiText.format("Flag named '%s'", uiString.str());
			break;
		case COMPARISON:
			switch (m_int) {
				case LESS_THAN: uiText.format("Less Than"); break;
				case LESS_EQUAL: uiText.format("Less Than or Equal"); break;
				case EQUAL: uiText.format("Equal To"); break;
				case GREATER_EQUAL: uiText.format("Greater Than or Equal To"); break;
				case GREATER: uiText.format("Greater Than"); break;
				case NOT_EQUAL: uiText.format("Not Equal To"); break;
				default : DEBUG_CRASH(("Unknown comparison type."));
			}
			break;
		
		case RELATION:
			switch (m_int) {
				case REL_ENEMY: uiText.format("Enemy"); break;
				case REL_NEUTRAL: uiText.format("Neutral"); break;
				case REL_FRIEND: uiText.format("Friend"); break;
				default : DEBUG_CRASH(("Unknown Relation type."));
			}
			break;

		case AI_MOOD:
			switch (m_int) {
				case ATTITUDE_SLEEP: uiText.format("Sleep"); break;
				case ATTITUDE_PASSIVE: uiText.format("Passive"); break;
				case ATTITUDE_NORMAL: uiText.format("Normal"); break;
				case ATTITUDE_ALERT: uiText.format("Alert"); break;
				case ATTITUDE_AGGRESSIVE: uiText.format("Aggressive"); break;
				default : DEBUG_CRASH(("Unknown AI Mood type."));
			}
			break;

		case RADAR_EVENT_TYPE:
			switch (m_int) {
				//case RADAR_EVENT_INVALID: ++m_int;	// continue to the next case.
				case RADAR_EVENT_INVALID: DEBUG_CRASH(("Invalid radar event\n")); uiText.format("Construction"); break;
				case RADAR_EVENT_CONSTRUCTION: uiText.format("Construction"); break;
				case RADAR_EVENT_UPGRADE: uiText.format("Upgrade"); break;
				case RADAR_EVENT_UNDER_ATTACK: uiText.format("Under Attack"); break;
				case RADAR_EVENT_INFORMATION: uiText.format("Information"); break;
				case RADAR_EVENT_INFILTRATION: uiText.format("Infiltration"); break;
				default : DEBUG_CRASH(("Unknown Radar event type."));
			}
			break;
			

		case DIALOG:
			uiText.format("'%s'", uiString.str());
			break;

		case SKIRMISH_WAYPOINT_PATH:
			uiText.format("'%s'", uiString.str());
			break;

		case COLOR:
			uiText.format(" R:%d G:%d B:%d ", (m_int&0x00ff0000)>>16, (m_int&0x0000ff00)>>8, (m_int&0x000000ff) );
			break;

		case MUSIC:
			uiText.format("'%s'", uiString.str());
			break;

		case MOVIE:
			uiText.format("'%s'", uiString.str());
			break;

		case SPECIAL_POWER:
			uiText.format("Special power '%s'", uiString.str());
			break;

		case SCIENCE:
			uiText.format("Science '%s'", uiString.str());
			break;

		case SCIENCE_AVAILABILITY:
			uiText.format( "Science availability '%s'", uiString.str() );
			break;

		case UPGRADE:
			uiText.format("Upgrade '%s'", uiString.str());
			break;
		
		case COMMANDBUTTON_ABILITY:
		case COMMANDBUTTON_ALL_ABILITIES:
			uiText.format( "Ability '%s'", uiString.str() );
			break;
			
		case EMOTICON:
			uiText.format( "Emoticon '%s'", uiString.str() );
			break;
			
		case BOUNDARY:
			uiText.format("Boundary %s", BORDER_COLORS[m_int % BORDER_COLORS_SIZE].m_colorName);
			break;

		case BUILDABLE:
			if (m_int >= BSTATUS_YES && m_int < BSTATUS_NUM_TYPES )
				uiText.format("Buildable (%s)", BuildableStatusNames[m_int - BSTATUS_YES]);
			else 
				uiText.format("Buildable (???)");
			break;
		
		case SURFACES_ALLOWED:
		{
			if (m_int > 0 && m_int <= 3)
				uiText.format("Surfaces Allowed: %s", Surfaces[m_int - 1]);
			else 
				uiText.format("Surfaces Allowed: ???");
			break;
		}

		case SHAKE_INTENSITY:
		{
			if (m_int > 0 && m_int < View::SHAKE_COUNT)
				uiText.format("Shake Intensity: %s", ShakeIntensities[m_int]);
			else
				uiText.format("Shake Intensity: ???");
			break;
		}
		
		case OBJECT_STATUS:
		{
			if (m_string.isEmpty()) {
				uiText.format("Object Status is '???'");
			} else {
				uiText.format("Object Status is '%s'", m_string.str());
			}
			break;
		}

		case FACTION_NAME:
		{
			uiText.format("Faction Name: %s", uiString.str());
			break;
		}

		case OBJECT_TYPE_LIST:
			uiText.format("'%s'", uiString.str());
			break;
			
		case REVEALNAME:
			uiText.format("Reveal Name: %s", uiString.str());
			break;

		case OBJECT_PANEL_FLAG:
			uiText.format("Object Flag: %s", uiString.str());
			break;
	}
	return uiText;
}

/**
* Parameter::WriteParameter - Writes an Parameter.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
void Parameter::WriteParameter(DataChunkOutput &chunkWriter)
{

	/**********Parameter  DATA ***********************/
	chunkWriter.writeInt(m_paramType);
	if (m_paramType == KIND_OF_PARAM) {
		// To get the proper kindof string stored.
		m_string = KindOfMaskType::getNameFromSingleBit(m_int);
	}
	if (m_paramType == COORD3D) {
		chunkWriter.writeReal(m_coord.x);
		chunkWriter.writeReal(m_coord.y);
		chunkWriter.writeReal(m_coord.z);
	} else {
		chunkWriter.writeInt(m_int);
		chunkWriter.writeReal(m_real);
		chunkWriter.writeAsciiString(m_string);
	}
}

/**
* Parameter::ReadParameter - read a parameter.
* Format is the newer CHUNKY format.
*	See Parameter::WriteParameter for the writer.
*	Input: DataChunkInput 
*		
*/
Parameter *Parameter::ReadParameter(DataChunkInput &file)
{
	Parameter *pParm = newInstance(Parameter)( (ParameterType)file.readInt());
	pParm->m_initialized = true;
	if (pParm->getParameterType() == COORD3D) {
		Coord3D pos;
		pos.x = file.readReal();
		pos.y = file.readReal();
		pos.z = file.readReal();
		pParm->setCoord3D(&pos);
	} 
	else 
	{
		pParm->m_int = file.readInt();
		pParm->m_real = file.readReal();
		pParm->m_string = file.readAsciiString();
	}

	if (pParm->getParameterType() == OBJECT_TYPE) 
	{
		// quick hack to make loading models with "Fundamentalist" switch to "GLA"
		if (pParm->m_string.startsWith("Fundamentalist")) 
		{
			char oldName[256];
			char newName[256];
			strcpy(oldName, pParm->m_string.str());
			strcpy(newName, "GLA");
			strcat(newName, oldName+strlen("Fundamentalist"));
			pParm->m_string.set(newName);
			DEBUG_LOG(("Changing Script Ref from %s to %s\n", oldName, newName));
		}
	}

	if (pParm->getParameterType() == UPGRADE) 
	{
		// quick hack to make obsolete capture building upgrades switch to the new one. jba.
		if (pParm->m_string == "Upgrade_AmericaRangerCaptureBuilding" ||
			pParm->m_string == "Upgrade_ChinaRedguardCaptureBuilding" ||
			pParm->m_string == "Upgrade_GLARebelCaptureBuilding") 
		{
			pParm->m_string.set("Upgrade_InfantryCaptureBuilding");
		}
	}

	if (pParm->getParameterType() == OBJECT_STATUS) 
	{
		// Need to change the string to an ObjectStatusMaskType
		for( int i = 0; i < OBJECT_STATUS_COUNT; ++i ) 
		{
			if( !pParm->m_string.compareNoCase( ObjectStatusMaskType::getBitNames()[i] ) ) 
			{
				pParm->setStatus( MAKE_OBJECT_STATUS_MASK( i ) );
				break;
			}
		}
	}

	if (pParm->getParameterType() == KIND_OF_PARAM) 
  {
		// Need to change the string to an integer
		const char** kindofNames = KindOfMaskType::getBitNames();
		if (!pParm->m_string.isEmpty()) 
    {
			Bool found = false;
			for (int i = 0; kindofNames[i]; ++i) 
			{
				if (pParm->m_string.compareNoCase(kindofNames[i]) == 0) 
        {
					pParm->setInt(i);
					found = true;
					break;
				}
				if( !pParm->m_string.compareNoCase( "CRUSHER" ) )
				{
					//????
					pParm->setInt(i);
					found = true;
					DEBUG_CRASH(( "Kindof CRUSHER no longer exists -- in order to get your map to load, it has been switched to OBSTACLE, please call Kris (x36844).", pParm->m_string.str()));
					break;
				}
				else if( !pParm->m_string.compareNoCase( "CRUSHABLE" ) )
				{
					//????
					pParm->setInt(i);
					found = true;
					DEBUG_CRASH(( "Kindof CRUSHABLE no longer exists -- in order to get your map to load, it has been switched to OBSTACLE, please call Kris (x36844).", pParm->m_string.str()));
					break;
				}
				else if( !pParm->m_string.compareNoCase( "OVERLAPPABLE" ) )
				{
					//????
					pParm->setInt(i);
					found = true;
					DEBUG_CRASH(( "Kindof OVERLAPPABLE no longer exists -- in order to get your map to load, it has been switched to OBSTACLE, please call Kris (x36844).", pParm->m_string.str()));
					break;
				}
				else if( !pParm->m_string.compareNoCase( "MISSILE" ) )
				{
					//MISSILE was split into two kinds -- SMALL_MISSILE and BALLISTIC_MISSILE.
					pParm->m_string.format( "SMALL_MISSILE" );
					for( i = 0; kindofNames[i]; ++i )
					{
						if (pParm->m_string.compareNoCase("SMALL_MISSILE") == 0) 
						{
							pParm->setInt(i);
							found = true;
							break;
						}
					}
					DEBUG_CRASH(("Unable to find Kindof SMALL_MISSILE', please call KrisM (x36844).", pParm->m_string.str()));
				}
				
			}
			if (!found) 
      {
				DEBUG_CRASH(("Unable to find Kindof '%s', please call JKM (x36872).", pParm->m_string.str()));
				throw ERROR_BUG;
			}
		} 
    else 
    {
			// Seems weird, but this is so WB will load them into the proper format.
			pParm->m_string = kindofNames[pParm->m_int];
		}
	}

	return pParm;
}

//-------------------------------------------------------------------------------------------------
// ******************************** class ScriptAction ***********************************************
//-------------------------------------------------------------------------------------------------
ScriptAction::ScriptAction():
m_actionType(NO_OP),
m_hasWarnings(false),
m_numParms(0),
//Added By Sadullah Nader
//Initializations inserted
m_nextAction(NULL)
//
{
}

ScriptAction::ScriptAction(enum ScriptActionType type):
m_actionType(type),
m_numParms(0)
{
	Int i;
	for (i=0; i<MAX_PARMS; i++) {
		m_parms[i] = NULL;
	}
	setActionType(type);
}

void ScriptAction::setActionType(enum ScriptActionType type)
{
	Int i;
	for (i=0; i<m_numParms; i++) {
		if (m_parms[i]) 
			m_parms[i]->deleteInstance();
		m_parms[i] = NULL;
	}
	m_actionType = type;
	const ActionTemplate *pTemplate = TheScriptEngine->getActionTemplate(m_actionType);
	m_numParms = pTemplate->getNumParameters();
	for (i=0; i<m_numParms; i++) {
		m_parms[i] = newInstance(Parameter)(pTemplate->getParameterType(i));
	}
}

ScriptAction *ScriptAction::duplicate(void) const 
{
	ScriptAction *pNew = newInstance(ScriptAction)(m_actionType);	
	Int i;
	for (i=0; i<m_numParms; i++) {
		if (pNew->m_parms[i]) {
			*pNew->m_parms[i] = *m_parms[i];
		}
	}
	ScriptAction *pLink = m_nextAction;
	ScriptAction *pCur = pNew;
	while (pLink) {
		pCur->m_nextAction = newInstance(ScriptAction)(pLink->m_actionType);
		pCur = pCur->m_nextAction;
		for (i=0; i<pLink->m_numParms; i++) {
			if (pCur->m_parms[i] && pLink->m_parms[i]) {
				*pCur->m_parms[i] = *pLink->m_parms[i];
			}
		}
		pLink = pLink->m_nextAction;
	}
	return pNew;
}

ScriptAction *ScriptAction::duplicateAndQualify(const AsciiString& qualifier, 
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const 
{
	ScriptAction *pNew = newInstance(ScriptAction)(m_actionType);
	Int i;
	for (i=0; i<m_numParms; i++) {
		if (pNew->m_parms[i]) {
			*pNew->m_parms[i] = *m_parms[i];
			pNew->m_parms[i]->qualify(qualifier, playerTemplateName, newPlayerName);
		}
	}
	ScriptAction *pLink = m_nextAction;
	ScriptAction *pCur = pNew;
	while (pLink) {
		pCur->m_nextAction = newInstance(ScriptAction)(pLink->m_actionType);
		pCur = pCur->m_nextAction;
		for (i=0; i<pLink->m_numParms; i++) {
			if (pCur->m_parms[i] && pLink->m_parms[i]) {
				*pCur->m_parms[i] = *pLink->m_parms[i];
				pCur->m_parms[i]->qualify(qualifier, playerTemplateName, newPlayerName);
			}
		}
		pLink = pLink->m_nextAction;
	}
	return pNew;
}

ScriptAction::~ScriptAction(void) 
{
	Int i;
	for (i=0; i<m_numParms; i++) {
		m_parms[i]->deleteInstance();
		m_parms[i] = NULL;
	}
	if (m_nextAction) {
		ScriptAction *cur = m_nextAction;
		ScriptAction *next;
		while (cur) {
			next = cur->getNext();
			cur->setNextAction(NULL); // prevents recursion. 
			cur->deleteInstance();
			cur = next; 
		}
	}
}



Int ScriptAction::getUiStrings(AsciiString strings[MAX_PARMS])
{
	const ActionTemplate *pTemplate = TheScriptEngine->getActionTemplate(m_actionType);
	return pTemplate->getUiStrings(strings);
}

AsciiString ScriptAction::getUiText(void)
{
	AsciiString uiText;
	AsciiString strings[MAX_PARMS];
	Int numStrings = getUiStrings(strings);
	Int i;

	if (m_hasWarnings) {
		uiText = "[???]";
	}

	for (i=0; i<MAX_PARMS; i++) {
		if (i<numStrings) {
			uiText.concat(strings[i]);
		}
		if (i<m_numParms) {
			uiText.concat(m_parms[i]->getUiText());
		}
	}

	return uiText;
}

/**
* ScriptAction::WriteActionDataChunk - Writes an Action chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
void ScriptAction::WriteActionDataChunk(DataChunkOutput &chunkWriter, ScriptAction	*pScriptAction)
{
	/**********ACTION  DATA ***********************/
	while (pScriptAction) {
		chunkWriter.openDataChunk("ScriptAction", K_SCRIPT_ACTION_VERSION_1);
			chunkWriter.writeInt(pScriptAction->m_actionType);
			chunkWriter.writeInt(pScriptAction->m_numParms);
			Int i;
			for (i=0; i<pScriptAction->m_numParms; i++) {
				pScriptAction->m_parms[i]->WriteParameter(chunkWriter);
			}
		chunkWriter.closeDataChunk();
		pScriptAction = pScriptAction->getNext();
	}	
}

/**
* ScriptAction::ParseActionDataChunk - read an action chunk in a script list.
* Format is the newer CHUNKY format.
*	See ScriptAction::WriteActionDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool ScriptAction::ParseActionDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Script *pScript = (Script *)userData;

	ScriptAction	*pScriptAction = newInstance(ScriptAction);

	pScriptAction->m_actionType = (enum ScriptActionType)file.readInt();

#if defined(RTS_DEBUG) || defined(_INTERNAL)
	const ActionTemplate* at = TheScriptEngine->getActionTemplate(pScriptAction->m_actionType);
	if (at && (at->getName().isEmpty() || (at->getName().compareNoCase("(placeholder)") == 0))) {
		DEBUG_CRASH(("Invalid Script Action found in script '%s'\n", pScript->getName().str()));
	}
#endif

	pScriptAction->m_numParms =file.readInt();
	Int i;
	for (i=0; i<pScriptAction->m_numParms; i++) 
	{
		pScriptAction->m_parms[i] = Parameter::ReadParameter(file);
	}

	// heal old files.
	switch (pScriptAction->getActionType()) 
	{
		case SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST:
			if (pScriptAction->m_numParms == 1)
			{
				pScriptAction->m_numParms = 2;
				pScriptAction->m_parms[1] = pScriptAction->m_parms[0];
				pScriptAction->m_parms[0] = newInstance(Parameter)(Parameter::SIDE, 0);
				pScriptAction->m_parms[0]->friend_setString(THIS_PLAYER);
			}
			break;
		case TEAM_FOLLOW_WAYPOINTS:
			if (pScriptAction->m_numParms == 2)
			{
				pScriptAction->m_numParms = 3;
				pScriptAction->m_parms[2] = newInstance(Parameter)(Parameter::BOOLEAN, 1);
			}
			break;
		case SKIRMISH_BUILD_BASE_DEFENSE_FRONT:
			if (pScriptAction->m_numParms == 1)
			{		
				Bool flank = pScriptAction->m_parms[0]->getInt()!=0;
				pScriptAction->m_parms[0]->deleteInstance();
				pScriptAction->m_numParms = 0;
				if (flank) pScriptAction->m_actionType = SKIRMISH_BUILD_BASE_DEFENSE_FLANK;
			}
			break;
		case NAMED_SET_ATTITUDE:
		case TEAM_SET_ATTITUDE:
			if (pScriptAction->m_numParms >= 2 && pScriptAction->m_parms[1]->getParameterType() == Parameter::INT) 
			{
				pScriptAction->m_parms[1] = newInstance(Parameter)(Parameter::AI_MOOD, pScriptAction->m_parms[1]->getInt());
			}
			break;
		case MAP_REVEAL_AT_WAYPOINT:
		case MAP_SHROUD_AT_WAYPOINT:
			if (pScriptAction->getNumParameters() == 2)
			{
				pScriptAction->m_numParms = 3;
				pScriptAction->m_parms[2] = newInstance(Parameter)(Parameter::SIDE);
			}
			break;
		case MAP_REVEAL_ALL:
		case MAP_REVEAL_ALL_PERM:
		case MAP_REVEAL_ALL_UNDO_PERM:
		case MAP_SHROUD_ALL:
			if (pScriptAction->getNumParameters() == 0)
			{
				pScriptAction->m_numParms = 1;
				pScriptAction->m_parms[0] = newInstance(Parameter)(Parameter::SIDE);
			}
			break;
		case SPEECH_PLAY:
			if (pScriptAction->getNumParameters() == 1) 
			{
				pScriptAction->m_numParms = 2;
				// Default it to TRUE, as per conversation with JohnL
				pScriptAction->m_parms[1] = newInstance(Parameter)(Parameter::BOOLEAN, 1);
				break;
			}
	}


	/// @todo - Verify read in parameters with current action template.  jba.
	ScriptAction *pLast = pScript->getAction();
	while (pLast && pLast->getNext()) 
	{
		pLast = pLast->getNext();
	}

	if (pLast) 
	{
		pLast->setNextAction(pScriptAction);
	} 
	else 
	{
		pScript->setAction(pScriptAction);
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));	
	return true;
}


/**
* ScriptAction::WriteActionFalseDataChunk - Writes a false Action chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
void ScriptAction::WriteActionFalseDataChunk(DataChunkOutput &chunkWriter, ScriptAction	*pScriptAction)
{
	/**********ACTION  DATA ***********************/
	while (pScriptAction) {
		chunkWriter.openDataChunk("ScriptActionFalse", K_SCRIPT_ACTION_VERSION_1);
			chunkWriter.writeInt(pScriptAction->m_actionType);
			chunkWriter.writeInt(pScriptAction->m_numParms);
			Int i;
			for (i=0; i<pScriptAction->m_numParms; i++) {
				pScriptAction->m_parms[i]->WriteParameter(chunkWriter);
			}
		chunkWriter.closeDataChunk();
		pScriptAction = pScriptAction->getNext();
	}	
}

/**
* ScriptAction::ParseActionFalseDataChunk - read a false action chunk in a script list.
* Format is the newer CHUNKY format.
*	See ScriptAction::WriteActionDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool ScriptAction::ParseActionFalseDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Script *pScript = (Script *)userData;

	ScriptAction	*pScriptAction = newInstance(ScriptAction);

	pScriptAction->m_actionType = (enum ScriptActionType)file.readInt();
	pScriptAction->m_numParms =file.readInt();
	Int i;
	for (i=0; i<pScriptAction->m_numParms; i++) {
		pScriptAction->m_parms[i] = Parameter::ReadParameter(file);
	}
	/// @todo - Verify read in parameters with current action template.  jba.
	ScriptAction *pLast = pScript->getFalseAction();
	while (pLast && pLast->getNext()) {
		pLast = pLast->getNext();
	}
	if (pLast) {
		pLast->setNextAction(pScriptAction);
	} else {
		pScript->setFalseAction(pScriptAction);
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));	
	return true;
}

// NOTE: Changing these or adding ot TheOBjectFlagNames requires changes to 
// ScriptActions::changeObjectPanelFlagForSingleObject
// THEY SHOULD STAY IN SYNC.
const char* TheObjectFlagsNames[] = 
{ 
	"Enabled",
	"Powered",
	"Indestructible",
	"Unsellable",
	"Selectable",
	"AI Recruitable",
	"Player Targetable",
	NULL,
};
