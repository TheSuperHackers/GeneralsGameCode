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

// FILE: CrushDie.h /////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   Default die module
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __CrushDie_H_
#define __CrushDie_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/AudioEventRTS.h"
#include "Common/INI.h"

#include "GameLogic/Module/DieModule.h"


// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

enum CrushEnum CPP_11(: Int)
{
	TOTAL_CRUSH,
	BACK_END_CRUSH,
	FRONT_END_CRUSH,
	NO_CRUSH,

	CRUSH_COUNT
};

//-------------------------------------------------------------------------------------------------
class CrushDieModuleData : public DieModuleData
{
public:
	AudioEventRTS		m_crushSounds[CRUSH_COUNT];
	Int							m_crushSoundPercent[CRUSH_COUNT];

	CrushDieModuleData()
	{
		for (int i = 0; i < CRUSH_COUNT; i++)
		{
			m_crushSoundPercent[i] = 100;
		}
	}

	static void buildFieldParse(MultiIniFieldParse& p) 
	{
    DieModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] = 
		{
			{ "TotalCrushSound",					INI::parseAudioEventRTS,			NULL, offsetof( CrushDieModuleData, m_crushSounds[TOTAL_CRUSH] ) },
			{ "BackEndCrushSound",				INI::parseAudioEventRTS,			NULL, offsetof( CrushDieModuleData, m_crushSounds[BACK_END_CRUSH] ) },
			{ "FrontEndCrushSound",				INI::parseAudioEventRTS,			NULL, offsetof( CrushDieModuleData, m_crushSounds[FRONT_END_CRUSH] ) },
			{ "TotalCrushSoundPercent",		INI::parseInt,						NULL, offsetof( CrushDieModuleData, m_crushSoundPercent[TOTAL_CRUSH] ) },
			{ "BackEndCrushSoundPercent",	INI::parseInt,						NULL, offsetof( CrushDieModuleData, m_crushSoundPercent[BACK_END_CRUSH] ) },
			{ "FrontEndCrushSoundPercent",INI::parseInt,						NULL, offsetof( CrushDieModuleData, m_crushSoundPercent[FRONT_END_CRUSH] ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
class CrushDie : public DieModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( CrushDie, "CrushDie" )
	
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( CrushDie, CrushDieModuleData );

public:

	CrushDie( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onDie( const DamageInfo *damageInfo ); 
	
};

#endif // __CrushDie_H_

