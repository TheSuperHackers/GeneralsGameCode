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

// FILE: W3DProjectileStreamDraw.cpp ////////////////////////////////////////////////////////////
// Tile a texture strung between Projectiles
// Graham Smallwood, May 2002
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Object.h"
#include "W3DDevice/GameClient/Module/W3DProjectileStreamDraw.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/segline.h"
#include "WWMath/vector3.h"

//-------------------------------------------------------------------------------------------------
W3DProjectileStreamDrawModuleData::W3DProjectileStreamDrawModuleData()
{
	m_textureName = "";
	m_width = 0.0f;
	m_tileFactor = 0.0f;
	m_scrollRate = 0.0f;
	m_maxSegments = 0;
}

//-------------------------------------------------------------------------------------------------
W3DProjectileStreamDrawModuleData::~W3DProjectileStreamDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
void W3DProjectileStreamDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Texture",			INI::parseAsciiString,	NULL, offsetof(W3DProjectileStreamDrawModuleData, m_textureName) },
		{ "Width",				INI::parseReal,					NULL, offsetof(W3DProjectileStreamDrawModuleData, m_width) },
		{ "TileFactor",		INI::parseReal,					NULL, offsetof(W3DProjectileStreamDrawModuleData, m_tileFactor) },
		{ "ScrollRate",		INI::parseReal,					NULL, offsetof(W3DProjectileStreamDrawModuleData, m_scrollRate) },
		{ "MaxSegments",	INI::parseInt,					NULL, offsetof(W3DProjectileStreamDrawModuleData, m_maxSegments) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DProjectileStreamDraw::~W3DProjectileStreamDraw()
{
	for( Int lineIndex = 0; lineIndex < m_linesValid; lineIndex++ )
	{
		SegmentedLineClass *deadLine = m_allLines[lineIndex];
		if (deadLine)
		{	if (deadLine->Peek_Scene())
				W3DDisplay::m_3DScene->Remove_Render_Object( deadLine );
			REF_PTR_RELEASE( deadLine );
		}
	}

	REF_PTR_RELEASE( m_texture );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DProjectileStreamDraw::W3DProjectileStreamDraw( Thing *thing, const ModuleData* moduleData ) : DrawModule( thing, moduleData )
{
	const W3DProjectileStreamDrawModuleData* d = getW3DProjectileStreamDrawModuleData();
	m_texture = WW3DAssetManager::Get_Instance()->Get_Texture( d->m_textureName.str() );
	for( Int index = 0; index < MAX_PROJECTILE_STREAM; index++ )
		m_allLines[index] = NULL;
	m_linesValid = 0;
}

void W3DProjectileStreamDraw::setFullyObscuredByShroud(Bool fullyObscured)
{
	if (fullyObscured)
	{	//we need to remove all our lines from the scene because they are hidden
		for( Int lineIndex = 0; lineIndex < m_linesValid; lineIndex++ )
		{
			SegmentedLineClass *deadLine = m_allLines[lineIndex];
			if (deadLine && deadLine->Peek_Scene())
				deadLine->Remove();
		}
	}
	else
	{	//we need to restore lines into scene
		for( Int lineIndex = 0; lineIndex < m_linesValid; lineIndex++ )
		{
			SegmentedLineClass *deadLine = m_allLines[lineIndex];
			if (deadLine && !deadLine->Peek_Scene())
				W3DDisplay::m_3DScene->Add_Render_Object(deadLine);
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Map behavior states into W3D animations. */
//-------------------------------------------------------------------------------------------------
void W3DProjectileStreamDraw::doDrawModule(const Matrix3D* )
{
	// get object from logic
	Object *me = getDrawable()->getObject();
	if (me == NULL)
		return;

	static NameKeyType key_ProjectileStreamUpdate = NAMEKEY("ProjectileStreamUpdate");
	ProjectileStreamUpdate* update = (ProjectileStreamUpdate*)me->findUpdateModule(key_ProjectileStreamUpdate);

	const W3DProjectileStreamDrawModuleData *data = getW3DProjectileStreamDrawModuleData();

	Vector3 allPoints[MAX_PROJECTILE_STREAM];
	Int pointsUsed;

	update->getAllPoints( allPoints, &pointsUsed );

	Vector3 stagingPoints[MAX_PROJECTILE_STREAM];
	Vector3 zeroVector(0, 0, 0);

	Int linesMade = 0;
	Int currentMasterPoint = 0;
	UnsignedInt currentStagingPoint = 0;

	if( data->m_maxSegments )
	{
		// If I have a drawing cap, I need to increase the start point in the array.  The furthest (oldest)
		// point from the tank is in spot zero.
		currentMasterPoint = pointsUsed - data->m_maxSegments;
		currentMasterPoint = max( 0, currentMasterPoint ); // (but if they say to draw more than exists, draw all)
	}

	// Okay.  I have an array of ordered points that may have blanks in it.  I need to copy to the staging area
	// until I hit a blank or the end.  Then if I have a line made, I'll overwrite it, otherwise I'll make a new one.
	// I'll keep doing this until I run out of valid points.
	while( currentMasterPoint < pointsUsed )
	{
		while( currentMasterPoint < pointsUsed  &&  allPoints[currentMasterPoint] != zeroVector )
		{
			// While I am not looking at a bad point (off edge or zero)
			stagingPoints[currentStagingPoint] = allPoints[currentMasterPoint];// copy to the staging
			currentStagingPoint++;// increment how many I have
			currentMasterPoint++;// increment what I am looking at
		}
		// Use or reuse a line
		if( currentStagingPoint > 1 )
		{
			// Don't waste a line on a double hole (0) or a one point line (1)
			makeOrUpdateLine( stagingPoints, currentStagingPoint, linesMade );
			linesMade++;// keep track of how many are real this frame
		}
		currentMasterPoint++;//I am either pointed off the edge anyway, or I am pointed at a zero I want to skip
		currentStagingPoint = 0;//start over in the staging area
	}

	Int oldLinesValid = m_linesValid;
	for( Int lineIndex = linesMade; lineIndex < oldLinesValid; lineIndex++ )
	{
		// Delete any line we aren't using anymore.
		SegmentedLineClass *deadLine = m_allLines[lineIndex];
		if (deadLine->Peek_Scene())
			W3DDisplay::m_3DScene->Remove_Render_Object( deadLine );
		REF_PTR_RELEASE( deadLine );

		m_allLines[lineIndex] = NULL;
		m_linesValid--;
	}
}

void W3DProjectileStreamDraw::makeOrUpdateLine( Vector3 *points, UnsignedInt pointCount, Int lineIndex )
{
	Bool newLine = FALSE;

	if( m_allLines[lineIndex] == NULL )
	{
		//Need a new one if this is blank, otherwise I'll reset the existing one
		m_allLines[lineIndex] = NEW SegmentedLineClass;
		m_linesValid++;
		newLine = TRUE;
	}

	SegmentedLineClass *line = m_allLines[lineIndex];

	line->Set_Points(pointCount, points);	//tell the line which points to use

	if( newLine )
	{
		// This is one time stuff we only need to do if this is a new and not a change
		const W3DProjectileStreamDrawModuleData *data = getW3DProjectileStreamDrawModuleData();
		line->Set_Texture(m_texture);	//set the texture
		line->Set_Shader(ShaderClass::_PresetAdditiveSpriteShader);	//pick the alpha blending mode you want - see shader.h for others.
		line->Set_Width(data->m_width);	//set line width in world units
		line->Set_Texture_Mapping_Mode(SegLineRendererClass::TILED_TEXTURE_MAP);	//this tiles the texture across the line
		line->Set_Texture_Tile_Factor(data->m_tileFactor);	//number of times to tile texture across each segment
		line->Set_UV_Offset_Rate(Vector2(0.0f,data->m_scrollRate));	//amount to scroll texture on each draw
		W3DDisplay::m_3DScene->Add_Render_Object( line);	//add it to our scene so it gets rendered with other objects.
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DProjectileStreamDraw::crc( Xfer *xfer )
{

	// extend base class
	DrawModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DProjectileStreamDraw::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DrawModule::xfer( xfer );

	// Graham says there is no data that needs saving here

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DProjectileStreamDraw::loadPostProcess( void )
{

	// extend base class
	DrawModule::loadPostProcess();

}  // end loadPostProcess
