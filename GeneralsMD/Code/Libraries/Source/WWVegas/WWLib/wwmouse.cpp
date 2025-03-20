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
 *                     $Archive:: /Commando/Code/Library/wwmouse.cpp                          $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/11/97 10:14a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Callback_Process_Mouse -- Mouse O/S callback function.                                    *
 *   WWMouseClass::WWMouseClass -- Constructor for mouse handler object.                       *
 *   WWMouseClass::~WWMouseClass -- Destructor for mouse handler object.                       *
 *   WWMouseClass::Get_Mouse_State -- Fetch the current mouse visibility state.                *
 *   WWMouseClass::Set_Cursor -- Set the mouse cursor shape.                                   *
 *   WWMouseClass::Is_Data_Valid -- Determines if there is valid shape image data.             *
 *   WWMouseClass::Validate_Copy_Buffer -- Checks for and validates the background copy buffer.*
 *   WWMouseClass::Matching_Rect -- Finds rectangle of current cursor position & size.         *
 *   WWMouseClass::Save_Background -- Saves the background to a copy buffer.                   *
 *   WWMouseClass::Restore_Background -- Restores the image back where it came from.           *
 *   WWMouseClass::Draw_Mouse -- Manually draw the mouse to the surface specified.             *
 *   WWMouseClass::Erase_Mouse -- Restores the surface after a Draw_Mouse call.                *
 *   WWMouseClass::Raw_Draw_Mouse -- Draws the mouse to the surface specified.                 *
 *   WWMouseClass::Low_Show_Mouse -- Shows the mouse and saves the background.                 *
 *   WWMouseClass::Low_Hide_Mouse -- Restores the surface image in order to hide the mouse.    *
 *   WWMouseClass::Show_Mouse -- Shows the mouse on the visible surface.                       *
 *   WWMouseClass::Hide_Mouse -- Hides the mouse from the visible surface.                     *
 *   WWMouseClass::Capture_Mouse -- Capture the mouse into the mouse handler region.           *
 *   WWMouseClass::Release_Mouse -- Release the mouse back to the O/S.                         *
 *   WWMouseClass::Conditional_Hide_Mouse -- Hides the mouse if it would overlap the region spe*
 *   WWMouseClass::Conditional_Show_Mouse -- Releases the mouse hiding region tracking.        *
 *   WWMouseClass::Convert_Coordinate -- Convert an O/S coordinate into a logical coordinate.  *
 *   WWMouseClass::Get_Bounded_Position -- Fetches the mouse position from the O/S.            *
 *   WWMouseClass::Update_Mouse_Position -- Updates the mouse position to match that specified.*
 *   WWMouseClass::Process_Mouse -- Mouse processing callback routine.                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"_convert.h"
#include	"_mono.h"
#include	"blit.h"
#include	"bsurface.h"
#include	"draw.h"
#include	"shapeset.h"
#include	"Surface.h"
#include	"win.h"
#include	"wwmouse.h"
#include	<assert.h>


/*
**	Persistant mouse object pointer that is used to facilitate access to the mouse
**	handler object outside of the context of a member function. This will be set to the
**	mouse object most recently created.
*/
static WWMouseClass * _MousePtr = NULL;


