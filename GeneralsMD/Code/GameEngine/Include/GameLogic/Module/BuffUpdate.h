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

// FILE: BuffUpdate.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002-2003 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Oct 25
//
//	Filename: 	BuffUpdate.h
//
//	author:		Andi W
//
//	purpose:	apply a buff/debuff effect to units in an area
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __BUFF_UPDATE_H_
#define __BUFF_UPDATE_H_

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "GameLogic/Module/UpdateModule.h"

//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class BuffUpdateModuleData : public UpdateModuleData
{
public:

	BuffUpdateModuleData();

	KindOfMaskType						m_requiredAffectKindOf;						///< Must be set on target
	KindOfMaskType						m_forbiddenAffectKindOf;	///< Must be clear on target
	Int									m_targetsMask;				///< ALLIES, ENEMIES or NEUTRALS
	Bool								m_isAffectAirborne;					///< Affect Airborne targets
	UnsignedInt								m_buffDuration;					///< How long a hit lasts on target
	UnsignedInt								m_buffDelay;							///< How often to pulse
	Real											m_buffRange;							///< How far to affect
	AsciiString							m_buffTemplateName;			///< Buff type to give

	static void buildFieldParse(MultiIniFieldParse& p);
};


//-------------------------------------------------------------------------------------------------
class BuffUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( BuffUpdate, "BuffUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( BuffUpdate, BuffUpdateModuleData )

public:

	BuffUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update( void );

protected:

};


//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

#endif
