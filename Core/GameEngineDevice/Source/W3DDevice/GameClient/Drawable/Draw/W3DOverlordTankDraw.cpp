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

// FILE: W3DOverlordTankDraw.cpp ////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, October 2002
// Desc: The Overlord has a super specific special need.  He needs his rider to draw explicitly after him,
// and he needs direct access to get that rider when everyone else can't see it because of the OverlordContain.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ContainModule.h"
#include "W3DDevice/GameClient/Module/W3DOverlordTankDraw.h"

//-------------------------------------------------------------------------------------------------
W3DOverlordTankDrawModuleData::W3DOverlordTankDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
W3DOverlordTankDrawModuleData::~W3DOverlordTankDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
void W3DOverlordTankDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DTankDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ nullptr, nullptr, nullptr, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DOverlordTankDraw::W3DOverlordTankDraw( Thing *thing, const ModuleData* moduleData )
: W3DTankDraw( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DOverlordTankDraw::~W3DOverlordTankDraw()
{
}

//-------------------------------------------------------------------------------------------------
void W3DOverlordTankDraw::doDrawModule(const Matrix3D* transformMtx)
{
	W3DTankDraw::doDrawModule(transformMtx);

	// Our big thing is that we get our specific passenger (the turret thing) and then wake it up and make it draw
	// It depends on us because our renderObject is only made correct in the act of drawing.
	Object *me = getDrawable()->getObject();
	if( me
		&& me->getContain()
		&& me->getContain()->friend_getRider()
		&& me->getContain()->friend_getRider()->getDrawable()
		)
	{
		Drawable *riderDraw = me->getContain()->friend_getRider()->getDrawable();
		riderDraw->setColorTintEnvelope( *getDrawable()->getColorTintEnvelope() );

		riderDraw->notifyDrawableDependencyCleared();
		riderDraw->draw();
	}
}

//-------------------------------------------------------------------------------------------------
void W3DOverlordTankDraw::setHidden(Bool h)
{
	W3DTankDraw::setHidden(h);

	// We need to hide our rider, since he won't realize he's being contained in a contained container
	Object *me = getDrawable()->getObject();
	if( me
		&& me->getContain()
		&& me->getContain()->friend_getRider()
		&& me->getContain()->friend_getRider()->getDrawable()
		)
	{
		me->getContain()->friend_getRider()->getDrawable()->setDrawableHidden(h);
	}
}

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DOverlordTankDraw::crc( Xfer *xfer )
{

	// extend base class
	W3DTankDraw::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DOverlordTankDraw::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	W3DTankDraw::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DOverlordTankDraw::loadPostProcess( void )
{

	// extend base class
	W3DTankDraw::loadPostProcess();

}