/***********************************************************************************************
 * Callback_Process_Mouse -- Mouse O/S callback function.                                      *
 *                                                                                             *
 *    This routine is called periodically by the operating system. It handles updating the     *
 *    mouse cursor position to match the mouse movement.                                       *
 *                                                                                             *
 * INPUT:   n/a                                                                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void CALLBACK Callback_Process_Mouse( UINT, UINT, DWORD, DWORD, DWORD  )
{
	if (_MousePtr != NULL) {
		_MousePtr->Process_Mouse();
	}
}


/***********************************************************************************************
 * WWMouseClass::WWMouseClass -- Constructor for mouse handler object.                         *
 *                                                                                             *
 *    This is the constructor for the mouse handler object. It is assigned to a surface and    *
 *    given a confining rectangle. The mouse begins in a non-captured state.                   *
 *                                                                                             *
 * INPUT:   surfaceptr  -- Pointer to the visible display surface that will show the mouse.    *
 *                                                                                             *
 *          confine     -- The confining rectangle within the visible surface. The mouse       *
 *                         coordinates are bound to this rectangle.                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
WWMouseClass::WWMouseClass(Surface * surfaceptr, HWND window) :
	Blocked(false),
	MouseState(-1),
	IsCaptured(false),
	MouseX(0),
	MouseY(0),
	SurfacePtr(surfaceptr),
	Window(window),
	MouseShape(NULL),
	ShapeNumber(0),
	MouseXHot(0),
	MouseYHot(0),
	Background(NULL),
	Alternate(NULL),
	SidebarAlternate(NULL),
	ConditionalRect(0,0,0,0),
	ConditionalState(-1),
	TimerHandle(0)
{
	_MousePtr = this;
	TimerHandle = timeSetEvent(1000/60, 1, Callback_Process_Mouse, 0, TIME_PERIODIC);
	Calc_Confining_Rect();
	MouseXHot = ConfiningRect.X + (ConfiningRect.Width/2);
	MouseYHot = ConfiningRect.Y + (ConfiningRect.Height/2);
}


/***********************************************************************************************
 * WWMouseClass::~WWMouseClass -- Destructor for mouse handler object.                         *
 *                                                                                             *
 *    This will remove the mouse handler object from being processed. It returns the mouse     *
 *    back to O/S control.                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
WWMouseClass::~WWMouseClass(void)
{
	if (TimerHandle != 0) {
		timeKillEvent(TimerHandle);
		_MousePtr = NULL;
	}

	delete Background;
	Background = NULL;

	delete Alternate;
	Alternate = NULL;
	delete SidebarAlternate;
	SidebarAlternate = NULL;
}


void WWMouseClass::Calc_Confining_Rect(void)
{
	RECT rect;
	GetClientRect(Window, &rect);

	POINT point;
	point.x = rect.left;
	point.y = rect.top;
	ClientToScreen(Window, &point);

	POINT lr;
	lr.x = rect.right;
	lr.y = rect.bottom;
	ClientToScreen(Window, &lr);

	ConfiningRect = Rect(point.x, point.y, lr.x-point.x, lr.y-point.y);
//	ConfiningRect = Rect(point.x, point.y, lr.x-point.x+1, lr.y-point.y+1);
}


/***********************************************************************************************
 * WWMouseClass::Get_Mouse_State -- Fetch the current mouse visibility state.                  *
 *                                                                                             *
 *    This routine is used to retrieve the current mouse state as it relates to visiblity.     *
 *    By using this routine it is possible to determine if the mouse is visible.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current mouse visibility state. If the return value is less than  *
 *          0 (i.e., negative), then the mouse is hidden.                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWMouseClass::Get_Mouse_State(void) const
{
	if (!Is_Captured()) {
		ShowCursor(FALSE);
		int state = ShowCursor(TRUE);
		return(state);
	}
	return(MouseState);
}


/***********************************************************************************************
 * WWMouseClass::Set_Cursor -- Set the mouse cursor shape.                                     *
 *                                                                                             *
 *    This routine sets the mouse cursor image and hot-spot. The shape only applies to the     *
 *    mouse when it is captured (the normal case). Repeated calls to this routine is used      *
 *    to give the mouse animation.                                                             *
 *                                                                                             *
 * INPUT:   xhotspot, yhotspot   -- The X,Y offset from the upper left corner of the shape     *
 *                                  that specifies the hot-spot of the image. Positive values  *
 *                                  are right and down from the upper left corner.             *
 *                                                                                             *
 *          cursor   -- Pointer to the shape data.                                             *
 *                                                                                             *
 *          shape    -- The shape number to use within the shape data set specified.           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Set_Cursor(int xhotspot, int yhotspot, ShapeSet const * cursor, int shape)
{
	if (cursor != NULL) {
		if (Is_Captured()) {
			Block_Mouse();
			if (!Is_Hidden()) Low_Hide_Mouse();
			MouseShape = cursor;
			ShapeNumber = shape;
			MouseXHot = xhotspot;
			MouseYHot = yhotspot;
			if (!Is_Hidden()) Low_Show_Mouse();
			Unblock_Mouse();
		} else {
			MouseShape = cursor;
			ShapeNumber = shape;
			MouseXHot = xhotspot;
			MouseYHot = yhotspot;
		}
	}
}


/***********************************************************************************************
 * WWMouseClass::Is_Data_Valid -- Determines if there is valid shape image data.               *
 *                                                                                             *
 *    This routine does a simple check to determine if the shape handler has been supplied     *
 *    a pointer to shape imagery. Any internal routine that requires the data to be present    *
 *    should call this routine to be sure it actually is.                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Has a shape image data pointer been supplied to this mouse handler?          *
 *                                                                                             *
 * WARNINGS:   When the mouse object is first created, no image data has been assigned. The    *
 *             Set_Cursor must be called before this routine will return TRUE.                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWMouseClass::Is_Data_Valid(void) const
{
	if (MouseShape != NULL) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * WWMouseClass::Validate_Copy_Buffer -- Checks for and validates the background copy buffer.  *
 *                                                                                             *
 *    This routine checks and allocates if necessary a buffer that is big enough to hold the   *
 *    background image under the mouse. Whenever the background buffer is needed, this routine *
 *    should be called to ensure that it is valid.                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the buffer valid?                                                         *
 *                                                                                             *
 * WARNINGS:   This routine might fail if there was insufficient memory.                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WWMouseClass::Validate_Copy_Buffer(void)
{
	if (Is_Data_Valid()) {

		/*
		**	If there is a background buffer already allocated, then verify that
		**	it is large enough for the current shape data. If not, then free the
		**	buffer and reallocate it at the larger size.
		*/
		if (Background != NULL) {
			if (MouseShape->Get_Width() > Background->Get_Width() ||
				MouseShape->Get_Height() > Background->Get_Height()) {

				delete Background;
				Background = NULL;
			}
		}
		if (Alternate != NULL) {
			if (MouseShape->Get_Width() > Alternate->Get_Width() ||
				MouseShape->Get_Height() > Alternate->Get_Height()) {

				delete Alternate;
				Alternate = NULL;
			}
		}
		if (SidebarAlternate != NULL) {
			if (MouseShape->Get_Width() > SidebarAlternate->Get_Width() ||
				MouseShape->Get_Height() > SidebarAlternate->Get_Height()) {

				delete SidebarAlternate;
				SidebarAlternate = NULL;
			}
		}

		/*
		**	Allocate a new background buffer if necessary. This must be big enough to
		**	hold the largest sized shape from the currently assigned shape set data.
		*/
		if (Background == NULL) {
			Background = new BSurface(MouseShape->Get_Width(), MouseShape->Get_Height(), SurfacePtr->Bytes_Per_Pixel());
		}
		if (Alternate == NULL) {
			Alternate = new BSurface(MouseShape->Get_Width(), MouseShape->Get_Height(), SurfacePtr->Bytes_Per_Pixel());
		}
		if (SidebarAlternate == NULL) {
			SidebarAlternate = new BSurface(MouseShape->Get_Width(), MouseShape->Get_Height(), SurfacePtr->Bytes_Per_Pixel());
		}

		return(Background != NULL && Alternate != NULL && SidebarAlternate != NULL);
	}
	return(false);
}


