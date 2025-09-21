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

// FILE: DynamicGeometryClientUpdate.cpp //////////////////////////////////////////////////////////////////
// Author: Andi W, August 2025
// Desc:   Simple Alpha/Scale transition
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/Drawable.h"
#include "GameClient/Module/DynamicGeometryClientUpdate.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"


//-------------------------------------------------------------------------------------------------
DynamicGeometryClientUpdateModuleData::DynamicGeometryClientUpdateModuleData()
{
	m_scaleInitial = 1.0;
	m_scaleMidpoint = 1.0;
	m_scaleFinal = 1.0;

	m_alphaInitial = 1.0;
	m_alphaMidpoint = 1.0;
	m_alphaFinal = 1.0;

	m_totalFrames = 30;
	m_midpointFrames = 15;
}

//-------------------------------------------------------------------------------------------------
DynamicGeometryClientUpdateModuleData::~DynamicGeometryClientUpdateModuleData()
{
}

//-------------------------------------------------------------------------------------------------
enum DynamicGeometryPhaseType CPP_11(: Int)
{
	DGPHASE_INITIAL = 0,
	DGPHASE_MIDPOINT,
	DGPHASE_FINAL
};

static const char* TheDynamicGeometryPhaseNames[] =
{
	"INITIAL",
	"MIDPOINT",
	"FINAL"
};
// -----------------------------------------------------------------------------------------------
enum DynamicGeometryInterpolationType CPP_11(: Int)
{
	DGINTERP_LINEAR = 0,
	DGINTERP_SMOOTH
};

static const char* TheInterpolationTypeNames[] =
{
	"LINEAR",
	"SMOOTH"
};

//-------------------------------------------------------------------------------------------------
static void parseAlpha(INI* ini, void* instance, void* /*store*/, const void* /*userData*/)
{
	DynamicGeometryClientUpdateModuleData* self = (DynamicGeometryClientUpdateModuleData*)instance;
	DynamicGeometryPhaseType phase = (DynamicGeometryPhaseType)INI::scanIndexList(ini->getNextToken(), TheDynamicGeometryPhaseNames);

	const char* token = ini->getNextToken();
	Real value = INI::scanReal(token);

	if (phase == DGPHASE_INITIAL)
		self->m_alphaInitial = value;
	else if (phase == DGPHASE_MIDPOINT)
		self->m_alphaMidpoint = value;
	else if (phase == DGPHASE_FINAL)
		self->m_alphaFinal = value;
}

//-------------------------------------------------------------------------------------------------
static void parseScale(INI* ini, void* instance, void* /*store*/, const void* /*userData*/)
{
	DynamicGeometryClientUpdateModuleData* self = (DynamicGeometryClientUpdateModuleData*)instance;
	DynamicGeometryPhaseType phase = (DynamicGeometryPhaseType)INI::scanIndexList(ini->getNextToken(), TheDynamicGeometryPhaseNames);

	const char* token = ini->getNextToken();
	Real value = INI::scanReal(token);

	if (phase == DGPHASE_INITIAL)
		self->m_scaleInitial = value;
	else if (phase == DGPHASE_MIDPOINT)
		self->m_scaleMidpoint = value;
	else if (phase == DGPHASE_FINAL)
		self->m_scaleFinal = value;
}

//-------------------------------------------------------------------------------------------------
void DynamicGeometryClientUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ClientUpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Opacity", parseAlpha, NULL, NULL},
		{ "Scale", parseScale, NULL, NULL},
		{ "Interpolation", INI::parseIndexList, TheInterpolationTypeNames,  offsetof(DynamicGeometryClientUpdateModuleData, m_interpolationType)},
		{ "TotalDuration",	INI::parseDurationUnsignedInt, NULL, offsetof(DynamicGeometryClientUpdateModuleData, m_totalFrames) },
		{ "MidpointDuration",	INI::parseDurationUnsignedInt, NULL, offsetof(DynamicGeometryClientUpdateModuleData, m_midpointFrames) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DynamicGeometryClientUpdate::DynamicGeometryClientUpdate( Thing *thing, const ModuleData* moduleData ) :
	ClientUpdateModule( thing, moduleData )
{
	m_startFrame = TheGameLogic->getFrame();
	m_overrideScale = 1.0;
	m_overrideAlpha = 1.0;
	m_prevScale = 1.0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DynamicGeometryClientUpdate::~DynamicGeometryClientUpdate( void )
{

}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/** The client update callback. */
//-------------------------------------------------------------------------------------------------
void DynamicGeometryClientUpdate::clientUpdate( void )
{
	Drawable *draw = getDrawable();

	if (!draw)
		return;

	const DynamicGeometryClientUpdateModuleData* data = getDynamicGeometryClientUpdateModuleData();

	UnsignedInt now = TheGameLogic->getFrame();

	Real alpha0, alpha1, scale0, scale1, progress;
	if (now < m_startFrame + data->m_midpointFrames) {
		alpha0 = data->m_alphaInitial;
		alpha1 = data->m_alphaMidpoint;
		scale0 = data->m_scaleInitial;
		scale1 = data->m_scaleMidpoint;
		progress = INT_TO_REAL(now - m_startFrame) / INT_TO_REAL(data->m_midpointFrames);
	}
	else if (now < m_startFrame + data->m_totalFrames) {
		alpha0 = data->m_alphaMidpoint;
		alpha1 = data->m_alphaFinal;
		scale0 = data->m_scaleMidpoint;
		scale1 = data->m_scaleFinal;
		progress = INT_TO_REAL(now - (m_startFrame + data->m_midpointFrames)) / INT_TO_REAL(data->m_totalFrames - data->m_midpointFrames);
	}
	else {
		// We are done
		return;
	}

	if (data->m_interpolationType == DGINTERP_SMOOTH) {
		progress = 0.5 - (Cos(progress * PI) * 0.5);
	}

	Real alpha = (1.0 - progress) * alpha0 + progress * alpha1;
	Real scale = (1.0 - progress) * scale0 + progress * scale1;

	Real newScale = m_overrideScale * scale;

	Real currentScale = draw->getInstanceScale();
	if (m_prevScale != 1.0) {
		currentScale = currentScale / MAX(0.001, m_prevScale);
	}

	draw->setInstanceScale(currentScale * newScale);

	m_prevScale = newScale;

	// This doesn't work for additive!
	draw->setDrawableOpacity(m_overrideAlpha * alpha);

	// For additive with emissive color, this seems to work.
	// But apparently not for all of them, and only for Static sorting?
	draw->setEmissiveOpacityScaling(true);

	// This doesn't seem to work for all models either
	//Real factor = (1.0 - alpha) * -2.5;
	//RGBColor color;
	//color.red = factor;
	//color.green = factor;
	//color.blue = factor;
	//draw->colorTint(&color);

	// DEBUG_LOG((">>> DGCU - progress = %f, alpha = %f, scale = %f, effectiveOpacity = %f, factor = %f", progress, alpha, scale, draw->getEffectiveOpacity(), factor));

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void DynamicGeometryClientUpdate::crc( Xfer *xfer )
{

	// extend base class
	ClientUpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void DynamicGeometryClientUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	ClientUpdateModule::xfer( xfer );

	// start frame
	xfer->xferUnsignedInt(&m_startFrame);

	// override scale value
	xfer->xferReal(&m_overrideScale);

	// override alpha value
	xfer->xferReal(&m_overrideAlpha);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void DynamicGeometryClientUpdate::loadPostProcess( void )
{

	// extend base class
	ClientUpdateModule::loadPostProcess();


}  // end loadPostProcess
