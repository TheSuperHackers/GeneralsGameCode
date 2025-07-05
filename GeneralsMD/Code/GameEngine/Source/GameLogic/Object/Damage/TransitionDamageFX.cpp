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

// FILE: TransitionDamageFX.cpp ///////////////////////////////////////////////////////////////////
// Author: Colin Day, March 2002
// Desc:   Damage module capable of launching various effects on damage transitions
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/Module/TransitionDamageFX.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TransitionDamageFXModuleData::TransitionDamageFXModuleData( void )
{
	Int i, j;

	for( i = 0; i < BODYDAMAGETYPE_COUNT; i++ )
	{

		for( j = 0; j < DAMAGE_MODULE_MAX_FX; j++ )
		{

			m_fxList[ i ][ j ].fx = NULL;
			m_fxList[ i ][ j ].locInfo.loc.x = 0.0f;
			m_fxList[ i ][ j ].locInfo.loc.y = 0.0f;
			m_fxList[ i ][ j ].locInfo.loc.z = 0.0f;
			m_fxList[ i ][ j ].locInfo.locType = FX_DAMAGE_LOC_TYPE_COORD;
			m_fxList[ i ][ j ].locInfo.randomBone = FALSE;
			m_OCL[ i ][ j ].ocl = NULL;
			m_OCL[ i ][ j ].locInfo.loc.x = 0.0f;
			m_OCL[ i ][ j ].locInfo.loc.y = 0.0f;
			m_OCL[ i ][ j ].locInfo.loc.z = 0.0f;
			m_OCL[ i ][ j ].locInfo.locType = FX_DAMAGE_LOC_TYPE_COORD;
			m_OCL[ i ][ j ].locInfo.randomBone = FALSE;
			m_particleSystem[ i ][ j ].particleSysTemplate = NULL;
			m_particleSystem[ i ][ j ].locInfo.loc.x = 0.0f;
			m_particleSystem[ i ][ j ].locInfo.loc.y = 0.0f;
			m_particleSystem[ i ][ j ].locInfo.loc.z = 0.0f;
			m_particleSystem[ i ][ j ].locInfo.locType = FX_DAMAGE_LOC_TYPE_COORD;
			m_particleSystem[ i ][ j ].locInfo.randomBone = FALSE;

		}  // end for j

	}  // end for i

	m_damageFXTypes = DAMAGE_TYPE_FLAGS_NONE;
	m_damageFXTypes.flip();
	m_damageOCLTypes = DAMAGE_TYPE_FLAGS_NONE;
	m_damageOCLTypes.flip();
	m_damageParticleTypes = DAMAGE_TYPE_FLAGS_NONE;
	m_damageParticleTypes.flip();

}  // end TransitionDamageFXModuleData

//-------------------------------------------------------------------------------------------------
/** Parse fx location info ... that is a named bone or a coord3d position */
//-------------------------------------------------------------------------------------------------
static void parseFXLocInfo( INI *ini, void *instance, FXLocInfo *locInfo )
{
	const char *token = ini->getNextToken( ini->getSepsColon() );

	if( stricmp( token, "bone" ) == 0 )
	{

		// save bone name and location type
		locInfo->boneName = AsciiString( ini->getNextToken() );
		locInfo->locType = FX_DAMAGE_LOC_TYPE_BONE;

		//
		// bones are followed by RandomBone:<Yes|No>, if random bone is yes, the bone name is
		// assumed to be a "base bone name" and we will find all the bones with that prefix
		// when picking an effect position.  If it's no, the bone name is assumed to be explicit
		//
		token = ini->getNextToken( ini->getSepsColon() );
		if( stricmp( token, "randombone" ) != 0 )
		{

			DEBUG_CRASH(( "parseFXLocInfo: Bone name not followed by RandomBone specifier\nPress IGNORE to see which INI file and line # is incorrect." ));
			throw INI_INVALID_DATA;

		}  // end if

		// parse the Bool definition
		ini->parseBool( ini, instance, &locInfo->randomBone, NULL );

	}  // end if
	else if( stricmp( token, "loc" ) == 0 )
	{

		// save location and location type
		locInfo->loc.x = ini->scanReal( ini->getNextSubToken("X") );
		locInfo->loc.y = ini->scanReal( ini->getNextSubToken("Y") );
		locInfo->loc.z = ini->scanReal( ini->getNextSubToken("Z") );
		locInfo->locType = FX_DAMAGE_LOC_TYPE_COORD;

	}  // end else
	else
	{

		// error
		throw INI_INVALID_DATA;

	}  // end else

}  // end parseFXLocInfo

//-------------------------------------------------------------------------------------------------
/** In the form of:
	* FXListSlot = <<Bone:BoneName BoneRandom:<Yes|No>> | <Loc: X:x Y:y Z:z>> FXList:FXListName */