/***********************************************************************************************
 * WWMouseClass::Matching_Rect -- Finds rectangle of current cursor position & size.           *
 *                                                                                             *
 *    This routine will return the logical rectangle that exactly encloses the cursor. This    *
 *    routine is typically used when drawing the cursor and manipulating the background buffer *
 *    under it.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the rectangle that surrounds the cursor.                              *
 *                                                                                             *
 * WARNINGS:   The rectangle is in logical coordinates. It may have to be biased to O/S        *
 *             coordinates if necessary.                                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
Rect WWMouseClass::Matching_Rect(void) const
{
	Rect rect;
	if (Is_Data_Valid()) {
		((WWMouseClass *)this)->Block_Mouse();

		/*
		**	Build the rectangle that the mouse shape will consume.
		*/
		rect = MouseShape->Get_Rect(ShapeNumber);
		rect.X += MouseX - MouseXHot;
		rect.Y += MouseY - MouseYHot;

		((WWMouseClass *)this)->Unblock_Mouse();
	}
	return(rect);
}


/***********************************************************************************************
 * WWMouseClass::Save_Background -- Saves the background to a copy buffer.                     *
 *                                                                                             *
 *    This routine will save the region under the mouse (or where the mouse would appear at)   *
 *    to a copy buffer.                                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The previous contents of the copy buffer will be overwritten by this routine.   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Save_Background(void)
{
	if (Validate_Copy_Buffer()) {

		/*
		**	Build the rectangle that the mouse shape will consume.
		*/
		SavedRegion = Matching_Rect();

		Rect rect = SavedRegion;
		rect.X += ConfiningRect.X;
		rect.Y += ConfiningRect.Y;

		/*
		**	Blit the background from the surface to the holding buffer.
		*/
