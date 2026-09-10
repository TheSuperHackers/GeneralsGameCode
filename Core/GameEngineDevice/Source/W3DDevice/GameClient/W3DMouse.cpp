#include "W3DDevice/GameClient/W3DRenderServices.h"
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


#include <span>
#include "Common/GameMemory.h"
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DCamera.h"

#include "W3DDevice/GameClient/W3DAssetCatalog.h"

#include "W3DDevice/Common/W3DConvert.h"
#include "W3DDevice/GameClient/W3DMouse.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "GameClient/Display.h"
#include "GameClient/Image.h"
#include "GameClient/InGameUI.h"
#include <SDL3/SDL.h>
#include "WWLib/ffactory.h"
#include <string>
#include <vector>
import Assets.Cache.Animations;
import Graphics.Cursors.Load;


//Since there can't be more than 1 mouse, might as well keep these static.
static const Image *cursorImages[Mouse::NUM_MOUSE_CURSORS];			///<Images for use with the RM_POLYGON method.
static W3DRenderObject *cursorModels[Mouse::NUM_MOUSE_CURSORS];	///< W3D models for each cursor type
static Assets::AnimationAssetHandle cursorAnims[Mouse::NUM_MOUSE_CURSORS];		///< W3D animations for each cursor type

W3DMouse::W3DMouse()
{
	// zero our event list
	for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		cursorModels[i]=nullptr;
		cursorAnims[i]=nullptr;
	}

	m_currentHardwareCursor=NONE;
	m_currentW3DCursor=NONE;
	m_currentPolygonCursor=NONE;
	m_camera = nullptr;

}

W3DMouse::~W3DMouse()
{
	SDL_HideCursor();
	m_hardwareCursor.Reset();
	freeW3DAssets();
}

void W3DMouse::initPolygonAssets()
{


	//Check if texture assets already loaded
	if (m_currentRedrawMode == RM_POLYGON && cursorImages[1] == nullptr)
	{
		for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
		{
			m_currentPolygonCursor = m_currentCursor;
			if (!m_cursorInfo[i].imageName.isEmpty())
				cursorImages[i]=TheMappedImageCollection->findImageByName(m_cursorInfo[i].imageName);
		}
	}
}

void W3DMouse::freePolygonAssets()
{

	for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		cursorImages[i]=nullptr;
	}
}

Bool W3DMouse::loadHardwareCursor(MouseCursor cursor)
{
	const auto& info = m_cursorInfo[cursor];
	return Graphics::Load_Cursor(m_hardwareCursor,
		{info.textureName.str(), info.numFrames, info.fps, info.hotSpotPosition.x, info.hotSpotPosition.y},
		[](std::string_view path, std::vector<std::byte>& bytes) {
			if (!_TheFileFactory) return false;
			const std::string name(path);
			file_auto_ptr file(_TheFileFactory, name.c_str());
			if (!file.get() || !file->Is_Available() || !file->Open()) return false;
			const int size = file->Size();
			if (size <= 0) return false;
			bytes.resize(size);
			return file->Read(bytes.data(), size) == size;
		});
}

void W3DMouse::applyCursor()
{
	if (m_currentRedrawMode == RM_SYSTEM)
	{
		SDL3Mouse::applyCursor();
		return;
	}
	if (m_currentRedrawMode == RM_HARDWARE && m_currentHardwareCursor != NONE
		&& m_visible && m_cursorFocused && m_hardwareCursor.Show()) return;
	SDL_HideCursor();
}

void W3DMouse::initW3DAssets()
{


	//Check if model assets already loaded
	if ((cursorModels[1] == nullptr && W3DDisplay::m_assetManager))
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
	if ((cursorAnims[1] == nullptr && W3DDisplay::m_assetManager))
	{
		for (Int i=1; i<NUM_MOUSE_CURSORS; i++)
		{
			if (!m_cursorInfo[i].W3DAnimName.isEmpty())
			{
				DEBUG_ASSERTCRASH(cursorAnims[i] == nullptr, ("hmm, leak festival"));
					cursorAnims[i] = W3DDisplay::m_assetManager->Catalog().Acquire_Animation(m_cursorInfo[i].W3DAnimName.str());
				if (cursorAnims[i] && cursorModels[i])
				{
					cursorModels[i]->Set_Animation(cursorAnims[i], 0, (m_cursorInfo[i].loop) ? W3DRenderObject::ANIM_MODE_LOOP : W3DRenderObject::ANIM_MODE_ONCE);
				}
			}
		}
	}

	// create the camera
	m_camera = NEW_REF( W3DCamera, () );
	m_camera->Set_Position( Vector3( 0, 1, 1 ) );
	Vector2 min = Vector2( -1, -1 );
	Vector2 max = Vector2( +1, +1 );
	m_camera->Set_View_Plane( min, max );
	m_camera->Set_Clip_Planes( 0.995f, 20.0f );
	if (m_orthoCamera)
		m_camera->Set_Projection_Type( W3DCamera::ORTHO );
}

