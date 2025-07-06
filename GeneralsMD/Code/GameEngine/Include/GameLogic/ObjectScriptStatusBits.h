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

// ObjectScriptStatusBits.h ///////////////////////////////////////////////////////////////////////
// Part of header detangling
// JKMCD Aug 2002

#pragma once
#ifndef __OBJECTSCRIPTSTATUSBITS_H__
#define __OBJECTSCRIPTSTATUSBITS_H__

//-------------------------------------------------------------------------------------------------
/** Object status bits */
//-------------------------------------------------------------------------------------------------
enum ObjectScriptStatusBit CPP_11(: Int)
{
	OBJECT_STATUS_SCRIPT_DISABLED						= 0x01,		///< this object is disabled via script
	OBJECT_STATUS_SCRIPT_UNPOWERED					= 0x02,		///< this object is unpowered via script
	OBJECT_STATUS_SCRIPT_UNSELLABLE					= 0x04,		///< this object is unsellable
	OBJECT_STATUS_SCRIPT_UNSTEALTHED				= 0x08,		///< this object can't stealth.
	OBJECT_STATUS_SCRIPT_TARGETABLE					= 0x10,  ///< This unit can be targeted by the player, but not autoacquired.
	// NOTE: Object currently only uses a Byte for this, so if you add status bits, you may need to enlarge that field.
};

#endif /* __OBJECTSCRIPTSTATUSBITS_H__ */

