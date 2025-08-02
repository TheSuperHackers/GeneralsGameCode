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

// FILE: GameCommon.h ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					         
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  GameCommon.h
//
// Created:    Steven Johnson, October 2001
//
// Desc:			 This is a catchall header for some basic types and definitions
//							needed by various bits of the GameLogic/GameClient, but that
//							we haven't found a good place for yet. Hopefully this file
//							should go away someday, but for now is a convenient spot.
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef _GAMECOMMON_H_
#define _GAMECOMMON_H_

// ----------------------------------------------------------------------------------------------
#include "Lib/BaseType.h"
#include "WWCommon.h"
#include "Common/GameDefines.h"

// ----------------------------------------------------------------------------------------------
#if defined(RTS_DEBUG)
	#define NO_DUMP_PERF_STATS
#else
	#define NO_DUMP_PERF_STATS
#endif

// ----------------------------------------------------------------------------------------------
enum
{
	LOGICFRAMES_PER_SECOND = 30,
	MSEC_PER_SECOND = 1000
};
const Real LOGICFRAMES_PER_MSEC_REAL = (((Real)LOGICFRAMES_PER_SECOND) / ((Real)MSEC_PER_SECOND));
const Real MSEC_PER_LOGICFRAME_REAL = (((Real)MSEC_PER_SECOND) / ((Real)LOGICFRAMES_PER_SECOND));
const Real LOGICFRAMES_PER_SECONDS_REAL = (Real)LOGICFRAMES_PER_SECOND;
const Real SECONDS_PER_LOGICFRAME_REAL = 1.0f / LOGICFRAMES_PER_SECONDS_REAL;

// ----------------------------------------------------------------------------------------------
// note that this returns a REAL value, not an int... most callers will want to
// call ceil() on the result, so that partial frames get converted to full frames!
inline Real ConvertDurationFromMsecsToFrames(Real msec) 
{	
	return (msec * LOGICFRAMES_PER_MSEC_REAL); 
}

// ----------------------------------------------------------------------------------------------
inline Real ConvertVelocityInSecsToFrames(Real distPerMsec) 
{	
	// this looks wrong, but is the correct conversion factor.
	return (distPerMsec * SECONDS_PER_LOGICFRAME_REAL); 
}

// ----------------------------------------------------------------------------------------------
inline Real ConvertAccelerationInSecsToFrames(Real distPerSec2) 
{	
	// this looks wrong, but is the correct conversion factor.
	const Real SEC_PER_LOGICFRAME_SQR = (SECONDS_PER_LOGICFRAME_REAL * SECONDS_PER_LOGICFRAME_REAL);
	return (distPerSec2 * SEC_PER_LOGICFRAME_SQR); 
}

// ----------------------------------------------------------------------------------------------
inline Real ConvertAngularVelocityInDegreesPerSecToRadsPerFrame(Real degPerSec) 
{	
	const Real RADS_PER_DEGREE = PI / 180.0f;
	return (degPerSec * (SECONDS_PER_LOGICFRAME_REAL * RADS_PER_DEGREE)); 
}

// ----------------------------------------------------------------------------------------------
enum 
{ 
	MAX_PLAYER_COUNT = 16											///< max number of Players.
};

// ----------------------------------------------------------------------------------------------
/**
	a bitmask that can uniquely represent each player.
*/
#if MAX_PLAYER_COUNT <= 16
	typedef UnsignedShort PlayerMaskType;
	const PlayerMaskType PLAYERMASK_ALL = 0xffff;
	const PlayerMaskType PLAYERMASK_NONE = 0x0;
#else
	#error "this is the wrong size"
#endif

//-------------------------------------------------------------------------------------------------
enum GameDifficulty CPP_11(: Int)
{
	DIFFICULTY_EASY,
	DIFFICULTY_NORMAL,
	DIFFICULTY_HARD,

	DIFFICULTY_COUNT
};

//-------------------------------------------------------------------------------------------------
enum PlayerType CPP_11(: Int)
{
	PLAYER_HUMAN,				///< player is human-controlled
	PLAYER_COMPUTER,		///< player is computer-controlled

