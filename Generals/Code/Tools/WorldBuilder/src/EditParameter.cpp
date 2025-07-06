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

// EditParameter.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "WorldBuilder.h"

// This is used to allow sounds to be played via PlaySound
#include <mmsystem.h>

#define DEFINE_BUILDABLE_STATUS_NAMES
#define DEFINE_OBJECT_STATUS_NAMES
#define DEFINE_SCIENCE_AVAILABILITY_NAMES

#include "EditParameter.h"
#include "EditCoordParameter.h"
#include "EditObjectParameter.h"


#include "Common/AudioEventInfo.h"
#include "Common/BorderColors.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GameAudio.h"
#include "Common/Player.h"
#include "Common/PlayerTemplate.h"
#include "Common/Radar.h"
#include "Common/SpecialPower.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/Upgrade.h"
#include "Common/WellKnownKeys.h"

#include "GameClient/ControlBar.h"
#include "GameClient/GameText.h"
#include "GameClient/VideoPlayer.h"
#include "GameClient/Anim2D.h"
#include "GameClient/ShellHooks.h"

#include "GameLogic/AI.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/Scripts.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/ScriptEngine.h"

#include "W3DDevice/GameClient/W3DGameFont.h"
#include "W3DDevice/GameClient/HeightMap.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma message("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// TYPE DEFINES ///////////////////////////////////////////////////////////////////////////////////
#define WORLDBUILDER_FONT_FILENAME		"GUIEFont.txt"

/////////////////////////////////////////////////////////////////////////////
// EditParameter dialog

AsciiString EditParameter::m_selectedLocalizedString = AsciiString::TheEmptyString;
AsciiString EditParameter::m_unitName = AsciiString::TheEmptyString;

