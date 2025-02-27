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

// FILE: DamageFX.cpp ///////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, November 2001
// Desc:   DamageFX descriptions
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/DamageFX.h"
#include "Common/GameAudio.h"

#include "GameClient/FXList.h"
#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameClient/GameClient.h"
#include "GameClient/InGameUI.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

DamageFXStore *TheDamageFXStore = NULL;					///< the DamageFX store definition

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE CLASSES ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
DamageFX::DamageFX()
{
	// not necessary.
	//clear();
}

//-------------------------------------------------------------------------------------------------
void DamageFX::clear()
{
	for (Int dt = 0; dt < DAMAGE_NUM_TYPES; ++dt)
	{
		for (Int v = LEVEL_FIRST; v <= LEVEL_LAST; ++v)
		{
			m_dfx[dt][v].clear();
		}
	}
}

//-------------------------------------------------------------------------------------------------
UnsignedInt DamageFX::getDamageFXThrottleTime(DamageType t, const Object* source) const
{
	return m_dfx[t][source ? source->getVeterancyLevel() : LEVEL_REGULAR].m_damageFXThrottleTime;
}

//-------------------------------------------------------------------------------------------------
void DamageFX::doDamageFX(DamageType t, Real damageAmount, const Object* source, const Object* victim) const
{
	ConstFXListPtr fx = getDamageFXList(t, damageAmount, source);
	// since the victim is receiving the damage, it's the "primary" object.
	// the source is the "secondary" object -- unused by most fx, but could be
	// useful in some cases.
	FXList::doFXObj(fx, victim, source);
}

//-------------------------------------------------------------------------------------------------
ConstFXListPtr DamageFX::getDamageFXList(DamageType t, Real damageAmount, const Object* source) const
{
	/*
		if damage is zero, never do damage fx. this is by design, since "zero" damage can happen
		with some special weapons, like the battleship, which is a "faux" weapon that never does damage.
		if you really need to change this for some reason, consider carefully... (srj)
	*/
	if (damageAmount == 0.0f)
		return NULL;

	const DFX& dfx = m_dfx[t][source ? source->getVeterancyLevel() : LEVEL_REGULAR];
	ConstFXListPtr fx =
		damageAmount >= dfx.m_amountForMajorFX ?
		dfx.m_majorDamageFXList :
		dfx.m_minorDamageFXList;

	return fx;
}

//-------------------------------------------------------------------------------------------------
const FieldParse* DamageFX::getFieldParse() const
{
	static const FieldParse myFieldParse[] =
	{
		{ "AmountForMajorFX",						parseAmount,			NULL, 0 },
		{ "MajorFX",										parseMajorFXList,	NULL, 0 },
		{ "MinorFX",										parseMinorFXList,	NULL, 0 },
		{ "ThrottleTime",								parseTime,				NULL, 0 },
		{ "VeterancyAmountForMajorFX",	parseAmount,			TheVeterancyNames, 0 },
		{ "VeterancyMajorFX",						parseMajorFXList,	TheVeterancyNames, 0 },
		{ "VeterancyMinorFX",						parseMinorFXList,	TheVeterancyNames, 0 },
		{ "VeterancyThrottleTime",			parseTime,				TheVeterancyNames, 0 },
		{ 0, 0, 0,0 }
	};
	return myFieldParse;
}

//-------------------------------------------------------------------------------------------Static
static void parseCommonStuff(
	INI* ini,
	ConstCharPtrArray names,
	VeterancyLevel& vetFirst,
	VeterancyLevel& vetLast,
	DamageType& damageFirst,
	DamageType& damageLast
)
{
	if (names)
	{
		vetFirst = (VeterancyLevel)INI::scanIndexList(ini->getNextToken(), names);
		vetLast = vetFirst;
	}
	else
	{
		vetFirst = LEVEL_FIRST;
		vetLast = LEVEL_LAST;
	}

	const char* damageName = ini->getNextToken();
	if (stricmp(damageName, "Default") == 0)
	{
		damageFirst = (DamageType)0;
		damageLast = (DamageType)(DAMAGE_NUM_TYPES - 1);
	}
	else
	{
		damageFirst = (DamageType)DamageTypeFlags::getSingleBitFromName(damageName);
		damageLast = damageFirst;
	}
}

