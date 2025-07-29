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

// FILE: W3DDebrisDraw.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   Default w3d draw module
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

#include "Common/FileSystem.h"	// this is only here to pull in LOAD_TEST_ASSETS
#include "Common/GlobalData.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Object.h"
#include "GameClient/Shadow.h"
#include "GameClient/FXList.h"
#include "GameLogic/TerrainLogic.h"

#include "WW3D2/hanim.h"
#include "WW3D2/hlod.h"
#include "WW3D2/rendobj.h"
#include "W3DDevice/GameClient/Module/W3DDebrisDraw.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShadow.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DDebrisDraw::W3DDebrisDraw(Thing *thing, const ModuleData* moduleData) : DrawModule(thing, moduleData)
{
  m_renderObject = NULL;
	for (int i = 0; i < STATECOUNT; ++i)
		m_anims[i] = NULL;
	m_fxFinal = NULL;
	m_state = INITIAL;
	m_frames = 0;
	m_shadow = NULL;
	m_finalStop = false;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DDebrisDraw::~W3DDebrisDraw(void)
{
	if (TheW3DShadowManager && m_shadow)
	{	
		TheW3DShadowManager->removeShadow(m_shadow);
		m_shadow = NULL;
	}
	if (m_renderObject)
	{
		if (W3DDisplay::m_3DScene != NULL)
			W3DDisplay::m_3DScene->Remove_Render_Object(m_renderObject);
  	REF_PTR_RELEASE(m_renderObject);
 		m_renderObject = NULL; 	
	}
	for (int i = 0; i < STATECOUNT; ++i)
	{
		REF_PTR_RELEASE(m_anims[i]);
		m_anims[i] = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
void W3DDebrisDraw::setShadowsEnabled(Bool enable)
{
	if (m_shadow)
		m_shadow->enableShadowRender(enable);
}

//-------------------------------------------------------------------------------------------------
void W3DDebrisDraw::setFullyObscuredByShroud(Bool fullyObscured)
{
	if (m_shadow)
		m_shadow->enableShadowInvisible(fullyObscured);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DDebrisDraw::setModelName(AsciiString name, Color color, ShadowType t)
{
  if (m_renderObject == NULL && !name.isEmpty())
	{
		Int hexColor = 0;
		if (color != 0)
			hexColor = color | 0xFF000000;
		m_renderObject = W3DDisplay::m_assetManager->Create_Render_Obj(name.str(), getDrawable()->getScale(), hexColor);
		DEBUG_ASSERTCRASH(m_renderObject, ("Debris model %s not found!",name.str()));
		if (m_renderObject)
		{
			if (W3DDisplay::m_3DScene != NULL)
				W3DDisplay::m_3DScene->Add_Render_Object(m_renderObject);

			m_renderObject->Set_User_Data(getDrawable()->getDrawableInfo());
			
			Matrix3D transform;
			///@todo: Change back to identity once we figure out why objects show up at 0,0,0
			/// OBJECT_PILE
//			transform.Set(Vector3(0,0,9999));
			transform.Set(Vector3(0,0,0));
			m_renderObject->Set_Transform(transform);
		}
		
		if (t != SHADOW_NONE)
		{
			Shadow::ShadowTypeInfo shadowInfo;
			shadowInfo.m_type = t;
			shadowInfo.m_sizeX=0;
			shadowInfo.m_sizeY=0;
  		m_shadow = TheW3DShadowManager->addShadow(m_renderObject, &shadowInfo);
		}
		else
		{
			if (TheW3DShadowManager && m_shadow)
			{	
				TheW3DShadowManager->removeShadow(m_shadow);
				m_shadow = NULL;
			}
		}

		// save the model name and color
		m_modelName = name;
		m_modelColor = color;

	}
}

//-------------------------------------------------------------------------------------------------
void W3DDebrisDraw::setAnimNames(AsciiString initial, AsciiString flying, AsciiString final, const FXList* finalFX)
{
	int i;
	for (i = 0; i < STATECOUNT; ++i)
	{
		REF_PTR_RELEASE(m_anims[i]);
		m_anims[i] = NULL;
	}

	m_anims[INITIAL] = initial.isEmpty() ? NULL : W3DDisplay::m_assetManager->Get_HAnim(initial.str());
	m_anims[FLYING] = flying.isEmpty() ? NULL : W3DDisplay::m_assetManager->Get_HAnim(flying.str());
	if (stricmp(final.str(), "STOP") == 0)
	{
		m_finalStop = true;
		final = flying;
	}
	else
	{
		m_finalStop = false;
	}
	m_anims[FINAL] = final.isEmpty() ? NULL : W3DDisplay::m_assetManager->Get_HAnim(final.str());
	m_state = 0;
	m_frames = 0;
	m_fxFinal = finalFX;

	m_animInitial = initial;
	m_animFlying = flying;
	m_animFinal = final;

}

//-------------------------------------------------------------------------------------------------
static Bool isAnimationComplete(RenderObjClass* r)
{
	if (r->Class_ID() == RenderObjClass::CLASSID_HLOD)
	{
		HLodClass *hlod = (HLodClass*)r;
		return hlod->Is_Animation_Complete();
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
static Bool isNearlyZero(const Coord3D* vel)
{
	const Real TINY = 0.01f;
	return fabs(vel->x) < TINY && fabs(vel->y) < TINY && fabs(vel->z) < TINY;
}

// ------------------------------------------------------------------------------------------------
void W3DDebrisDraw::reactToTransformChange( const Matrix3D *oldMtx, 
																						const Coord3D *oldPos, 
																						Real oldAngle )
{

	if( m_renderObject )
		m_renderObject->Set_Transform( *getDrawable()->getTransformMatrix() );

}

//-------------------------------------------------------------------------------------------------
void W3DDebrisDraw::doDrawModule(const Matrix3D* transformMtx)
{
	if (m_renderObject)
	{

		Matrix3D scaledTransform;
		if (getDrawable()->getInstanceScale() != 1.0f)
		{	//do custom scaling of the W3D model.
			scaledTransform=*transformMtx;
			scaledTransform.Scale(getDrawable()->getInstanceScale());
			transformMtx = &scaledTransform;
			m_renderObject->Set_ObjectScale(getDrawable()->getInstanceScale());
		}
		m_renderObject->Set_Transform(*transformMtx);

		static const RenderObjClass::AnimMode TheAnimModes[STATECOUNT] = 
		{
			RenderObjClass::ANIM_MODE_ONCE,
			RenderObjClass::ANIM_MODE_LOOP,
			RenderObjClass::ANIM_MODE_ONCE
		};
		
		Int oldState = m_state;
		Object* obj = getDrawable()->getObject();
		const Int MIN_FINAL_FRAMES = 3;
		if (m_state != FINAL && obj != NULL && !obj->isAboveTerrain() && m_frames > MIN_FINAL_FRAMES)
		{
			m_state = FINAL;
		}
		else if (m_state < FINAL && (isAnimationComplete(m_renderObject)))
		{
			++m_state;
		}
		HAnimClass* hanim = m_anims[m_state];
		if (hanim != NULL && (hanim != m_renderObject->Peek_Animation() || oldState != m_state))
		{
			RenderObjClass::AnimMode m = TheAnimModes[m_state];
			if (m_state == FINAL)
			{
				FXList::doFXPos(m_fxFinal, getDrawable()->getPosition(), getDrawable()->getTransformMatrix(), 0, NULL, 0.0f);
				if (m_finalStop)
					m = RenderObjClass::ANIM_MODE_MANUAL;
			}
			m_renderObject->Set_Animation(hanim, 0, m);
		}
		++m_frames;
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DDebrisDraw::crc( Xfer *xfer )
{

	// extend base class
	DrawModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DDebrisDraw::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DrawModule::xfer( xfer );

	// model name
	xfer->xferAsciiString( &m_modelName );
	
	// model color
	xfer->xferColor( &m_modelColor );

	// set the model and color
	if( xfer->getXferMode() == XFER_LOAD )
		setModelName( m_modelName, m_modelColor, SHADOW_NONE );

	// animation initial
	xfer->xferAsciiString( &m_animInitial );

	// anim flying
	xfer->xferAsciiString( &m_animFlying );

	// anim final
	xfer->xferAsciiString( &m_animFinal );

	// when loading, set the animations
	if( xfer->getXferMode() == XFER_LOAD )
		setAnimNames( m_animInitial, m_animFlying, m_animFinal, NULL );

	// state
	xfer->xferInt( &m_state );

	// frames
	xfer->xferInt( &m_frames );

	// final stop
	xfer->xferBool( &m_finalStop );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DDebrisDraw::loadPostProcess( void )
{

	// extend base class
	DrawModule::loadPostProcess();

}  // end loadPostProcess