//		Rect old = SurfacePtr->Get_Rect();
//		SurfacePtr->Window.Reset();
		Background->Blit_From(Rect(0, 0, rect.Width, rect.Height), *SurfacePtr, rect);
//		SurfacePtr->Window.Set(old);
	}
}


/***********************************************************************************************
 * WWMouseClass::Restore_Background -- Restores the image back where it came from.             *
 *                                                                                             *
 *    This is the counterpart routine to the Save_Background function. It will restore the     *
 *    image from the copy buffer back to the screen where it came from.                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Restore_Background(void)
{
	if (SavedRegion.Is_Valid()) {

		Rect rect = SavedRegion;
		rect.X += ConfiningRect.X;
		rect.Y += ConfiningRect.Y;

		/*
		**	Blit the background from the holding buffer to the surface.
		*/
//		Rect old = SurfacePtr->Get_Rect();
//		SurfacePtr->Window.Reset();
		SurfacePtr->Blit_From(rect, *Background, Rect(0, 0, rect.Width, rect.Height));
//		SurfacePtr->Window.Set(old);
	}
}


/***********************************************************************************************
 * WWMouseClass::Draw_Mouse -- Manually draw the mouse to the surface specified.               *
 *                                                                                             *
 *    This is a kludge function that can be used to reduce mouse flicker. Normally the mouse   *
 *    must be hidden before an image is copied to the visible surface. By drawing the mouse    *
 *    in the correct position on the source image prior to the copy, the mouse doesn't need    *
 *    to be hidden and no mouse flicker occurs. This routine handles this manual draw process. *
 *                                                                                             *
 * INPUT:   surface  -- Pointer to the surface that the mouse will be drawn to.                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The destination surface is presumed to be the exact dimensions of the bouding   *
 *             rectangle specified in the mouse constructor. The call to Erase_Mouse must      *
 *             occur as soon as possible since the mouse is frozen until it is called.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Draw_Mouse(Surface * surface, bool issidebarsurface)
{

	BSurface *savesurface;
	Rect *savedregion;
	int xoffset;
	int yoffset;

	if (issidebarsurface){
		xoffset = -480;
		yoffset = 0;
		savesurface = SidebarAlternate;
		savedregion = &SidebarAltRegion;
	}else{
		xoffset = 0;
		yoffset = 0;
		savesurface = Alternate;
		savedregion = &AltRegion;
	}

	if (!Is_Hidden() && surface != NULL && surface != SurfacePtr && savesurface != NULL) {
		Block_Mouse();

		/*
		**	Blit the background from the surface to the holding buffer.
		*/
		//Rect old = surface->Window.Get_Rect();
		//surface->Window.Reset();
		*savedregion = SavedRegion;
		savedregion->X += xoffset;
		savedregion->Y += yoffset;
		savesurface->Blit_From(Rect(0, 0, savedregion->Width, savedregion->Height), *surface, *savedregion);
		if (issidebarsurface){
			if (savedregion->X < 0 && -savedregion->X < Background->Get_Width()){
				Background->Blit_From(Rect(-savedregion->X, 0, savedregion->Width, savedregion->Height),
											*surface, Rect(0,savedregion->Y, savedregion->Width, savedregion->Height));
			}
		}else{
			Background->Blit_From(Rect(0, 0, savedregion->Width, savedregion->Height), *surface, *savedregion);
		}
		//surface->Window.Set(old);
		Raw_Draw_Mouse(surface, xoffset, yoffset);
		Unblock_Mouse();
	} else {
		savedregion->Width = 0;
	}
}


