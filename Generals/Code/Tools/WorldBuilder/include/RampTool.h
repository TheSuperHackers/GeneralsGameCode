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

// FILE: RampTool.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  RampTool.h                                                    */
/* Created:    John K. McDonald, Jr., 4/19/2002                              */
/* Desc:       // The ramp tool for the developers.                          */
/* Revision History:                                                         */
/*		4/19/2002 : Initial creation																					 */
/*---------------------------------------------------------------------------*/

#pragma once
#ifndef _H_RAMPTOOL_
#define _H_RAMPTOOL_

// INCLUDES ///////////////////////////////////////////////////////////////////
#include "Tool.h"
// DEFINES ////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

class RampTool : public Tool
{
	Coord3D mStartPoint;
	Coord3D mEndPoint;
	Coord3D mCurvePoint;

	Bool mIsMouseDown;

	public:
		RampTool();
		virtual void activate();
		virtual void deactivate(); ///< Become not the current tool.

		virtual Bool followsTerrain(void);

		virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
		virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
		virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);

	protected:
		void drawFeedback(Coord3D* endPoint);
		void applyRamp(CWorldBuilderDoc* pDoc);
};


#endif /* _H_RAMPTOOL_ */
