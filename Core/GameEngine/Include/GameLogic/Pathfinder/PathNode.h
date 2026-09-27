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

#pragma once

#include "Common/GameMemory.h"

/**
 * PathNodes are used to create a final Path to return from the
 * pathfinder.  Note that these are not used during the A* search.
 */
class PathNode : public MemoryPoolObject
{
public:
	PathNode();

	Coord3D *getPosition() { return &m_pos; }			///< return position of this node
	const Coord3D *getPosition() const { return &m_pos; }			///< return position of this node
	void setPosition( const Coord3D *pos ) { m_pos = *pos; }	///< set the position of this path node

	const Coord3D *computeDirectionVector();			///< compute direction to next node

	PathNode *getNext() { return m_next; }				///< return next node in the path
	PathNode *getPrevious() { return m_prev; }		///< return previous node in the path
	const PathNode *getNext() const { return m_next; }				///< return next node in the path
	const PathNode *getPrevious() const { return m_prev; }		///< return previous node in the path

	PathfindLayerEnum getLayer() const { return m_layer; }				///< return layer of this node.
	void setLayer( PathfindLayerEnum layer ) { m_layer = layer; }	///< set the layer of this path node

	void setNextOptimized( PathNode *node );

	PathNode *getNextOptimized(Coord2D* dir = nullptr, Real* dist = nullptr)  	///< return next node in optimized path
	{
		if (dir)
			*dir = m_nextOptiDirNorm2D;
		if (dist)
			*dist = m_nextOptiDist2D;
		return m_nextOpti;
	}

	const PathNode *getNextOptimized(Coord2D* dir = nullptr, Real* dist = nullptr) const  	///< return next node in optimized path
	{
		if (dir)
			*dir = m_nextOptiDirNorm2D;
		if (dist)
			*dist = m_nextOptiDist2D;
		return m_nextOpti;
	}

	void setCanOptimize(Bool canOpt) { m_canOptimize = canOpt;}
	Bool getCanOptimize() const { return m_canOptimize;}

	/// given a list, prepend this node, return new list
	PathNode *prependToList( PathNode *list );

	/// given a node, append to this node
	void append( PathNode *list );

public:
	mutable Int					m_id; // Used in Path::xfer() to save & recreate the path list.

private:
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( PathNode, "PathNodePool"  );		///< @todo Set real numbers for mem alloc

	PathNode*						m_nextOpti;													///< next node in the optimized path
	PathNode*						m_next;															///< next node in the path
	PathNode*						m_prev;															///< previous node in the path
	Coord3D							m_pos;															///< position of node in space
	PathfindLayerEnum		m_layer;														///< Layer for this section.
	Bool								m_canOptimize;											///< True if this cell can be optimized out.

	Real								m_nextOptiDist2D;										///< if nextOpti is nonnull, the dist to it.
	Coord2D							m_nextOptiDirNorm2D;								///< if nextOpti is nonnull, normalized dir vec towards it.

};
