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

// FILE: W3DMouse.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Mark W.
// Desc:   W3D Mouse cursor implementations
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Common/GameMemory.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/rendobj.h"
#include "WW3D2/hanim.h"
#include "WW3D2/camera.h"

#include "assetmgr.h"

#include "W3DDevice/Common/W3DConvert.h"
#include "W3DDevice/GameClient/W3DMouse.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "GameClient/Display.h"
#include "GameClient/Image.h"
#include "GameClient/InGameUI.h"
#include "mutex.h"
#include "thread.h"


//Since there can't be more than 1 mouse, might as well keep these static.
static CriticalSectionClass mutex;
static Bool isThread;
static TextureClass *cursorTextures[Mouse::NUM_MOUSE_CURSORS][MAX_2D_CURSOR_ANIM_FRAMES];	///<Textures for each cursor type
static const Image *cursorImages[Mouse::NUM_MOUSE_CURSORS];			///<Images for use with the RM_POLYGON method.
static RenderObjClass *cursorModels[Mouse::NUM_MOUSE_CURSORS];	///< W3D models for each cursor type
static HAnimClass			*cursorAnims[Mouse::NUM_MOUSE_CURSORS];		///< W3D animations for each cursor type

///Mouse polling/update thread function
static class MouseThreadClass : public ThreadClass
{

public:
	MouseThreadClass() : ThreadClass() {}

	void Thread_Function();

} thread;

void MouseThreadClass::Thread_Function()
{

	//poll mouse and update position

	while (running)
	{
		isThread=TRUE;
		if (TheMouse)
			TheMouse->draw();
		isThread=FALSE;
		Switch_Thread();
	}
}

W3DMouse::W3DMouse( void )
{
	// zero our event list
	for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		for (Int j=0; j<MAX_2D_CURSOR_ANIM_FRAMES; j++)
			cursorTextures[i][j]=NULL;
		cursorModels[i]=NULL;
		cursorAnims[i]=NULL;
	}

	m_currentD3DCursor=NONE;
	m_currentW3DCursor=NONE;
	m_currentPolygonCursor=NONE;
	m_currentAnimFrame = 0;
	m_currentD3DFrame = 0;
	m_currentFrames = 0;
	m_currentFMS= 1.0f/1000.0f;

	m_camera = NULL;
	m_drawing = FALSE;

}  // end Win32Mouse

W3DMouse::~W3DMouse( void )
{
	LPDIRECT3DDEVICE8 m_pDev=DX8Wrapper::_Get_D3D_Device8();

	if (m_pDev)
	{
		m_pDev->ShowCursor(FALSE);	//kill DX8 cursor
		Win32Mouse::setCursor(ARROW); //enable default windows cursor
	}

	freeD3DAssets();
	freeW3DAssets();

	thread.Stop();

}  // end Win32Mouse

void W3DMouse::initPolygonAssets(void)
{
	CriticalSectionClass::LockClass m(mutex);

	//don't allow the mouse thread to initialize
	//wait for main app to do initialization.
	if (isThread)
		return;

	//Check if texture assets already loaded
	if (m_currentRedrawMode == RM_POLYGON && cursorImages[1] == NULL)
	{
		for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
		{
			m_currentPolygonCursor = m_currentCursor;
			if (!m_cursorInfo[i].imageName.isEmpty())
				cursorImages[i]=TheMappedImageCollection->findImageByName(m_cursorInfo[i].imageName);
		}
	}
}

void W3DMouse::freePolygonAssets(void)
{

	for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		cursorImages[i]=NULL;
	}
}

/**Release the textures required to display the selected cursor*/
Bool W3DMouse::releaseD3DCursorTextures(MouseCursor cursor)
{
	if (cursor == NONE || !cursorTextures[cursor][0])
		return TRUE;	//no texture for this cursor or texture never loaded

	for (Int i=0; i<MAX_2D_CURSOR_ANIM_FRAMES; i++)
	{
		REF_PTR_RELEASE(m_currentD3DSurface[i]);
		REF_PTR_RELEASE(cursorTextures[cursor][i]);
	}

	return TRUE;
}

