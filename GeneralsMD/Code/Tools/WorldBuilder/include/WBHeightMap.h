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


#ifndef __WBHEIGHTMAP_H_
#define __WBHEIGHTMAP_H_

#include "W3DDevice/GameClient/FlatHeightMap.h"
#include "W3DDevice/GameClient/HeightMap.h"
#define dont_USE_FLAT_HEIGHT_MAP // Use the origina height map for mission disk. jba. [4/15/2003]
#ifdef USE_FLAT_HEIGHT_MAP
class WBHeightMap : public FlatHeightMapRenderObjClass
#else
class WBHeightMap : public HeightMapRenderObjClass
#endif
{

public:
	WBHeightMap(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface (W3D methods)
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(RenderInfoClass & rinfo);
	virtual Bool					Cast_Ray(RayCollisionTestClass & raytest);

	virtual Real getHeightMapHeight(Real x, Real y, Coord3D* normal);	///<return height and normal at given point
	virtual Real getMaxCellHeight(Real x, Real y);	///< returns maximum height of the 4 cell corners.

	void setDrawEntireMap(Bool entire) {m_drawEntireMap = entire;};
	Bool getDrawEntireMap(void) {return m_drawEntireMap;};
	void setFlattenHeights(Bool flat);

protected:
	void flattenHeights(void);
protected:
	Bool m_drawEntireMap;
	Bool m_flattenHeights;
};

#endif  // end __WBHEIGHTMAP_H_
