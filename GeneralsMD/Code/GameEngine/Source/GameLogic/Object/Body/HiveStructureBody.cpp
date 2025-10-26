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

// FILE: HiveStructureBody.cpp ////////////////////////////////////////////////////////////////////////
// Desc:	 Hive structure bodies are structure bodies with the ability to propagate specified
//         damage types to slaves when available. If there are no slaves, the the structure
//         will take the damage.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "Common/ThingTemplate.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Damage.h"

#include "GameLogic/Module/HiveStructureBody.h"
#include "GameLogic/Module/SpawnBehavior.h"

#include "GameLogic/Module/ContainModule.h"


// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
HiveStructureBodyModuleData::HiveStructureBodyModuleData()
{
	m_damageTypesToPropagateToSlaves = DAMAGE_TYPE_FLAGS_NONE;
	m_damageTypesToSwallow = DAMAGE_TYPE_FLAGS_NONE;
}


//-------------------------------------------------------------------------------------------------
HiveStructureBody::HiveStructureBody( Thing *thing, const ModuleData* moduleData )
							: StructureBody( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
HiveStructureBody::~HiveStructureBody( void )
{
}

//------------------------------------------------------------------------------------------------
void HiveStructureBody::attemptDamage( DamageInfo *damageInfo )
{
	const HiveStructureBodyModuleData *data = getHiveStructureBodyModuleData();
	Object *hive = getObject();

	if( getDamageTypeFlag( data->m_damageTypesToPropagateToSlaves, damageInfo->in.m_damageType ) )
	{

    //We have the right type of damage types incoming to propagate to slaves. Do we have slaves?
		SpawnBehaviorInterface *spawnInterface = hive->getSpawnBehaviorInterface();
		if( spawnInterface )
		{
			//We found the spawn interface, now get some slaves!
			Object *shooter = TheGameLogic->findObjectByID( damageInfo->in.m_sourceID );
			if( shooter )
			{
				Object *slave = spawnInterface->getClosestSlave( shooter->getPosition() );
				if( slave )
				{
					//Propagate damage and return!
					slave->attemptDamage( damageInfo );
					return;
				}
				else if( getDamageTypeFlag( data->m_damageTypesToSwallow, damageInfo->in.m_damageType ) )
				{
					//no slave to give to, so eat it
					damageInfo->out.m_actualDamageDealt = 0.0f;
					damageInfo->out.m_actualDamageClipped = 0.0f;
					damageInfo->out.m_noEffect = true;
					return;
				}
			}
		}
		else if ( hive->getContain() )
		{
      ContainModuleInterface *contain = hive->getContain();
			//We found the spawn interface, now get some slaves!
			Object *shooter = TheGameLogic->findObjectByID( damageInfo->in.m_sourceID );
			if( shooter )
			{
				Object *rider = contain->getClosestRider( shooter->getPosition() );
				if( rider )
				{
					//Propagate damage and return!
					rider->attemptDamage( damageInfo );
					return;
				}
				else if( getDamageTypeFlag( data->m_damageTypesToSwallow, damageInfo->in.m_damageType ) )
				{
					//no slave to give to, so eat it
					damageInfo->out.m_actualDamageDealt = 0.0f;
					damageInfo->out.m_actualDamageClipped = 0.0f;
					damageInfo->out.m_noEffect = true;
					return;
				}
			}
    }
		else
		{
			DEBUG_CRASH( ("%s has a HiveStructureBody module, which requires a SpawnBehavior module. Thus it is unable to propagate damage to slaves.", hive->getTemplate()->getName().str() ) );
		}


	}

	//Nothing to propagate (either different damage type or no slaves),
	//so damage me instead!
	StructureBody::attemptDamage( damageInfo );
}

//------------------------------------------------------------------------------------------------
void HiveStructureBody::crc( Xfer *xfer )
{

	// extend parent class
	StructureBody::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void HiveStructureBody::xfer( Xfer *xfer )
{
	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// parent class
	StructureBody::xfer( xfer );
}

//------------------------------------------------------------------------------------------------
void HiveStructureBody::loadPostProcess( void )
{

	// extend parent class
	StructureBody::loadPostProcess();

}