/**Load the textures required to display the selected cursor*/
Bool W3DMouse::loadD3DCursorTextures(MouseCursor cursor)
{
	if (cursor == NONE || cursorTextures[cursor][0])
		return TRUE;	//no texture for this cursor or texture already loaded

	WW3DAssetManager *am=WW3DAssetManager::Get_Instance();
	Int animFrames=m_cursorInfo[cursor].numFrames;

	if (!animFrames)
		return FALSE;	//no animation frames defined.

	const char *baseName=m_cursorInfo[cursor].textureName.str();
	char FrameName[64];

	//Clamp to reasonable number
	if (animFrames > MAX_2D_CURSOR_ANIM_FRAMES)
		animFrames = MAX_2D_CURSOR_ANIM_FRAMES;

	m_currentFrames=0;

	if (animFrames == 1)
	{	//single animation frame without trailing numbers
		sprintf(FrameName,"%s.tga",baseName);
		cursorTextures[cursor][0]=	am->Get_Texture(FrameName);
		m_currentD3DSurface[0]=cursorTextures[cursor][0]->Get_Surface_Level();
		m_currentFrames = 1;
	}
	else
	for (Int i=0; i<animFrames; i++)
	{
		sprintf(FrameName,"%s%04d.tga",baseName,i);
		if ((cursorTextures[cursor][i]=am->Get_Texture(FrameName)) != NULL)
		{	m_currentD3DSurface[m_currentFrames]=cursorTextures[cursor][i]->Get_Surface_Level();
			m_currentFrames++;
		}
	}
	return TRUE;
}

void W3DMouse::initD3DAssets(void)
{
	//Nothing to do here unless we want to preload all possible cursors which would
	//probably not be practical for memory reasons.

	CriticalSectionClass::LockClass m(mutex);

	//don't allow the mouse thread to initialize
	//wait for main app to do initialization.
	if (isThread)
		return;

	WW3DAssetManager *am=WW3DAssetManager::Get_Instance();

	//Check if texture assets already loaded
	if (m_currentRedrawMode == RM_DX8 && cursorTextures[1] == NULL && am)
	{
		for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
		{
			for (Int j=0; j < MAX_2D_CURSOR_ANIM_FRAMES; j++)
			{
				cursorTextures[i][j]=NULL;//am->Get_Texture(m_cursorInfo[i].textureName.str());
			}
		}

		for (Int x = 0; x < MAX_2D_CURSOR_ANIM_FRAMES; x++)
			m_currentD3DSurface[x]=NULL;
	}
}

void W3DMouse::freeD3DAssets(void)
{
	//free pointers to texture surfaces.
	Int i=0;
	for (; i<MAX_2D_CURSOR_ANIM_FRAMES; i++)
		REF_PTR_RELEASE(m_currentD3DSurface[i]);

	//free textures.
	for (i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		for (Int j=0; j<MAX_2D_CURSOR_ANIM_FRAMES; j++)
			REF_PTR_RELEASE(cursorTextures[i][j]);
	}

}

void W3DMouse::initW3DAssets(void)
{
	CriticalSectionClass::LockClass m(mutex);

	//don't allow the mouse thread to initialize
	//wait for main app to do initialization.
	if (isThread)
		return;

	//Check if model assets already loaded
	if ((cursorModels[1] == NULL && W3DDisplay::m_assetManager))
	{
		for (Int i=1; i<NUM_MOUSE_CURSORS; i++)
		{
			if (!m_cursorInfo[i].W3DModelName.isEmpty())
			{
				if (m_orthoCamera)
					cursorModels[i] = W3DDisplay::m_assetManager->Create_Render_Obj(m_cursorInfo[i].W3DModelName.str(), m_cursorInfo[i].W3DScale*m_orthoZoom, 0);
				else
					cursorModels[i] = W3DDisplay::m_assetManager->Create_Render_Obj(m_cursorInfo[i].W3DModelName.str(), m_cursorInfo[i].W3DScale, 0);
				if (cursorModels[i])
				{
					cursorModels[i]->Set_Position(Vector3(0.0f, 0.0f, -1.0f));
					//W3DDisplay::m_3DInterfaceScene->Add_Render_Object(cursorModels[i]);
				}
			}
		}
	}
	if ((cursorAnims[1] == NULL && W3DDisplay::m_assetManager))
	{
		for (Int i=1; i<NUM_MOUSE_CURSORS; i++)
		{
			if (!m_cursorInfo[i].W3DAnimName.isEmpty())
			{
				DEBUG_ASSERTCRASH(cursorAnims[i] == NULL, ("hmm, leak festival"));
				cursorAnims[i] = W3DDisplay::m_assetManager->Get_HAnim(m_cursorInfo[i].W3DAnimName.str());
				if (cursorAnims[i] && cursorModels[i])
				{
					cursorModels[i]->Set_Animation(cursorAnims[i], 0, (m_cursorInfo[i].loop) ? RenderObjClass::ANIM_MODE_LOOP : RenderObjClass::ANIM_MODE_ONCE);
				}
			}
		}
	}

	// create the camera
	m_camera = NEW_REF( CameraClass, () );
	m_camera->Set_Position( Vector3( 0, 1, 1 ) );
	Vector2 min = Vector2( -1, -1 );
	Vector2 max = Vector2( +1, +1 );
	m_camera->Set_View_Plane( min, max );
	m_camera->Set_Clip_Planes( 0.995f, 20.0f );
	if (m_orthoCamera)
		m_camera->Set_Projection_Type( CameraClass::ORTHO );
}