//-------------------------------------------------------------------------------------------Static
/*static*/ void DamageFX::parseAmount( INI* ini, void* instance, void* /*store*/, const void* userData )
{
	DamageFX* self = (DamageFX*)instance;
	ConstCharPtrArray names = (ConstCharPtrArray)userData;

	VeterancyLevel vetFirst, vetLast;
	DamageType damageFirst, damageLast;
	parseCommonStuff(ini, names, vetFirst, vetLast, damageFirst, damageLast);

	Real amt = INI::scanReal(ini->getNextToken());

	for (Int dt = damageFirst; dt <= damageLast; ++dt)
	{
		for (Int v = vetFirst; v <= vetLast; ++v)
		{
			self->m_dfx[dt][v].m_amountForMajorFX = amt;
		}
	}
}

//-------------------------------------------------------------------------------------------Static
/*static*/ void DamageFX::parseMajorFXList( INI* ini, void* instance, void* /*store*/, const void* userData )
{
	DamageFX* self = (DamageFX*)instance;
	ConstCharPtrArray names = (ConstCharPtrArray)userData;

	VeterancyLevel vetFirst, vetLast;
	DamageType damageFirst, damageLast;
	parseCommonStuff(ini, names, vetFirst, vetLast, damageFirst, damageLast);

	ConstFXListPtr fx;
	INI::parseFXList(ini, NULL, &fx, NULL);

	for (Int dt = damageFirst; dt <= damageLast; ++dt)
	{
		for (Int v = vetFirst; v <= vetLast; ++v)
		{
			self->m_dfx[dt][v].m_majorDamageFXList = fx;
		}
	}
}

//-------------------------------------------------------------------------------------------Static
/*static*/ void DamageFX::parseMinorFXList( INI* ini, void* instance, void* /*store*/, const void* userData )
{
	DamageFX* self = (DamageFX*)instance;
	ConstCharPtrArray names = (ConstCharPtrArray)userData;

	VeterancyLevel vetFirst, vetLast;
	DamageType damageFirst, damageLast;
	parseCommonStuff(ini, names, vetFirst, vetLast, damageFirst, damageLast);

	ConstFXListPtr fx;
	INI::parseFXList(ini, NULL, &fx, NULL);

	for (Int dt = damageFirst; dt <= damageLast; ++dt)
	{
		for (Int v = vetFirst; v <= vetLast; ++v)
		{
			self->m_dfx[dt][v].m_minorDamageFXList = fx;
		}
	}
}

//-------------------------------------------------------------------------------------------Static
/*static*/ void DamageFX::parseTime( INI* ini, void* instance, void* /*store*/, const void* userData )
{
	DamageFX* self = (DamageFX*)instance;
	ConstCharPtrArray names = (ConstCharPtrArray)userData;

	VeterancyLevel vetFirst, vetLast;
	DamageType damageFirst, damageLast;
	parseCommonStuff(ini, names, vetFirst, vetLast, damageFirst, damageLast);

	UnsignedInt t;
	INI::parseDurationUnsignedInt(ini, NULL, &t, NULL);

	for (Int dt = damageFirst; dt <= damageLast; ++dt)
	{
		for (Int v = vetFirst; v <= vetLast; ++v)
		{
			self->m_dfx[dt][v].m_damageFXThrottleTime = t;
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DamageFXStore::DamageFXStore()
{
	m_dfxmap.clear();
}

//-------------------------------------------------------------------------------------------------
DamageFXStore::~DamageFXStore()
{
	m_dfxmap.clear();
}

//-------------------------------------------------------------------------------------------------
const DamageFX *DamageFXStore::findDamageFX(AsciiString name) const
{
	NameKeyType namekey = TheNameKeyGenerator->nameToKey(name);
  DamageFXMap::const_iterator it = m_dfxmap.find(namekey);
  if (it == m_dfxmap.end())
	{
		return NULL;
	}
	else
	{
		return &(*it).second;
	}
}

//-------------------------------------------------------------------------------------------------
void DamageFXStore::init()
{
}

//-------------------------------------------------------------------------------------------------
void DamageFXStore::reset()
{
}

//-------------------------------------------------------------------------------------------------
void DamageFXStore::update()
{
}

//-------------------------------------------------------------------------------------------------
/*static */ void DamageFXStore::parseDamageFXDefinition(INI* ini)
{

	const char *c = ini->getNextToken();
	NameKeyType key = TheNameKeyGenerator->nameToKey(c);
	DamageFX& dfx = TheDamageFXStore->m_dfxmap[key];
	dfx.clear();
	ini->initFromINI(&dfx, dfx.getFieldParse());
}
