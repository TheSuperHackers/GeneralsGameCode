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

// FILE: W3DDependencyModelDraw.cpp ////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, October 2002
// Desc: Draw module just like Model, except it can't draw unless somebody else explicitly says to, since they
// have to draw first.
//
// This draw module can be used in a general case (although I don't see why), m_attachToDrawableBoneInContainer
// is for the one present and main reason to use this module.  Our transport needs to tell us it is okay to
// draw after he draws.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ContainModule.h"
#include "W3DDevice/GameClient/Module/W3DDependencyModelDraw.h"





//-------------------------------------------------------------------------------------------------
W3DDependencyModelDrawModuleData::W3DDependencyModelDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
W3DDependencyModelDrawModuleData::~W3DDependencyModelDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
void W3DDependencyModelDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "AttachToBoneInContainer", INI::parseAsciiString, NULL, offsetof(W3DDependencyModelDrawModuleData, m_attachToDrawableBoneInContainer) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DDependencyModelDraw::W3DDependencyModelDraw( Thing *thing, const ModuleData* moduleData ) : W3DModelDraw( thing, moduleData )
{
	m_dependencyCleared = FALSE;
}

//-------------------------------------------------------------------------------------------------
W3DDependencyModelDraw::~W3DDependencyModelDraw()
{
}

//-------------------------------------------------------------------------------------------------
// All this does is stop the call path if we haven't been cleared to draw yet
void W3DDependencyModelDraw::doDrawModule(const Matrix3D* transformMtx)
{
	if( m_dependencyCleared )
	{
		// We've been cleared by the thing we were waiting to draw, so we can draw.
		W3DModelDraw::doDrawModule( transformMtx );
		m_dependencyCleared = FALSE;
	}
}

//-------------------------------------------------------------------------------------------------
void W3DDependencyModelDraw::notifyDrawModuleDependencyCleared( )
{
	m_dependencyCleared = TRUE;
}

// ------------------------------------------------------------------------------------------------
void W3DDependencyModelDraw::adjustTransformMtx(Matrix3D& mtx) const
{
	W3DModelDraw::adjustTransformMtx(mtx);

	// We have an additional adjustment to make, we want to use a bone in our container if there is one
	const Object *me = getDrawable()->getObject();
	const W3DDependencyModelDrawModuleData *md = getW3DDependencyModelDrawModuleData();

	if( md->m_attachToDrawableBoneInContainer.isNotEmpty()
		&& me
		&& me->getContainedBy()
		&& !me->getContainedBy()->getContain()->isEnclosingContainerFor(me)
		)
	{
		// If we are currently "riding on", then our client position is determined by the client position of
		// a particular bone in our container object.  Our logic position is updated by OpenContain.
		const Drawable *theirDrawable = me->getContainedBy()->getDrawable();
		if( theirDrawable )
		{
			Matrix3D theirBoneMtx;
			if( theirDrawable->getCurrentWorldspaceClientBonePositions( md->m_attachToDrawableBoneInContainer.str(), theirBoneMtx ) )
			{
				mtx = theirBoneMtx;
			}
			else
			{
				DEBUG_LOG(("m_attachToDrawableBoneInContainer %s not found",getW3DDependencyModelDrawModuleData()->m_attachToDrawableBoneInContainer.str()));
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DDependencyModelDraw::crc( Xfer *xfer )
{

	// extend base class
	W3DModelDraw::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DDependencyModelDraw::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	W3DModelDraw::xfer( xfer );

	// Dependency status
	xfer->xferBool( &m_dependencyCleared );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DDependencyModelDraw::loadPostProcess( void )
{

	// extend base class
	W3DModelDraw::loadPostProcess();

}  // end loadPostProcess