/***********************************************************************************************
 * WWMouseClass::Erase_Mouse -- Restores the surface after a Draw_Mouse call.                  *
 *                                                                                             *
 *    This is the counterpart routine to Draw_Mouse. It will restore the specified surface     *
 *    back to its original state. The mouse is frozen between the calls to Draw_Mouse and      *
 *    Erase_Mouse, so it is imperative to call this routine at the first legal opportunity.    *
 *                                                                                             *
 * INPUT:   surface  -- Pointer to the surface that the mouse was previously drawn to.         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Erase_Mouse(Surface * surface, bool issidebarsurface)
{
	if (!Is_Hidden() && surface != NULL && surface != SurfacePtr && Alternate != NULL && SidebarAlternate != NULL) {

		BSurface *savesurface;
		Rect savedregion;

		if (issidebarsurface){
			savesurface = SidebarAlternate;
			savedregion = SidebarAltRegion;
		}else{
			savesurface = Alternate;
			savedregion = AltRegion;
		}

		/*
		**	Blit the background from the holding buffer to the surface.
		*/
		Block_Mouse();
		//Rect old = surface->Window.Get_Rect();
		//surface->Window.Reset();
		surface->Blit_From(savedregion, *savesurface, Rect(0, 0, savedregion.Width, savedregion.Height));
		//surface->Window.Set(old);
		Unblock_Mouse();
	}
}


/***********************************************************************************************
 * WWMouseClass::Raw_Draw_Mouse -- Draws the mouse to the surface specified.                   *
 *                                                                                             *
 *    This will draw the mouse image to the surface specified. It does not do any checking     *
 *    except to make sure that the mouse image data is nominally valid.                        *
 *                                                                                             *
 * INPUT:   surface  -- The surface to draw the mouse upon.                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The background is not preserved by this routine.                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Raw_Draw_Mouse(Surface * surface, int xoffset, int yoffset)
{
	if (Is_Data_Valid() && surface != NULL) {

		/*
		**	Determine the rectangle that the mouse will be drawn
		**	to.
		*/

		Rect rect = SavedRegion;
		rect.X += xoffset;
		rect.Y += yoffset;
//		if (surface == SurfacePtr) {
//			rect.X += ConfiningRect.X;
//			rect.Y += ConfiningRect.Y;
//		}

//		Rect old = surface->Get_Rect();
//		surface->Window.Reset();
		BSurface mdata(rect.Width, rect.Height, 1, MouseShape->Get_Data(ShapeNumber));

		if (surface == SurfacePtr) {
			Blit_Block(*surface, *NormalDrawer, mdata, Rect(0, 0, rect.Width, rect.Height), Point2D(rect.X, rect.Y), ConfiningRect);
		} else {
			Blit_Block(*surface, *NormalDrawer, mdata, Rect(0, 0, rect.Width, rect.Height), Point2D(rect.X, rect.Y), surface->Get_Rect());
		}

		//surface->Window.Set(old);
	}
}


