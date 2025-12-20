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

// FILE: SwayClientUpdate.h ////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, May 2002
// Desc:   Tree sway client update module
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/ClientUpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

//-------------------------------------------------------------------------------------------------
/** The tree way client update module */
//-------------------------------------------------------------------------------------------------
class SwayClientUpdate : public ClientUpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SwayClientUpdate, "SwayClientUpdate" )
	MAKE_STANDARD_MODULE_MACRO( SwayClientUpdate );

public:

	SwayClientUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	/// the client update callback
	virtual void clientUpdate( void );

	void stopSway( void ) { m_swaying = false; }

protected:

	Real			m_curValue;
	Real			m_curAngle;
	Real			m_curDelta;
	Real			m_curAngleLimit;
	Real			m_leanAngle;							///<Angle that the tree leans away from the wind.
	Short			m_curVersion;
	Bool			m_swaying;
	Bool			m_unused;

	void updateSway(void);
};
