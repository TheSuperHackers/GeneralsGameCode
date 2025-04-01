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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Library/wwmouse.h                            $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/11/97 10:11a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef WW_MOUSE_H
#define WW_MOUSE_H

#include	"win.h"
#include	"xmouse.h"
#include    <vfw.h>

class BSurface;

/*
**	Handles the mouse as it relates to the C&C game engine. It is expected that only
**	one object of this type will be created during the lifetime of the game.
*/
class WWMouseClass : public Mouse {
	public:
		/*
		**	Private constructor.
		*/
		WWMouseClass(Surface * surfaceptr, HWND window);
		virtual ~WWMouseClass(void);

		/*
		**	Maintenance callback routine.
		*/
		void Process_Mouse(void);

		/*
		**	Sets the game-drawn mouse imagery.
		*/
		virtual void Set_Cursor(int xhotspot, int yhotspot, ShapeSet const * cursor, int shape);

		/*
		**	Controls visibility of the game-drawn mouse.
		*/
		virtual void Hide_Mouse(void);
		virtual void Show_Mouse(void);

		/*
		**	Takes control of and releases control of the mouse with
		**	respect to the operating system. The mouse must be released
		**	during operations with the operating system. When the mouse is
		**	relased, it may move outside of the confining rectangle and its
		**	shape is controlled by the operating sytem.
		*/
		virtual void Release_Mouse(void);
		virtual void Capture_Mouse(void);
		virtual bool Is_Captured(void) const {return(IsCaptured);}

		/*
		**	Hide the mouse if it falls within this game screen region.
		*/
		virtual void Conditional_Hide_Mouse(Rect region);
		virtual void Conditional_Show_Mouse(void);

		/*
		**	Query about the mouse visiblity state and location.
		*/
		virtual int Get_Mouse_State(void) const;
		virtual int Get_Mouse_X(void) const {return(MouseX);}
		virtual int Get_Mouse_Y(void) const {return(MouseY);}

		/*
		**	Set the mouse location.
		*/
		virtual void Set_Mouse_XY( int xpos, int ypos );

		/*
		** The following two routines can be used to render the mouse onto an alternate
		**	surface.
		*/
		virtual void Draw_Mouse(Surface * scr, bool issidebarsurface = false);
		virtual void Erase_Mouse(Surface * scr, bool issidebarsurface = false);
		//virtual void Draw_Mouse(Surface * scr);
		//virtual void Erase_Mouse(Surface * scr);

		/*
		**	Converts O/S screen coordinates into game coordinates.
		*/
		virtual void Convert_Coordinate(int & x, int & y) const;

		/*
		**	Recalculate the confining rectangle from the window.
		*/
		void Calc_Confining_Rect(void);

	private:

		/*
		**	Used to prevent conflict between the processing callback and
		**	the normal mouse processing routines. The only potential conflict
		**	would be with the maintenance callback routine. Since this callback
		**	and the mouse class maintain a strict master/slave relationship, a
		**	simple critial section flag is all that is needed.
		*/
		long Blocked;

		/*
		**	Mouse hide/show state. If zero or greater, the mouse is visible. Otherwise
		**	it is invisible.
		*/
		long MouseState;

		/*
		**	If the mouse is being managed by this class (for the game), then this flag
		**	will be true. When the mouse has been released to be managed by the operating
		**	system, this flag will be false. However, this class will still track the mouse
		**	position.
		*/
		bool IsCaptured;

		/*
		**	This is the last recorded mouse position that it was drawn to.
		*/
		int MouseX;
		int MouseY;

		/*
		**	Points to the main display surface that the mouse will be drawn
		**	to as it moves.
		*/
		Surface * SurfacePtr;

		/*
		**	This is the window handle that is used to bind and bias the mouse
		**	position and drawing procedures.
		*/
		HWND Window;

		/*
		**	This specifies the rectangle that the game oriented mouse will be
		**	confined to on the visible surface. If the mouse is manually drawn
		**	on another surface, then this rectangle cooresponds to the hidden
		**	surface area where the mouse is to be drawn.
		*/
		Rect ConfiningRect;

		/*
		**	This specifies the mouse shape data. It records the shape set
		**	data as well as the particular image contained within.
		*/
		ShapeSet const * MouseShape;
		int ShapeNumber;

		/*
		**	Specifies the hot spot where the image click maps to. This is an
		**	offset from the upper left corner of the shape image.
		*/
		int MouseXHot;
		int MouseYHot;

		/*
		**	Holds the background image behind the mouse to be used for
		**	restoring the surface pixels.
		*/
		BSurface * Background;
		Rect SavedRegion;

		/*
		**	This is the alternate mouse background buffer to be used when the
		**	mouse is manually drawn to an alternate surface by the Draw_Mouse()
		**	function.
		*/
		BSurface * Alternate;
		Rect AltRegion;

		/*
		** This is another alternate buffer for drawing the mouse pointer across a second, adjoining
		** offscreen buffer.
		*/
		BSurface * SidebarAlternate;
		Rect SidebarAltRegion;

		/*
		**	Conditional hide mouse bounding rectangle and mouse state
		**	flag.
		*/
		Rect ConditionalRect;
		int ConditionalState;

		/*
		**	Maintenance timer handle.
		*/
		MMRESULT TimerHandle;

		// Determines if there is valid mouse shape data available.
		bool Is_Data_Valid(void) const;
		bool Validate_Copy_Buffer(void);

		void Save_Background(void);
		void Restore_Background(void);

		Rect Matching_Rect(void) const;
		void Raw_Draw_Mouse(Surface * surface, int xoffset, int yoffset);
		void Get_Bounded_Position(int & x, int & y) const;
		void Update_Mouse_Position(int x, int y);

		void Low_Show_Mouse(void);
		void Low_Hide_Mouse(void);

		void Block_Mouse(void) {InterlockedIncrement(&Blocked);/*Blocked++;*/}
		void Unblock_Mouse(void) {InterlockedDecrement(&Blocked);/*Blocked--;*/}
		bool Is_Blocked(void) const {return(Blocked != 0);}

		bool Is_Hidden(void) const {return(MouseState < 0);}
};

#endif