void W3DMouse::freeW3DAssets(void)
{

	for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		if (W3DDisplay::m_3DInterfaceScene && cursorModels[i])
		{
			W3DDisplay::m_3DInterfaceScene->Remove_Render_Object(cursorModels[i]);
		}
		REF_PTR_RELEASE(cursorModels[i]);
		REF_PTR_RELEASE(cursorAnims[i]);
	}

	REF_PTR_RELEASE(m_camera);
}

//-------------------------------------------------------------------------------------------------
/** Initialize our device */
//-------------------------------------------------------------------------------------------------
void W3DMouse::init( void )
{

	//check if system already initialized and texture assets loaded.
	Win32Mouse::init();
	setCursor(ARROW);	//set default starting cursor image

	WWASSERT(!thread.Is_Running());


	isThread=FALSE;
	if (m_currentRedrawMode == RM_DX8)
		thread.Execute();
	thread.Set_Priority(0);

}  // end int

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void W3DMouse::reset( void )
{

	// extend
	Win32Mouse::reset();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Super basic simplistic cursor */
//-------------------------------------------------------------------------------------------------
void W3DMouse::setCursor( MouseCursor cursor )
{

	CriticalSectionClass::LockClass m(mutex);

	m_directionFrame=0;
	if (m_currentRedrawMode == RM_WINDOWS)
	{	//Windows default cursor needs to refreshed whenever we get a WM_SETCURSOR
		m_currentD3DCursor=NONE;
		m_currentW3DCursor=NONE;
		m_currentPolygonCursor=NONE;
		setCursorDirection(cursor);
		if (m_drawing)	//only allow cursor to change when drawing the cursor (once per frame) to fix flickering.
			Win32Mouse::setCursor(cursor);
		m_currentCursor = cursor;
		return;
	}

	// extend
	Mouse::setCursor( cursor );

	// if we're already on this cursor ignore the rest of code to stop cursor flickering.
	if( m_currentCursor == cursor && m_currentD3DCursor == cursor)
		return;

	//make sure Windows didn't reset our cursor
	if (m_currentRedrawMode == RM_DX8)
	{
		SetCursor(NULL);	//Kill Windows Cursor

		LPDIRECT3DDEVICE8 m_pDev=DX8Wrapper::_Get_D3D_Device8();
		Bool doImageChange=FALSE;

		if (m_pDev != NULL)
		{
			m_pDev->ShowCursor(FALSE);	//disable DX8 cursor
			if (cursor != m_currentD3DCursor)
			{	if (!isThread)
				{	releaseD3DCursorTextures(m_currentD3DCursor);
					//Since this type of cursor is updated from a non-D3D thread, we need
					//to preallocate all surfaces in main thread.
					loadD3DCursorTextures(cursor);
				}
			}
			if (m_currentD3DSurface[0])
				doImageChange=TRUE;
		}
		//For DX8 Cursors, we continually set the image on every call even when
		//it didn't change.  This is needed to prevent the cursor from flickering.
		if (doImageChange)
		{
			HRESULT res;
			m_currentHotSpot = m_cursorInfo[cursor].hotSpotPosition;
			m_currentFMS = m_cursorInfo[cursor].fps/1000.0f;
			m_currentAnimFrame = 0;	//reset animation when cursor changes
			res = m_pDev->SetCursorProperties(m_currentHotSpot.x,m_currentHotSpot.y,m_currentD3DSurface[(Int)m_currentAnimFrame]->Peek_D3D_Surface());
			m_pDev->ShowCursor(TRUE);	//Enable DX8 cursor
			m_currentD3DFrame=(Int)m_currentAnimFrame;
			m_currentD3DCursor = cursor;
			m_lastAnimTime=timeGetTime();
		}
	}
	else if (m_currentRedrawMode == RM_POLYGON)
	{
		SetCursor(NULL);	//Kill Windows Cursor
		m_currentD3DCursor=NONE;
		m_currentW3DCursor=NONE;
		m_currentPolygonCursor = cursor;
		m_currentHotSpot = m_cursorInfo[cursor].hotSpotPosition;
	}
	else if (m_currentRedrawMode == RM_W3D)
	{
		SetCursor(NULL);	//Kill Windows Cursor
		m_currentD3DCursor=NONE;
		m_currentPolygonCursor=NONE;
		if (cursor != m_currentW3DCursor)
		{
			// set the new model visible
			if (!cursorModels[1])
				initW3DAssets();

			if (cursorModels[1])
			{
				if (cursorModels[m_currentW3DCursor])
				{
					W3DDisplay::m_3DInterfaceScene->Remove_Render_Object(cursorModels[m_currentW3DCursor]);
				}

				m_currentW3DCursor=cursor;

				if (cursorModels[m_currentW3DCursor])
				{
					W3DDisplay::m_3DInterfaceScene->Add_Render_Object(cursorModels[m_currentW3DCursor]);
					if (m_cursorInfo[m_currentW3DCursor].loop == FALSE && cursorAnims[m_currentW3DCursor])
					{
						cursorModels[m_currentW3DCursor]->Set_Animation(cursorAnims[m_currentW3DCursor], 0, RenderObjClass::ANIM_MODE_ONCE);
					}
				}
			}
		}
		else
		{
			m_currentW3DCursor=cursor;
		}
	}

	// save current cursor
	m_currentCursor = cursor;

}  // end setCursor

extern HWND ApplicationHWnd;

void W3DMouse::draw(void)
{
	CriticalSectionClass::LockClass m(mutex);

	m_drawing = TRUE;

	//make sure the correct cursor image is selected
	setCursor(m_currentCursor);

	if (m_currentRedrawMode == RM_DX8 && m_currentD3DCursor != NONE)
	{
		//called from upate thread or rendering loop.  Tells D3D where
		//to draw the mouse cursor.
		LPDIRECT3DDEVICE8 m_pDev=DX8Wrapper::_Get_D3D_Device8();
		if (m_pDev)
		{	m_pDev->ShowCursor(TRUE);	//Enable DX8 cursor

			if (TheDisplay && !TheDisplay->getWindowed())
			{	//if we're full-screen, need to manually move cursor image
				POINT ptCursor;

				GetCursorPos( &ptCursor );
				ScreenToClient( ApplicationHWnd, &ptCursor );
				m_pDev->SetCursorPosition( ptCursor.x, ptCursor.y, D3DCURSOR_IMMEDIATE_UPDATE);
			}
			//Check if animated cursor and new frame
			if (m_currentFrames > 1)
			{
				Int msTime=timeGetTime();
				m_currentAnimFrame += (msTime-m_lastAnimTime) * m_currentFMS;
				m_currentAnimFrame=fmod(m_currentAnimFrame,m_currentFrames);
				m_lastAnimTime=msTime;

				if ((Int)m_currentAnimFrame != m_currentD3DFrame)
				{
					m_currentD3DFrame=(Int)m_currentAnimFrame;
					m_pDev->SetCursorProperties(m_currentHotSpot.x,m_currentHotSpot.y,m_currentD3DSurface[m_currentD3DFrame]->Peek_D3D_Surface());
				}
			}
		}
	}
	else if (m_currentRedrawMode == RM_POLYGON)
	{
		const Image *image=cursorImages[m_currentPolygonCursor];
		if (image)
		{
			TheDisplay->drawImage(image,m_currMouse.pos.x-m_currentHotSpot.x,m_currMouse.pos.y-m_currentHotSpot.y,
				m_currMouse.pos.x+image->getImageWidth()-m_currentHotSpot.x, m_currMouse.pos.y+image->getImageHeight()-m_currentHotSpot.y);
		}
	}
	else if (m_currentRedrawMode == RM_WINDOWS)
	{
	}
	else if (m_currentRedrawMode == RM_W3D)
	{
		if ( W3DDisplay::m_3DInterfaceScene && m_camera && m_visible)
		{
			if (cursorModels[m_currentW3DCursor])
			{
				Real xPercent = (1.0f - (TheDisplay->getWidth() - m_currMouse.pos.x) / (Real)TheDisplay->getWidth());
				Real yPercent = ((TheDisplay->getHeight() - m_currMouse.pos.y) / (Real)TheDisplay->getHeight());

				Real x, y, z = -1.0f;

				if (m_orthoCamera)
				{
					x = xPercent*2 - 1;
					y = yPercent*2;
				}
				else
				{
					//W3D Screen coordinates are -1 to 1, so we need to do some conversion:
					Real logX, logY;
					PixelScreenToW3DLogicalScreen(m_currMouse.pos.x - 0, m_currMouse.pos.y - 0, &logX, &logY, TheDisplay->getWidth(), TheDisplay->getHeight());

					Vector3 rayStart;
					Vector3 rayEnd;
					rayStart = m_camera->Get_Position();							//get camera location
					m_camera->Un_Project(rayEnd,Vector2(logX,logY));	//get world space point
					rayEnd -= rayStart;																//vector camera to world space point
					rayEnd.Normalize();																//make unit vector
					rayEnd *= m_camera->Get_Depth();									//adjust length to reach far clip plane
					rayEnd += rayStart;																//get point on far clip plane along ray from camera.

					x = Vector3::Find_X_At_Z(z, rayStart, rayEnd);
					y = Vector3::Find_Y_At_Z(z, rayStart, rayEnd);
				}

				Matrix3D tm(1);
				tm.Set_Translation(Vector3(x, y, z));
				Coord2D offset = {0, 0};
				if (TheInGameUI && TheInGameUI->isScrolling())
				{
					offset = TheInGameUI->getScrollAmount();
					offset.normalize();
					Real theta = atan2(-offset.y, offset.x);
					theta -= (Real)M_PI/2;
					tm.Rotate_Z(theta);
				}
				cursorModels[m_currentW3DCursor]->Set_Transform(tm);

				WW3D::Render( W3DDisplay::m_3DInterfaceScene, m_camera );
			}
		}
	}

	//@todo: In DX8 mode the mouse is drawn in another thread which isn't allowed
	//access to D3D so we can't do any drawing here.
	// draw the cursor text
	if (!isThread)
		drawCursorText();

	// draw tooltip text
	if (m_visible && !isThread)
		drawTooltip();

	m_drawing = FALSE;
}

void W3DMouse::setRedrawMode(RedrawMode mode)
{
	MouseCursor cursor = getMouseCursor();

	//Turn off the previous cursor mode
	setCursor(NONE);

	m_currentRedrawMode=mode;

	switch (mode)
	{
		case RM_WINDOWS:
		{	//Windows mouse doesn't need an update thread.
			if (thread.Is_Running())
				thread.Stop();
			freeD3DAssets();	//using Windows resources
			freeW3DAssets();
			freePolygonAssets();
			m_currentD3DCursor = NONE;
			m_currentW3DCursor = NONE;
			m_currentPolygonCursor = NONE;
		}
		break;

		case RM_W3D:
		{	//Model mouse updated only at render time so doesn't
			//require thread.
			if (thread.Is_Running())
				thread.Stop();
			freeD3DAssets();	//using packed Image data, not textures.
			freePolygonAssets();
			m_currentD3DCursor = NONE;
			m_currentPolygonCursor = NONE;
			initW3DAssets();
		}
		break;

		case RM_POLYGON:
		{	//Polygon mouse updated only at render time so doesn't
			//require thread.
			if (thread.Is_Running())
				thread.Stop();
			freeD3DAssets();	//using packed Image data, not textures.
			freeW3DAssets();
			m_currentD3DCursor = NONE;
			m_currentW3DCursor = NONE;
			m_currentPolygonCursor = NONE;
			initPolygonAssets();
		}
		break;

		case RM_DX8:
		{	//this cursor type is drawn by DX8 and can be refreshed
			//independent of rendering rate.  Uses another thread to do
			//position updates.
			initD3DAssets();	//make sure textures loaded.
			freeW3DAssets();
			freePolygonAssets();
			if (!thread.Is_Running())
				thread.Execute();
			m_currentW3DCursor = NONE;
			m_currentPolygonCursor = NONE;
			break;
		}
	}

	setCursor(NONE);
	setCursor(cursor);
}

void W3DMouse::setCursorDirection(MouseCursor cursor)
{
	Coord2D offset = {0, 0};
	//Check if we have a directional cursor that needs different images for each direction
	if (m_cursorInfo[cursor].numDirections > 1 && TheInGameUI && TheInGameUI->isScrolling())
	{
		offset = TheInGameUI->getScrollAmount();
		if (offset.x || offset.y)
		{
			offset.normalize();
			Real theta = atan2(offset.y, offset.x);
			theta = fmod(theta+M_PI*2,M_PI*2);
			Int numDirections=m_cursorInfo[m_currentCursor].numDirections;
			//Figure out which of our predrawn cursor orientations best matches the
			//actual cursor direction.  Frame 0 is assumed to point right and continue
			//clockwise.
			m_directionFrame=(Int)(theta/(2.0f*M_PI/(Real)numDirections)+0.5f);
			if (m_directionFrame >= numDirections)
				m_directionFrame = 0;
		}
		else
		{
			m_directionFrame=0;
		}
	}
	else
		m_directionFrame = 0;
}
