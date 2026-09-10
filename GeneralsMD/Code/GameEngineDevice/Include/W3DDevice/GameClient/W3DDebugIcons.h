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

#pragma once

#include "WWLib/always.h"
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "Lib/BaseType.h"

#if defined(RTS_DEBUG)
import Graphics.Scene.Surfaces.Renderer;
struct DebugIcon;
//
/// W3DDebugIcons: Draws huge numbers of debug icons for pathfinding quickly.
//
//
class W3DDebugIcons : public W3DRenderObject
{

public:

	W3DDebugIcons(Int mapWidth, Int mapHeight);
	W3DDebugIcons(const W3DDebugIcons & src);
	W3DDebugIcons & operator = (const W3DDebugIcons &);
	~W3DDebugIcons();

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface
	/////////////////////////////////////////////////////////////////////////////
	virtual W3DRenderObject *	Clone() const;
	virtual int						Class_ID() const;
	virtual void					Render(W3DRenderContext & rinfo);

	virtual bool					Cast_Ray(W3DRayCastQuery & raytest);

	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
  virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & aabox) const;

protected:
	Graphics::SurfaceMeshHandle m_mesh;

protected:
	static DebugIcon        *m_debugIcons;
	static Int              m_numDebugIcons;
	static Int              m_maxDebugIcons;

protected:
	void allocateIconsArray();
	void compressIconsArray();

public:
	static void addIcon(const Coord3D *pos, Real width, Int numFramesDuration, RGBColor color);
};
#endif // RTS_DEBUG
