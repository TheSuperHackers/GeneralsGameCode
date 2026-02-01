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

// FILE: W3DTankTruckDraw.cpp
// Draw TankTrucks.  Actually, this draws quad cannon which has both treads and wheels.
// Author: Mark Wilczynski, August 2002

#include <stdlib.h>
#include <math.h>

#include "Common/Thing.h"
#include "Common/ThingFactory.h"
#include "Common/GameAudio.h"
#include "Common/GlobalData.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameClient/Drawable.h"
#include "GameClient/ParticleSys.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/GameClient/Module/W3DTankTruckDraw.h"
#include "WW3D2/matinfo.h"

//#define SHOW_TANK_DEBRIS
//-------------------------------------------------------------------------------------------------
W3DTankTruckDrawModuleData::W3DTankTruckDrawModuleData():
	m_treadDebrisNameLeft("TrackDebrisDirtLeft"),
	m_treadDebrisNameRight("TrackDebrisDirtRight"),
	m_treadAnimationRate(0.0f),
	m_treadPivotSpeedFraction(0.6f),
	m_treadDriveSpeedFraction(0.3f)
{
}

//-------------------------------------------------------------------------------------------------
W3DTankTruckDrawModuleData::~W3DTankTruckDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
void W3DTankTruckDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Dust", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_dustEffectName) },
		{ "DirtSpray", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_dirtEffectName) },
		{ "PowerslideSpray", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_powerslideEffectName) },
		{ "LeftFrontTireBone", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_frontLeftTireBoneName) },
		{ "RightFrontTireBone", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_frontRightTireBoneName) },
		{ "LeftRearTireBone", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_rearLeftTireBoneName) },
		{ "RightRearTireBone", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_rearRightTireBoneName) },
		{ "MidLeftFrontTireBone", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_midFrontLeftTireBoneName) },
		{ "MidRightFrontTireBone", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_midFrontRightTireBoneName) },
		{ "MidLeftRearTireBone", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_midRearLeftTireBoneName) },
		{ "MidRightRearTireBone", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_midRearRightTireBoneName) },
		{ "TireRotationMultiplier", INI::parseReal, nullptr, offsetof(W3DTankTruckDrawModuleData, m_rotationSpeedMultiplier) },
		{ "PowerslideRotationAddition", INI::parseReal, nullptr, offsetof(W3DTankTruckDrawModuleData, m_powerslideRotationAddition) },
		{ "TreadDebrisLeft", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_treadDebrisNameLeft) },
		{ "TreadDebrisRight", INI::parseAsciiString, nullptr, offsetof(W3DTankTruckDrawModuleData, m_treadDebrisNameRight) },
		{ "TreadAnimationRate", INI::parseVelocityReal, nullptr, offsetof(W3DTankTruckDrawModuleData, m_treadAnimationRate) },
		{ "TreadPivotSpeedFraction", INI::parseReal, nullptr, offsetof(W3DTankTruckDrawModuleData, m_treadPivotSpeedFraction) },
		{ "TreadDriveSpeedFraction", INI::parseReal, nullptr, offsetof(W3DTankTruckDrawModuleData, m_treadDriveSpeedFraction) },
		{ nullptr, nullptr, nullptr, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTankTruckDraw::W3DTankTruckDraw( Thing *thing, const ModuleData* moduleData ) : W3DModelDraw( thing, moduleData ),
m_dirtEffect(nullptr), m_dustEffect(nullptr), m_powerslideEffect(nullptr), m_effectsInitialized(false),
m_wasAirborne(false), m_isPowersliding(false), m_frontWheelRotation(0), m_rearWheelRotation(0),
m_frontRightTireBone(0), m_frontLeftTireBone(0), m_rearLeftTireBone(0),m_rearRightTireBone(0),
m_prevRenderObj(nullptr)
{
	//Truck Data
	m_landingSound = *(thing->getTemplate()->getPerUnitSound("TruckLandingSound"));
	m_powerslideSound = *(thing->getTemplate()->getPerUnitSound("TruckPowerslideSound"));

	//Tank data
	m_treadDebrisLeft = nullptr;
	m_treadDebrisRight = nullptr;

	for (Int i=0; i<MAX_TREADS_PER_TANK; i++)
		m_treads[i].m_robj = nullptr;

	m_treadCount=0;

#ifdef SHOW_TANK_DEBRIS
	if (getW3DTankTruckDrawModuleData())
	{
		ParticleSystemTemplate *sysTemplate;

		sysTemplate = TheParticleSystemManager->findTemplate(getW3DTankTruckDrawModuleData()->m_treadDebrisNameLeft);
		if (sysTemplate)
		{
			m_treadDebrisLeft = TheParticleSystemManager->createParticleSystem( sysTemplate );
			m_treadDebrisLeft->attachToDrawable(getDrawable());
DEBUG_CRASH(("test me, may not work (srj)"));
			// important: mark it as do-not-save, since we'll just re-create it when we reload.
			m_treadDebrisLeft->setSaveable(FALSE);
		}

		sysTemplate = TheParticleSystemManager->findTemplate(getW3DTankTruckDrawModuleData()->m_treadDebrisNameRight);
		if (sysTemplate)
		{
			m_treadDebrisRight = TheParticleSystemManager->createParticleSystem( sysTemplate );
			m_treadDebrisRight->attachToDrawable(getDrawable());
DEBUG_CRASH(("test me, may not work (srj)"));
			// important: mark it as do-not-save, since we'll just re-create it when we reload.
			m_treadDebrisRight->setSaveable(FALSE);
		}
	}
#endif
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTankTruckDraw::~W3DTankTruckDraw()
{
	tossEmitters();

	for (Int i=0; i<MAX_TREADS_PER_TANK; i++)
		if (m_treads[i].m_robj)
			REF_PTR_RELEASE(m_treads[i].m_robj);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**

 * Start creating debris from the tank treads
 */
void W3DTankTruckDraw::startMoveDebris( void )
{
	if (getDrawable()->isDrawableEffectivelyHidden())
		return;
	if (m_treadDebrisLeft)
    m_treadDebrisLeft->start();
	if (m_treadDebrisRight)
    m_treadDebrisRight->start();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * Stop creating debris from the tank treads
 */
void W3DTankTruckDraw::stopMoveDebris( void )
{
  if (m_treadDebrisLeft)
  	m_treadDebrisLeft->stop();
  if (m_treadDebrisRight)
  	m_treadDebrisRight->stop();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DTankTruckDraw::tossEmitters()
{
	if (m_dustEffect)
	{
		m_dustEffect->attachToObject(nullptr);
		m_dustEffect->destroy();
		m_dustEffect = nullptr;
	}
	if (m_dirtEffect)
	{
		m_dirtEffect->attachToObject(nullptr);
		m_dirtEffect->destroy();
		m_dirtEffect = nullptr;
	}
	if (m_powerslideEffect)
	{
		m_powerslideEffect->attachToObject(nullptr);
		m_powerslideEffect->destroy();
		m_powerslideEffect = nullptr;
	}
}

//-------------------------------------------------------------------------------------------------
void W3DTankTruckDraw::setFullyObscuredByShroud(Bool fullyObscured)
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
void W3DTankTruckDraw::createEmitters( void )
{
	if (getDrawable()->isDrawableEffectivelyHidden())
		return;
	if (getW3DTankTruckDrawModuleData())
	{
		const ParticleSystemTemplate *sysTemplate;

		if (!m_dustEffect) {

			sysTemplate = TheParticleSystemManager->findTemplate(getW3DTankTruckDrawModuleData()->m_dustEffectName);
			if (sysTemplate)
			{
				m_dustEffect = TheParticleSystemManager->createParticleSystem( sysTemplate );
				m_dustEffect->attachToObject(getDrawable()->getObject());
				// important: mark it as do-not-save, since we'll just re-create it when we reload.
				m_dustEffect->setSaveable(FALSE);
			}	else {
				if (!getW3DTankTruckDrawModuleData()->m_dustEffectName.isEmpty()) {
					DEBUG_LOG(("*** ERROR - Missing particle system '%s' in thing '%s'",
						getW3DTankTruckDrawModuleData()->m_dustEffectName.str(), getDrawable()->getObject()->getTemplate()->getName().str()));
				}
			}

		}
		if (!m_dirtEffect) {
			sysTemplate = TheParticleSystemManager->findTemplate(getW3DTankTruckDrawModuleData()->m_dirtEffectName);
			if (sysTemplate)
			{
				m_dirtEffect = TheParticleSystemManager->createParticleSystem( sysTemplate );
				m_dirtEffect->attachToObject(getDrawable()->getObject());
				// important: mark it as do-not-save, since we'll just re-create it when we reload.
				m_dirtEffect->setSaveable(FALSE);
			}	else {
				if (!getW3DTankTruckDrawModuleData()->m_dirtEffectName.isEmpty()) {
					DEBUG_LOG(("*** ERROR - Missing particle system '%s' in thing '%s'",
						getW3DTankTruckDrawModuleData()->m_dirtEffectName.str(), getDrawable()->getObject()->getTemplate()->getName().str()));
				}
			}
		}
		if (!m_powerslideEffect) {
			sysTemplate = TheParticleSystemManager->findTemplate(getW3DTankTruckDrawModuleData()->m_powerslideEffectName);
			if (sysTemplate)
			{
				m_powerslideEffect = TheParticleSystemManager->createParticleSystem( sysTemplate );
				m_powerslideEffect->attachToObject(getDrawable()->getObject());
				// important: mark it as do-not-save, since we'll just re-create it when we reload.
				m_powerslideEffect->setSaveable(FALSE);
			}	else {
				if (!getW3DTankTruckDrawModuleData()->m_powerslideEffectName.isEmpty()) {
					DEBUG_LOG(("*** ERROR - Missing particle system '%s' in thing '%s'",
						getW3DTankTruckDrawModuleData()->m_powerslideEffectName.str(), getDrawable()->getObject()->getTemplate()->getName().str()));
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
void W3DTankTruckDraw::enableEmitters( Bool enable  )
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
void W3DTankTruckDraw::updateBones( void ) {
	if( getW3DTankTruckDrawModuleData() )
	{
		//Front tires
		if( !getW3DTankTruckDrawModuleData()->m_frontLeftTireBoneName.isEmpty() )
		{
			m_frontLeftTireBone = getRenderObject()->Get_Bone_Index(getW3DTankTruckDrawModuleData()->m_frontLeftTireBoneName.str());
			DEBUG_ASSERTCRASH(m_frontLeftTireBone, ("Missing front-left tire bone %s in model %s", getW3DTankTruckDrawModuleData()->m_frontLeftTireBoneName.str(), getRenderObject()->Get_Name()));

			m_frontRightTireBone = getRenderObject()->Get_Bone_Index(getW3DTankTruckDrawModuleData()->m_frontRightTireBoneName.str());
			DEBUG_ASSERTCRASH(m_frontRightTireBone, ("Missing front-right tire bone %s in model %s", getW3DTankTruckDrawModuleData()->m_frontRightTireBoneName.str(), getRenderObject()->Get_Name()));

			if (!m_frontRightTireBone )
			{
				m_frontLeftTireBone = 0;
			}
		}
		//Rear tires
		if( !getW3DTankTruckDrawModuleData()->m_rearLeftTireBoneName.isEmpty() )
		{
			m_rearLeftTireBone = getRenderObject()->Get_Bone_Index(getW3DTankTruckDrawModuleData()->m_rearLeftTireBoneName.str());
			DEBUG_ASSERTCRASH(m_rearLeftTireBone, ("Missing rear-left tire bone %s in model %s", getW3DTankTruckDrawModuleData()->m_rearLeftTireBoneName.str(), getRenderObject()->Get_Name()));

			m_rearRightTireBone = getRenderObject()->Get_Bone_Index(getW3DTankTruckDrawModuleData()->m_rearRightTireBoneName.str());
			DEBUG_ASSERTCRASH(m_rearRightTireBone, ("Missing rear-left tire bone %s in model %s", getW3DTankTruckDrawModuleData()->m_rearRightTireBoneName.str(), getRenderObject()->Get_Name()));

			if (!m_rearRightTireBone)
			{
				m_rearLeftTireBone = 0;
			}
		}

		//midFront tires
		if( !getW3DTankTruckDrawModuleData()->m_midFrontLeftTireBoneName.isEmpty() )
		{
			m_midFrontLeftTireBone = getRenderObject()->Get_Bone_Index(getW3DTankTruckDrawModuleData()->m_midFrontLeftTireBoneName.str());
			DEBUG_ASSERTCRASH(m_midFrontLeftTireBone, ("Missing mid-front-left tire bone %s in model %s", getW3DTankTruckDrawModuleData()->m_midFrontLeftTireBoneName.str(), getRenderObject()->Get_Name()));

			m_midFrontRightTireBone = getRenderObject()->Get_Bone_Index(getW3DTankTruckDrawModuleData()->m_midFrontRightTireBoneName.str());
			DEBUG_ASSERTCRASH(m_midFrontRightTireBone, ("Missing mid-front-right tire bone %s in model %s", getW3DTankTruckDrawModuleData()->m_midFrontRightTireBoneName.str(), getRenderObject()->Get_Name()));

			if (!m_midFrontRightTireBone )
			{
				m_midFrontLeftTireBone = 0;
			}
		}

		//midRear tires
		if( !getW3DTankTruckDrawModuleData()->m_midRearLeftTireBoneName.isEmpty() )
		{
			m_midRearLeftTireBone = getRenderObject()->Get_Bone_Index(getW3DTankTruckDrawModuleData()->m_midRearLeftTireBoneName.str());
			DEBUG_ASSERTCRASH(m_midRearLeftTireBone, ("Missing mid-rear-left tire bone %s in model %s", getW3DTankTruckDrawModuleData()->m_midRearLeftTireBoneName.str(), getRenderObject()->Get_Name()));

			m_midRearRightTireBone = getRenderObject()->Get_Bone_Index(getW3DTankTruckDrawModuleData()->m_midRearRightTireBoneName.str());
			DEBUG_ASSERTCRASH(m_midRearRightTireBone, ("Missing mid-rear-right tire bone %s in model %s", getW3DTankTruckDrawModuleData()->m_midRearRightTireBoneName.str(), getRenderObject()->Get_Name()));

			if (!m_midRearRightTireBone)
			{
				m_midRearLeftTireBone = 0;
			}
		}
	}

	m_prevRenderObj = getRenderObject();
}

//-------------------------------------------------------------------------------------------------
void W3DTankTruckDraw::setHidden(Bool h)
{
	W3DModelDraw::setHidden(h);
	if (h)
	{
		enableEmitters(false);
#ifdef SHOW_TANK_DEBRIS
		stopMoveDebris();
#endif
	}
}

/**Update uv coordinates on each tread object to simulate movement*/
void W3DTankTruckDraw::updateTreadPositions(Real uvDelta)
{
	Real offset_u;
	TreadObjectInfo *pTread=m_treads;

	for (Int i=0; i<m_treadCount; i++)
	{
		if (pTread->m_type == TREAD_MIDDLE)	//this tread needs to scroll backwards
			offset_u = pTread->m_materialSettings.customUVOffset.X + uvDelta;
		else
		if (pTread->m_type == TREAD_LEFT)	//this tread needs to scroll forwards
			offset_u = pTread->m_materialSettings.customUVOffset.X + uvDelta;
		else
		if (pTread->m_type == TREAD_RIGHT)	//this tread needs to scroll backwards
			offset_u = pTread->m_materialSettings.customUVOffset.X - uvDelta;
		else
		{
			DEBUG_CRASH(("Unhandled case in W3DTankTruckDraw::updateTreadPositions"));
			offset_u = 0.0f;
		}

		// ensure coordinates of offset are in [0, 1] range:
		offset_u = offset_u - WWMath::Floor(offset_u);
		pTread->m_materialSettings.customUVOffset.Set(offset_u,0);
		pTread++;
	}
}

/**Grab pointers to the sub-meshes for each tread*/
void W3DTankTruckDraw::updateTreadObjects(void)
{
	RenderObjClass *robj=getRenderObject();

	//clear all previous tread pointers
	for (Int i=0; i<m_treadCount; i++)
		REF_PTR_RELEASE(m_treads[i].m_robj);
	m_treadCount = 0;

	//Make sure this object has defined a speed for tread scrolling.
	if (getW3DTankTruckDrawModuleData() && getW3DTankTruckDrawModuleData()->m_treadAnimationRate && robj)
	{
		for (Int i=0; i < robj->Get_Num_Sub_Objects() && m_treadCount < MAX_TREADS_PER_TANK; i++)
		{
			RenderObjClass *subObj=robj->Get_Sub_Object(i);
			const char *meshName;
			//Check if subobject name starts with "TREADS".
			if (subObj && subObj->Class_ID() == RenderObjClass::CLASSID_MESH && subObj->Get_Name()
				&& ( (meshName=strchr(subObj->Get_Name(),'.') ) != nullptr && *(meshName++))
				&&_strnicmp(meshName,"TREADS", 6) == 0)
			{	//check if sub-object has the correct material to do texture scrolling.
				MaterialInfoClass *mat=subObj->Get_Material_Info();
				if (mat)
				{	for (Int j=0; j<mat->Vertex_Material_Count(); j++)
					{
						VertexMaterialClass *vmaterial=mat->Peek_Vertex_Material(j);
						LinearOffsetTextureMapperClass *mapper=(LinearOffsetTextureMapperClass *)vmaterial->Peek_Mapper();
						if (mapper && mapper->Mapper_ID() == TextureMapperClass::MAPPER_ID_LINEAR_OFFSET)
						{	mapper->Set_UV_Offset_Delta(Vector2(0,0));	//disable automatic scrolling
							subObj->Add_Ref();	//increase reference since we're storing the pointer
							m_treads[m_treadCount].m_robj=subObj;
							m_treads[m_treadCount].m_type = TREAD_MIDDLE;	//default type
							subObj->Set_User_Data(&m_treads[m_treadCount].m_materialSettings);	//tell W3D about custom material settings
							m_treads[m_treadCount].m_materialSettings.customUVOffset=Vector2(0,0);
							//Commented out since on vehicles with wheels, it makes no sense to turn with treads.
/*							switch (meshName[6])	//check next character after 'TREADS'
							{
								case 'L':
								case 'l':	m_treads[m_treadCount].m_type = TREAD_LEFT;
										break;
								case 'R':
								case 'r':	m_treads[m_treadCount].m_type = TREAD_RIGHT;
										break;
							}*/
							m_treadCount++;
						}
					}
					REF_PTR_RELEASE(mat);
				}
			}
			REF_PTR_RELEASE(subObj);
		}
	}

	m_prevRenderObj = robj;
}

//-------------------------------------------------------------------------------------------------
void W3DTankTruckDraw::onRenderObjRecreated(void)
{
	//DEBUG_LOG(("Old obj %x, newObj %x, new bones %d, old bones %d",
	//	m_prevRenderObj, getRenderObject(), getRenderObject()->Get_Num_Bones(),
	//	m_prevNumBones));
	m_prevRenderObj = nullptr;
	m_frontLeftTireBone = 0;
	m_frontRightTireBone = 0;
	m_rearLeftTireBone = 0;
	m_rearRightTireBone = 0;
	m_midFrontLeftTireBone = 0;
	m_midFrontRightTireBone = 0;
	m_midRearLeftTireBone = 0;
	m_midRearRightTireBone = 0;
	updateBones();
	updateTreadObjects();
}

//-------------------------------------------------------------------------------------------------
/** Map behavior states into W3D animations. */
//-------------------------------------------------------------------------------------------------
void W3DTankTruckDraw::doDrawModule(const Matrix3D* transformMtx)
{

	W3DModelDraw::doDrawModule(transformMtx);

	if (!TheGlobalData->m_showClientPhysics)
		return;

	// TheSuperHackers @tweak Update the draw on every WW Sync only.
	// All calculations are originally catered to a 30 fps logic step.
	if (WW3D::Get_Sync_Frame_Time() == 0)
		return;

	const Real ACCEL_THRESHOLD = 0.01f;
	const Real SIZE_CAP = 2.0f;
	// get object from logic
	Object *obj = getDrawable()->getObject();
	if (obj == nullptr)
		return;

	if (getRenderObject()==nullptr) return;
	if (getRenderObject() != m_prevRenderObj) {
		updateBones();
		updateTreadObjects();
	}
	// get object physics state
	PhysicsBehavior *physics = obj->getPhysics();
	if (physics == nullptr)
		return;

	const Coord3D *vel = physics->getVelocity();
	Real speed = physics->getVelocityMagnitude();

	const TWheelInfo *wheelInfo = getDrawable()->getWheelInfo();	// note, can return null!
	if (wheelInfo && (m_frontLeftTireBone || m_rearLeftTireBone))
	{
		const Real rotationFactor = getW3DTankTruckDrawModuleData()->m_rotationSpeedMultiplier;
		const Real powerslideRotationAddition = getW3DTankTruckDrawModuleData()->m_powerslideRotationAddition * m_isPowersliding;

		m_frontWheelRotation += rotationFactor*speed;
		m_rearWheelRotation += rotationFactor*(speed+powerslideRotationAddition);
		m_frontWheelRotation = WWMath::Normalize_Angle(m_frontWheelRotation);
		m_rearWheelRotation = WWMath::Normalize_Angle(m_rearWheelRotation);

		Matrix3D wheelXfrm(1);
		if (m_frontLeftTireBone)
		{
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
		if (m_rearLeftTireBone)
		{
			wheelXfrm.Make_Identity();
			wheelXfrm.Rotate_Y(m_rearWheelRotation);
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_rearLeftHeightOffset);
			getRenderObject()->Capture_Bone( m_rearLeftTireBone );
			getRenderObject()->Control_Bone( m_rearLeftTireBone, wheelXfrm );

			wheelXfrm.Make_Identity();
			wheelXfrm.Rotate_Y(m_rearWheelRotation);
			wheelXfrm.Adjust_Z_Translation(wheelInfo->m_rearRightHeightOffset);
			getRenderObject()->Capture_Bone( m_rearRightTireBone );
			getRenderObject()->Control_Bone( m_rearRightTireBone, wheelXfrm );
		}
		if (m_midFrontLeftTireBone)
		{
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
		if (m_midRearLeftTireBone)
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
			if (wheelInfo && wheelInfo->m_framesAirborne>3) {
				Real factor = 1 + wheelInfo->m_framesAirborne/16;
				if (factor>2.0) factor = 2.0;
				m_dustEffect->setSizeMultiplier(factor*SIZE_CAP);
				m_dustEffect->trigger();
				m_landingSound.setPosition(obj->getPosition());
				TheAudio->addAudioEvent(&m_landingSound);
			} else {
				m_dustEffect->setSizeMultiplier(speed);
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
			if (!accelerating) {
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

	//Tank update
#ifdef SHOW_TANK_DEBRIS
	const Real DEBRIS_THRESHOLD = 0.00001f;

		// if tank is moving, kick up dust and debris
	Real velMag = vel->x*vel->x + vel->y*vel->y;		// only care about moving on the ground

	if (velMag > DEBRIS_THRESHOLD && !getDrawable()->isDrawableEffectivelyHidden() && !getFullyObscuredByShroud())
		startMoveDebris();
	else
		stopMoveDebris();

	// kick debris higher the faster we move
	Coord3D velMult;
	velMag = (Real)sqrt( velMag );

	velMult.x = 0.5f * velMag + 0.1f;
	if (velMult.x > 1.0f)
		velMult.x = 1.0f;

	velMult.y = velMult.x;

	velMult.z = velMag + 0.1f;
	if (velMult.z > 1.0f)
		velMult.z = 1.0f;

	m_treadDebrisLeft->setVelocityMultiplier( &velMult );
	m_treadDebrisRight->setVelocityMultiplier( &velMult );

	m_treadDebrisLeft->setBurstCountMultiplier( velMult.z );
	m_treadDebrisRight->setBurstCountMultiplier( velMult.z );
#endif
	//Update movement of treads
	if (m_treadCount)
	{
		Real offset_u;
		Real treadScrollSpeed=getW3DTankTruckDrawModuleData()->m_treadAnimationRate;
		TreadObjectInfo *pTread=m_treads;
		Real maxSpeed=obj->getAIUpdateInterface()->getCurLocomotorSpeed();
/* Commented out because these vehicles are presumed not to turn via treads.
		PhysicsTurningType turn=physics->getTurning();
		//For optimization sake, we only do complex tread scrolling when tank
		//is mostly stationary and turning
		if ((turn=physics->getTurning()) != TURN_NONE && physics->getSpeed()/maxSpeed < getW3DTankTruckDrawModuleData()->m_treadPivotSpeedFraction)
		{
			if (turn == TURN_NEGATIVE)	//turning right
				updateTreadPositions(-treadScrollSpeed);
			else	//turning left
				updateTreadPositions(treadScrollSpeed);
		}
		else*/
		if (physics->isMotive() && physics->getVelocityMagnitude()/maxSpeed >= getW3DTankTruckDrawModuleData()->m_treadDriveSpeedFraction)
		{	//do simple scrolling based only on speed when tank is moving straight at high speed.
			//we stop scrolling when tank slows down to reduce the appearance of sliding
			//tread scrolling speed was not directly tied into tank velocity because it looked odd
			//under certain situations when tank moved sideways.
			for (Int i=0; i<m_treadCount; i++)
			{
				offset_u = pTread->m_materialSettings.customUVOffset.X - treadScrollSpeed;
				// ensure coordinates of offset are in [0, 1] range:
				offset_u = offset_u - WWMath::Floor(offset_u);
				pTread->m_materialSettings.customUVOffset.Set(offset_u,0);
				pTread++;
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DTankTruckDraw::crc( Xfer *xfer )
{

	// extend base class
	W3DModelDraw::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DTankTruckDraw::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	W3DModelDraw::xfer( xfer );

	// John A and Mark W say there is no data to save here

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DTankTruckDraw::loadPostProcess( void )
{

	// extend base class
	W3DModelDraw::loadPostProcess();

	// toss any existing ones (no need to re-create; we'll do that on demand)
	tossEmitters();

}
