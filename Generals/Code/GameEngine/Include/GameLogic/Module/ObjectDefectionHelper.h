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

// FILE: ObjectDefectionHelper.h //////////////////////////////////////////////////////////////////
// Author: Steven Johnson, Colin Day - September 202
// Desc:   Object helpder - defection
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/ObjectHelper.h"

//-------------------------------------------------------------------------------------------------
#define DEFECTION_DETECTION_TIME_MAX (LOGICFRAMES_PER_SECOND * 10)

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class ObjectDefectionHelperModuleData : public ModuleData
{

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class ObjectDefectionHelper : public ObjectHelper
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ObjectDefectionHelper, ObjectDefectionHelperModuleData )
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ObjectDefectionHelper, "ObjectDefectionHelperPool" )

private:

	UnsignedInt   m_defectionDetectionStart;				///< this is the timer, mentioned above (absolute frame, NOT counter)
	UnsignedInt   m_defectionDetectionEnd;					///< this is the timer, mentioned above (absolute frame, NOT counter)
	Real          m_defectionDetectionFlashPhase;   ///< keeps track of the flashing rate logarithmic curve
	Bool					m_doDefectorFX;	///<AmericaInfPilot uses defect to become temporarily "invulnerable"

public:

	ObjectDefectionHelper( Thing *thing, const ModuleData *modData ) : ObjectHelper( thing, modData )
	{
		m_defectionDetectionEnd = 0;
		m_defectionDetectionFlashPhase = FALSE;
		m_defectionDetectionStart = 0;
		m_doDefectorFX = FALSE;
	}
	// virtual destructor prototype provided by memory pool object

	virtual UpdateSleepTime update();

	// Disabled conditions to process -- defection helper must ignore all disabled types.
	virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL; }

	// specific to this class.
	void startDefectionTimer(UnsignedInt numFrames, Bool withDefectorFX = TRUE);

};