/***********************************************************************************************
 * WWMouseClass::Low_Show_Mouse -- Shows the mouse and saves the background.                   *
 *                                                                                             *
 *    This routine will save the background and then draw the mouse to the visible surface.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Low_Show_Mouse(void)
{
	Block_Mouse();
	Save_Background();
	Raw_Draw_Mouse(SurfacePtr, 0, 0);
	Unblock_Mouse();
}


/***********************************************************************************************
 * WWMouseClass::Low_Hide_Mouse -- Restores the surface image in order to hide the mouse.      *
 *                                                                                             *
 *    This routine will hide the mouse on the visible surface. It does this by restoring the   *
 *    pixels under where the mouse was previously drawn.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Low_Hide_Mouse(void)
{
	Block_Mouse();
	Restore_Background();
	Unblock_Mouse();
}


/***********************************************************************************************
 * WWMouseClass::Show_Mouse -- Shows the mouse on the visible surface.                         *
 *                                                                                             *
 *    This routine is called when the mouse can be shown on the visible surface.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Show_Mouse(void)
{
	if (!Is_Captured()) {
		ShowCursor(TRUE);
	} else {
		Block_Mouse();
		InterlockedIncrement(&MouseState);
		if (MouseState == 0) {
			Low_Show_Mouse();
		}
		assert(MouseState != 1);
		if (MouseState > 0) MouseState = 0;
		Unblock_Mouse();
	}
}


/***********************************************************************************************
 * WWMouseClass::Hide_Mouse -- Hides the mouse from the visible surface.                       *
 *                                                                                             *
 *    This routine is called when the mouse is desired to be hidden from the visible surface.  *
 *    Typically, this must occur if the pixels where the mouse is located will be accessed.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Hide_Mouse(void)
{
	if (!Is_Captured()) {
		ShowCursor(FALSE);
	} else {
		Block_Mouse();
		InterlockedDecrement(&MouseState);
		if (MouseState == -1) {
			Low_Hide_Mouse();
		}
		Unblock_Mouse();
	}
}


/***********************************************************************************************
 * WWMouseClass::Capture_Mouse -- Capture the mouse into the mouse handler region.             *
 *                                                                                             *
 *    This routine will confine the mouse to the confining rectangle and take over drawing     *
 *    of the mouse image from the operating system. The typical state is to keep the mouse     *
 *    captured throughout the lifetime of the owning program.                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Capture_Mouse(void)
{
	if (this != NULL && !Is_Captured()) {
		Block_Mouse();
		while (ShowCursor(FALSE) > -1) {}
		while (ShowCursor(TRUE) < -1) {}
		Hide_Mouse();
		IsCaptured = true;

		Show_Mouse();
		Unblock_Mouse();
	}
}


/***********************************************************************************************
 * WWMouseClass::Release_Mouse -- Release the mouse back to the O/S.                           *
 *                                                                                             *
 *    This is the counterpart routine to Capture_Mouse. This routine will return the drawing   *
 *    and movement controls back to the operating system. Although the mouse will probably     *
 *    be able to roam outside the confining rectangle, the coordinates returned by this class  *
 *    are clipped to the confining rectangle anyway. This gives the impression that the mouse  *
 *    is still at a legal position. The presumption is that the mouse needs to be released to  *
 *    the O/S for reasons outside of the game itself. As such, the shouldn't detect any        *
 *    illegal mouse position.                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   All mouse shape changes won't be relected while the mouse is released. The O/S  *
 *             handles drawing the mouse in that case.                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Release_Mouse(void)
{
	if (this != NULL && Is_Captured()) {
		Block_Mouse();
		Hide_Mouse();
		IsCaptured = false;
		while (ShowCursor(FALSE) > -1) {}
		while (ShowCursor(TRUE) < -1) {}
		Show_Mouse();

		Unblock_Mouse();
	}
}


/***********************************************************************************************
 * WWMouseClass::Conditional_Hide_Mouse -- Hides the mouse if it would overlap the region spec *
 *                                                                                             *
 *    This routine will hide the mouse if it lies within the region specified or if it moves   *
 *    within the region.                                                                       *
 *                                                                                             *
 * INPUT:   rect  -- The rectangle that the mouse should not be drawn within.                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Conditional_Hide_Mouse(Rect )
{
	Hide_Mouse();
}


/***********************************************************************************************
 * WWMouseClass::Conditional_Show_Mouse -- Releases the mouse hiding region tracking.          *
 *                                                                                             *
 *    This routine will release the region hiding tracking that was set up with a previous     *
 *    call to Conditional_Hide_Mouse().                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Conditional_Show_Mouse(void)
{
	Show_Mouse();
}


/***********************************************************************************************
 * WWMouseClass::Convert_Coordinate -- Convert an O/S coordinate into a logical coordinate.    *
 *                                                                                             *
 *    Sometimes you come across system mouse coordinates and they need to be converted into    *
 *    game logical coordinates. This routine will perform this function.                       *
 *                                                                                             *
 * INPUT:   x,y   -- Reference to the coordinates that will be converted into game logical     *
 *                   coordinates.                                                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The coordinates will be bound as well as transformed by the confining rectangle.*
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Convert_Coordinate(int & x, int & y) const
{
	/*
	**	Convert the mouse position to legal bounds.
	*/
	x -= ConfiningRect.X;
	y -= ConfiningRect.Y;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= ConfiningRect.Width) x = ConfiningRect.Width-1;
	if (y >= ConfiningRect.Height) y = ConfiningRect.Height-1;
}


