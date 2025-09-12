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

// FILE: W3DInGameUI.h ////////////////////////////////////////////////////////
//
// W3D implementation of the in game user interface, the in game ui is
// responsible for
//
// Author: Colin Day, April 2001
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DINGAMEUI_H_
#define __W3DINGAMEUI_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/InGameUI.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/W3DView.h"
#include "WW3D2/render2d.h"
#include "WW3D2/rendobj.h"
#include "WW3D2/line3d.h"

class HAnimClass;

// W3DInGameUI ----------------------------------------------------------------
/** Implementation for the W3D game user interface.  This singleton is
  * responsible for all user interaction and feedback with as part of the
	* user interface */
//-----------------------------------------------------------------------------
class W3DInGameUI : public InGameUI
{

public:

	W3DInGameUI();
	virtual ~W3DInGameUI();

	// Inherited from subsystem interface -----------------------------------------------------------
	virtual	void init( void );		///< Initialize the in-game user interface
	virtual void update( void );	//< Update the UI by calling preDraw(), draw(), and postDraw()
	virtual void reset( void );		///< Reset
	//-----------------------------------------------------------------------------------------------

	virtual void draw( void ); ///< Render the in-game user interface

protected:

	/// factory for views
	virtual View *createView( void ) { return NEW W3DView; }

	virtual void drawSelectionRegion( void );			///< draw the selection region on screen
	virtual void drawMoveHints( View *view );			///< draw move hint visual feedback
	virtual void drawAttackHints( View *view );		///< draw attack hint visual feedback
	virtual void drawPlaceAngle( View *view ); 		///< draw place building angle if needed

	RenderObjClass *m_moveHintRenderObj[ MAX_MOVE_HINTS ];
	HAnimClass		 *m_moveHintAnim[ MAX_MOVE_HINTS ];
	RenderObjClass *m_buildingPlacementAnchor;
	RenderObjClass *m_buildingPlacementArrow;

};

#endif  // end __W3DINGAMEUI_H_
