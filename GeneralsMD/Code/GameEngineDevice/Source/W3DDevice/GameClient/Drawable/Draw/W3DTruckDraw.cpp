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

// FILE: W3DTruckDraw.cpp
// Draw Trucks.  Actually, this draws rocket buggies.
// Author: John Ahlquist, March 2002

#include <stdlib.h>
#include <math.h>

#include "Common/Thing.h"
#include "Common/ThingFactory.h"
#include "Common/GameAudio.h"
#include "Common/GlobalData.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/ParticleSys.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/GameLogic.h"		// for logic frame count
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/ScriptEngine.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/GameClient/Module/W3DTruckDraw.h"


//-------------------------------------------------------------------------------------------------
W3DTruckDrawModuleData::W3DTruckDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
W3DTruckDrawModuleData::~W3DTruckDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
void W3DTruckDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Dust", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_dustEffectName) },
		{ "DirtSpray", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_dirtEffectName) },
		{ "PowerslideSpray", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_powerslideEffectName) },

		{ "LeftFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_frontLeftTireBoneName) },
		{ "RightFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_frontRightTireBoneName) },
		{ "LeftRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_rearLeftTireBoneName) },
		{ "RightRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_rearRightTireBoneName) },
		{ "MidLeftFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midFrontLeftTireBoneName) },
		{ "MidRightFrontTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midFrontRightTireBoneName) },
		{ "MidLeftRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midRearLeftTireBoneName) },
		{ "MidRightRearTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midRearRightTireBoneName) },
		{ "MidLeftMidTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midMidLeftTireBoneName) },
		{ "MidRightMidTireBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_midMidRightTireBoneName) },

		{ "TireRotationMultiplier", INI::parseReal, NULL, offsetof(W3DTruckDrawModuleData, m_rotationSpeedMultiplier) },
		{ "PowerslideRotationAddition", INI::parseReal, NULL, offsetof(W3DTruckDrawModuleData, m_powerslideRotationAddition) },
		{ "CabBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_cabBoneName) },
		{ "TrailerBone", INI::parseAsciiString, NULL, offsetof(W3DTruckDrawModuleData, m_trailerBoneName) },
		{ "CabRotationMultiplier", INI::parseReal, NULL, offsetof(W3DTruckDrawModuleData, m_cabRotationFactor) },
		{ "TrailerRotationMultiplier", INI::parseReal, NULL, offsetof(W3DTruckDrawModuleData, m_trailerRotationFactor) },
		{ "RotationDamping", INI::parseReal, NULL, offsetof(W3DTruckDrawModuleData, m_rotationDampingFactor) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTruckDraw::W3DTruckDraw( Thing *thing, const ModuleData* moduleData ) : W3DModelDraw( thing, moduleData ),
m_dirtEffect(NULL), m_dustEffect(NULL), m_powerslideEffect(NULL), m_effectsInitialized(false),
m_wasAirborne(false), m_isPowersliding(false),
m_frontWheelRotation(0), m_rearWheelRotation(0), m_midFrontWheelRotation(0), m_midRearWheelRotation(0),
m_frontRightTireBone(0), m_frontLeftTireBone(0), m_rearLeftTireBone(0),m_rearRightTireBone(0),
m_midFrontRightTireBone(0), m_midFrontLeftTireBone(0), m_midRearLeftTireBone(0),m_midRearRightTireBone(0),
m_midMidRightTireBone(0), m_midMidLeftTireBone(0), m_prevRenderObj(NULL)
{
	const AudioEventRTS * event;
	event = thing->getTemplate()->getPerUnitSound("TruckLandingSound");
	if (event) {
		m_landingSound = *event;
	}
	event = thing->getTemplate()->getPerUnitSound("TruckPowerslideSound");
	if (event) {
		m_powerslideSound = *event;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTruckDraw::~W3DTruckDraw()
{
	tossEmitters();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DTruckDraw::tossEmitters()
{
	if (m_dustEffect)
	{
		m_dustEffect->attachToObject(NULL);
		m_dustEffect->destroy();
		m_dustEffect = NULL;
	}
	if (m_dirtEffect)
	{
		m_dirtEffect->attachToObject(NULL);
		m_dirtEffect->destroy();
		m_dirtEffect = NULL;
	}
	if (m_powerslideEffect)
	{
		m_powerslideEffect->attachToObject(NULL);
		m_powerslideEffect->destroy();
		m_powerslideEffect = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
void W3DTruckDraw::setFullyObscuredByShroud(Bool fullyObscured)
{
	if (fullyObscured != getFullyObscuredByShroud())
	{
		if (fullyObscured)
			tossEmitters();
		else
			createEmitters();
	}
	W3DModelDraw::setFullyObscuredByShroud(fullyObscured);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**

 * Start creating debris from the tank treads
 */
void W3DTruckDraw::createEmitters( void )
{
	if (getDrawable()->isDrawableEffectivelyHidden())
		return;
	if (getW3DTruckDrawModuleData())
	{
		const ParticleSystemTemplate *sysTemplate;

		if (!m_dustEffect) {

			sysTemplate = TheParticleSystemManager->findTemplate(getW3DTruckDrawModuleData()->m_dustEffectName);
			if (sysTemplate)
			{
				m_dustEffect = TheParticleSystemManager->createParticleSystem( sysTemplate );
				m_dustEffect->attachToObject(getDrawable()->getObject());
				// important: mark it as do-not-save, since we'll just re-create it when we reload.
				m_dustEffect->setSaveable(FALSE);
			}	else {
				if (!getW3DTruckDrawModuleData()->m_dustEffectName.isEmpty()) {
					DEBUG_LOG(("*** ERROR - Missing particle system '%s' in thing '%s'",
						getW3DTruckDrawModuleData()->m_dustEffectName.str(), getDrawable()->getObject()->getTemplate()->getName().str()));
				}
			}

		}
		if (!m_dirtEffect) {
			sysTemplate = TheParticleSystemManager->findTemplate(getW3DTruckDrawModuleData()->m_dirtEffectName);
			if (sysTemplate)
			{
				m_dirtEffect = TheParticleSystemManager->createParticleSystem( sysTemplate );
				m_dirtEffect->attachToObject(getDrawable()->getObject());
				// important: mark it as do-not-save, since we'll just re-create it when we reload.
				m_dirtEffect->setSaveable(FALSE);
			}	else {
				if (!getW3DTruckDrawModuleData()->m_dirtEffectName.isEmpty()) {
					DEBUG_LOG(("*** ERROR - Missing particle system '%s' in thing '%s'",
						getW3DTruckDrawModuleData()->m_dirtEffectName.str(), getDrawable()->getObject()->getTemplate()->getName().str()));
				}
			}
		}
		if (!m_powerslideEffect) {
			sysTemplate = TheParticleSystemManager->findTemplate(getW3DTruckDrawModuleData()->m_powerslideEffectName);
			if (sysTemplate)
			{
				m_powerslideEffect = TheParticleSystemManager->createParticleSystem( sysTemplate );
				m_powerslideEffect->attachToObject(getDrawable()->getObject());
				// important: mark it as do-not-save, since we'll just re-create it when we reload.
				m_powerslideEffect->setSaveable(FALSE);
			}	else {
				if (!getW3DTruckDrawModuleData()->m_powerslideEffectName.isEmpty()) {
					DEBUG_LOG(("*** ERROR - Missing particle system '%s' in thing '%s'",
						getW3DTruckDrawModuleData()->m_powerslideEffectName.str(), getDrawable()->getObject()->getTemplate()->getName().str()));
				}
			}
		}
	}

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * Stop creating debris from the tank treads
 */
void W3DTruckDraw::enableEmitters( Bool enable  )
{
	// don't check... if we are hidden the first time thru, then we'll never create the emitters.
	// eg, if we are loading a game and the unit is in a tunnel, he'll never get emitteres even when he exits.
	//if (!m_effectsInitialized)
	{
		createEmitters();
		m_effectsInitialized=true;
	}
	if (m_dustEffect)
	{
		if (enable)
			m_dustEffect->start();
		else
			m_dustEffect->stop();
	}
	if (m_dirtEffect)
	{
		if (enable)
			m_dirtEffect->start();
		else
			m_dirtEffect->stop();
	}
	if (m_powerslideEffect)
	{
		if (!enable)
			m_powerslideEffect->stop();
	}
}
//-------------------------------------------------------------------------------------------------
void W3DTruckDraw::updateBones( void )
{
	if( getW3DTruckDrawModuleData() )
	{
		//Front tires
		if( !getW3DTruckDrawModuleData()->m_frontLeftTireBoneName.isEmpty() )
		{
			m_frontLeftTireBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_frontLeftTireBoneName.str());
			DEBUG_ASSERTCRASH(m_frontLeftTireBone, ("Missing front-left tire bone %s in model %s", getW3DTruckDrawModuleData()->m_frontLeftTireBoneName.str(), getRenderObject()->Get_Name()));
		}

		if( !getW3DTruckDrawModuleData()->m_frontRightTireBoneName.isEmpty() )
		{
			m_frontRightTireBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_frontRightTireBoneName.str());
			DEBUG_ASSERTCRASH(m_frontRightTireBone, ("Missing front-right tire bone %s in model %s", getW3DTruckDrawModuleData()->m_frontRightTireBoneName.str(), getRenderObject()->Get_Name()));
		}

		//Rear tires
		if( !getW3DTruckDrawModuleData()->m_rearLeftTireBoneName.isEmpty() )
		{
			m_rearLeftTireBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_rearLeftTireBoneName.str());
			DEBUG_ASSERTCRASH(m_rearLeftTireBone, ("Missing rear-left tire bone %s in model %s", getW3DTruckDrawModuleData()->m_rearLeftTireBoneName.str(), getRenderObject()->Get_Name()));
		}

		if( !getW3DTruckDrawModuleData()->m_rearRightTireBoneName.isEmpty() )
		{
			m_rearRightTireBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_rearRightTireBoneName.str());
			DEBUG_ASSERTCRASH(m_rearRightTireBone, ("Missing rear-left tire bone %s in model %s", getW3DTruckDrawModuleData()->m_rearRightTireBoneName.str(), getRenderObject()->Get_Name()));
		}

		//midFront tires
		if( !getW3DTruckDrawModuleData()->m_midFrontLeftTireBoneName.isEmpty() )
		{
			m_midFrontLeftTireBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_midFrontLeftTireBoneName.str());
			DEBUG_ASSERTCRASH(m_midFrontLeftTireBone, ("Missing mid-front-left tire bone %s in model %s", getW3DTruckDrawModuleData()->m_midFrontLeftTireBoneName.str(), getRenderObject()->Get_Name()));

			m_midFrontRightTireBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_midFrontRightTireBoneName.str());
			DEBUG_ASSERTCRASH(m_midFrontRightTireBone, ("Missing mid-front-right tire bone %s in model %s", getW3DTruckDrawModuleData()->m_midFrontRightTireBoneName.str(), getRenderObject()->Get_Name()));

			if (!m_midFrontRightTireBone )
			{
				m_midFrontLeftTireBone = 0;
			}
		}

		//midRear tires
		if( !getW3DTruckDrawModuleData()->m_midRearLeftTireBoneName.isEmpty() )
		{
			m_midRearLeftTireBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_midRearLeftTireBoneName.str());
			DEBUG_ASSERTCRASH(m_midRearLeftTireBone, ("Missing mid-rear-left tire bone %s in model %s", getW3DTruckDrawModuleData()->m_midRearLeftTireBoneName.str(), getRenderObject()->Get_Name()));

			m_midRearRightTireBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_midRearRightTireBoneName.str());
			DEBUG_ASSERTCRASH(m_midRearRightTireBone, ("Missing mid-rear-right tire bone %s in model %s", getW3DTruckDrawModuleData()->m_midRearRightTireBoneName.str(), getRenderObject()->Get_Name()));

			if (!m_midRearRightTireBone)
			{
				m_midRearLeftTireBone = 0;
			}
		}

		//midMid tires
		if( !getW3DTruckDrawModuleData()->m_midMidLeftTireBoneName.isEmpty() )
		{
			m_midMidLeftTireBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_midMidLeftTireBoneName.str());
			DEBUG_ASSERTCRASH(m_midMidLeftTireBone, ("Missing mid-mid-left tire bone %s in model %s", getW3DTruckDrawModuleData()->m_midMidLeftTireBoneName.str(), getRenderObject()->Get_Name()));

			m_midMidRightTireBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_midMidRightTireBoneName.str());
			DEBUG_ASSERTCRASH(m_midMidRightTireBone, ("Missing mid-mid-right tire bone %s in model %s", getW3DTruckDrawModuleData()->m_midMidRightTireBoneName.str(), getRenderObject()->Get_Name()));

			if (!m_midMidRightTireBone)
			{
				m_midMidLeftTireBone = 0;
			}
		}

		//Cab
		if( !getW3DTruckDrawModuleData()->m_cabBoneName.isEmpty() )
		{
			m_cabBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_cabBoneName.str());
			DEBUG_ASSERTCRASH(m_cabBone, ("Missing cab bone %s in model %s", getW3DTruckDrawModuleData()->m_cabBoneName.str(), getRenderObject()->Get_Name()));
			m_trailerBone = getRenderObject()->Get_Bone_Index(getW3DTruckDrawModuleData()->m_trailerBoneName.str());
		}
	}

	m_prevRenderObj = getRenderObject();
	m_prevNumBones = m_prevRenderObj->Get_Num_Bones();
}

//-------------------------------------------------------------------------------------------------
void W3DTruckDraw::setHidden(Bool h)
{
	W3DModelDraw::setHidden(h);
	if (h)
	{
		enableEmitters(false);
	}
}

//-------------------------------------------------------------------------------------------------
void W3DTruckDraw::onRenderObjRecreated(void)
{
	//DEBUG_LOG(("Old obj %x, newObj %x, new bones %d, old bones %d",
	//	m_prevRenderObj, getRenderObject(), getRenderObject()->Get_Num_Bones(),
	//	m_prevNumBones));
	m_prevRenderObj = NULL;
	m_frontLeftTireBone = 0;
	m_frontRightTireBone = 0;
	m_rearLeftTireBone = 0;
	m_rearRightTireBone = 0;
	m_midFrontLeftTireBone = 0;
	m_midFrontRightTireBone = 0;
	m_midRearLeftTireBone = 0;
	m_midRearRightTireBone = 0;
	m_midMidLeftTireBone = 0;
	m_midMidRightTireBone = 0;
	updateBones();
}

//-------------------------------------------------------------------------------------------------
/** Rotate and position wheels and other truck parts. */
//-------------------------------------------------------------------------------------------------
void W3DTruckDraw::doDrawModule(const Matrix3D* transformMtx)
{

	W3DModelDraw::doDrawModule(transformMtx);

	if (!TheGlobalData->m_showClientPhysics)
		return;

	const W3DTruckDrawModuleData *moduleData = getW3DTruckDrawModuleData();
	if (moduleData==NULL)
		return; // shouldn't ever happen.

	// TheSuperHackers @tweak Update the draw on every WW Sync only.
	// All calculations are originally catered to a 30 fps logic step.
	if (WW3D::Get_Frame_Time() == 0)
		return;

	const Real ACCEL_THRESHOLD = 0.01f;
	const Real SIZE_CAP = 2.0f;
	// get object from logic
	Object *obj = getDrawable()->getObject();
	if (obj == NULL)
		return;

	if (getRenderObject()==NULL) return;
	if (getRenderObject() != m_prevRenderObj) {
		DEBUG_LOG(("W3DTruckDraw::doDrawModule - shouldn't update bones. jba"));
		updateBones();
	}

	// get object physics state
	PhysicsBehavior *physics = obj->getPhysics();
	if (physics == NULL)
		return;

	const Coord3D *vel = physics->getVelocity();
	Real speed = physics->getVelocityMagnitude();

	const TWheelInfo *wheelInfo = getDrawable()->getWheelInfo();	// note, can return null!
	AIUpdateInterface *ai = obj->getAI();
	if (m_cabBone && wheelInfo) {
		Matrix3D cabXfrm(1);
		cabXfrm.Make_Identity();
		Real desiredAngle = wheelInfo->m_wheelAngle*moduleData->m_cabRotationFactor;

		// Check goal angle.
		if (ai && ai->getPath())
		{
			Coord3D pointOnPath;
			ai->getPath()->peekCachedPointOnPath(pointOnPath);
			Real angleToGoal = ThePartitionManager->getRelativeAngle2D( obj, &pointOnPath );
			//DEBUG_LOG(("To goal %f, desired %f ", 180*angleToGoal/PI, 180*desiredAngle/PI));
			if (angleToGoal<0) {
				if (desiredAngle<angleToGoal) desiredAngle=angleToGoal;
				if (desiredAngle>0) desiredAngle = 0;
			} else {
				if (desiredAngle>angleToGoal) desiredAngle = angleToGoal;
				if (desiredAngle<0) desiredAngle = 0;
			}
			//DEBUG_LOG(("final desired %f ", 180*desiredAngle/PI));
		}

		Real deltaAngle = desiredAngle - m_curCabRotation;
		deltaAngle *= moduleData->m_rotationDampingFactor;
		m_curCabRotation += deltaAngle;
		cabXfrm.Rotate_Z(m_curCabRotation);
		getRenderObject()->Capture_Bone( m_cabBone );
		getRenderObject()->Control_Bone( m_cabBone, cabXfrm );
		if (m_trailerBone && wheelInfo) {
			desiredAngle = -wheelInfo->m_wheelAngle*moduleData->m_trailerRotationFactor;
			Real deltaAngle = desiredAngle - m_curTrailerRotation;
			deltaAngle *= moduleData->m_rotationDampingFactor;
			m_curTrailerRotation += deltaAngle;
			cabXfrm.Make_Identity();
			cabXfrm.Rotate_Z(m_curTrailerRotation);
			getRenderObject()->Capture_Bone( m_trailerBone );
			getRenderObject()->Control_Bone( m_trailerBone, cabXfrm );
		}
	}

	if (m_frontLeftTireBone || m_rearLeftTireBone)
	{
		const Real rotationFactor = moduleData->m_rotationSpeedMultiplier;
		Real powerslideRotationAddition = moduleData->m_powerslideRotationAddition * m_isPowersliding;

		if (ai) {
			Locomotor *loco = ai->getCurLocomotor();
			if (loco) {
				if (loco->isMovingBackwards()) {
					speed = -speed; // rotate wheels backwards.  jba.
					powerslideRotationAddition = -powerslideRotationAddition;
				}
			}
		}

		m_frontWheelRotation += rotationFactor*speed;
		m_rearWheelRotation += rotationFactor*(speed + powerslideRotationAddition);
		m_frontWheelRotation = WWMath::Normalize_Angle(m_frontWheelRotation);
		m_rearWheelRotation = WWMath::Normalize_Angle(m_rearWheelRotation);

		// For now, just use the same values for mid wheels -- may want to do independent calcs later...
		m_midFrontWheelRotation = m_frontWheelRotation;
		m_midRearWheelRotation = m_rearWheelRotation;

		Matrix3D wheelXfrm(1);



		if (m_frontLeftTireBone && wheelInfo)
		{
			wheelXfrm.Make_Identity();
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_frontLeftHeightOffset);
			wheelXfrm.Rotate_Z(wheelInfo->m_wheelAngle);
			wheelXfrm.Rotate_Y(m_frontWheelRotation);
			getRenderObject()->Capture_Bone( m_frontLeftTireBone );
			getRenderObject()->Control_Bone( m_frontLeftTireBone, wheelXfrm );


			wheelXfrm.Make_Identity();
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_frontRightHeightOffset);
			wheelXfrm.Rotate_Z(wheelInfo->m_wheelAngle);
			wheelXfrm.Rotate_Y(m_frontWheelRotation);
			getRenderObject()->Capture_Bone( m_frontRightTireBone );
			getRenderObject()->Control_Bone( m_frontRightTireBone, wheelXfrm );
		}
		if (m_rearLeftTireBone && wheelInfo)
		{
			wheelXfrm.Make_Identity();
			wheelXfrm.Rotate_Y(m_rearWheelRotation);
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_rearLeftHeightOffset);
			getRenderObject()->Capture_Bone( m_rearLeftTireBone );
			getRenderObject()->Control_Bone( m_rearLeftTireBone, wheelXfrm );

			wheelXfrm.Make_Identity();
			wheelXfrm.Rotate_Y(m_rearWheelRotation);
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_rearRightHeightOffset);

			//@todo TROUBLE HERE, THE BONE INDICES DO NOT MATCH THE RENDEROBJECTS BONES, SOMETIMES

			getRenderObject()->Capture_Bone( m_rearRightTireBone );
			getRenderObject()->Control_Bone( m_rearRightTireBone, wheelXfrm );
		}
		if (m_midFrontLeftTireBone && wheelInfo)
		{
			wheelXfrm.Make_Identity();
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_frontLeftHeightOffset);
			wheelXfrm.Rotate_Z(wheelInfo->m_wheelAngle);
			wheelXfrm.Rotate_Y(m_midFrontWheelRotation);
			getRenderObject()->Capture_Bone( m_midFrontLeftTireBone );
			getRenderObject()->Control_Bone( m_midFrontLeftTireBone, wheelXfrm );

			wheelXfrm.Make_Identity();
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_frontRightHeightOffset);
			wheelXfrm.Rotate_Z(wheelInfo->m_wheelAngle);
			wheelXfrm.Rotate_Y(m_midFrontWheelRotation);
			getRenderObject()->Capture_Bone( m_midFrontRightTireBone );
			getRenderObject()->Control_Bone( m_midFrontRightTireBone, wheelXfrm );
		}
		if (m_midRearLeftTireBone && wheelInfo)
		{
			wheelXfrm.Make_Identity();
			wheelXfrm.Rotate_Y(m_midRearWheelRotation);
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_rearLeftHeightOffset);
			getRenderObject()->Capture_Bone( m_midRearLeftTireBone );
			getRenderObject()->Control_Bone( m_midRearLeftTireBone, wheelXfrm );

			wheelXfrm.Make_Identity();
			wheelXfrm.Rotate_Y(m_midRearWheelRotation);
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_rearRightHeightOffset);
			getRenderObject()->Capture_Bone( m_midRearRightTireBone );
			getRenderObject()->Control_Bone( m_midRearRightTireBone, wheelXfrm );
		}
		if (m_midMidLeftTireBone && wheelInfo)
		{
			wheelXfrm.Make_Identity();
			wheelXfrm.Rotate_Y(m_midRearWheelRotation);
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_rearLeftHeightOffset);
			getRenderObject()->Capture_Bone( m_midMidLeftTireBone );
			getRenderObject()->Control_Bone( m_midMidLeftTireBone, wheelXfrm );

			wheelXfrm.Make_Identity();
			wheelXfrm.Rotate_Y(m_midRearWheelRotation);
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_rearRightHeightOffset);
			getRenderObject()->Capture_Bone( m_midMidRightTireBone );
			getRenderObject()->Control_Bone( m_midMidRightTireBone, wheelXfrm );
		}
	}

	Bool wasPowersliding = m_isPowersliding;
	m_isPowersliding = false;
	if (physics->isMotive() && !obj->isSignificantlyAboveTerrain()) {
		enableEmitters(true);
		Coord3D accel = *physics->getAcceleration();
		accel.z = 0; // ignore gravitational force.
		Bool accelerating = accel.length()>ACCEL_THRESHOLD;
		//DEBUG_LOG(("Accel %f, speed %f", accel.length(), speed));
		if (accelerating)	{
			Real dot = accel.x*vel->x + accel.y*vel->y;
			if (dot<0) {
				accelerating = false;  // decelerating, actually.
			}
		}
		if (m_dustEffect) {
			// Need more dust the faster we go.
			if (speed>SIZE_CAP) {
				speed = SIZE_CAP;
			}
			m_dustEffect->setSizeMultiplier(speed);
		}
		if (m_dirtEffect) {
			if (wheelInfo && wheelInfo->m_framesAirborne>3) {
				Real factor = 1 + wheelInfo->m_framesAirborne/16;
				if (factor>2.0) factor = 2.0;
				m_dustEffect->setSizeMultiplier(factor*SIZE_CAP);
				m_dustEffect->trigger();
				m_landingSound.setObjectID(obj->getID());
				TheAudio->addAudioEvent(&m_landingSound);
			} else {
				if (!accelerating || speed>2.0f) {
					m_dirtEffect->stop();
				}
			}
		}
		if (m_powerslideEffect) {
			if (physics->getTurning() == TURN_NONE) {
				m_powerslideEffect->stop();
			}	else {
				m_isPowersliding = true;
				m_powerslideEffect->start();
			}
		}
		if (m_dirtEffect) {
			if (!accelerating || speed>2.0f) {
				m_dirtEffect->stop();
			}
		}
	}
	else
		enableEmitters(false);

	m_wasAirborne = obj->isSignificantlyAboveTerrain();

	if(!wasPowersliding && m_isPowersliding) {
		// start sound
		m_powerslideSound.setObjectID(obj->getID());
		m_powerslideSound.setPlayingHandle(TheAudio->addAudioEvent(&m_powerslideSound));
	}	else if (wasPowersliding && !m_isPowersliding) {
		TheAudio->removeAudioEvent(m_powerslideSound.getPlayingHandle());
	}

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DTruckDraw::crc( Xfer *xfer )
{

	// extend base class
	W3DModelDraw::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DTruckDraw::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	W3DModelDraw::xfer( xfer );

	// John A and Mark W say there is no data to save here

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DTruckDraw::loadPostProcess( void )
{

	// extend base class
	W3DModelDraw::loadPostProcess();

	// toss any existing ones (no need to re-create; we'll do that on demand)
	tossEmitters();

}  // end loadPostProcess