/***********************************************************************************************
 * WWMouseClass::Get_Bounded_Position -- Fetches the mouse position from the O/S.              *
 *                                                                                             *
 *    Fetches the mouse coordinates from the O/S and converts them into logical coordinates.   *
 *                                                                                             *
 * INPUT:   x,y   -- Reference to the coordinates that will be set by this routine.            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Get_Bounded_Position(int & x, int & y) const
{
	/*
	** Get the mouse's current real cursor position
	*/
	POINT pt;
	GetCursorPos(&pt);			// get the current cursor position
	x = pt.x;
	y = pt.y;
	Convert_Coordinate(x, y);
}


/***********************************************************************************************
 * WWMouseClass::Update_Mouse_Position -- Updates the mouse position to match that specified.  *
 *                                                                                             *
 *    This routine will update the mouse to match the position speicifed.                      *
 *                                                                                             *
 * INPUT:   x,y   -- The coordinates to position the mouse (hot spot). The coordinates are     *
 *                   specified as logical not O/S relative.                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Update_Mouse_Position(int x, int y)
{
	/*
	**	If the desired position is not the same as the current
	**	position, then hide the mouse, reposition it, then show
	**	the mouse.
	*/
	Block_Mouse();
	if (x != MouseX || y != MouseY) {
		if (Is_Captured() && !Is_Hidden()) Low_Hide_Mouse();
		MouseX = x;
		MouseY = y;
		if (Is_Captured() && !Is_Hidden()) Low_Show_Mouse();
	}
	Unblock_Mouse();
}


/***********************************************************************************************
 * WWMouseClass::Process_Mouse -- Mouse processing callback routine.                           *
 *                                                                                             *
 *    This routine should be called periodically (recommended 15 times per second). It will    *
 *    examine the operating system mouse position and then update the visible mouse to match.  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Process_Mouse(void)
{
	if (!Is_Blocked()) {
		Block_Mouse();

		/*
		** Fetch and update the mouse position.
		*/
		int x;
		int y;
		Get_Bounded_Position(x, y);
		Update_Mouse_Position(x, y);
		Unblock_Mouse();
	}
}


/***********************************************************************************************
 * WWMouseClass::Set_Mouse_XY -- Sets the cursor position                                      *
 *                                                                                             *
 *    This routine will convert the position passed into screen coordinates and tell Windows   *
 *    to position the mouse cursor there                                                       *
 *                                                                                             *
 * INPUT:   x and y in game coordinates                                                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 BG : Created.                                                                 *
 *=============================================================================================*/
void WWMouseClass::Set_Mouse_XY( int x, int y )
{
	if (x < 0) x = 0;					// clamp to game coordinates
	if (y < 0) y = 0;
	if (x >= ConfiningRect.Width) x = ConfiningRect.Width-1;
	if (y >= ConfiningRect.Height) y = ConfiningRect.Height-1;

	x += ConfiningRect.X;			// convert to screen coordinates
	y += ConfiningRect.Y;

	SetCursorPos( x, y );			// set the current cursor position
}