void W3DMouse::freeW3DAssets()
{

	for (Int i=0; i<NUM_MOUSE_CURSORS; i++)
	{
		if (W3DDisplay::m_3DInterfaceScene && cursorModels[i])
		{
			W3DDisplay::m_3DInterfaceScene->Remove_Render_Object(cursorModels[i]);
		}
		REF_PTR_RELEASE(cursorModels[i]);
		Assets::Release_Animation(cursorAnims[i]);
	}

	REF_PTR_RELEASE(m_camera);
}

//-------------------------------------------------------------------------------------------------
/** Initialize our device */
//-------------------------------------------------------------------------------------------------
void W3DMouse::init()
{

	//check if system already initialized and texture assets loaded.
	SDL3Mouse::init();
	setCursor(ARROW);	//set default starting cursor image


}

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void W3DMouse::reset()
{

	// extend
	SDL3Mouse::reset();

}

//-------------------------------------------------------------------------------------------------
/** Super basic simplistic cursor */
//-------------------------------------------------------------------------------------------------
void W3DMouse::setCursor( MouseCursor cursor )
{


	m_directionFrame=0;
	if (m_currentRedrawMode == RM_SYSTEM)
	{	//The system cursor is refreshed whenever its selection changes.
		m_currentHardwareCursor=NONE;
		m_currentW3DCursor=NONE;
		m_currentPolygonCursor=NONE;
		setCursorDirection(cursor);
		// SDL owns the system cursor, so it is safe to apply the selection
		// immediately.
		SDL3Mouse::setCursorWithDirection(cursor, m_directionFrame);
		m_currentCursor = cursor;
		return;
	}

	// extend
	Mouse::setCursor( cursor );

	// if we're already on this cursor ignore the rest of code to stop cursor flickering.
	if( m_currentCursor == cursor && m_currentHardwareCursor == cursor)
	{
		applyCursor();
		return;
	}

	if (m_currentRedrawMode == RM_HARDWARE)
	{
		m_hardwareCursor.Reset();
		m_currentHardwareCursor = NONE;
		if (cursor != NONE)
		{
			m_currentHardwareCursor = cursor;
			if (!loadHardwareCursor(cursor)) DEBUG_LOG(("Unable to load hardware cursor %s", m_cursorInfo[cursor].textureName.str()));
		}
	}
	else if (m_currentRedrawMode == RM_POLYGON)
	{
		SDL_HideCursor();	//Kill the SDL cursor while the polygon cursor is active.
		m_currentHardwareCursor=NONE;
		m_currentW3DCursor=NONE;
		m_currentPolygonCursor = cursor;
		m_currentHotSpot = m_cursorInfo[cursor].hotSpotPosition;
	}
	else if (m_currentRedrawMode == RM_W3D)
	{
		SDL_HideCursor();	//Kill the SDL cursor while the W3D cursor is active.
		m_currentHardwareCursor=NONE;
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
						cursorModels[m_currentW3DCursor]->Set_Animation(cursorAnims[m_currentW3DCursor], 0, W3DRenderObject::ANIM_MODE_ONCE);
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
	applyCursor();

}

void W3DMouse::draw()
{


	//make sure the correct cursor image is selected
	setCursor(m_currentCursor);

	if (m_currentRedrawMode == RM_POLYGON)
	{
		const Image *image=cursorImages[m_currentPolygonCursor];
		if (image)
		{
			TheDisplay->drawImage(image,m_currMouse.pos.x-m_currentHotSpot.x,m_currMouse.pos.y-m_currentHotSpot.y,
				m_currMouse.pos.x+image->getImageWidth()-m_currentHotSpot.x, m_currMouse.pos.y+image->getImageHeight()-m_currentHotSpot.y);
		}
	}
	else if (m_currentRedrawMode == RM_SYSTEM)
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

				Get_W3D_Render_Services().Render( W3DDisplay::m_3DInterfaceScene, m_camera );
			}
		}
	}

	drawCursorText();
	if (m_visible) drawTooltip();

}

void W3DMouse::setRedrawMode(RedrawMode mode)
{
	MouseCursor cursor = getMouseCursor();

	//Turn off the previous cursor mode
	setCursor(NONE);

	m_currentRedrawMode=mode;

	m_hardwareCursor.Reset();
	freeW3DAssets();
	freePolygonAssets();
	m_currentHardwareCursor = NONE;
	m_currentW3DCursor = NONE;
	m_currentPolygonCursor = NONE;
	if (mode == RM_W3D) initW3DAssets();
	else if (mode == RM_POLYGON) initPolygonAssets();

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