EditParameter::EditParameter(CWnd* pParent /*=NULL*/)
	: CDialog(EditParameter::IDD, pParent),
	m_int(0),
	m_real(0)
{
	//{{AFX_DATA_INIT(EditParameter)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void EditParameter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditParameter)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EditParameter, CDialog)
	//{{AFX_MSG_MAP(EditParameter)
	ON_EN_CHANGE(IDC_EDIT, OnChangeEdit)
	ON_CBN_EDITCHANGE(IDC_COMBO, OnEditchangeCombo)
	ON_BN_CLICKED(IDC_PREVIEWSOUND, OnPreviewSound)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditParameter message handlers
SidesList *EditParameter::m_sidesListP = NULL;

Int EditParameter::edit( Parameter *pParm, AsciiString unitName ) 
{
	if (pParm->getParameterType() == Parameter::COORD3D) 
	{
		EditCoordParameter editCoordDlg;
		editCoordDlg.m_parameter = pParm;
		Int ret = editCoordDlg.DoModal();
		return ret;
	}
	else if (pParm->getParameterType() == Parameter::OBJECT_TYPE) 
	{
		EditObjectParameter editObjDlg;
		editObjDlg.m_parameter = pParm;
		Int ret = editObjDlg.DoModal();
		return ret;
	}
	else if (pParm->getParameterType() == Parameter::COLOR) 
	{
		// convert from aarrggbb to 00bbggrr, with 0% alpha
		UnsignedInt b = (pParm->getInt() & 0x000000ff);
		UnsignedInt g = (pParm->getInt() & 0x0000ff00)>>8;
		UnsignedInt r = (pParm->getInt() & 0x00ff0000)>>16;
		UnsignedInt a = 0x00;
		UnsignedInt colorref = (a<<24)|(b<<16)|(g<<8)|(r);

		CColorDialog editColorDlg(colorref, CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT|CC_SOLIDCOLOR);
		Int ret = editColorDlg.DoModal();
		if (ret == IDOK)
		{
			colorref = editColorDlg.GetColor();
			// convert from 00bbggrr to aarrggbb, with 100% alpha
			r = (colorref & 0x000000ff);
			g = (colorref & 0x0000ff00)>>8;
			b = (colorref & 0x00ff0000)>>16;
			a = 0xff;
			pParm->friend_setInt((a<<24)|(r<<16)|(g<<8)|(b));
		}
		return ret;
	}
	else
	{
		EditParameter editDlg;
		editDlg.m_parameter = pParm;
		//Set the name of the unit, because some parameters build information from the unit itself.
		editDlg.m_unitName = unitName;
		Int ret = editDlg.DoModal();
		return ret;
	}
}

AsciiString EditParameter::getWarningText(Parameter *pParm) 
{
	AsciiString warningText;
	AsciiString uiString = pParm->getString();
	if (uiString.isEmpty()) 
		uiString = "???";
	switch (pParm->getParameterType()) {
		default:
			DEBUG_CRASH(("Unknown parameter type."));
			break;
		case Parameter::SCRIPT:
			if (!loadScripts(NULL, false, uiString)) {
				warningText.format("Script '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::SCRIPT_SUBROUTINE:
			if (!loadScripts(NULL, true, uiString)) {
				if (!loadScripts(NULL, false, uiString)) {
					warningText.format("Script '%s' does not exist.", uiString.str());
				} else {
					warningText.format("Script '%s' is not a subroutine.", uiString.str());
				}
			}
			break;
		case Parameter::ATTACK_PRIORITY_SET:
			if (!loadAttackPrioritySets(NULL, uiString)) {
				warningText.format("Attack priority set '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::WAYPOINT:
			if (!loadWaypoints(NULL, uiString)) {
				warningText.format("Waypoint '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::WAYPOINT_PATH:
			if (!loadWaypointPaths(NULL, uiString)) {
				warningText.format("Waypoint '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::TRIGGER_AREA:
			if (!loadTriggerAreas(NULL, uiString)) {
				warningText.format("Waypoint '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::COMMAND_BUTTON:
			if (!loadCommandButtons(NULL, uiString)) {
				warningText.format("Command button '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::FONT_NAME:
			if(!loadFontNames(NULL, uiString)) {
				warningText.format("Font '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::TEAM_STATE:
			break;
		case Parameter::TEXT_STRING:
			break;
		case Parameter::LOCALIZED_TEXT:
			if (loadLocalizedText(NULL, uiString) == AsciiString::TheEmptyString) {
				warningText.format("Localized string '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::SOUND:
			if (!loadAudioType(Parameter::SOUND, NULL, uiString)) {
				warningText.format("Sound '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::TEAM:
			if (!loadTeams(NULL, uiString)) {
				warningText.format("Team '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::BRIDGE:
			if (!loadBridges(NULL, uiString)) {
				warningText.format("Bridge '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::UNIT:
			if (!loadUnits(NULL, uiString)) {
				warningText.format("Unit '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::OBJECT_TYPE:
			if (!loadObjectType(NULL, uiString)) {
				warningText.format("Object type '%s' does not exist.", uiString.str());
			}
			break;
		case Parameter::SIDE:
			if (!loadSides(NULL, uiString)) {
				warningText.format("Player '%s' does not exist.", uiString.str());
			}
			break;

		case Parameter::OBJECT_PANEL_FLAG:
			if (!loadObjectFlags(NULL, uiString)) {
				warningText.format("Object flag '%s' is unrecognized.", uiString.str());
			}
			break;
	

		case Parameter::OBJECT_TYPE_LIST:
			// No warning is possible.
			break;
		case Parameter::COUNTER:
			/// @todo - perhaps add more complex syntaxing for counters.
			break;
		case Parameter::INT:
			break;
		case Parameter::COLOR:
			break;
		case Parameter::KIND_OF_PARAM:
			break;
		case Parameter::COORD3D:
			break;
		case Parameter::ANGLE:
			break;
		case Parameter::BOOLEAN:
			break;
					 
		case Parameter::REAL:
			break;

		case Parameter::FLAG:
			/// @todo - perhaps add more complex syntaxing for flags.
			break;
		case Parameter::COMPARISON:
			break;

		case Parameter::AI_MOOD:
			break;

		case Parameter::SKIRMISH_WAYPOINT_PATH:
			break;

		case Parameter::RADAR_EVENT_TYPE:
			break;

		case Parameter::RELATION:
			break;

		case Parameter::DIALOG:
			if (!loadAudioType(Parameter::DIALOG, NULL, uiString)) {
				warningText.format("Dialog '%s' does not exist.", uiString.str());
			}
			break;

		case Parameter::MUSIC:
			if (!loadAudioType(Parameter::MUSIC, NULL, uiString)) {
				warningText.format("Track '%s' does not exist.", uiString.str());
			}
			break;
			
		case Parameter::MOVIE:
			if (!loadMovies(NULL, uiString)) {
				AsciiString commentFromINI;
				if (!getMovieComment(uiString, commentFromINI)) {
					warningText.format("Movie '%s' does not exit.", uiString.str());
				} else {
					warningText.format("'%s': %s", uiString.str(), commentFromINI.str());
				}
			}
			break;

		case Parameter::SPECIAL_POWER:
			if (!loadSpecialPowers(NULL, uiString)) {
				warningText.format("Special Power '%s' does not exist.", uiString.str());
			}
			break;

		case Parameter::SCIENCE:
			if (!loadSciences(NULL, uiString)) {
				warningText.format("Science '%s' does not exist.", uiString.str());
			}
			break;
		
		case Parameter::SCIENCE_AVAILABILITY:
			if( !loadScienceAvailabilities( NULL, uiString ) )
			{
				warningText.format( "Science availability '%s' does not exist.", uiString.str() );
			}
			break;

		case Parameter::UPGRADE:
			if (!loadUpgrades(NULL, uiString)) {
				warningText.format("Upgrade '%s' does not exist.", uiString.str());
			}
			break;

		case Parameter::COMMANDBUTTON_ABILITY:
		case Parameter::COMMANDBUTTON_ALL_ABILITIES:
			//Not sure if I need to do anything here.
			break;
		
		
		case Parameter::BOUNDARY:
			if (TheTerrainRenderObject->getMap()->getAllBoundaries().size() <= pParm->getInt()) {
				warningText.format("Border %s does not exist.", BORDER_COLORS[pParm->getInt() % BORDER_COLORS_SIZE]);
			} 
			break;
		
		case Parameter::BUILDABLE:
			break;

		case Parameter::SURFACES_ALLOWED:
			break;

		case Parameter::SHAKE_INTENSITY:
			break;

		case Parameter::OBJECT_STATUS:
			break;

		case Parameter::FACTION_NAME:
			break;
		
		case Parameter::EMOTICON:
			break;

		case Parameter::REVEALNAME:
			break;
			
	}
	return warningText;
}


AsciiString EditParameter::getInfoText(Parameter *pParm)
{
	AsciiString infoText;
	AsciiString uiString = pParm->getString();
	if (uiString.isEmpty()) 
		uiString = "???";
	switch (pParm->getParameterType()) 
	{
		default:
			DEBUG_CRASH(("Unknown parameter type."));
			break;
		case Parameter::SCRIPT:
		case Parameter::SCRIPT_SUBROUTINE:
		case Parameter::ATTACK_PRIORITY_SET:
		case Parameter::WAYPOINT:
		case Parameter::WAYPOINT_PATH:
		case Parameter::TRIGGER_AREA:
		case Parameter::COMMAND_BUTTON:
		case Parameter::FONT_NAME:
		case Parameter::TEAM_STATE:
		case Parameter::TEXT_STRING:
		case Parameter::SOUND:
		case Parameter::TEAM:
		case Parameter::BRIDGE:
		case Parameter::UNIT:
		case Parameter::OBJECT_TYPE:
		case Parameter::KIND_OF_PARAM:
		case Parameter::SIDE:
		case Parameter::COUNTER:
		case Parameter::INT:
		case Parameter::COLOR:
		case Parameter::COORD3D:
		case Parameter::ANGLE:
		case Parameter::BOOLEAN:
		case Parameter::REAL:
		case Parameter::FLAG:
		case Parameter::COMPARISON:
		case Parameter::AI_MOOD:
		case Parameter::SKIRMISH_WAYPOINT_PATH:
		case Parameter::RADAR_EVENT_TYPE:
		case Parameter::RELATION:
		case Parameter::DIALOG:
		case Parameter::MUSIC:
		case Parameter::SPECIAL_POWER:
		case Parameter::SCIENCE:
		case Parameter::SCIENCE_AVAILABILITY:
		case Parameter::UPGRADE:
		case Parameter::COMMANDBUTTON_ABILITY:
		case Parameter::COMMANDBUTTON_ALL_ABILITIES:
		case Parameter::BOUNDARY:
		case Parameter::BUILDABLE:
		case Parameter::SURFACES_ALLOWED:
		case Parameter::SHAKE_INTENSITY:
		case Parameter::OBJECT_STATUS:
		case Parameter::FACTION_NAME:
		case Parameter::EMOTICON:
		case Parameter::OBJECT_TYPE_LIST:
		case Parameter::REVEALNAME:
		case Parameter::OBJECT_PANEL_FLAG:
			break;

		case Parameter::LOCALIZED_TEXT:
		{
				AsciiString localizedString;
				localizedString.translate(TheGameText->fetch(uiString.str()));
				infoText.format("'%s': %s", uiString.str(), localizedString.str());
			break;
		}
		case Parameter::MOVIE:
		{
			AsciiString commentFromINI;
			if (getMovieComment(uiString, commentFromINI)) {
				infoText.format("'%s': %s", uiString.str(), commentFromINI.str());
			}
			break;
		}
	}
	return infoText;
}



void EditParameter::OnChangeEdit() 
{

}

void EditParameter::OnEditchangeCombo() 
{
	
}


void EditParameter::loadConditionParameter(Script *pScr, Parameter::ParameterType type, CComboBox *pCombo) 
{
	OrCondition *pOr;
	for (pOr= pScr->getOrCondition(); pOr; pOr = pOr->getNextOrCondition()) {
		Condition *pCondition;
		for (pCondition = pOr->getFirstAndCondition(); pCondition; pCondition = pCondition->getNext()) {
			Int i;
			for (i=0; i<pCondition->getNumParameters(); i++) {
				if (type == pCondition->getParameter(i)->getParameterType()) {
					if (CB_ERR == pCombo->FindStringExact(-1, pCondition->getParameter(i)->getString().str())) {
						pCombo->AddString(pCondition->getParameter(i)->getString().str());
					}
				}
			}
		}
	}
	if(type == Parameter::FLAG)
	{
		for(int i = 0; i < SHELL_SCRIPT_HOOK_TOTAL; i++)
		{
			if(pCombo->FindStringExact(-1,TheShellHookNames[i]) < 0)
				pCombo->AddString(TheShellHookNames[i]);
		}
	}
}

void EditParameter::loadActionParameter(Script *pScr, Parameter::ParameterType type, 	CComboBox *pCombo) 
{
	ScriptAction *pAction;
	for (pAction = pScr->getAction(); pAction; pAction = pAction->getNext()) {
		Int i;
		for (i=0; i<pAction->getNumParameters(); i++) {
			if (type == pAction->getParameter(i)->getParameterType()) {
				if (CB_ERR == pCombo->FindStringExact(-1, pAction->getParameter(i)->getString().str())) {
					pCombo->AddString(pAction->getParameter(i)->getString().str());
				}
			}
		}
	}
}

Bool EditParameter::loadAttackSetParameter(Script *pScr, CComboBox *pCombo, AsciiString match) 
{
	ScriptAction *pAction;
	Bool found = false;
	for (pAction = pScr->getAction(); pAction; pAction = pAction->getNext()) {
		// Attack priorities are created by SET_ATTACK_PRIORITY_* actions, but 
		// referenced by *_APPLY_PRIORITY actions.  So just load from the SET_... ones.
		if (pAction->getActionType() != ScriptAction::SET_ATTACK_PRIORITY_KIND_OF &&
			pAction->getActionType() != ScriptAction::SET_DEFAULT_ATTACK_PRIORITY &&
			pAction->getActionType() != ScriptAction::SET_ATTACK_PRIORITY_THING) {
			continue;
		}
		Int i;
		for (i=0; i<pAction->getNumParameters(); i++) {
			if (Parameter::ATTACK_PRIORITY_SET == pAction->getParameter(i)->getParameterType()) {
				if (match==pAction->getParameter(i)->getString()) {
					found = true;
				}
				if (pCombo && CB_ERR == pCombo->FindStringExact(-1, pAction->getParameter(i)->getString().str())) {
					pCombo->AddString(pAction->getParameter(i)->getString().str());
				}
			}
		}
	}
	return found;
}

Bool EditParameter::loadCreateUnitParameter(Script *pScr, CComboBox *pCombo, AsciiString match) 
{
	ScriptAction *pAction;
	Bool found = false;
	for (pAction = pScr->getAction(); pAction; pAction = pAction->getNext()) {
		if (pAction->getActionType() != ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT
				&& pAction->getActionType() != ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION) {
			continue;
		}
		Int i;
		for (i=0; i<pAction->getNumParameters(); i++) {
			if (Parameter::UNIT == pAction->getParameter(i)->getParameterType()) {
				if (match==pAction->getParameter(i)->getString()) {
					found = true;
				}
				if (pCombo && CB_ERR == pCombo->FindStringExact(-1, pAction->getParameter(i)->getString().str())) {
					pCombo->AddString(pAction->getParameter(i)->getString().str());
				}
			}
		}
	}
	return found;
}

Bool EditParameter::loadCreateObjectListsParameter(Script *pScr, CComboBox *pCombo, std::vector<AsciiString> *strings, AsciiString match)
{
	ScriptAction *pAction;
	Bool found = false;
	for (pAction = pScr->getAction(); pAction; pAction = pAction->getNext()) {
		if (pAction->getActionType() != ScriptAction::OBJECTLIST_ADDOBJECTTYPE) {
			continue;
		}
		
		if (Parameter::OBJECT_TYPE_LIST == pAction->getParameter(0)->getParameterType()) {
			if (match == pAction->getParameter(0)->getString()) {
				found = true;
			}
			if (pCombo && CB_ERR == pCombo->FindStringExact(-1, pAction->getParameter(0)->getString().str())) {
				pCombo->AddString(pAction->getParameter(0)->getString().str());
			}
			if (strings && std::find(strings->begin(), strings->end(), pAction->getParameter(0)->getString().str()) == strings->end()) {
				strings->push_back(pAction->getParameter(0)->getString().str());
			}
		}
	}

	return found;
}

AsciiString EditParameter::getCreatedUnitTemplateName(AsciiString unitName)
{
	SidesList *sidesListP = m_sidesListP;

	for (int i=0; i<sidesListP->getNumSides(); i++) {
		BuildListInfo *bldList = sidesListP->getSideInfo(i)->getBuildList();
		while (bldList) {
			if (bldList->getBuildingName() == unitName) {
				return bldList->getTemplateName();
			}
			bldList = bldList->getNext();
		}
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		Script *pScr;
		ScriptAction *pAction;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			for (pAction = pScr->getAction(); pAction; pAction = pAction->getNext()) {
				if (pAction->getActionType() != ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT 
						&& pAction->getActionType() != ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION) {
					continue;
				}
				Int i;
				Bool thisOne = false;
				for (i=0; i<pAction->getNumParameters(); i++) {
					if (Parameter::UNIT == pAction->getParameter(i)->getParameterType()) {
						if (unitName==pAction->getParameter(i)->getString()) {
							thisOne = true;
						}
					}
				}
				if (thisOne) {
					for (i=0; i<pAction->getNumParameters(); i++) {
						if (Parameter::OBJECT_TYPE == pAction->getParameter(i)->getParameterType()) {
							return pAction->getParameter(i)->getString();
						}
					}
				}
			}
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
				for (pAction = pScr->getAction(); pAction; pAction = pAction->getNext()) {
					if (pAction->getActionType() != ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT
							&& pAction->getActionType() != ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION) {
						continue;
					}
					Int i;
					Bool thisOne = false;
					for (i=0; i<pAction->getNumParameters(); i++) {
						if (Parameter::UNIT == pAction->getParameter(i)->getParameterType()) {
							if (unitName==pAction->getParameter(i)->getString()) {
								thisOne = true;
							}
						}
					}
					if (thisOne) {
						for (i=0; i<pAction->getNumParameters(); i++) {
							if (Parameter::OBJECT_TYPE == pAction->getParameter(i)->getParameterType()) {
								return pAction->getParameter(i)->getString();
							}
						}
					}
				}
			}		
		}
	}
	return AsciiString::TheEmptyString;
}

void EditParameter::loadCounters(CComboBox *pCombo) 
{
	pCombo->ResetContent();
	Int i;
	SidesList *sidesListP = m_sidesListP;
	if (sidesListP==NULL) sidesListP = TheSidesList;
	for (i=0; i<sidesListP->getNumSides(); i++) {
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			loadConditionParameter(pScr, Parameter::COUNTER, pCombo);
			loadActionParameter(pScr, Parameter::COUNTER, pCombo);
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				loadConditionParameter(pScr, Parameter::COUNTER, pCombo);
				loadActionParameter(pScr, Parameter::COUNTER, pCombo);
			}
		}
	}
}

Bool EditParameter::loadAttackPrioritySets(CComboBox *pCombo, AsciiString match) 
{
	if (pCombo) pCombo->ResetContent();
	Int i;
	Bool found = false;
	SidesList *sidesListP = m_sidesListP;
	if (sidesListP==NULL) sidesListP = TheSidesList;
	for (i=0; i<sidesListP->getNumSides(); i++) {
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			// Attack priority sets get created in actions.
			if (loadAttackSetParameter(pScr, pCombo, match)) {
				found=true;
			}
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
			// Attack priority sets get created in actions.
				if (loadAttackSetParameter(pScr, pCombo, match)) {
					found	= true;
				}
			}
		}
	}
	return found;
}

Bool EditParameter::loadSpecialPowers(CComboBox *pCombo, AsciiString match)
{
	Bool retVal = false;
	Int numPowers = TheSpecialPowerStore->getNumSpecialPowers();
	for (int i = 0; i < numPowers; ++i) {
		const SpecialPowerTemplate *pPower = TheSpecialPowerStore->getSpecialPowerTemplateByIndex(i);
		if (!pPower) {
			continue;
		}
		
		AsciiString powerName = pPower->getName();
		if (pCombo) {
			pCombo->AddString(powerName.str());
		}

		if (match == powerName) {
				retVal = true;
		}
	}

	return retVal;
}

//=============================================================================
Bool EditParameter::loadSciences(CComboBox *pCombo, AsciiString match)
{
	Bool retVal = false;

	std::vector<AsciiString> v = TheScienceStore->friend_getScienceNames();
	for (int i = 0; i < v.size(); ++i) 
	{
		if (pCombo) 
		{
			pCombo->AddString(v[i].str());
		}

		if (match.compare(v[i]) == 0) 
		{
			retVal = true;
		}
	}


	return retVal;
}

//=============================================================================
Bool EditParameter::loadScienceAvailabilities( CComboBox *pCombo, AsciiString match )
{
	Bool retVal = false;
	for( Int i = 0; i < SCIENCE_AVAILABILITY_COUNT; i++ )
	{
		if( pCombo )
		{
			pCombo->AddString( ScienceAvailabilityNames[ i ] );
		}

		if( !match.compareNoCase( ScienceAvailabilityNames[ i ] ) )
		{
			retVal = true;
		}
	}

	return retVal;
}

//=============================================================================
Bool EditParameter::loadUpgrades(CComboBox *pCombo, AsciiString match)
{
	Bool retVal = false;
	std::vector<AsciiString> upgradeNames = TheUpgradeCenter->getUpgradeNames();
	Int numUpgrades = upgradeNames.size();

	for (int i = 0; i < numUpgrades; ++i) {
		
		AsciiString upgradeName = upgradeNames[i];
		if (pCombo) {
			pCombo->AddString(upgradeName.str());
		}

		if (match == upgradeName) {
				retVal = true;
		}
	}

	return retVal;
}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadAbilities( CComboBox *pCombo, AsciiString match )
{
	Bool retVal = FALSE;
	
	MapObject *theUnit;
	for( theUnit = MapObject::getFirstMapObject(); theUnit; theUnit = theUnit->getNext() ) 
	{
		Bool exists;
		AsciiString objName = theUnit->getProperties()->getAsciiString(TheKey_objectName, &exists); 
		if( !exists ) 
		{
			continue;
		}
		if( objName.isEmpty() ) 
		{
			continue;
		}
		if( theUnit->getFlag(FLAG_BRIDGE_FLAGS) ) 
		{
			continue;
		}
		if( objName == m_unitName )
		{
			break;
		}
	}
	const ThingTemplate *theTemplate = NULL; 

	if ( theUnit ) {
		theTemplate = theUnit->getThingTemplate();
	} else {
		// see if the object was created via script, and therefore wasn't in the mapobject list.
		AsciiString tmplName = getCreatedUnitTemplateName(m_unitName);
		if (!tmplName.isEmpty()) {
			theTemplate = TheThingFactory->findTemplate(tmplName);
		}
	}

	if (!theTemplate) {
		return false;
	}

	//Find the object's commandset.
	if( !TheControlBar )
	{
		// create the command bar
		TheControlBar = new ControlBar;
		TheControlBar->init();
	}
	const CommandSet *commandSet = TheControlBar->findCommandSet( theTemplate->friend_getCommandSetString() );

	if( commandSet )
	{
		for( Int i = 0; i < MAX_COMMANDS_PER_SET; i++ )
		{
			//Get the command button.
			const CommandButton *commandButton = commandSet->getCommandButton(i);

			if( commandButton )
			{
				if( !commandButton->getName().isEmpty() )
				{
					if( pCombo )
					{
						pCombo->AddString( commandButton->getName().str() );
						if( match == commandButton->getName() )
						{
							retVal = TRUE;
						}
					}
				}
			}
		}
	}
	else
	{
		//This unit doesn't have a commandset.
		return FALSE;
	}
	return retVal;
}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadAllAbilities( CComboBox *pCombo, AsciiString match )
{
	Bool retVal = FALSE;
	
	if( !TheControlBar )
	{
		// create the command bar
		TheControlBar = new ControlBar;
		TheControlBar->init();
	}

	//Iterate through all the command button definitions
	const CommandButton *commandButton = TheControlBar->getCommandButtons();
	while( commandButton )
	{
		if( !commandButton->getName().isEmpty() )
		{
			if( pCombo )
			{
				pCombo->AddString( commandButton->getName().str() );
				if( match == commandButton->getName() )
				{
					retVal = TRUE;
				}
			}
		}
		commandButton = commandButton->getNext();
	}
	return retVal;
}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadEmoticons( CComboBox *pCombo, AsciiString match )
{
	Bool retVal = FALSE;
	
	Anim2DTemplate *animTemplate = TheAnim2DCollection->getTemplateHead();
	//Iterate through all the definitions

	while( animTemplate )
	{
		if( animTemplate->getName().isNotEmpty() )
		{
			if( pCombo )
			{
				pCombo->AddString( animTemplate->getName().str() );
				if( match == animTemplate->getName() )
				{
					retVal = TRUE;
				}
			}
		}
		animTemplate = TheAnim2DCollection->getNextTemplate( animTemplate );
	}
	return retVal;
}

//-------------------------------------------------------------------------------------------------
void EditParameter::loadFlags(CComboBox *pCombo) 
{
	pCombo->ResetContent();
	Int i;
	SidesList *sidesListP = m_sidesListP;
	if (sidesListP==NULL) sidesListP = TheSidesList;
	for (i=0; i<sidesListP->getNumSides(); i++) {
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			loadConditionParameter(pScr, Parameter::FLAG, pCombo);
			loadActionParameter(pScr, Parameter::FLAG, pCombo);
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				loadConditionParameter(pScr, Parameter::FLAG, pCombo);
				loadActionParameter(pScr, Parameter::FLAG, pCombo);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadObjectType(CComboBox *pCombo, AsciiString match) 
{
	if (pCombo) 
		pCombo->ResetContent();
	
	Bool didMatch = false;

	didMatch = loadObjectTypeList(pCombo, NULL, match);

	// add entries from the thing factory as the available objects to use
	const ThingTemplate *tTemplate;
	for( tTemplate = TheThingFactory->firstTemplate();
		tTemplate;
		tTemplate = tTemplate->friend_getNextTemplate() )
	{
		if (pCombo) 
			pCombo->AddString( tTemplate->getName().str());
		
		if ((match==tTemplate->getName())) 
			didMatch = true;
	}
	return didMatch;
}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadObjectTypeList(CComboBox *pCombo, std::vector<AsciiString> *strings, AsciiString match)
{
	Bool didMatch = false;

	if (pCombo)
		pCombo->ResetContent();

	if (strings)
		strings->clear();

	SidesList *sidesListP = m_sidesListP;

	// first, add any object lists that exist anywhere in the scripts
	for (Int i=0; i<sidesListP->getNumSides(); ++i) {
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			// Attack priority sets get created in actions.
			if (loadCreateObjectListsParameter(pScr, pCombo, strings, match)) {
				didMatch=true;
			}
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
			// Attack priority sets get created in actions.
				if (loadCreateObjectListsParameter(pScr, pCombo, strings, match)) {
					didMatch	= true;
				}
			}
		}
	}

	return didMatch;
}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadRevealNames(CComboBox *pCombo, AsciiString match)
{
	Bool didMatch = false;

	if (pCombo)
		pCombo->ResetContent();

	SidesList *sidesListP = m_sidesListP;

	for (Int i = 0; i < sidesListP->getNumSides(); ++i) {
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr = pScr->getNext()) {
			if (loadRevealNamesParameter(pScr, pCombo, match)) {
				didMatch = true;
			}
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				if (loadRevealNamesParameter(pScr, pCombo, match)) {
					didMatch = true;
				}
			}
		}
	}

	return didMatch;	
}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadRevealNamesParameter(Script *pScr, CComboBox *pCombo, AsciiString match)
{
	ScriptAction *pAction;
	Bool found = false;
	for (pAction = pScr->getAction(); pAction; pAction = pAction->getNext()) {
		if (pAction->getActionType() != ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT) {
			continue;
		}
		
		if (Parameter::REVEALNAME == pAction->getParameter(3)->getParameterType()) {
			if (match == pAction->getParameter(3)->getString()) {
				found = true;
			}
			if (pCombo && CB_ERR == pCombo->FindStringExact(-1, pAction->getParameter(3)->getString().str())) {
				pCombo->AddString(pAction->getParameter(3)->getString().str());
			}
		}
	}

	return found;
}


//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadAudioType(Parameter::ParameterType  comboType, CComboBox *pCombo, AsciiString match)
{
	Bool retVal = false;
	AudioType type = AT_SoundEffect;
	switch (comboType)
	{
		case Parameter::DIALOG: type = AT_Streaming; break;
		case Parameter::SOUND: type = AT_SoundEffect; break;
		case Parameter::MUSIC: type = AT_Music; break;
	}

	std::vector<AudioEventInfo *> eventInfos;
	TheAudio->findAllAudioEventsOfType(type, eventInfos);
	
	for (int i = 0; i < eventInfos.size(); ++i) {
		if (eventInfos[i]) {
			if (pCombo) {
				pCombo->AddString(eventInfos[i]->m_audioName.str());
			}
			
			if (match == eventInfos[i]->m_audioName) {
				retVal = true;
			}
		}
	}
	return retVal;

}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadMovies(CComboBox *pCombo, AsciiString match)
{
	Bool retVal = false;
	Int numVideos = TheVideoPlayer->getNumVideos();
	for (int i = 0; i < numVideos; ++i) {
		const Video *pVideo = TheVideoPlayer->getVideo(i);
		if (!pVideo) {
			continue;
		}
		
		AsciiString videoName = pVideo->m_internalName;
		if (pCombo) {
			pCombo->AddString(videoName.str());
		}

		if (match == videoName) {
				retVal = true;
		}
	}

	return retVal;
}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::getMovieComment(AsciiString match, AsciiString& outCommentFromINI)
{
	Bool retVal = false;
	Int numVideos = TheVideoPlayer->getNumVideos();
	for (int i = 0; i < numVideos; ++i) {
		const Video *pVideo = TheVideoPlayer->getVideo(i);
		if (!pVideo) {
			continue;
		}
		
		AsciiString videoName = pVideo->m_internalName;
		if (match == videoName && pVideo->m_commentForWB != AsciiString::TheEmptyString) {
			outCommentFromINI = pVideo->m_commentForWB;
			retVal = true;
		}
	}

	return retVal;
}


//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadTriggerAreas(CComboBox *pCombo, AsciiString match) 
{
	if (pCombo) pCombo->ResetContent();
	Bool didMatch = false;
	AsciiString trigName;
	for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
		trigName = pTrig->getTriggerName();
		if (pCombo) pCombo->AddString(trigName.str());
		if ((match==trigName)) didMatch = true;
	}
	trigName = MY_INNER_PERIMETER;
	if (pCombo) pCombo->AddString(trigName.str());
	if ((match==trigName)) didMatch = true;
	trigName = MY_OUTER_PERIMETER;
	if (pCombo) pCombo->AddString(trigName.str());
	if ((match==trigName)) didMatch = true;
	trigName = ENEMY_OUTER_PERIMETER;
	if (pCombo) pCombo->AddString(trigName.str());
	if ((match==trigName)) didMatch = true;
	trigName = ENEMY_INNER_PERIMETER;
	if (pCombo) pCombo->AddString(trigName.str());
	if ((match==trigName)) didMatch = true;
	trigName = WATER_GRID;
	if (pCombo) pCombo->AddString(trigName.str());
	if ((match==trigName)) didMatch = true;
	return didMatch;
}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadCommandButtons(CComboBox *pCombo, AsciiString match)
{
	if (pCombo) pCombo->ResetContent();
	Bool didMatch = false;
	// gets the list of command buttons
	File *fp = TheFileSystem->openFile("Data\\INI\\CommandButton.ini", File::READ | File::TEXT);
	//sanity
	DEBUG_ASSERTCRASH( fp, ("Cannot find file CommandButton.ini"));

	char buf[1024];
	char *string;
	char *token;
	char seps[]   = " ,\t\n";

	fp->nextLine(buf, 1024);
	string = buf;
	while (fp->eof() == FALSE)
	{
		token = strtok(string, seps);
		if( token != NULL )
		{
			if( strcmp( token, "CommandButton" ) == 0)
			{
				token = strtok(NULL, seps);
				if( token != NULL )
				{
					if (pCombo) pCombo->AddString(token);
					if (strcmp(match.str(), token) == 0) didMatch = true;
				}
			}
		}
		fp->nextLine(buf, 1024);
		string = buf;
	}

	fp->close();
	fp = NULL;

	return didMatch;
}

//-------------------------------------------------------------------------------------------------
Bool EditParameter::loadFontNames(CComboBox *pCombo, AsciiString match)
{
	if (pCombo) pCombo->ResetContent();
	Bool didMatch = false;
	GameFont *font;


	// create the font library
	TheFontLibrary = new W3DFontLibrary;
	if( TheFontLibrary )
		TheFontLibrary->init();

	// reads the font file into TheFontLibrary
	readFontFile( WORLDBUILDER_FONT_FILENAME );

	for( font = TheFontLibrary->firstFont(); font; font = TheFontLibrary->nextFont( font ) )
	{
		AsciiString string = font->nameString;
		string.concat( " - Size:" );
		Int size = font->pointSize;
		char buffer[33];
    itoa( size, buffer, 10 );
		string.concat( buffer );
		if( font->bold )
			string.concat( " [Bold]" );
		if( string != AsciiString::TheEmptyString )
		{
			if (pCombo) pCombo->AddString( string.str() );
			if( (match==string) ) didMatch = true;
		}
	}

	// delete the font library
	TheFontLibrary->reset();
	delete TheFontLibrary;
	TheFontLibrary = NULL;

	return didMatch;
}

// EditParameter::readFontFile ======================================================
/** Read the font file defintitions and load them */
//=============================================================================
void EditParameter::readFontFile( const char *filename )
{
	File *fp;

	// sanity
	if( filename == NULL )
		return;

	// open the file
	fp = TheFileSystem->openFile( filename, File::READ | File::TEXT);
	if( fp == NULL )
		return;

	// read how many entries follow
	Int fontCount;
	fp->read(NULL, sizeof("AVAILABLE_FONT_COUNT = "));
	fp->scanInt(fontCount);

	for( Int i = 0; i < fontCount; i++ )
	{

		// read all the font defitions
		char fontBuffer[ 512 ];
		Int size, bold;
		char c;
		fp->read(&c, 1);

		// skip past the first quote
		while( c != '\"' )
			fp->read(&c, 1);
		fp->read(&c, 1);

		// copy the name till the next quote is read
		Int index = 0;
		while( c != '\"' )
		{

			fontBuffer[ index++ ] = c;
			fp->read(&c, 1);

		}  // end while
		fontBuffer[ index ] = '\0';
		fp->read(&c, 1);

		// read the size and bold data elements
		fp->seek(strlen(" Size: "));
		fp->scanInt(size);
		fp->seek(strlen(" Bold: "));
		fp->scanInt(bold);
		fp->nextLine();

		// set the font
		GameFont *font = TheFontLibrary->getFont( AsciiString(fontBuffer), size, bold );
		if( font == NULL )
		{
			char buffer[ 1024 ];

			sprintf( buffer, "Warning: The font '%s' Size: '%d' Bold: '%d', specified in the config file could not be loaded.  Does that font exist?",
							 fontBuffer, size, bold );
			//MessageBox( m_appHWnd, buffer, "Cannot Load Font", MB_OK );
			
		}  // end if

	}  // end for i

	// close the file
	fp->close();
	fp = NULL;

}  // end readFontFile

Bool EditParameter::loadWaypoints(CComboBox *pCombo, AsciiString match) 
{
	if (pCombo) pCombo->ResetContent();
	Bool didMatch = false;
	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->isWaypoint()) {
			AsciiString wayName = pMapObj->getWaypointName();
			if (pCombo&& (CB_ERR == pCombo->FindStringExact(-1, wayName.str()))) pCombo->AddString(wayName.str());
			if ((match==wayName)) didMatch = true;
		}
	}
	return didMatch;
}

Bool EditParameter::loadWaypointPaths(CComboBox *pCombo, AsciiString match) 
{
	if (pCombo) pCombo->ResetContent();
	Bool didMatch = false;
	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->isWaypoint()) {
			AsciiString wayName;
			Bool exists;
			wayName= pMapObj->getProperties()->getAsciiString(TheKey_waypointPathLabel1, &exists);
			if (!wayName.isEmpty()) {
				if (pCombo && (CB_ERR == pCombo->FindStringExact(-1, wayName.str()))) {
					pCombo->AddString(wayName.str());
				}
				if ((match==wayName)) didMatch = true;
			}
			wayName= pMapObj->getProperties()->getAsciiString(TheKey_waypointPathLabel2, &exists);
			if (!wayName.isEmpty()) {
				if (pCombo && (CB_ERR == pCombo->FindStringExact(-1, wayName.str()))) {
					pCombo->AddString(wayName.str());
				}
				if ((match==wayName)) didMatch = true;
			}
			wayName= pMapObj->getProperties()->getAsciiString(TheKey_waypointPathLabel3, &exists);
			if (!wayName.isEmpty()) {
				if (pCombo && (CB_ERR == pCombo->FindStringExact(-1, wayName.str()))) {
					pCombo->AddString(wayName.str());
				}
				if ((match==wayName)) didMatch = true;
			}
		}
	}
	return didMatch;
}

Bool EditParameter::loadObjectFlags(CComboBox *pCombo, AsciiString match)
{
	if (pCombo) pCombo->ResetContent();

	Int i;
	Bool didMatch = false;

	for (i = 0; TheObjectFlagsNames[i]; ++i) {
		if (pCombo)
			pCombo->AddString(TheObjectFlagsNames[i]);
		if (match == TheObjectFlagsNames[i]) {
			didMatch = true;
		}
	}

	return didMatch;
}

Bool EditParameter::loadScripts(CComboBox *pCombo, Bool subr, AsciiString match) 
{
	if (pCombo) pCombo->ResetContent();
	Int i;
	SidesList *sidesListP = m_sidesListP;
	if (sidesListP==NULL) sidesListP = TheSidesList;
	Bool didMatch = false;
	for (i=0; i<sidesListP->getNumSides(); i++) {
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		if (pSL == NULL) continue;
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			if (subr && !pScr->isSubroutine()) continue;
			if (pCombo) pCombo->AddString(pScr->getName().str());
			if ((match==pScr->getName())) didMatch = true;
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			if (!subr || pGroup->isSubroutine()) {
				if (pCombo) pCombo->AddString(pGroup->getName().str());
				if ((match==pGroup->getName())) didMatch = true;
			}
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				if (subr && !pScr->isSubroutine()) continue;
				if (pCombo) pCombo->AddString(pScr->getName().str());
				if ((match==pScr->getName())) didMatch = true;
			}
		}
	}
	return didMatch;
}

Bool EditParameter::loadSides(CComboBox *pCombo, AsciiString match) 
{
	if (pCombo) {
		pCombo->ResetContent();
		pCombo->AddString(LOCAL_PLAYER);
		pCombo->AddString(THIS_PLAYER);
		pCombo->AddString(THIS_PLAYER_ENEMY);
	}
	Bool didMatch = false;
	if (match == LOCAL_PLAYER) didMatch=true;
	if (match == THIS_PLAYER) didMatch=true;
	if (match == THIS_PLAYER_ENEMY) didMatch=true;
	Int i;
	SidesList *sidesListP = m_sidesListP;
	if (sidesListP==NULL) sidesListP = TheSidesList;
	for (i=0; i<sidesListP->getNumSides(); i++) {
		Dict *d = sidesListP->getSideInfo(i)->getDict();
		AsciiString name = d->getAsciiString(TheKey_playerName);
		if (name.isEmpty()) continue;
		if (pCombo) pCombo->AddString(name.str());
		if ((name==match)) didMatch = true;
	}
	return didMatch;
}

Bool EditParameter::loadTeams(CComboBox *pCombo, AsciiString match) 
{
	if (pCombo) {
		pCombo->ResetContent();
		pCombo->AddString(THIS_TEAM);
		//pCombo->AddString(ANY_TEAM);
	}
	Bool didMatch = false;
	if (match == THIS_TEAM) didMatch=true;
	if (match == ANY_TEAM) didMatch=true;
	Int i;
	SidesList *sidesListP = m_sidesListP;
	if (sidesListP==NULL) sidesListP = TheSidesList;
	for (i = 0; i < sidesListP->getNumTeams(); i++)
	{
		Dict *d = sidesListP->getTeamInfo(i)->getDict();
		AsciiString name = d->getAsciiString(TheKey_teamName);
		DEBUG_ASSERTCRASH(!name.isEmpty(),("bad"));

		if (name == "team") {
			// Neutral team.
			continue;
		}
		if (pCombo) pCombo->AddString(name.str());
		if ((name==match)) didMatch = true;
	}
	return didMatch;
}

Bool EditParameter::loadTeamOrUnit(CComboBox *pCombo, AsciiString match) 
{
	if (pCombo) {
		pCombo->ResetContent();
	}
	Bool didMatch = false;
	Int i;
	SidesList *sidesListP = m_sidesListP;
	if (sidesListP==NULL) sidesListP = TheSidesList;
	for (i = 0; i < sidesListP->getNumTeams(); i++)
	{
		Dict *d = sidesListP->getTeamInfo(i)->getDict();
		AsciiString name = d->getAsciiString(TheKey_teamName);
		DEBUG_ASSERTCRASH(!name.isEmpty(),("bad"));

		if (name == "team") {
			// Neutral team.
			continue;
		}
		if (pCombo) pCombo->AddString(name.str());
		if ((name==match)) didMatch = true;
	}	
	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		Bool exists;
		AsciiString objName = pMapObj->getProperties()->getAsciiString(TheKey_objectName, &exists); 
		if (!exists) continue;
		if (objName.isEmpty()) continue;
		if (pCombo) pCombo->AddString(objName.str());
		if (objName == match) didMatch = true;
	}
	return didMatch;
}

Bool EditParameter::loadUnits(CComboBox *pCombo, AsciiString match)
{
	if (pCombo) 
	{
		pCombo->ResetContent();
		pCombo->AddString(THIS_OBJECT);
		//pCombo->AddString(ANY_OBJECT);
	}
	Bool didMatch = false;
	if( match == THIS_OBJECT )
	{
		didMatch = true;
	}
	if( match == ANY_OBJECT )
	{ 
		didMatch=true;
	}

	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) 
	{
		Bool exists;
		AsciiString objName = pMapObj->getProperties()->getAsciiString(TheKey_objectName, &exists); 
		if (!exists) 
		{
			continue;
		}
		if (objName.isEmpty()) 
		{
			continue;
		}
		if (pMapObj->getFlag(FLAG_ROAD_FLAGS)) 
		{
			continue;
		}
		if (pCombo) 
		{
			pCombo->AddString(objName.str());
		}
		if (objName == match) 
		{
			didMatch = true;
		}
	}

	SidesList *sidesListP = m_sidesListP;
	Int i;
	if (sidesListP==NULL) 
	{
		sidesListP = TheSidesList;
	}
	for (i = 0; i < sidesListP->getNumSides(); i++)
	{
		SidesInfo *pSide = sidesListP->getSideInfo(i); 
	
		BuildListInfo *pBI = pSide->getBuildList();
		while (pBI)
		{
			if (pBI->getBuildingName().isNotEmpty()) 
			{
				if (pCombo) 
				{
					pCombo->AddString(pBI->getBuildingName().str());
				}
				if (pBI->getBuildingName() == match) 
				{
					didMatch = true;
				}
			}
			pBI = pBI->getNext();
		}
	}
	for (i=0; i<sidesListP->getNumSides(); i++) {
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			// Attack priority sets get created in actions.
			if (loadCreateUnitParameter(pScr, pCombo, match)) {
				didMatch=true;
			}
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
			// Attack priority sets get created in actions.
				if (loadCreateUnitParameter(pScr, pCombo, match)) {
					didMatch	= true;
				}
			}
		}
	}

	return didMatch;
}

Bool EditParameter::loadTransports(CComboBox *pCombo, AsciiString match)
{
	if (pCombo) {
		pCombo->ResetContent();
	}
	Bool didMatch = false;

	// add transports from the thing factory as the available objects to use
	const ThingTemplate *tTemplate;
	for( tTemplate = TheThingFactory->firstTemplate();
		tTemplate;
		tTemplate = tTemplate->friend_getNextTemplate() )
	{

		if (tTemplate->isKindOf(KINDOF_TRANSPORT)) {
			if (pCombo) pCombo->AddString( tTemplate->getName().str());
			if ((match==tTemplate->getName())) didMatch = true;
		}
	}
	return didMatch;
}

Bool EditParameter::loadBridges(CComboBox *pCombo, AsciiString match)
{
	if (pCombo) pCombo->ResetContent();
	Bool didMatch = false;

	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		Bool exists;
		AsciiString objName = pMapObj->getProperties()->getAsciiString(TheKey_objectName, &exists); 
		if (!exists) continue;
		if (objName.isEmpty()) continue;
		if (pMapObj->getFlag(FLAG_BRIDGE_POINT1)) {
			if (pMapObj->getNext() && pMapObj->getNext()->getFlag(FLAG_BRIDGE_POINT2)) {
				if (pCombo) pCombo->AddString(objName.str());
				if (objName == match) didMatch = true;
			}
		}
	}
	return didMatch;
}

BOOL EditParameter::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd *pCaption = GetDlgItem(IDC_CAPTION);
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_COMBO);
	CComboBox *pList = (CComboBox*)GetDlgItem(IDC_LIST);
	CEdit *pEdit = (CEdit *)GetDlgItem(IDC_EDIT);

	pCombo->ShowWindow(SW_HIDE);
	pList->ShowWindow(SW_HIDE);
	pEdit->ShowWindow(SW_HIDE);

	CString captionText;
	Bool showCombo = false;
	Bool showAudioButton = false;
	CString editText;
	Bool showList = false;
	Int i;

	switch (m_parameter->getParameterType()) {
		default:
			DEBUG_CRASH(("Unknown parameter type."));
			break;
		case Parameter::SCRIPT:
			captionText = "Script named:";
			showCombo = true;
			loadScripts(pCombo, false);
			break;
		case Parameter::SCRIPT_SUBROUTINE:
			captionText = "Subroutine named:";
			showCombo = true;
			loadScripts(pCombo, true);
			break;
		case Parameter::ATTACK_PRIORITY_SET:
			captionText = "Attack priority set named:";
			showCombo = true;
			loadAttackPrioritySets(pCombo);
			break;
		case Parameter::WAYPOINT:
			captionText = "Waypoint named:";
			showCombo = true;
			loadWaypoints(pCombo);
			break;

		case Parameter::LOCALIZED_TEXT:
			captionText = "Localized string:";
			showCombo = true;
			loadLocalizedText(pCombo);
			break;
		case Parameter::WAYPOINT_PATH:
			captionText = "Waypoint labeled:";
			showCombo = true;
			loadWaypointPaths(pCombo);
			break;
		case Parameter::TRIGGER_AREA:
			captionText = "Trigger area named:";
			showCombo = true;
			loadTriggerAreas(pCombo);
			break;
		case Parameter::COMMAND_BUTTON:
			captionText = "Command button named:";
			showCombo = true;
			loadCommandButtons(pCombo);
			break;
		case Parameter::FONT_NAME:
			captionText = "Text type named:";
			showCombo = true;
			loadFontNames(pCombo);
			break;
		case Parameter::TEXT_STRING:
			captionText = "Text String:";
			editText = m_parameter->getString().str();
			showCombo = false;
			break;
		case Parameter::TEAM_STATE:
			captionText = "Team State:";
			editText = m_parameter->getString().str();
			showCombo = false;
			break;
		case Parameter::BRIDGE:
			captionText = "Bridge named:";
			showCombo = true;
			loadBridges(pCombo);
			break;
		case Parameter::TEAM:
			captionText = "Team named:";
			showCombo = true;
			loadTeams(pCombo);
			break;
		case Parameter::UNIT:
			captionText = "Unit named:";
			showCombo = true;
			loadUnits(pCombo);
			break;
		case Parameter::OBJECT_TYPE:
			captionText = "Object type:";
			showList = true;
			loadObjectType(pList);
			break;
		case Parameter::SIDE:
			captionText = "Player named:";
			showCombo = true;
			loadSides(pCombo);
			break;
		case Parameter::COUNTER:
			captionText = "Counter named:";
			showCombo = true;
			loadCounters(pCombo);
			break;
		case Parameter::INT:
			captionText = "Integer:";
			editText.Format("%d", m_parameter->getInt());
			showCombo = false;
			break;
		case Parameter::COLOR:
			DEBUG_CRASH(("should never get here for this data type"));
			captionText = "Color:";
			editText.Format("%08lx", m_parameter->getInt());
			showCombo = false;
			break;
		case Parameter::BOOLEAN:
			captionText = "Boolean:";
			pList->InsertString(-1, "False");
			pList->InsertString(-1, "True");
			pList->SetCurSel(m_parameter->getInt());
			showList = true;
			break;
					 
		case Parameter::REAL:
			captionText = "Real number:";
			editText.Format("%.2f", m_parameter->getReal());
			showCombo = false;
			break;

		case Parameter::ANGLE:
			captionText = "Angle (degrees):";
			editText.Format("%.2f", m_parameter->getReal()*180/PI);
			showCombo = false;
			break;

		case Parameter::FLAG:
			captionText = "Flag named:";
			showCombo = true;
			loadFlags(pCombo);
			break;
		case Parameter::COMPARISON:
			captionText = "Comparison:";
			pList->InsertString(-1, "Less Than");
			pList->InsertString(-1, "Less Than or Equal");
			pList->InsertString(-1, "Equal To");
			pList->InsertString(-1, "Greater Than or Equal");
			pList->InsertString(-1, "Greater Than");
			pList->InsertString(-1, "Not Equal To");
			showList = true;
			break;

		case Parameter::KIND_OF_PARAM:
			captionText = "Kind of:";
			showList = true;
			for (i=KINDOF_FIRST; i<KINDOF_COUNT; i++) {
				pList->InsertString(-1, KindOfMaskType::getBitNames()[i-KINDOF_FIRST]);				
			}
			pList->SetCurSel(m_parameter->getInt());
			break;

		case Parameter::OBJECT_PANEL_FLAG:
			captionText = "Object Panel Flag:";
			showCombo = true;
			loadObjectFlags(pCombo);
			break;

		case Parameter::AI_MOOD:
			captionText = "AI Mood:";
			pList->InsertString(-1, "Sleep");
			pList->InsertString(-1, "Passive");
			pList->InsertString(-1, "Normal");
			pList->InsertString(-1, "Alert");
			pList->InsertString(-1, "Agressive");
			pList->SetCurSel(m_parameter->getInt() - ATTITUDE_SLEEP);
			showList = true;
			break;

		case Parameter::SKIRMISH_WAYPOINT_PATH:
			captionText = "Skirmish Approach Path:";
			pList->InsertString(-1, "Center");
			pList->InsertString(-1, "Backdoor");
			pList->InsertString(-1, "Flank");
			pList->InsertString(-1, "Special");
			i = pList->FindStringExact(-1, m_parameter->getString().str());
			if (i!=CB_ERR) 
				pList->SetCurSel(i);
			else 
				pList->SetCurSel(0);
			showList = true;
			break;

		case Parameter::RADAR_EVENT_TYPE:
			captionText = "Radar Event Type:";
			pList->InsertString(-1, "Construction");
			pList->InsertString(-1,"Upgrade");
			pList->InsertString(-1,"Under Attack");
			pList->InsertString(-1,"Information");
			pList->SetCurSel(m_parameter->getInt() - RADAR_EVENT_CONSTRUCTION);
			showList = true;
			break;

		case Parameter::RELATION:
			captionText = "Relation:";
			pList->InsertString(-1, "Enemy");
			pList->InsertString(-1, "Neutral");
			pList->InsertString(-1, "Friend");
			pList->SetCurSel(m_parameter->getInt());
			showList = true;
			break;

		case Parameter::DIALOG:
			captionText = "Dialog :";
		case Parameter::MUSIC:
			captionText = "Music :";
		case Parameter::SOUND:
			if (m_parameter->getParameterType() == Parameter::SOUND) {
				captionText = "Sound :";
			}
			captionText = "Locate the Audio name:";
			showCombo = true;
			// enable the preview sound button only if we are dealing with a soundfx or speech
			showAudioButton = true;
			if (m_parameter->getParameterType() == Parameter::MUSIC)
				showAudioButton = false;
			loadAudioType(m_parameter->getParameterType(), pCombo);
			break;
		case Parameter::MOVIE:
			captionText = "Locate the Audio name:";
			showCombo = true;
			loadMovies(pCombo);
			break;

		case Parameter::SPECIAL_POWER:
			captionText = "Special Power:";
			showCombo = true;
			loadSpecialPowers(pCombo);
			break;
		
		case Parameter::SCIENCE:
			captionText = "Science:";
			showCombo = true;
			loadSciences(pCombo);
			break;

		case Parameter::SCIENCE_AVAILABILITY:
			captionText = "Science Availability:";
			showCombo = true;
			loadScienceAvailabilities( pCombo );
			break;

		case Parameter::UPGRADE:
			captionText = "Upgrade:";
			showCombo = true;
			loadUpgrades(pCombo);
			break;

		case Parameter::COMMANDBUTTON_ABILITY:
			captionText = "Ability:";
			showCombo = true;
			loadAbilities( pCombo );
			break;
		
		case Parameter::COMMANDBUTTON_ALL_ABILITIES:
			captionText = "Ability:";
			showCombo = true;
			loadAllAbilities( pCombo );
			break;

		case Parameter::BOUNDARY:
		{
			captionText = "Boundary:";
			showCombo = true;
			int numBoundaries = TheTerrainRenderObject->getMap()->getAllBoundaries().size();
			for (int i = 0; i < numBoundaries; ++i) {
				pCombo->InsertString(-1, BORDER_COLORS[i % BORDER_COLORS_SIZE].m_colorName);
			}
			break;
		}

		case Parameter::BUILDABLE:
			captionText = "Buidable:";
			showList = true;

			for (i = BSTATUS_YES; i < BSTATUS_NUM_TYPES; ++i) {
				pList->InsertString(-1, BuildableStatusNames[i - BSTATUS_YES]);
			}
			pList->SetCurSel(m_parameter->getInt());
			break;

		case Parameter::SURFACES_ALLOWED:
		{
			captionText = "Surfaces allowed: ";
			showList = true;

			for (i = 0; i < 3; ++i) {
				pList->InsertString(-1, Surfaces[i]);
			}
			
			// 0 is invalid for surfaces, so change this to a 3 (which means AIR and GROUND)
			if (m_parameter->getInt() == 0) {
				m_parameter->friend_setInt(3);
			}

			pList->SetCurSel(m_parameter->getInt() - 1);
			break;
		}

		case Parameter::SHAKE_INTENSITY:
		{
			captionText = "Shake Intensity: ";
			showList = true;
			for (i = 0; i < View::SHAKE_COUNT; ++i) {
				pList->InsertString(-1, ShakeIntensities[i]);
			}

			pList->SetCurSel(m_parameter->getInt());
			break;
		}

		case Parameter::OBJECT_STATUS:
		{
			captionText = "Object status:";
			showList = true;
			for( i = 0; i < OBJECT_STATUS_COUNT; i++ )
			{
				pList->InsertString( -1, ObjectStatusMaskType::getBitNames()[i] );				
			}
			pList->SelectString( -1, m_parameter->getString().str() );
			break;
		}

		case Parameter::FACTION_NAME:
		{
			captionText = "Faction Name: ";
			showList = true;

			AsciiStringList sideList;
			ThePlayerTemplateStore->getAllSideStrings(&sideList);

			for (AsciiStringListIterator it = sideList.begin(); it != sideList.end(); ++it) {
				pList->InsertString(-1, it->str());
			}

			pList->SetCurSel(0);
			break;
		}

		case Parameter::EMOTICON:
		{
			captionText = "Emoticons:";
			showCombo = true;
			loadEmoticons( pCombo );
			break;
		}
		case Parameter::OBJECT_TYPE_LIST:
		{	
			captionText = "Object type list:";
			showCombo = true;
			loadObjectTypeList(pCombo);
			break;
		}
		case Parameter::REVEALNAME:
		{
			captionText = "Reveal Name:";
			showCombo = true;
			loadRevealNames(pCombo);
			break;
		}


	}
	if (showCombo) {
		pCombo->ShowWindow(SW_SHOW);
		pCombo->SetWindowText(m_parameter->getString().str());
		if (m_parameter->getString().isEmpty()) {
			pCombo->SetCurSel(0);
		}
		pCombo->SetFocus();
	}	else if (showList) {
		pList->ShowWindow(SW_SHOW);
		pList->SetFocus();
	}	else {
		pEdit->ShowWindow(SW_SHOW);
		pEdit->SetWindowText(editText);
		pEdit->SetFocus();
	}
	pCaption->SetWindowText(captionText);

	CButton *previewSound = (CButton*)GetDlgItem(IDPREVIEWSOUND);
	if (previewSound) {
		previewSound->ShowWindow(showAudioButton ? SW_SHOW : SW_HIDE);
	}

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void EditParameter::OnOK() 
{
	CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_COMBO);
	CEdit *pEdit = (CEdit *)GetDlgItem(IDC_EDIT);
	CComboBox *pList = (CComboBox*)GetDlgItem(IDC_LIST);
	CString txt;
	AsciiString comboText;
	switch (m_parameter->getParameterType()) {
		default:
			DEBUG_CRASH(("Unknown parameter type."));
			break;
		case Parameter::UNIT:
		case Parameter::SCRIPT:
		case Parameter::SCRIPT_SUBROUTINE:
		case Parameter::ATTACK_PRIORITY_SET:
		case Parameter::SIDE:
		case Parameter::WAYPOINT:
		case Parameter::WAYPOINT_PATH:
		case Parameter::TRIGGER_AREA:
		case Parameter::COMMAND_BUTTON:
		case Parameter::FONT_NAME:
		case Parameter::TEAM:
		case Parameter::OBJECT_TYPE:
		case Parameter::OBJECT_TYPE_LIST:
		case Parameter::BRIDGE:
		case Parameter::FLAG:
		case Parameter::COUNTER:
		case Parameter::DIALOG:
		case Parameter::MUSIC:
		case Parameter::SPECIAL_POWER:
		case Parameter::SCIENCE:
		case Parameter::SCIENCE_AVAILABILITY:
		case Parameter::UPGRADE:
		case Parameter::SOUND:
		case Parameter::MOVIE:
		case Parameter::COMMANDBUTTON_ABILITY:
		case Parameter::COMMANDBUTTON_ALL_ABILITIES:
		case Parameter::EMOTICON:
		case Parameter::REVEALNAME:
			pCombo->GetWindowText(txt);
			comboText = AsciiString(txt);
			m_parameter->friend_setString(comboText);
			break;
		case Parameter::TEAM_STATE:
		case Parameter::TEXT_STRING:
			pEdit->GetWindowText(txt);
			comboText = AsciiString(txt);
			m_parameter->friend_setString(comboText);
			break;
		case Parameter::INT:
			pEdit->GetWindowText(txt);
			Int theInt;
			if (1==sscanf(txt, "%d", &theInt)) {
				m_parameter->friend_setInt(theInt);
			} else {
				pEdit->SetFocus();
				::MessageBeep(MB_ICONEXCLAMATION);
				return;
			}
			break;

		case Parameter::COLOR:
			DEBUG_CRASH(("should never get here for this data type"));
			pEdit->GetWindowText(txt);
			if (1==sscanf(txt, "%08lx", &theInt)) {
				m_parameter->friend_setInt(theInt);
			} else {
				pEdit->SetFocus();
				::MessageBeep(MB_ICONEXCLAMATION);
				return;
			}
			break;
					 
		case Parameter::REAL:
			pEdit->GetWindowText(txt);
			Real theReal;
			if (1==sscanf(txt, "%f", &theReal)) {
				m_parameter->friend_setReal(theReal);
			} else {
				pEdit->SetFocus();
				::MessageBeep(MB_ICONEXCLAMATION);
				return;
			}
			break;

		case Parameter::ANGLE:
			pEdit->GetWindowText(txt);
			if (1==sscanf(txt, "%f", &theReal)) {
				m_parameter->friend_setReal(theReal*PI/180);
			} else {
				pEdit->SetFocus();
				::MessageBeep(MB_ICONEXCLAMATION);
				return;
			}
			break;

		case Parameter::BOOLEAN:
		case Parameter::COMPARISON:
			m_parameter->friend_setInt(pList->GetCurSel());
			break;
		case Parameter::KIND_OF_PARAM:
			m_parameter->friend_setInt(pList->GetCurSel()+KINDOF_FIRST);
			break;

		case Parameter::RELATION:
			m_parameter->friend_setInt(pList->GetCurSel() + Parameter::REL_ENEMY);
			break;
		case Parameter::AI_MOOD:
			m_parameter->friend_setInt(pList->GetCurSel() + ATTITUDE_SLEEP);
			break;
		case Parameter::SKIRMISH_WAYPOINT_PATH:	{
			CString cstr;			 
			pList->GetLBText(pList->GetCurSel(), cstr);
			m_parameter->friend_setString((LPCTSTR)cstr);
			break;
		}
		case Parameter::RADAR_EVENT_TYPE:
			m_parameter->friend_setInt(pList->GetCurSel() + RADAR_EVENT_CONSTRUCTION);
			break;
		case Parameter::LOCALIZED_TEXT:
			pCombo->GetWindowText(txt);
			comboText = AsciiString(txt);
			m_parameter->friend_setString(loadLocalizedText(NULL, comboText));
			break;
		case Parameter::BOUNDARY:
		{	
			Int curSel = pCombo->GetCurSel();
			if (curSel >= 0) {
				m_parameter->friend_setInt(curSel);
			}
			break;
		}

		case Parameter::BUILDABLE:
			m_parameter->friend_setInt(pList->GetCurSel()+BSTATUS_YES);
			break;

		case Parameter::SURFACES_ALLOWED:
			m_parameter->friend_setInt(pList->GetCurSel() + 1);
			break;

		case Parameter::SHAKE_INTENSITY:
			m_parameter->friend_setInt(pList->GetCurSel());
			break;

		case Parameter::OBJECT_STATUS:
		{
			Int curSel = pList->GetCurSel();
			if( curSel >= 0 ) 
			{
				m_parameter->friend_setString( ObjectStatusMaskType::getBitNames()[curSel] );
			} 
			else 
			{
				m_parameter->friend_setString( AsciiString::TheEmptyString );
			}
			break;
		}

		case Parameter::FACTION_NAME:
		{
			Int curSel = pList->GetCurSel();
			if (curSel >= 0) {
				CString str;
				pList->GetWindowText(str);
				m_parameter->friend_setString(str.GetBuffer(0));
			}
			break;
		}

		case Parameter::OBJECT_PANEL_FLAG:
		{
			CString str;
			pCombo->GetWindowText(str);
			m_parameter->friend_setString(str.GetBuffer(0));
			break;
		}
	}
	CDialog::OnOK();
}

void EditParameter::OnCancel() 
{

	CDialog::OnCancel();
}

/* This function handles a left click on
   the "preview sound" button */
void EditParameter::OnPreviewSound() 
{
	CString txt;
	AsciiString comboText;

	//only execute if the script in the combo box deals with sound
	if (m_parameter->getParameterType() == Parameter::SOUND ||
		  m_parameter->getParameterType() == Parameter::DIALOG ||
		  m_parameter->getParameterType() == Parameter::MUSIC) {
		CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_COMBO);
		pCombo->GetWindowText(txt);
		comboText = AsciiString(txt);

		//acquire and play song based on the string in the combo box
		AudioEventRTS event;
		event.setEventName(comboText);
		event.setAudioEventInfo(TheAudio->findAudioEventInfo(comboText));
		event.generateFilename();
		
		if (!event.getFilename().isEmpty()) {
			PlaySound(event.getFilename().str(), NULL, SND_ASYNC | SND_FILENAME | SND_PURGE);
		}
	}
}

AsciiString EditParameter::loadLocalizedText(CComboBox *pCombo, AsciiString isStringInTable)
{
	static const AsciiString theScriptPrefix = "SCRIPT";
	AsciiStringVec vec = TheGameText->getStringsWithLabelPrefix(theScriptPrefix);
	if (pCombo) {
		pCombo->Clear();
		for (int i = 0; i < vec.size(); ++i) {
			pCombo->AddString(vec[i].str());
		}
	}
	
	if (isStringInTable != AsciiString::TheEmptyString) {
		for (int i = 0; i < vec.size(); ++i) {
			if (isStringInTable.compare(vec[i].str()) == 0) {
				return vec[i];
			}
		}
	}

	return AsciiString::TheEmptyString;
}
