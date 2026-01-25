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

// FILE: W3DTerrainLogic.h ////////////////////////////////////////////////////////////////////////
// W3D implementation details for logical terrain
// Author: Colin Day, April 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameLogic/TerrainLogic.h"

//-------------------------------------------------------------------------------------------------
/** W3D specific implementation details for logical terrain ... we have this
  * because the logic and visual terrain are closely tied together in that
	* they represent the same thing, but need to be broken up into logical
	* and graphical representations */
//-------------------------------------------------------------------------------------------------
class W3DTerrainLogic : public TerrainLogic
{

public:

	W3DTerrainLogic();
	virtual ~W3DTerrainLogic();

	virtual void init( void );		///< Init
	virtual void reset( void );		///< Reset
	virtual void update( void );	///< Update

	/// @todo The loading of the raw height data should be device independent
	virtual Bool loadMap( AsciiString filename , Bool query );
	virtual void newMap( Bool saveGame );	///< Initialize the logic for new map.

	virtual Real getGroundHeight( Real x, Real y, Coord3D* normal = nullptr ) const;

	virtual Bool isCliffCell( Real x, Real y) const;			///< is point cliff cell.

	virtual Real getLayerHeight(Real x, Real y, PathfindLayerEnum layer, Coord3D* normal = nullptr, Bool clip = true) const;

	virtual void getExtent( Region3D *extent ) const ;					///< Get the 3D extent of the terrain in world coordinates

	virtual void getMaximumPathfindExtent( Region3D *extent ) const;

	virtual void getExtentIncludingBorder( Region3D *extent ) const;

	virtual Bool isClearLineOfSight(const Coord3D& pos, const Coord3D& posOther) const;

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	Real m_mapMinZ;	///< Minimum terrain z value.
	Real m_mapMaxZ;	///< Maximum terrain z value.

};