	PLAYERTYPE_COUNT
};

//-------------------------------------------------------------------------------------------------
/// A PartitionCell can be one of three states for Shroud
enum CellShroudStatus CPP_11(: Int)
{
	CELLSHROUD_CLEAR,
	CELLSHROUD_FOGGED,
	CELLSHROUD_SHROUDED,

	CELLSHROUD_COUNT
};

//-------------------------------------------------------------------------------------------------
/// Since an object can take up more than a single PartitionCell, this is a status that applies to the whole Object
enum ObjectShroudStatus CPP_11(: Int)
{
	OBJECTSHROUD_INVALID,				///< indeterminate state, will recompute
	OBJECTSHROUD_CLEAR,					///< object is not shrouded at all (ie, completely visible)
	OBJECTSHROUD_PARTIAL_CLEAR,	///< object is partly clear (rest is shroud or fog)
	OBJECTSHROUD_FOGGED,				///< object is completely fogged
	OBJECTSHROUD_SHROUDED,			///< object is completely shrouded
	OBJECTSHROUD_INVALID_BUT_PREVIOUS_VALID,			///< indeterminate state, will recompute, BUT previous status is valid, don't reset (used for save/load)

	OBJECTSHROUD_COUNT
};

//-------------------------------------------------------------------------------------------------
enum GuardMode CPP_11(: Int)
{
	GUARDMODE_NORMAL,
	GUARDMODE_GUARD_WITHOUT_PURSUIT,	// no pursuit out of guard area
	GUARDMODE_GUARD_FLYING_UNITS_ONLY	// ignore nonflyers
};

// ---------------------------------------------------
enum 
{ 
	NEVER				= 0,
	FOREVER			= 0x3fffffff			// (we use 0x3fffffff so that we can add offsets and not overflow...
																//		at 30fps we're still pretty safe!)
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/// Veterancy level define needed by several files that don't need the full Experience code.
// NOTE NOTE NOTE: Keep TheVeterencyNames in sync with these.
enum VeterancyLevel CPP_11(: Int)
{
	LEVEL_REGULAR = 0,
	LEVEL_VETERAN,
	LEVEL_ELITE,
	LEVEL_HEROIC,

	LEVEL_COUNT,
	LEVEL_INVALID,

	LEVEL_FIRST = LEVEL_REGULAR,
	LEVEL_LAST = LEVEL_HEROIC
};

// TheVeterancyNames is defined in GameCommon.cpp
extern const char *TheVeterancyNames[]; 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
enum CommandSourceType CPP_11(: Int) 
{ 

	CMD_FROM_PLAYER = 0, 
	CMD_FROM_SCRIPT, 
	CMD_FROM_AI,
	CMD_FROM_DOZER,							// Special rare command when the dozer originates a command to attack a mine. Mines are not ai-attackable, and it seems deceitful for the dozer to generate a player or script command. jba.

};		///< the source of a command

//-------------------------------------------------------------------------------------------------
enum AbleToAttackType CPP_11(: Int)
{
	_ATTACK_FORCED			= 0x01,
	_ATTACK_CONTINUED		= 0x02,
	_ATTACK_TUNNELNETWORK_GUARD = 0x04,

	/**
		can we attack if this is a new target?
	*/
	ATTACK_NEW_TARGET = (0),

	/**
		can we attack if this is a new target, via force-fire?
		(The only current difference between this and ATTACK_NEW_TARGET is that disguised units
		are force-attackable even when stealthed.)
	*/
	ATTACK_NEW_TARGET_FORCED= (_ATTACK_FORCED),	

	/**
		can we attack if this is continuation of an existing attack?
		(The only current difference between this and ATTACK_NEW_TARGET is you are allowed to follow
		immobile shrouded units into the fog)
	*/
	ATTACK_CONTINUED_TARGET = (_ATTACK_CONTINUED),

	/**
		can we attack if this is continuation of an existing attack?
		(The only current difference between this and ATTACK_NEW_TARGET is you are allowed to follow
		immobile shrouded units into the fog)
	*/
	ATTACK_CONTINUED_TARGET_FORCED = (_ATTACK_FORCED | _ATTACK_CONTINUED),

