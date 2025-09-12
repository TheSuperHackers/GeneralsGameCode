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

// FILE: FireSpreadUpdate.cpp /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, April 2002
// Desc:   Update looks for ::Aflame and explicitly ignites someone nearby if set
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/FireSpreadUpdate.h"
#include "GameLogic/Module/FlammableUpdate.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// This is a one sided query, as in I am not checking for "Flammable By Me", I'm simply testing for a property
class PartitionFilterFlammable : public PartitionFilter
{
public:

	PartitionFilterFlammable(){ }

	virtual Bool allow(Object *objOther);
#if defined(RTS_DEBUG)
	virtual const char* debugGetName() { return "PartitionFilterFlammable"; }
#endif
};

//-------------------------------------------------------------------------------------------------
Bool PartitionFilterFlammable::allow(Object *objOther)
{
	// It must be burnable in general, and burnable now
	static NameKeyType key_FlammableUpdate = NAMEKEY("FlammableUpdate");
	FlammableUpdate* fu = (FlammableUpdate*)objOther->findUpdateModule(key_FlammableUpdate);
	if (fu == NULL)
		return FALSE;

	if( ! fu->wouldIgnite() )
		return FALSE;

	return TRUE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
FireSpreadUpdateModuleData::FireSpreadUpdateModuleData()
{
	m_minSpreadTryDelayData = 0;
	m_maxSpreadTryDelayData = 0;
	m_oclEmbers = NULL;
	m_spreadTryRange = 0;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void FireSpreadUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "OCLEmbers",				INI::parseObjectCreationList,		NULL, offsetof( FireSpreadUpdateModuleData, m_oclEmbers ) },
		{ "MinSpreadDelay",		INI::parseDurationUnsignedInt,	NULL, offsetof( FireSpreadUpdateModuleData, m_minSpreadTryDelayData ) },
		{ "MaxSpreadDelay",		INI::parseDurationUnsignedInt,	NULL, offsetof( FireSpreadUpdateModuleData, m_maxSpreadTryDelayData ) },
		{ "SpreadTryRange",		INI::parseReal,									NULL, offsetof( FireSpreadUpdateModuleData, m_spreadTryRange ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FireSpreadUpdate::FireSpreadUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FireSpreadUpdate::~FireSpreadUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime FireSpreadUpdate::update( void )
{
	const FireSpreadUpdateModuleData* d = getFireSpreadUpdateModuleData();
	Object* me = getObject();

	if( !me->getStatusBits().test( OBJECT_STATUS_AFLAME ) )
		return UPDATE_SLEEP_FOREVER;		// not on fire -- sleep forever
	{
		ObjectCreationList::create( d->m_oclEmbers, getObject(), NULL );

		if( d->m_spreadTryRange != 0 )
		{
			// This will spread fire explicitly
			PartitionFilterFlammable fFilter;
			PartitionFilter *filters[] = { &fFilter, NULL };

//			SimpleObjectIterator *iter = NULL;
//			iter = ThePartitionManager->iterateObjectsInRange(getObject(),
//																									d->m_spreadTryRange,
//																									FROM_CENTER_3D,
//																									filters,
//																									ITER_SORTED_NEAR_TO_FAR
//																									);
//			MemoryPoolObjectHolder hold(iter);
//			Object *objectToLight = iter->first();
//
// srj sez: the above code is stupid and slow. since we only want the closest object,
// just ask for that; the above has to find ALL objects in range, but we ignore all
// but the first (closest).
//
			Object* objectToLight = ThePartitionManager->getClosestObject(getObject(), d->m_spreadTryRange, FROM_CENTER_3D, filters);
			if( objectToLight )
			{
				static NameKeyType key_FlammableUpdate = NAMEKEY("FlammableUpdate");
				FlammableUpdate* fu = (FlammableUpdate*)objectToLight->findUpdateModule(key_FlammableUpdate);
				if( fu )
					fu->tryToIgnite();
			}
		}

		return UPDATE_SLEEP(calcNextSpreadDelay());
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void FireSpreadUpdate::startFireSpreading()
{
	if( !getObject()->getStatusBits().test( OBJECT_STATUS_AFLAME ) )
		return;	// sorry, must be on fire

	setWakeFrame(getObject(), UPDATE_SLEEP(calcNextSpreadDelay()));
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnsignedInt FireSpreadUpdate::calcNextSpreadDelay()
{
	const FireSpreadUpdateModuleData* d = getFireSpreadUpdateModuleData();
	UnsignedInt delay = GameLogicRandomValue( d->m_minSpreadTryDelayData, d->m_maxSpreadTryDelayData );
	if (delay < 1)
		delay = 1;
	return delay;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void FireSpreadUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void FireSpreadUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void FireSpreadUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
