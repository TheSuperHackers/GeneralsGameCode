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

// W3DParticleSys.cpp
// W3D Particle System implementation
// Author: Michael S. Booth, November 2001

#include "Common/GlobalData.h"
#include "GameClient/Color.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DSmudge.h"
#include "W3DDevice/GameClient/W3DSnow.h"
#include "WW3D2/camera.h"


//------------------------------------------------------------------------------ Performance Timers
//#include "Common/PerfMetrics.h"
//#include "Common/PerfTimer.h"

//-------------------------------------------------------------------------------------------------


W3DParticleSystemManager::W3DParticleSystemManager()
{
	m_pointGroup = nullptr;
	m_streakLine = nullptr;
	m_posBuffer = nullptr;
	m_RGBABuffer = nullptr;
	m_sizeBuffer = nullptr;
	m_angleBuffer = nullptr;
	m_readyToRender = false;

	m_onScreenParticleCount = 0;

	m_pointGroup = NEW PointGroupClass();
	//m_streakLine = nullptr;
	m_streakLine = NEW StreakLineClass();

	m_posBuffer = NEW_REF( ShareBufferClass<Vector3>, (MAX_POINTS_PER_GROUP, "W3DParticleSystemManager::m_posBuffer") );
	m_RGBABuffer = NEW_REF( ShareBufferClass<Vector4>, (MAX_POINTS_PER_GROUP, "W3DParticleSystemManager::m_RGBABuffer") );
	m_sizeBuffer = NEW_REF( ShareBufferClass<float>, (MAX_POINTS_PER_GROUP, "W3DParticleSystemManager::m_sizeBuffer") );
	m_angleBuffer = NEW_REF( ShareBufferClass<uint8>, (MAX_POINTS_PER_GROUP, "W3DParticleSystemManager::m_angleBuffer") );
}

W3DParticleSystemManager::~W3DParticleSystemManager()
{
	delete m_pointGroup;

//	W3DDisplay::m_3DScene->Remove_Render_Object( m_streakLine );

	if (m_streakLine)
	{
		REF_PTR_RELEASE(m_streakLine);
	}

	REF_PTR_RELEASE(m_posBuffer);
	REF_PTR_RELEASE(m_RGBABuffer);
	REF_PTR_RELEASE(m_sizeBuffer);
	REF_PTR_RELEASE(m_angleBuffer);
}

/**
 * Hack because DoParticles is called from Flush(), which is called
 * multiple times per frame.  We only want to render once.
 * @todo Clean up the flag/Flush hack.
 */
void W3DParticleSystemManager::queueParticleRender()
{
	m_readyToRender = true;
}

/**
 * Nasty hack to render particles last. Called directly by WW3D::Flush()
 */
void DoParticles( RenderInfoClass &rinfo )
{
	if (TheParticleSystemManager)
		TheParticleSystemManager->doParticles(rinfo);
}