//-------------------------------------------------------------------------------------------------
void TransitionDamageFXModuleData::parseFXList( INI *ini, void *instance, 
																								void *store, const void *userData )
{
	const char *token;
	FXDamageFXListInfo *info = (FXDamageFXListInfo *)store;

	// parse the location bone or location
	parseFXLocInfo( ini, instance, &info->locInfo );

	// make sure we have an "FXList:" token
	token = ini->getNextToken( ini->getSepsColon() );
	if( stricmp( token, "fxlist" ) != 0 )
	{

		// error
		throw INI_INVALID_DATA;

	}  // end if

	// parse the fx list name
	ini->parseFXList( ini, instance, &info->fx, NULL );

}  // end parseFXList

//-------------------------------------------------------------------------------------------------
/** In the form of:
	* OCLSlot = <<Bone:BoneName BoneRandom:<Yes|No>> | <Loc: X:x Y:y Z:z>> OCL:OCLName */
//-------------------------------------------------------------------------------------------------
void TransitionDamageFXModuleData::parseObjectCreationList( INI *ini, void *instance, 
																														void *store, const void *userData )
{
	const char *token;
	FXDamageOCLInfo *info = (FXDamageOCLInfo *)store;

	// parse the location bone or location
	parseFXLocInfo( ini, instance, &info->locInfo );

	// make sure we have an "OCL:" token
	token = ini->getNextToken( ini->getSepsColon() );
	if( stricmp( token, "ocl" ) != 0 )
	{

		// error
		throw INI_INVALID_DATA;

	}  // end if

	// parse the ocl name
	ini->parseObjectCreationList( ini, instance, store, &info->ocl );

}  // end parseObjectCreationList

//-------------------------------------------------------------------------------------------------
/** In the form of:
	* ParticleSlot = <<Bone:BoneName BoneRandom:<Yes|No>> | <Loc: X:x Y:y Z:z>> PSys:PSysName */
//-------------------------------------------------------------------------------------------------
void TransitionDamageFXModuleData::parseParticleSystem( INI *ini, void *instance, 
																												void *store, const void *userData )
{
	const char *token;
	FXDamageParticleSystemInfo *info = (FXDamageParticleSystemInfo *)store;

	// parse the location bone or location
	parseFXLocInfo( ini, instance, &info->locInfo );

	// make sure we have an "PSys:" token
	token = ini->getNextToken( ini->getSepsColon() );
	if( stricmp( token, "psys" ) != 0 )
	{

		// error
		throw INI_INVALID_DATA;

	}  // end if

	// parse the particle system name
	ini->parseParticleSystemTemplate( ini, instance, store, &info->particleSysTemplate );

}  // end parseParticleSystem

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TransitionDamageFX::TransitionDamageFX( Thing *thing, const ModuleData* moduleData ) 
																		  : DamageModule( thing, moduleData )
{
	Int i, j;

	for( i = 0; i < BODYDAMAGETYPE_COUNT; i++ )
		for( j = 0; j < DAMAGE_MODULE_MAX_FX; j++ )
			m_particleSystemID[ i ][ j ] = INVALID_PARTICLE_SYSTEM_ID;

}  // end TransitionDamageFX

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TransitionDamageFX::~TransitionDamageFX( void )
{

}  // end ~TransitionDamageFX

/*
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void TransitionDamageFX::onDelete( void )
{

	//
	// we would in theory delete any particle systems we have created and attached, but the
	// particle system will automatically delete itself when the object is destroyed
	//

}  // end onDelete
*/

//-------------------------------------------------------------------------------------------------
/** Given an FXLoc info struct, return the effect position that we are supposed to use. 
	* The position is local to to the object */
//-------------------------------------------------------------------------------------------------
static Coord3D getLocalEffectPos( const FXLocInfo *locInfo, Drawable *draw )
{

	DEBUG_ASSERTCRASH( locInfo, ("getLocalEffectPos: locInfo is NULL") );

	if( locInfo->locType == FX_DAMAGE_LOC_TYPE_BONE && draw )
	{

		if( locInfo->randomBone == FALSE )
		{
			Coord3D pos;
		
			// get the bone position
			Int count = draw->getPristineBonePositions( locInfo->boneName.str(), 0, &pos, NULL, 1 );

			// sanity, if bone not found revert back to location defined in struct (which is 0,0,0)
			if( count == 0 )
				return locInfo->loc;

			// return the position retrieved
			return pos;

		}  // end if
		else
		{
		  const Int MAX_BONES = 32;
			Coord3D positions[ MAX_BONES ];

			// get the bone positions
			Int boneCount;
			boneCount = draw->getPristineBonePositions( locInfo->boneName.str(), 1, positions, NULL, MAX_BONES );

			// sanity, if bone not found revert back to location defined in struct (which is 0,0,0)
			if( boneCount == 0 )
				return locInfo->loc;

			// pick one of the bone positions
			Int pick = GameLogicRandomValue( 0, boneCount - 1 );
			return positions[ pick ];

		}  // end else

	}  // end if
	else
		return locInfo->loc;

}  // end getLocalEffectPos

