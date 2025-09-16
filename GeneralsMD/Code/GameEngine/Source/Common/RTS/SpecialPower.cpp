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

// FILE: SpecialPower.cpp /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2002
// Desc:   Special power templates and the system that holds them
// Edited: Kris Morness -- July 2002 (added BitFlag system)
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Player.h"
#include "Common/Science.h"
#include "Common/SpecialPower.h"
#include "GameLogic/Object.h"
#include "Common/BitFlagsIO.h"


// GLOBAL /////////////////////////////////////////////////////////////////////////////////////////
SpecialPowerStore *TheSpecialPowerStore = NULL;

#define DEFAULT_DEFECTION_DETECTION_PROTECTION_TIME_LIMIT (LOGICFRAMES_PER_SECOND * 10)

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Externs ////////////////////////////////////////////////////////////////////////////////////////
template<>
const char* SpecialPowerMaskType::s_bitNameList[] =
{
	"SPECIAL_INVALID",

	//Superweapons
	"SPECIAL_DAISY_CUTTER",
	"SPECIAL_PARADROP_AMERICA",
	"SPECIAL_CARPET_BOMB",
	"SPECIAL_CLUSTER_MINES",
	"SPECIAL_EMP_PULSE",
	"SPECIAL_NAPALM_STRIKE",
	"SPECIAL_CASH_HACK",
	"SPECIAL_NEUTRON_MISSILE",
	"SPECIAL_SPY_SATELLITE",
	"SPECIAL_DEFECTOR",
	"SPECIAL_TERROR_CELL",
	"SPECIAL_AMBUSH",
	"SPECIAL_BLACK_MARKET_NUKE",
	"SPECIAL_ANTHRAX_BOMB",
	"SPECIAL_SCUD_STORM",
#ifdef ALLOW_DEMORALIZE
	"SPECIAL_DEMORALIZE",
#else
	"SPECIAL_DEMORALIZE_OBSOLETE",
#endif
	"SPECIAL_CRATE_DROP",
	"SPECIAL_A10_THUNDERBOLT_STRIKE",
	"SPECIAL_DETONATE_DIRTY_NUKE",
	"SPECIAL_ARTILLERY_BARRAGE",

	//Special abilities
	"SPECIAL_MISSILE_DEFENDER_LASER_GUIDED_MISSILES",
	"SPECIAL_REMOTE_CHARGES",
	"SPECIAL_TIMED_CHARGES",
	"SPECIAL_HELIX_NAPALM_BOMB",
	"SPECIAL_HACKER_DISABLE_BUILDING",
	"SPECIAL_TANKHUNTER_TNT_ATTACK",
	"SPECIAL_BLACKLOTUS_CAPTURE_BUILDING",
	"SPECIAL_BLACKLOTUS_DISABLE_VEHICLE_HACK",
	"SPECIAL_BLACKLOTUS_STEAL_CASH_HACK",
	"SPECIAL_INFANTRY_CAPTURE_BUILDING",
	"SPECIAL_RADAR_VAN_SCAN",
	"SPECIAL_SPY_DRONE",
	"SPECIAL_DISGUISE_AS_VEHICLE",
	"SPECIAL_BOOBY_TRAP",
	"SPECIAL_REPAIR_VEHICLES",
	"SPECIAL_PARTICLE_UPLINK_CANNON",
	"SPECIAL_CASH_BOUNTY",
	"SPECIAL_CHANGE_BATTLE_PLANS",
	"SPECIAL_CIA_INTELLIGENCE",
	"SPECIAL_CLEANUP_AREA",
	"SPECIAL_LAUNCH_BAIKONUR_ROCKET",
  "SPECIAL_SPECTRE_GUNSHIP",
  "SPECIAL_GPS_SCRAMBLER",
	"SPECIAL_FRENZY",
	"SPECIAL_SNEAK_ATTACK",

	"SPECIAL_CHINA_CARPET_BOMB",
	"EARLY_SPECIAL_CHINA_CARPET_BOMB",
	"SPECIAL_LEAFLET_DROP",
	"EARLY_SPECIAL_LEAFLET_DROP",
	"EARLY_SPECIAL_FRENZY",
	"SPECIAL_COMMUNICATIONS_DOWNLOAD",
	"EARLY_SPECIAL_REPAIR_VEHICLES",
	"SPECIAL_TANK_PARADROP",
	"SUPW_SPECIAL_PARTICLE_UPLINK_CANNON",
	"AIRF_SPECIAL_DAISY_CUTTER",
	"NUKE_SPECIAL_CLUSTER_MINES",
	"NUKE_SPECIAL_NEUTRON_MISSILE",
	"AIRF_SPECIAL_A10_THUNDERBOLT_STRIKE",
	"AIRF_SPECIAL_SPECTRE_GUNSHIP",
	"INFA_SPECIAL_PARADROP_AMERICA",
	"SLTH_SPECIAL_GPS_SCRAMBLER",
	"AIRF_SPECIAL_CARPET_BOMB",
	"SUPR_SPECIAL_CRUISE_MISSILE",
	"LAZR_SPECIAL_PARTICLE_UPLINK_CANNON",
	"SUPW_SPECIAL_NEUTRON_MISSILE",

	"SPECIAL_BATTLESHIP_BOMBARDMENT",

	NULL
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerStore::parseSpecialPowerDefinition( INI *ini )
{
	// read the name
	AsciiString name = ini->getNextToken();

	SpecialPowerTemplate* specialPower = TheSpecialPowerStore->findSpecialPowerTemplatePrivate( name );

	if ( ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES )
	{
		if (specialPower)
		{
			SpecialPowerTemplate* child = (SpecialPowerTemplate*)specialPower->friend_getFinalOverride();
			specialPower = newInstance(SpecialPowerTemplate);
			*specialPower = *child;
			child->setNextOverride(specialPower);
			specialPower->markAsOverride();
			//TheSpecialPowerStore->m_specialPowerTemplates.push_back(specialPower); // nope, do NOT do this
		}
		else
		{
			specialPower = newInstance(SpecialPowerTemplate);
			const SpecialPowerTemplate *defaultTemplate = TheSpecialPowerStore->findSpecialPowerTemplate( "DefaultSpecialPower" );
			if( defaultTemplate )
				*specialPower = *defaultTemplate;
			specialPower->friend_setNameAndID(name, ++TheSpecialPowerStore->m_nextSpecialPowerID);
			specialPower->markAsOverride();
			TheSpecialPowerStore->m_specialPowerTemplates.push_back(specialPower);
		}
	}
	else
	{
		if (specialPower)
		{
			throw INI_INVALID_DATA;
		}
		else
		{
			specialPower = newInstance(SpecialPowerTemplate);
			const SpecialPowerTemplate *defaultTemplate = TheSpecialPowerStore->findSpecialPowerTemplate( "DefaultSpecialPower" );
			if( defaultTemplate )
				*specialPower = *defaultTemplate;
			specialPower->friend_setNameAndID(name, ++TheSpecialPowerStore->m_nextSpecialPowerID);
			TheSpecialPowerStore->m_specialPowerTemplates.push_back(specialPower);
		}
	}

	// parse the ini definition
	if (specialPower)
		ini->initFromINI( specialPower, specialPower->getFieldParse() );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/* static */ const FieldParse SpecialPowerTemplate::m_specialPowerFieldParse[] =
{

	{ "ReloadTime",								INI::parseDurationUnsignedInt,		NULL,	offsetof( SpecialPowerTemplate, m_reloadTime ) },
	{ "RequiredScience",					INI::parseScience,								NULL, offsetof( SpecialPowerTemplate, m_requiredScience ) },
	{ "InitiateSound",						INI::parseAudioEventRTS,					NULL,	offsetof( SpecialPowerTemplate, m_initiateSound ) },
	{ "InitiateAtLocationSound",	INI::parseAudioEventRTS,					NULL,	offsetof( SpecialPowerTemplate, m_initiateAtLocationSound ) },
	{ "PublicTimer",							INI::parseBool,										NULL, offsetof( SpecialPowerTemplate, m_publicTimer ) },
	{ "Enum",											INI::parseIndexList,							SpecialPowerMaskType::getBitNames(), offsetof( SpecialPowerTemplate, m_type ) },
	{ "DetectionTime",						INI::parseDurationUnsignedInt,		NULL,	offsetof( SpecialPowerTemplate, m_detectionTime ) },
	{ "SharedSyncedTimer",				INI::parseBool,										NULL, offsetof( SpecialPowerTemplate, m_sharedNSync ) },
	{ "ViewObjectDuration",				INI::parseDurationUnsignedInt,		NULL,	offsetof( SpecialPowerTemplate, m_viewObjectDuration ) },
	{ "ViewObjectRange",					INI::parseReal,										NULL,	offsetof( SpecialPowerTemplate, m_viewObjectRange ) },
	{ "RadiusCursorRadius",				INI::parseReal,										NULL,	offsetof( SpecialPowerTemplate, m_radiusCursorRadius ) },
	{ "ShortcutPower",						INI::parseBool,										NULL, offsetof( SpecialPowerTemplate, m_shortcutPower ) },
	{ "AcademyClassify",					INI::parseIndexList,			TheAcademyClassificationTypeNames, offsetof( SpecialPowerTemplate, m_academyClassificationType ) },
	{ NULL,	NULL, NULL,	0 }

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpecialPowerTemplate::SpecialPowerTemplate()
{
	m_id = 0;
	m_type = SPECIAL_INVALID;
	m_reloadTime = 0;
	m_requiredScience = SCIENCE_INVALID;
	m_publicTimer = FALSE;
	m_detectionTime = DEFAULT_DEFECTION_DETECTION_PROTECTION_TIME_LIMIT;
	m_sharedNSync = FALSE;
	m_viewObjectDuration = 0;
	m_viewObjectRange = 0;
	m_radiusCursorRadius = 0;
	m_shortcutPower = FALSE;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpecialPowerTemplate::~SpecialPowerTemplate()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpecialPowerStore::SpecialPowerStore( void )
{

	m_nextSpecialPowerID = 0;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpecialPowerStore::~SpecialPowerStore( void )
{

	// delete all templates
	for( size_t i = 0; i < m_specialPowerTemplates.size(); ++i )
		deleteInstance(m_specialPowerTemplates[ i ]);

	// erase the list
	m_specialPowerTemplates.clear();

	// set our count to zero
	m_nextSpecialPowerID = 0;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpecialPowerTemplate* SpecialPowerStore::findSpecialPowerTemplatePrivate( AsciiString name )
{

	// search the template list for matching name
	for( size_t i = 0; i < m_specialPowerTemplates.size(); ++i )
		if( m_specialPowerTemplates[ i ]->getName() == name )
			return m_specialPowerTemplates[ i ];

	return NULL;  // not found

}

//-------------------------------------------------------------------------------------------------
/** Find a special power template given unique ID */
//-------------------------------------------------------------------------------------------------
const SpecialPowerTemplate *SpecialPowerStore::findSpecialPowerTemplateByID( UnsignedInt id )
{

	// search the template list for matching name
	for( size_t i = 0; i < m_specialPowerTemplates.size(); ++i )
		if( m_specialPowerTemplates[ i ]->getID() == id )
			return m_specialPowerTemplates[ i ];

	return NULL;  // not found

}

//-------------------------------------------------------------------------------------------------
/** Find a special power template given index (WB) */
//-------------------------------------------------------------------------------------------------
const SpecialPowerTemplate *SpecialPowerStore::getSpecialPowerTemplateByIndex( UnsignedInt index )
{

	if (index >= 0 && index < m_specialPowerTemplates.size())
		return m_specialPowerTemplates[ index ];

	return NULL;  // not found

}

//-------------------------------------------------------------------------------------------------
/** Return the size of the store (WB) */
//-------------------------------------------------------------------------------------------------
Int SpecialPowerStore::getNumSpecialPowers( void )
{

	return m_specialPowerTemplates.size();

}

//-------------------------------------------------------------------------------------------------
/** does the object (and therefore the player) meet all the requirements to use this power */
//-------------------------------------------------------------------------------------------------
Bool SpecialPowerStore::canUseSpecialPower( Object *obj, const SpecialPowerTemplate *specialPowerTemplate )
{

	// sanity
	if( obj == NULL || specialPowerTemplate == NULL )
		return FALSE;

	// as a first sanity check, the object must have a module capable of executing the power
	if( obj->getSpecialPowerModule( specialPowerTemplate ) == NULL )
		return FALSE;

	//
	// in order to execute the special powers we have attached special power modules to the objects
	// that can use them.  However, just because an object has a module that is capable of
	// doing the power, does not mean the object and the player can actually execute the
	// power because some powers require a specialized science that the player must select and
	// they cannot have all of them.
	//

	// check for requried science
	ScienceType requiredScience = specialPowerTemplate->getRequiredScience();
	if( requiredScience != SCIENCE_INVALID )
	{
		Player *player = obj->getControllingPlayer();

		if( player->hasScience( requiredScience ) == FALSE )
			return FALSE;

	}


	// I THINK THIS IS WHERE WE BAIL OUT IF A DIFFERENT CONYARD IS ALREADY CHARGIN THIS SPECIAL RIGHT NOW //LORENZEN


	// all is well
	return TRUE;

}

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void SpecialPowerStore::reset( void )
{
	for (SpecialPowerTemplatePtrVector::iterator it = m_specialPowerTemplates.begin(); it != m_specialPowerTemplates.end(); /*++it*/)
	{
		SpecialPowerTemplate* si = *it;
		Overridable* temp = si->deleteOverrides();
		if (temp == NULL)
		{
			it = m_specialPowerTemplates.erase(it);
		}
		else
		{
			++it;
		}
	}
}