	/**
		Special case that bypasses some of the checks for units guarding from within tunnel networks!
		For example, a unit inside couldn't normally see outside and would fail.
	*/
	ATTACK_TUNNEL_NETWORK_GUARD = (_ATTACK_TUNNELNETWORK_GUARD)
	
};

//-------------------------------------------------------------------------------------------------
inline Bool isForcedAttack(AbleToAttackType t)
{
	return (((Int)t) & _ATTACK_FORCED) != 0;
}

//-------------------------------------------------------------------------------------------------
inline Bool isContinuedAttack(AbleToAttackType t)
{
	return (((Int)t) & _ATTACK_CONTINUED) != 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

typedef UnsignedInt VeterancyLevelFlags;

const VeterancyLevelFlags VETERANCY_LEVEL_FLAGS_ALL = 0xffffffff;
const VeterancyLevelFlags VETERANCY_LEVEL_FLAGS_NONE = 0x00000000;

inline Bool getVeterancyLevelFlag(VeterancyLevelFlags flags, VeterancyLevel dt)
{
	return (flags & (1UL << (dt - 1))) != 0;
}

inline VeterancyLevelFlags setVeterancyLevelFlag(VeterancyLevelFlags flags, VeterancyLevel dt)
{
	return (flags | (1UL << (dt - 1)));
}

inline VeterancyLevelFlags clearVeterancyLevelFlag(VeterancyLevelFlags flags, VeterancyLevel dt)
{
	return (flags & ~(1UL << (dt - 1)));
}

// ----------------------------------------------------------------------------------------------
#define BOGUSPTR(p) ((((unsigned int)(p)) & 1) != 0)

// ----------------------------------------------------------------------------------------------
#define MAKE_DLINK_HEAD(OBJCLASS, LISTNAME)																						\
public:																																								\
	inline DLINK_ITERATOR<OBJCLASS> iterate_##LISTNAME() const													\
	{																																										\
		DEBUG_ASSERTCRASH(!BOGUSPTR(m_dlinkhead_##LISTNAME.m_head), ("bogus head ptr"));	\
		return DLINK_ITERATOR<OBJCLASS>(m_dlinkhead_##LISTNAME.m_head, &OBJCLASS::dlink_next_##LISTNAME);	\
	}																																										\
	inline OBJCLASS *getFirstItemIn_##LISTNAME() const																	\
	{																																										\
		DEBUG_ASSERTCRASH(!BOGUSPTR(m_dlinkhead_##LISTNAME.m_head), ("bogus head ptr"));	\
		return m_dlinkhead_##LISTNAME.m_head;																							\
	}																																										\
	inline Bool isInList_##LISTNAME(OBJCLASS* o) const																	\
	{																																										\
		DEBUG_ASSERTCRASH(!BOGUSPTR(m_dlinkhead_##LISTNAME.m_head), ("bogus head ptr"));	\
		return o->dlink_isInList_##LISTNAME(&m_dlinkhead_##LISTNAME.m_head);							\
	}																																										\
	inline void prependTo_##LISTNAME(OBJCLASS* o)																				\
	{																																										\
		DEBUG_ASSERTCRASH(!BOGUSPTR(m_dlinkhead_##LISTNAME.m_head), ("bogus head ptr"));	\
		if (!isInList_##LISTNAME(o))																											\
			o->dlink_prependTo_##LISTNAME(&m_dlinkhead_##LISTNAME.m_head);									\
	}																																										\
	inline void removeFrom_##LISTNAME(OBJCLASS* o)																			\
	{																																										\
		DEBUG_ASSERTCRASH(!BOGUSPTR(m_dlinkhead_##LISTNAME.m_head), ("bogus head ptr"));	\
		if (isInList_##LISTNAME(o))																												\
			o->dlink_removeFrom_##LISTNAME(&m_dlinkhead_##LISTNAME.m_head);									\
	}																																										\
	typedef void (*RemoveAllProc_##LISTNAME)(OBJCLASS* o);															\
	inline void removeAll_##LISTNAME(RemoveAllProc_##LISTNAME p = NULL)									\
	{																																										\
		while (m_dlinkhead_##LISTNAME.m_head)																							\
		{																																									\
			DEBUG_ASSERTCRASH(!BOGUSPTR(m_dlinkhead_##LISTNAME.m_head), ("bogus head ptr"));\
			OBJCLASS *tmp = m_dlinkhead_##LISTNAME.m_head;																	\
			removeFrom_##LISTNAME(tmp);																											\
			if (p) (*p)(tmp);																																\
		}																																									\
	}																																										\
	inline void reverse_##LISTNAME()																										\
	{																																										\
		OBJCLASS* cur = m_dlinkhead_##LISTNAME.m_head;																		\
		OBJCLASS* prev = NULL;																														\
		while (cur)																																				\
		{																																									\
			OBJCLASS* originalNext = cur->dlink_next_##LISTNAME();													\
			cur->dlink_swapLinks_##LISTNAME();																							\
			prev = cur;																																			\
			cur = originalNext;																															\
		}																																									\
		m_dlinkhead_##LISTNAME.m_head = prev;																							\
	}																																										\
private:																																							\
	/* a trick: init head to zero */																										\
	struct DLINKHEAD_##LISTNAME																													\
	{																																										\
	public:																																							\
		OBJCLASS* m_head;																																	\
		inline DLINKHEAD_##LISTNAME() :																										\
			m_head(0) { }																																		\
		inline ~DLINKHEAD_##LISTNAME()																										\
			{ DEBUG_ASSERTCRASH(!m_head,("destroying dlinkhead still in a list " #LISTNAME)); }				\
	};																																									\
	DLINKHEAD_##LISTNAME m_dlinkhead_##LISTNAME;	

// ----------------------------------------------------------------------------------------------
#define MAKE_DLINK(OBJCLASS, LISTNAME)	\
public:																	\
	OBJCLASS* dlink_prev_##LISTNAME() const { return m_dlink_##LISTNAME.m_prev; }										\
	OBJCLASS* dlink_next_##LISTNAME() const	{ return m_dlink_##LISTNAME.m_next; }										\
	void dlink_swapLinks_##LISTNAME()																									\
	{																																													\
		OBJCLASS* originalNext = m_dlink_##LISTNAME.m_next;																			\
		m_dlink_##LISTNAME.m_next = m_dlink_##LISTNAME.m_prev;																	\
		m_dlink_##LISTNAME.m_prev = originalNext;																								\
	}																																													\
	Bool dlink_isInList_##LISTNAME(OBJCLASS* const* pListHead) const										\
	{																																													\
		DEBUG_ASSERTCRASH(!BOGUSPTR(*pListHead) && !BOGUSPTR(m_dlink_##LISTNAME.m_next) && !BOGUSPTR(m_dlink_##LISTNAME.m_prev), ("bogus ptrs")); \
		return *pListHead == this || m_dlink_##LISTNAME.m_prev || m_dlink_##LISTNAME.m_next;		\
	}																																													\
	void dlink_prependTo_##LISTNAME(OBJCLASS** pListHead)															\
	{																																													\
		DEBUG_ASSERTCRASH(!dlink_isInList_##LISTNAME(pListHead), ("already in list " #LISTNAME));					\
		DEBUG_ASSERTCRASH(!BOGUSPTR(*pListHead) && !BOGUSPTR(m_dlink_##LISTNAME.m_next) && !BOGUSPTR(m_dlink_##LISTNAME.m_prev), ("bogus ptrs")); \
		m_dlink_##LISTNAME.m_next = *pListHead;																									\
		if (*pListHead)																																					\
			(*pListHead)->m_dlink_##LISTNAME.m_prev = this;																				\
		*pListHead = this;																																			\
		DEBUG_ASSERTCRASH(!BOGUSPTR(*pListHead) && !BOGUSPTR(m_dlink_##LISTNAME.m_next) && !BOGUSPTR(m_dlink_##LISTNAME.m_prev), ("bogus ptrs")); \
	}																																													\
	void dlink_removeFrom_##LISTNAME(OBJCLASS** pListHead)															\
	{																																													\
		DEBUG_ASSERTCRASH(dlink_isInList_##LISTNAME(pListHead), ("not in list" #LISTNAME));			\
		DEBUG_ASSERTCRASH(!BOGUSPTR(*pListHead) && !BOGUSPTR(m_dlink_##LISTNAME.m_next) && !BOGUSPTR(m_dlink_##LISTNAME.m_prev), ("bogus ptrs")); \
		if (m_dlink_##LISTNAME.m_next)																													\
			m_dlink_##LISTNAME.m_next->m_dlink_##LISTNAME.m_prev = m_dlink_##LISTNAME.m_prev;			\
		if (m_dlink_##LISTNAME.m_prev)																													\
			m_dlink_##LISTNAME.m_prev->m_dlink_##LISTNAME.m_next = m_dlink_##LISTNAME.m_next;			\
		else																																										\
			*pListHead = m_dlink_##LISTNAME.m_next;																								\
		m_dlink_##LISTNAME.m_prev = 0;																													\
		m_dlink_##LISTNAME.m_next = 0;																													\
		DEBUG_ASSERTCRASH(!BOGUSPTR(*pListHead) && !BOGUSPTR(m_dlink_##LISTNAME.m_next) && !BOGUSPTR(m_dlink_##LISTNAME.m_prev), ("bogus ptrs")); \
	}																																													\
private:																\
	/* a trick: init links to zero */			\
	struct DLINK_##LISTNAME								\
	{																			\
	public:																\
		OBJCLASS* m_prev;										\
		OBJCLASS* m_next;										\
		inline DLINK_##LISTNAME() :					\
			m_prev(0), m_next(0) { }					\
		inline ~DLINK_##LISTNAME()					\
			{ DEBUG_ASSERTCRASH(!m_prev && !m_next,("destroying dlink still in a list "  #LISTNAME)); } \
	};																		\
	DLINK_##LISTNAME m_dlink_##LISTNAME;	

// ------------------------------------------------------------------------
// this is the weird C++ syntax for "call pointer-to-member-function"... see C++ FAQ LITE for details.
#define callMemberFunction(object,ptrToMember)  ((object).*(ptrToMember)) 

// ------------------------------------------------------------------------
template<class OBJCLASS>
class DLINK_ITERATOR
{
public:
	// this is the weird C++ syntax for "pointer-to-member-function"
	typedef OBJCLASS* (OBJCLASS::*GetNextFunc)() const;	
private:
	OBJCLASS* m_cur;
	GetNextFunc m_getNextFunc;	// this is the weird C++ syntax for "pointer-to-member-function"
public:
	DLINK_ITERATOR(OBJCLASS* cur, GetNextFunc getNextFunc) : m_cur(cur), m_getNextFunc(getNextFunc) 
	{ 
	}
	
	void advance()
	{ 
		if (m_cur)
			m_cur = callMemberFunction(*m_cur, m_getNextFunc)();
	}
	
	Bool done() const
	{ 
		return m_cur == NULL; 
	}

	OBJCLASS* cur() const 
	{ 
		return m_cur; 
	}

};

// ------------------------------------------------------------------------

enum WhichTurretType CPP_11(: Int)
{
	TURRET_INVALID = -1,

	TURRET_MAIN = 0,
	TURRET_ALT,

	MAX_TURRETS
};

// ------------------------------------------------------------------------
// this normalizes an angle to the range -PI...PI.
extern Real normalizeAngle(Real angle);

// ------------------------------------------------------------------------
// this returns the difference between a1 and a2, normalized.
inline Real stdAngleDiff(Real a1, Real a2)
{
	return normalizeAngle(a1 - a2);
}

// ------------------------------------------------------------------------
// NOTE NOTE NOTE: Keep TheRelationShipNames in sync with this enum
enum Relationship CPP_11(: Int)
{
	ENEMIES = 0,
	NEUTRAL,
	ALLIES
};


// TheRelationShipNames is defined in Common/GameCommon.cpp
extern const char *TheRelationshipNames[];

#endif // _GAMECOMMON_H_