//-------------------------------------------------------------------------------------------------
/** Switching damage states */
//-------------------------------------------------------------------------------------------------
void TransitionDamageFX::onBodyDamageStateChange( const DamageInfo* damageInfo, 
																									BodyDamageType oldState, 
																									BodyDamageType newState )
{
	Object *damageSource = NULL;
	Int i;
	Drawable *draw = getObject()->getDrawable();
	const TransitionDamageFXModuleData *modData = getTransitionDamageFXModuleData();

	// get the source of the damage if present
	if( damageInfo )
		damageSource = TheGameLogic->findObjectByID( damageInfo->in.m_sourceID );
	
	// remove any particle systems that might be emitting from our old state
	for( i = 0; i < DAMAGE_MODULE_MAX_FX; i++ )
	{

		if( m_particleSystemID[ oldState ][ i ] != INVALID_PARTICLE_SYSTEM_ID )
		{

			TheParticleSystemManager->destroyParticleSystemByID( m_particleSystemID[ oldState ][ i ] );
			m_particleSystemID[ oldState ][ i ] = INVALID_PARTICLE_SYSTEM_ID;

		}  // end if

	}  // end for i

	//
	// when we are transitioning to a "worse" state we will play a set of effects for that
	// new state to make the transition
	//
	if( IS_CONDITION_WORSE( newState, oldState ) )
	{
		const ParticleSystemTemplate *pSystemT;
		Coord3D pos;

		// if we are restricted by the damage type executing effect, bail out of here
		const DamageInfo *lastDamageInfo = getObject()->getBodyModule()->getLastDamageInfo();

		for( i = 0; i < DAMAGE_MODULE_MAX_FX; i++ )
		{

			// play fx list for our new state
			if( modData->m_fxList[ newState ][ i ].fx )
			{

				if( lastDamageInfo == NULL || 
						getDamageTypeFlag( modData->m_damageFXTypes, lastDamageInfo->in.m_damageType ) )
				{

					pos = getLocalEffectPos( &modData->m_fxList[ newState ][ i ].locInfo, draw );
					getObject()->convertBonePosToWorldPos( &pos, NULL, &pos, NULL );
					FXList::doFXPos( modData->m_fxList[ newState ][ i ].fx, &pos );

				}  // end if

			}  // end if
					
			// do any object creation list for our new state
			if( damageSource && modData->m_OCL[ newState ][ i ].ocl )
			{

				if( lastDamageInfo == NULL || 
						getDamageTypeFlag( modData->m_damageOCLTypes, lastDamageInfo->in.m_damageType ) )
				{

					pos = getLocalEffectPos( &modData->m_OCL[ newState ][ i ].locInfo, draw );
					getObject()->convertBonePosToWorldPos( &pos, NULL, &pos, NULL );
					ObjectCreationList::create( modData->m_OCL[ newState ][ i ].ocl, 
																			getObject(), &pos, damageSource->getPosition(), INVALID_ANGLE );

				}  // end if

			}  // end if

			// get the template of the system to create
			pSystemT = modData->m_particleSystem[ newState ][ i ].particleSysTemplate;
			if( pSystemT )
			{

				if( lastDamageInfo == NULL || 
						getDamageTypeFlag( modData->m_damageParticleTypes, lastDamageInfo->in.m_damageType ) )
				{

					// create a new particle system based on the template provided
					ParticleSystem* pSystem = TheParticleSystemManager->createParticleSystem( pSystemT );
					if( pSystem )
					{
			
						// get the what is the position we're going to playe the effect at
						pos = getLocalEffectPos( &modData->m_particleSystem[ newState ][ i ].locInfo, draw );

						//
						// set position on system given any bone position provided, the bone position is
						// local to the object and that's what we want for the particle system ... the
						// transormation into world space using the object position is taken care of in
						// the particle system attachToObject method
						//
						pSystem->setPosition( &pos );

						// attach to object
						pSystem->attachToObject( getObject() );

						// save the id of this particle system so we can remove it later if it still exists
						m_particleSystemID[ newState ][ i ] = pSystem->getSystemID();

					}  // end if

				}  // end if

			}  // end if
						
		}  // end for i

	}  // end if
			
}  // end onBodyDamageStateChange

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TransitionDamageFX::crc( Xfer *xfer )
{

	// extend base class
	DamageModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void TransitionDamageFX::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DamageModule::xfer( xfer );

	// particle systems ids
	xfer->xferUser( m_particleSystemID, sizeof( ParticleSystemID ) * BODYDAMAGETYPE_COUNT * DAMAGE_MODULE_MAX_FX );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TransitionDamageFX::loadPostProcess( void )
{

	// extend base class
	DamageModule::loadPostProcess();

}  // end loadPostProcess
