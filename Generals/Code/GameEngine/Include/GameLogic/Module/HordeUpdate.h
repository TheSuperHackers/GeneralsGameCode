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

// FILE: HordeUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, Feb 2002
// Desc:   Horde update module
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
#include "Common/KindOf.h"

class SimpleObjectIterator;
class UpgradeTemplate;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

enum HordeActionType CPP_11(: Int)
{
	HORDEACTION_HORDE = 0,

	HORDEACTION_COUNT
};

#ifdef DEFINE_HORDEACTION_NAMES
static const char *const TheHordeActionTypeNames[] =
{
	"HORDE",
	NULL
};
static_assert(ARRAY_SIZE(TheHordeActionTypeNames) == HORDEACTION_COUNT + 1, "Incorrect array size");
#endif

//-------------------------------------------------------------------------------------------------
class HordeUpdateModuleData : public ModuleData
{
public:
  UnsignedInt								m_updateRate;   ///< how often to recheck our horde status
	KindOfMaskType						m_kindof;				///< the kind(s) of units that count towards horde-ness
	Int												m_minCount;		  ///< min count to get "horde" status
  Real											m_minDist;      ///< min dist to contribute to horde-ness
	Bool											m_alliesOnly;		///< if true, only allied units count towards hordeness
	Bool											m_exactMatch;		///< if true, only exact same type of units count towards hordeness
	Real											m_rubOffRadius;///< If I am this close to another guy who is a true hordesman, it'll rub off on me
	HordeActionType						m_action;				///< what to do if we get horde-ness
	Bool											m_allowedNationalism; ///< Nationalism is hard ocded.  Yeah!  Add to the goodness with this flag instead of rewriting after Alpha.
	std::vector<AsciiString>	m_flagSubObjNames;		///< name(s) of the flag subobj

	HordeUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

// ------------------------------------------------------------------------------------------------
class HordeUpdateInterface
{
public:
	virtual Bool isInHorde() const = 0;
	virtual Bool hasFlag() const = 0;
	virtual Bool isTrueHordeMember() const = 0;
	virtual Bool isAllowedNationalism() const = 0;

};

//-------------------------------------------------------------------------------------------------
class HordeUpdate : public UpdateModule, public HordeUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( HordeUpdate, "HordeUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( HordeUpdate, HordeUpdateModuleData )

public:
	HordeUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	HordeUpdateInterface *getHordeUpdateInterface() { return this; }

	virtual void onDrawableBoundToObject();
	virtual Bool isInHorde() const { return m_inHorde; }
	virtual Bool isTrueHordeMember() const { return m_trueHordeMember && m_inHorde; }
	virtual Bool isAllowedNationalism() const;
	virtual Bool hasFlag() const { return m_hasFlag; }
	virtual UpdateSleepTime update();	///< update this object's AI

protected:

	void showHideFlag(Bool show);
	void joinOrLeaveHorde(SimpleObjectIterator *iter, Bool join);

private:
	UnsignedInt m_lastHordeRefreshFrame; //Just like it sounds
	Bool				m_inHorde;				 //I amy be a trueMember, or I may merely inherit hordehood from a neighbor who is
	Bool				m_trueHordeMember; //meaning, I have enough hordesman near me to qualify
	Bool				m_hasFlag;

};