void W3DParticleSystemManager::doParticles(RenderInfoClass &rinfo)
{

	if (m_readyToRender == false)
		return;

	// external mechanism must tell us when it's OK to render again...
	m_readyToRender = false;

	//reset each frame
	/// @todo lorenzen sez: this should be debug only:
	m_onScreenParticleCount = 0;

	Int visibleSmudgeCount = 0;
	if (TheSmudgeManager)
		TheSmudgeManager->setSmudgeCountLastFrame(0);	//keep track of visible smudges

 	const FrustumClass & frustum = rinfo.Camera.Get_Frustum();
	AABoxClass bbox;

	//Get a bounding box around our visible universe.  Bounded by terrain and the sky
	//so much tighter fitting volume than what's actually visible.  This will cull
	//particles falling under the ground.

 	TheTerrainRenderObject->getMaximumVisibleBox(frustum, &bbox, TRUE);

	//@todo lorenzen sez: put these in registers for sure
	Real bcX = bbox.Center.X;
	Real bcY = bbox.Center.Y;
	Real bcZ = bbox.Center.Z;
	Real beX = bbox.Extent.X;
	Real beY = bbox.Extent.Y;
	Real beZ = bbox.Extent.Z;

	unsigned int personalities[MAX_POINTS_PER_GROUP];


	m_fieldParticleCount = 0;

	SmudgeSet *set=nullptr;
	if (TheSmudgeManager)
		set=TheSmudgeManager->addSmudgeSet();	//global smudge set through which all smudges are rendered.

	ParticleSystemManager::ParticleSystemList &particleSysList = TheParticleSystemManager->getAllParticleSystems();
	for( ParticleSystemManager::ParticleSystemListIt it = particleSysList.begin(); it != particleSysList.end(); ++it)
	{
		ParticleSystem *sys = (*it);
		if (!sys) {
			continue;
		}

		// only look at particle/point style systems
		if (sys->isUsingDrawables())
			continue;

		//temporary hack that checks if texture name starts with "SMUD" - if so, we can assume it's a smudge type
		if (/*sys->isUsingSmudge()*/ *((DWORD *)sys->getParticleTypeName().str()) == 0x44554D53)
		{
			if (TheSmudgeManager && ((W3DSmudgeManager*)TheSmudgeManager)->getHardwareSupport() && TheGlobalData->m_useHeatEffects)
			{
				//set-up all the per-particle
				for (Particle *p = sys->getFirstParticle(); p; p = p->m_systemNext)
				{
					const Coord3D *pos = p->getPosition();
					Real psize = p->getSize();

					//Cull particle to edges of screen and terrain.
					if (WWMath::Fabs( pos->x - bcX ) > ( beX + psize ) )
						continue;

					if (WWMath::Fabs( pos->y - bcY ) > ( beY + psize ) )
						continue;

					if (WWMath::Fabs( pos->z - bcZ ) > ( beZ + psize ) )
						continue;

					Smudge *smudge = set->addSmudgeToSet();

					smudge->m_pos.Set( pos->x, pos->y, pos->z );
					smudge->m_offset.Set( GameClientRandomValueReal(-0.06f,0.06f), GameClientRandomValueReal(-0.03f,0.03f) );
					smudge->m_size = psize;
					smudge->m_opacity = p->getAlpha();
					visibleSmudgeCount++;
				}
			}
			continue;
		}

		/// @todo lorenzen sez: declare these outside the sys loop, and put some in registers
		// initialize them here still, of course
		// build W3D particle buffer
		Int count = 0;
		Vector3 *posArray = m_posBuffer->Get_Array();
		Real *sizeArray = m_sizeBuffer->Get_Array();
		Vector4 *RGBAArray = m_RGBABuffer->Get_Array();
		uint8 *angleArray = m_angleBuffer->Get_Array();
		const Coord3D *pos;
		const RGBColor *color;
		Real psize;



		//set-up all the per-particle
		for (Particle *p = sys->getFirstParticle(); p; p = p->m_systemNext)
		{
			pos = p->getPosition();
			psize = p->getSize();

			//Cull particle to edges of screen and terrain.
			if (WWMath::Fabs(pos->x - bcX) > (beX + psize))
				continue;

			if (WWMath::Fabs(pos->y - bcY) > (beY + psize))
				continue;

			if (WWMath::Fabs(pos->z - bcZ) > (beZ + psize))
				continue;

			m_fieldParticleCount += ( sys->getPriority() == AREA_EFFECT && sys->m_isGroundAligned != FALSE );

			//@todo lorenzen sez: use pointer arithmetic for these arrays
			personalities[count] = p->getPersonality();

			posArray[count].X = pos->x;
			posArray[count].Y = pos->y;
			posArray[count].Z = pos->z;

			sizeArray[count] = psize;

			color = p->getColor();
			RGBAArray[count].X = color->red;
			RGBAArray[count].Y = color->green;
			RGBAArray[count].Z = color->blue;
			RGBAArray[count].W = p->getAlpha();

			angleArray[count] = (uint8)(p->getAngle() * 255.0f / (2.0f * PI));

			if (++count == MAX_POINTS_PER_GROUP)
				break;
		}

		if ( count == 0 )
			continue;	//this system has no particles to render

		TextureClass *texture = W3DDisplay::m_assetManager->Get_Texture( sys->getParticleTypeName().str() );

		if ( m_streakLine && sys->isUsingStreak() && (count >= 2) )
		{
			m_streakLine->Reset_Line();

			m_streakLine->Set_Texture( texture );
			texture->Release_Ref();//release reference since it's held by streakline
			switch( sys->getShaderType() )
			{
				case ParticleSystemInfo::ADDITIVE:
					m_streakLine->Set_Shader( ShaderClass::_PresetAdditiveSpriteShader );
					break;
				case ParticleSystemInfo::ALPHA:
					m_streakLine->Set_Shader( ShaderClass::_PresetAlphaSpriteShader );
					break;
				case ParticleSystemInfo::ALPHA_TEST:
					m_streakLine->Set_Shader( ShaderClass::_PresetATestSpriteShader );
					break;
				case ParticleSystemInfo::MULTIPLY:
					m_streakLine->Set_Shader( ShaderClass::_PresetMultiplicativeSpriteShader );
					break;
			}

			//UPDATE THE STREAK'S ARRAYS
			m_streakLine->Set_LocsWidthsColors(
				count,
				m_posBuffer->Get_Array(),
				m_sizeBuffer->Get_Array(),
				m_RGBABuffer->Get_Array(),
				&personalities[0]
				);

			//WWASSERT( m_streakLine->Get_Num_Points() == count );

			// This is the happy place for this!
			RGBAArray[0].X = 0;//eliminates the scissor edge on the trailing edge of the streak
			RGBAArray[0].Y = 0;
			RGBAArray[0].Z = 0;
			RGBAArray[0].W = 0;


			//RENDER STREAK!
			m_streakLine->Render( rinfo );

		}
		else
		{

			WWASSERT( m_pointGroup );

			if ( m_pointGroup ) // this catches the particle and volumeparticle cases
			{
				// render all the systems' particles
				m_pointGroup->Set_Texture( texture );
				texture->Release_Ref();//release reference since it's held by pointGroup
				m_pointGroup->Set_Flag( PointGroupClass::TRANSFORM, true );	// transform to screen space

				switch( sys->getShaderType() )
				{
					case ParticleSystemInfo::ADDITIVE:
						m_pointGroup->Set_Shader( ShaderClass::_PresetAdditiveSpriteShader );
						break;
					case ParticleSystemInfo::ALPHA:
						m_pointGroup->Set_Shader( ShaderClass::_PresetAlphaSpriteShader );
						break;
					case ParticleSystemInfo::ALPHA_TEST:
						m_pointGroup->Set_Shader( ShaderClass::_PresetATestSpriteShader );
						break;
					case ParticleSystemInfo::MULTIPLY:
						m_pointGroup->Set_Shader( ShaderClass::_PresetMultiplicativeSpriteShader );
						break;
				}

				/// @todo Use both QUADS and TRIS for particles
				m_pointGroup->Set_Point_Mode( PointGroupClass::QUADS );
				m_pointGroup->Set_Arrays( m_posBuffer, m_RGBABuffer, nullptr, m_sizeBuffer, m_angleBuffer, nullptr, count );
				m_pointGroup->Set_Billboard(sys->shouldBillboard());

				/// @todo Support animated texture particles
				/// @todo lorenzen sez: unimplemented code wastes cpu cycles
				m_pointGroup->Set_Point_Frame( 0 );

				//RENDER IT!
				if( sys->getVolumeParticleDepth() > 1 )
				{
					m_pointGroup->RenderVolumeParticle( rinfo, sys->getVolumeParticleDepth() );
				}
				else
					m_pointGroup->Render( rinfo );

			}
		}


		/// @todo lorenzen sez: this should be debug only:
		//add particle count to total
		m_onScreenParticleCount += count;

	/*
		// draw the wind vector for this particle system on the screen
		UnsignedInt width = TheDisplay->getWidth();
		UnsignedInt height = TheDisplay->getHeight();
		Coord3D worldStart, worldEnd;
		ICoord2D pixelStart, pixelEnd;
		sys->getPosition( &worldStart );
		worldEnd.x = Cos( sys->getWindAngle() ) * 50.0f + worldStart.x;
		worldEnd.y = Sin( sys->getWindAngle() ) * 50.0f + worldStart.y;
		worldEnd.z = worldStart.z;
		TheTacticalView->worldToScreen( &worldStart, &pixelStart );
		TheTacticalView->worldToScreen( &worldEnd, &pixelEnd );
		Color colorStart = GameMakeColor( 255, 255, 255, 255 );
		Color colorEnd = GameMakeColor( 255, 128, 128, 255 );
		TheDisplay->drawLine( pixelStart.x, pixelStart.y, pixelEnd.x, pixelEnd.y, 1.0f, colorStart, colorEnd );
	*/


	}

		/// @todo lorenzen sez: this should be debug only:
	TheParticleSystemManager->setOnScreenParticleCount(m_onScreenParticleCount);

	//Draw any particles belonging to weather effects
	if (TheSnowManager)
		((W3DSnowManager *)TheSnowManager)->render(rinfo);

	//Now process screen smudges which are particles that distort the background behind them.
	if(TheSmudgeManager)
	{
		((W3DSmudgeManager *)TheSmudgeManager)->render(rinfo);
		TheSmudgeManager->reset();	//clear all the smudges after rendering since we fill again each frame.
		TheSmudgeManager->setSmudgeCountLastFrame(visibleSmudgeCount);
	}
}
