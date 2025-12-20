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

// FILE: DynamicShroudClearingRangeUpdate.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, August 2002
// Desc:   Changes the Objects shroud clearing range
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
#include "GameClient/RadiusDecal.h"///< For the pseudo-wireframe decal effect


#define GRID_FX_DECAL_COUNT (30)

//-------------------------------------------------------------------------------------------------
class DynamicShroudClearingRangeUpdateModuleData : public UpdateModuleData
{
public:
	UnsignedInt m_shrinkDelay;	///< wait until
	UnsignedInt m_shrinkTime;		///< then shrink this fast
	UnsignedInt m_growDelay;    ///< wait until
	UnsignedInt m_growTime;     ///< then grow this fast

	Real m_finalVision;										///< Then change to this
	UnsignedInt m_changeInterval;			///< And update my Object every this long
	UnsignedInt m_growInterval;				///< Update evey this long while growing
	Bool m_doSpySatFX;										///< Do I do the pseudo-wireframe decal and blip effects?
	RadiusDecalTemplate m_gridDecalTemplate;///< For the pseudo-wireframe decal effect


	DynamicShroudClearingRangeUpdateModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DynamicShroudClearingRangeUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DynamicShroudClearingRangeUpdate, "DynamicShroudClearingRangeUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( DynamicShroudClearingRangeUpdate, DynamicShroudClearingRangeUpdateModuleData )

public:

	DynamicShroudClearingRangeUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

	void createGridDecals( const RadiusDecalTemplate& tmpl, Real radius, const Coord3D& pos );
	void killGridDecals( void );
	void animateGridDecals( void );


protected:



	enum DSCRU_STATE
	{
		DSCRU_NOT_STARTED_YET,
		DSCRU_GROWING,
		DSCRU_SUSTAINING,
		DSCRU_SHRINKING,
		DSCRU_DONE_FOREVER,
		DSCRU_SLEEPING
	};

	DSCRU_STATE m_state;

//	Bool m_shrinkFinished;								///< Nothing left to do
//	Bool m_shrinkStarted;									///< Working it


//	UnsignedInt m_shrinkStartCountdown;		///< When I start to shrink
//	UnsignedInt m_shrinkDeadline;					///< When I am done

	//New Timer Parameters

	Int m_stateCountDown;
	Int m_totalFrames;
	UnsignedInt m_growStartDeadline;
	UnsignedInt m_sustainDeadline;
	UnsignedInt m_shrinkStartDeadline;
	UnsignedInt m_doneForeverFrame; ///< Just in case interval and state timing goes awry
																	///< This supercedes and makes us quit


	UnsignedInt m_changeIntervalCountdown;///< How long till I change my vision range again



	Bool m_decalsCreated;											///< Have I created the fx decals yet?
	Real m_visionChangePerInterval;						///< How much I change each time.
	Real m_nativeClearingRange;										///< What is my objects native vision range?
	Real m_currentClearingRange;							///<ToKeepTrackOfWhere We are at

	RadiusDecal m_gridDecal[GRID_FX_DECAL_COUNT];///< For the pseudo-wireframe decal effect

};
