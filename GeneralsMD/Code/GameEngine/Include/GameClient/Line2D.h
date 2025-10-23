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

// FILE: Line2D.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, January 2002
// Desc:   2D line helping stuff
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

typedef std::vector<ICoord2D> Coord2DVector;
typedef std::vector<ICoord3D> Coord3DVector;


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////////
extern Bool ClipLine2D( ICoord2D *p1, ICoord2D *p2, ICoord2D *c1, ICoord2D *c2,
												IRegion2D *clipRegion );

///< IntersectLine2D will take two segments delimited by ab and cd and will return whether
///< they intersect within the length of ab. They will also return the intersection point out
///< intersection if it is non-NULL.
extern Bool IntersectLine2D( const Coord2D *a, const Coord2D *b,
															const Coord2D *c, const Coord2D *d,
															Coord2D *intersection = NULL);

///< PointInsideRect2D will return true iff inputPoint lies iside of the rectangle specified
///< by bl, tl, br, tr.
extern Bool PointInsideRect2D( const Coord2D *bl, const Coord2D *tl,
															 const Coord2D *br, const Coord2D *tr,
															 const Coord2D *inputPoint);

///< Checks if a point is inside a perfect rectangle (top left and bottom right)
extern Bool Coord3DInsideRect2D( const Coord3D *inputPoint, const Coord2D *tl, const Coord2D *br );

///< Scales a rect by a factor either growing or shrinking it.
extern void ScaleRect2D( Coord2D *tl, Coord2D *br, Real scaleFactor );

/** PointInsideRect3D will return true iff inputPoint lies iside of the rectangle specified
by bl, tl, br, tr. It does not actually consider the Z value, it is merely a convenience function
for calling PointInsideRect2D */
extern Bool PointInsideRect3D( const Coord3D *bl, const Coord3D *tl,
															 const Coord3D *br, const Coord3D *tr,
															 const Coord3D *inputPoint);


///< This function will take the ptToTest and will perform even-odd checking against the area.
///< If the area is not closed, it will be closed for this check.
extern Bool PointInsideArea2D( const Coord2D *ptToTest,
															 const Coord2D *area,
															 Int numPointsInArea);

///< This function will take the ptToTest and will perform even-odd checking against the area.
///< The area and the ptToTest will be flattened first, so a 2-D check will be sufficient.
///< This function is only for convenience so that points do not need to first be flattened.
extern Bool PointInsideArea2D( const Coord3D *ptToTest,
															 const Coord3D *area,
															 Int numPointsInArea);

///< This function will find the shortest distance between the given segment (ab) and the pt.
///< It will also give the intersection points on the segment (ab) if desired.
///< outU will return the U value determined. This is a shortcut for panning
extern void ShortestDistancePointToSegment2D( const Coord2D *a, const Coord2D *b, const Coord2D *pt,
																							Real *outDistance, Coord2D *outPosition, Real *outU );
