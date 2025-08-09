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

// FILE: W3DTruckDraw.h ////////////////////////////////////////////////////////////////////////////
// Draw a tank
// Author: John Ahlquist, March 2002
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _W3D_TRUCK_DRAW_H_
#define _W3D_TRUCK_DRAW_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/DrawModule.h"
#include "Common/AudioEventRTS.h"
#include "GameClient/ParticleSys.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "WW3D2/hanim.h"
#include "WW3D2/rendobj.h"
#include "WW3D2/part_emt.h"

//-------------------------------------------------------------------------------------------------
class W3DTruckDrawModuleData : public W3DModelDrawModuleData
{
public:
	AsciiString m_dustEffectName;
	AsciiString m_dirtEffectName;
	AsciiString m_powerslideEffectName;

	AsciiString m_frontLeftTireBoneName;
	AsciiString m_frontRightTireBoneName;
	AsciiString m_rearLeftTireBoneName;
	AsciiString m_rearRightTireBoneName;
	//4 extra tires to support up to 8 tires.
	AsciiString m_midFrontLeftTireBoneName;
	AsciiString m_midFrontRightTireBoneName;
	AsciiString m_midRearLeftTireBoneName;
	AsciiString m_midRearRightTireBoneName;
	//And some more
	AsciiString m_midMidLeftTireBoneName;
	AsciiString m_midMidRightTireBoneName;

	// Cab bone for a segmented truck.
	AsciiString m_cabBoneName;
	AsciiString m_trailerBoneName;
	Real				m_cabRotationFactor;
	Real				m_trailerRotationFactor;
	Real				m_rotationDampingFactor;


	Real				m_rotationSpeedMultiplier;
	Real				m_powerslideRotationAddition;

	W3DTruckDrawModuleData();
	~W3DTruckDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class W3DTruckDraw : public W3DModelDraw
{

 	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DTruckDraw, "W3DTruckDraw" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( W3DTruckDraw, W3DTruckDrawModuleData )

public:

	W3DTruckDraw( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void setHidden(Bool h);
	virtual void doDrawModule(const Matrix3D* transformMtx);
	virtual void setFullyObscuredByShroud(Bool fullyObscured);
	virtual void reactToGeometryChange() { }

protected:
	virtual void onRenderObjRecreated(void);

protected:
	Bool						m_effectsInitialized;
	Bool						m_wasAirborne;
	Bool						m_isPowersliding;
	/// debris emitters for when tank is moving
	ParticleSystem* m_dustEffect;
	ParticleSystem* m_dirtEffect;
	ParticleSystem* m_powerslideEffect;

	Real						m_frontWheelRotation;
	Real						m_rearWheelRotation;
	Real						m_midFrontWheelRotation;
	Real						m_midRearWheelRotation;

	Int							m_frontLeftTireBone;
	Int							m_frontRightTireBone;
	Int							m_rearLeftTireBone;
	Int							m_rearRightTireBone;
	//4 extra tires to support up to 8 tires
	Int							m_midFrontLeftTireBone;
	Int							m_midFrontRightTireBone;
	Int							m_midRearLeftTireBone;
	Int							m_midRearRightTireBone;
	//And some more
	Int							m_midMidLeftTireBone;
	Int							m_midMidRightTireBone;

	Int							m_cabBone;
	Real						m_curCabRotation;
	Int							m_trailerBone;
	Real						m_curTrailerRotation;

	Int							m_prevNumBones;
	AudioEventRTS		m_powerslideSound;
	AudioEventRTS		m_landingSound;

	RenderObjClass *m_prevRenderObj;

	void createEmitters( void );					///< Create particle effects.
	void tossEmitters( void );					///< Create particle effects.
	void enableEmitters( Bool enable );						///< stop creating debris from the tank treads
	void updateBones( void );
};

#endif // _W3D_TRUCK_DRAW_H_

