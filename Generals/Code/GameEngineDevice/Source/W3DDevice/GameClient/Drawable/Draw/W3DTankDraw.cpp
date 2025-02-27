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

// FILE: W3DTankDraw.cpp //////////////////////////////////////////////////////////////////////////
// Draw turreted tanks
// Michael S. Booth, October 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <math.h>
#include "Common/Thing.h"
#include "Common/ThingFactory.h"
#include "Common/GameAudio.h"
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
#include "W3DDevice/GameClient/Module/W3DTankDraw.h"
#include "WW3D2/matinfo.h"


class Matrix3D;

//-------------------------------------------------------------------------------------------------
W3DTankDrawModuleData::W3DTankDrawModuleData() :
	m_treadDebrisNameLeft("TrackDebrisDirtLeft"),
	m_treadDebrisNameRight("TrackDebrisDirtRight"),
	m_treadAnimationRate(0.0f),
	m_treadPivotSpeedFraction(0.6f),
	m_treadDriveSpeedFraction(0.3f)
{
}

//-------------------------------------------------------------------------------------------------
W3DTankDrawModuleData::~W3DTankDrawModuleData()
{
}

//-------------------------------------------------------------------------------------------------
void W3DTankDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  W3DModelDrawModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "TreadDebrisLeft", INI::parseAsciiString, NULL, offsetof(W3DTankDrawModuleData, m_treadDebrisNameLeft) },
		{ "TreadDebrisRight", INI::parseAsciiString, NULL, offsetof(W3DTankDrawModuleData, m_treadDebrisNameRight) },
		{ "TreadAnimationRate", INI::parseVelocityReal, NULL, offsetof(W3DTankDrawModuleData, m_treadAnimationRate) },
		{ "TreadPivotSpeedFraction", INI::parseReal, NULL, offsetof(W3DTankDrawModuleData, m_treadPivotSpeedFraction) },
		{ "TreadDriveSpeedFraction", INI::parseReal, NULL, offsetof(W3DTankDrawModuleData, m_treadDriveSpeedFraction) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTankDraw::W3DTankDraw( Thing *thing, const ModuleData* moduleData )
: W3DModelDraw( thing, moduleData ),m_prevRenderObj(NULL), m_treadDebrisLeft(NULL), m_treadDebrisRight(NULL)
{
	m_treadDebrisLeft = NULL;
	m_treadDebrisRight = NULL;

	for (Int i=0; i<MAX_TREADS_PER_TANK; i++)
		m_treads[i].m_robj = NULL;

	m_treadCount=0;
	//Assume all things face along x axis when created.
	m_lastDirection.x=1.0f;
	m_lastDirection.y=0.0f;
	m_lastDirection.z=0.0f;

	createEmitters();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DTankDraw::tossEmitters( void )
{
	if (m_treadDebrisLeft)
	{
		m_treadDebrisLeft->attachToObject(NULL);
		m_treadDebrisLeft->destroy();
		m_treadDebrisLeft = NULL;
	}
	if (m_treadDebrisRight)
	{
		m_treadDebrisRight->attachToObject(NULL);
		m_treadDebrisRight->destroy();
		m_treadDebrisRight = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DTankDraw::createEmitters( void )
{
	if (!m_treadDebrisLeft)
	{
		const ParticleSystemTemplate *sysTemplate;
		sysTemplate = TheParticleSystemManager->findTemplate(getW3DTankDrawModuleData()->m_treadDebrisNameLeft);
		if (sysTemplate)
		{
			m_treadDebrisLeft = TheParticleSystemManager->createParticleSystem( sysTemplate );
			m_treadDebrisLeft->attachToDrawable(getDrawable());
			// important: mark it as do-not-save, since we'll just re-create it when we reload.
			m_treadDebrisLeft->setSaveable(FALSE);
			// they come into being stopped.
	  	m_treadDebrisLeft->stop();
		}
	}
	if (!m_treadDebrisRight)
	{
		const ParticleSystemTemplate *sysTemplate;
		sysTemplate = TheParticleSystemManager->findTemplate(getW3DTankDrawModuleData()->m_treadDebrisNameRight);
		if (sysTemplate)
		{
			m_treadDebrisRight = TheParticleSystemManager->createParticleSystem( sysTemplate );
			m_treadDebrisRight->attachToDrawable(getDrawable());
			// important: mark it as do-not-save, since we'll just re-create it when we reload.
			m_treadDebrisRight->setSaveable(FALSE);
			// they come into being stopped.
	  	m_treadDebrisRight->stop();
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTankDraw::~W3DTankDraw()
{
	// TheSuperHackers @fix Mauller 16/04/2025 Delete particle systems
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
void W3DTankDraw::startMoveDebris( void )
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
void W3DTankDraw::stopMoveDebris( void )
{
  if (m_treadDebrisLeft)
  	m_treadDebrisLeft->stop();
  if (m_treadDebrisRight)
  	m_treadDebrisRight->stop();
}

//-------------------------------------------------------------------------------------------------
void W3DTankDraw::setHidden(Bool h)
{
	W3DModelDraw::setHidden(h);
	if (h)
	{
		stopMoveDebris();
	}
}

//-------------------------------------------------------------------------------------------------
void W3DTankDraw::setFullyObscuredByShroud(Bool fullyObscured)
{
	if (fullyObscured != getFullyObscuredByShroud())
	{
		if (fullyObscured)
			stopMoveDebris();
	}
	W3DModelDraw::setFullyObscuredByShroud(fullyObscured);
}

/**Update uv coordinates on each tread object to simulate movement*/
void W3DTankDraw::updateTreadPositions(Real uvDelta)
{
	Real offset_u;
	TreadObjectInfo *pTread=m_treads;

	for (Int i=0; i<m_treadCount; i++)
	{
		if (pTread->m_type == TREAD_LEFT)	//this tread needs to scroll forwards
			offset_u = pTread->m_materialSettings.customUVOffset.X + uvDelta;
		else
		if (pTread->m_type == TREAD_RIGHT)	//this tread needs to scroll backwards
			offset_u = pTread->m_materialSettings.customUVOffset.X - uvDelta;
		else
		{
			DEBUG_CRASH(("Unhandled case in W3DTankDraw::updateTreadPositions"));
			offset_u = 0.0f;
		}

		// ensure coordinates of offset are in [0, 1] range:
		offset_u = offset_u - WWMath::Floor(offset_u);
		pTread->m_materialSettings.customUVOffset.Set(offset_u,0);
		pTread++;
	}
}

/**Grab pointers to the sub-meshes for each tread*/
void W3DTankDraw::updateTreadObjects(void)
{
	RenderObjClass *robj=getRenderObject();

	//clear all previous tread pointers
	for (Int i=0; i<m_treadCount; i++)
		REF_PTR_RELEASE(m_treads[i].m_robj);
	m_treadCount = 0;

	//Make sure this object has defined a speed for tread scrolling.
	if (getW3DTankDrawModuleData() && getW3DTankDrawModuleData()->m_treadAnimationRate && robj)
	{
		for (Int i=0; i < robj->Get_Num_Sub_Objects() && m_treadCount < MAX_TREADS_PER_TANK; i++)
		{
			RenderObjClass *subObj=robj->Get_Sub_Object(i);
			const char *meshName;
			//Check if subobject name starts with "TREADS".
			if (subObj && subObj->Class_ID() == RenderObjClass::CLASSID_MESH && subObj->Get_Name()
				&& ( (meshName=strchr(subObj->Get_Name(),'.') ) != 0 && *(meshName++))
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
							switch (meshName[6])	//check next character after 'TREADS'
							{
								case 'L':
								case 'l':	m_treads[m_treadCount].m_type = TREAD_LEFT;
										break;
								case 'R':
								case 'r':	m_treads[m_treadCount].m_type = TREAD_RIGHT;
										break;
							}
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
void W3DTankDraw::onRenderObjRecreated(void)
{
	updateTreadObjects();
}

//-------------------------------------------------------------------------------------------------
/** Map behavior states into W3D animations. */
//-------------------------------------------------------------------------------------------------
void W3DTankDraw::doDrawModule(const Matrix3D* transformMtx)
{
	W3DModelDraw::doDrawModule(transformMtx);

	// TheSuperHackers @tweak Update the draw on every WW Sync only.
	// All calculations are originally catered to a 30 fps logic step.
	if (WW3D::Get_Sync_Frame_Time() == 0)
		return;

	const Real DEBRIS_THRESHOLD = 0.00001f;

	if (getRenderObject()==NULL) return;
	if (getRenderObject() != m_prevRenderObj) {
		updateTreadObjects();
	}

	// get object from logic
	Object *obj = getDrawable()->getObject();
	if (obj == NULL)
		return;

	// get object physics state
	PhysicsBehavior *physics = obj->getPhysics();
	if (physics == NULL)
		return;

	const Coord3D *vel = physics->getVelocity();

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

	//Update movement of treads
	if (m_treadCount)
	{
		PhysicsTurningType turn=physics->getTurning();
		Real offset_u;
		Real treadScrollSpeed=getW3DTankDrawModuleData()->m_treadAnimationRate;
		TreadObjectInfo *pTread=m_treads;
		Real maxSpeed=obj->getAIUpdateInterface()->getCurLocomotorSpeed();

		//For optimization sake, we only do complex tread scrolling when tank
		//is mostly stationary and turning
		if (turn != TURN_NONE && physics->getVelocityMagnitude()/maxSpeed < getW3DTankDrawModuleData()->m_treadPivotSpeedFraction)
		{
				//Check if we have turned enough since last draw to require animation
				Coord3D dir;
				obj->getUnitDirectionVector2D(dir);
				Real angleToGoal = dir.x * m_lastDirection.x + dir.y * m_lastDirection.y;

				if (fabs(1.0f-angleToGoal) > 0.00001f)	//check if difference in angle cosines is greater than some cutoff.
				{
					if (turn == TURN_NEGATIVE)	//turning right
						updateTreadPositions(-treadScrollSpeed);
					else	//turning left
						updateTreadPositions(treadScrollSpeed);
				}
				m_lastDirection=dir;	//update for next frame
		}
		else
		if (physics->isMotive() && physics->getVelocityMagnitude()/maxSpeed >= getW3DTankDrawModuleData()->m_treadDriveSpeedFraction)
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
void W3DTankDraw::crc( Xfer *xfer )
{

	// extend base class
	W3DModelDraw::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DTankDraw::xfer( Xfer *xfer )
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
void W3DTankDraw::loadPostProcess( void )
{

	// extend base class
	W3DModelDraw::loadPostProcess();

	// toss any existing ones and re-create 'em (since this module expects 'em to always be around)
	tossEmitters();
	createEmitters();

}
