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

// FILE: DrawableInfo.h ///////////////////////////////////////////////////////////////////////////////
// Simple structure used to bind W3D renderObjects to our own Drawables.
// Author: Mark Wilczynski, August 2002

#pragma once

#include "Common/GameType.h"

class Drawable;
class GhostObject;
class Object;

struct DrawableInfo
{
	enum ExtraRenderFlags
	{	ERF_IS_NORMAL=0,
		ERF_IS_OCCLUDED=	0x00000001,
		ERF_POTENTIAL_OCCLUDER=	0x00000002,
		ERF_POTENTIAL_OCCLUDEE=	0x00000004,
		ERF_IS_TRANSLUCENT= 0x00000008,
		ERF_IS_NON_OCCLUDER_OR_OCCLUDEE = 0x00000010,
		ERF_DELAYED_RENDER = ERF_IS_TRANSLUCENT|ERF_POTENTIAL_OCCLUDEE,
	};

	DrawableInfo(void) : m_shroudStatusObjectID(INVALID_ID), m_drawable(NULL), m_ghostObject(NULL), m_flags(ERF_IS_NORMAL) {}

	ObjectID m_shroudStatusObjectID;	///<since we sometimes have drawables without objects, this points to a parent object from which we pull shroud status.
	Drawable *m_drawable;	///<pointer back to drawable containing this DrawableInfo
	GhostObject *m_ghostObject;	///<pointer to ghostObject for this drawable used for fogged versions.
	Int m_flags;	///<extra render settings flags that are tied to render objects with drawables.
};
