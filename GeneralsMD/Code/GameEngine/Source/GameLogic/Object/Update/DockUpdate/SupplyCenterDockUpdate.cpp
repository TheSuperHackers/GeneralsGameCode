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

// FILE: SupplyCenterDockUpdate.h /////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood Feb 2002
// Desc:   The action of this dock update is taking boxes and turning them into money for my ownerplayer
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameLogic/Module/SupplyCenterDockUpdate.h"
#include "GameLogic/Module/SupplyTruckAIUpdate.h"
#include "GameClient/Color.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"
#include "GameClient/GameText.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SupplyCenterDockUpdateModuleData::SupplyCenterDockUpdateModuleData( void )
{
	m_grantTemporaryStealthFrames = 0;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void SupplyCenterDockUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{

	DockUpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "GrantTemporaryStealth",		INI::parseDurationUnsignedInt,  NULL, offsetof( SupplyCenterDockUpdateModuleData, m_grantTemporaryStealthFrames ) },
		{ 0, 0, 0, 0 }
	};

  p.add(dataFieldParse);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SupplyCenterDockUpdate::SupplyCenterDockUpdate( Thing *thing, const ModuleData* moduleData ) : DockUpdate( thing, moduleData )
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SupplyCenterDockUpdate::~SupplyCenterDockUpdate()
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool SupplyCenterDockUpdate::action( Object* docker, Object *drone )
{
	const SupplyCenterDockUpdateModuleData *data = getSupplyCenterDockUpdateModuleData();
	SupplyTruckAIInterface* supplyTruckAI = NULL;
	if( docker->getAIUpdateInterface() == NULL )
		return FALSE;

	supplyTruckAI = docker->getAIUpdateInterface()->getSupplyTruckAIInterface();

	DEBUG_ASSERTCRASH( supplyTruckAI != NULL, ("Something Docking with a Supply Center must have a Supply-truck like AIUpdate") );
	if( supplyTruckAI == NULL )
		return FALSE;

	UnsignedInt value = 0;
	Player *ownerPlayer = getObject()->getControllingPlayer();
	while( supplyTruckAI->loseOneBox() )
		value += ownerPlayer->getSupplyBoxValue();

	// Add money boost from upgrades that give extra money
	value += supplyTruckAI->getUpgradedSupplyBoost();

	if( value > 0)
	{
		Money *ownerPlayerMoney = ownerPlayer->getMoney();
		ownerPlayerMoney->deposit(value);
		ownerPlayer->getScoreKeeper()->addMoneyEarned(value);


		if( data->m_grantTemporaryStealthFrames > 0 )
		{
			StealthUpdate *stealth = docker->getStealth();
			//Only grant temporary stealth to the default stealth update. It's
			//possible that another type of stealth was granted... like the
			//GPS scrambler. We want that to take precendence.
			if( getObject()->testStatus( OBJECT_STATUS_STEALTHED ) )
			{
				if( !stealth )
				{
					DEBUG_CRASH( ("SupplyCenterDockUpdate::action() -- It shouldn't be possible for a unit to be OBJECT_STATUS_STEALTHED without a StealthUpdate module!") );
				}
				else if( stealth->isTemporaryGrant() || !docker->testStatus( OBJECT_STATUS_CAN_STEALTH ) )
				{
					stealth->receiveGrant( TRUE, data->m_grantTemporaryStealthFrames );
				}
			}
		}
	}

	if (value > 0 && getObject()->getDrawable()->isVisible())
	{
		// OY LOOK!  I AM USING LOCAL PLAYER.  Do not put anything other than TheInGameUI->addFloatingText in the block this controls!!!
		// Setup info for adding a floating text
		Coord3D pos;
		const Coord3D *dockerPos;
		UnicodeString moneys;
		moneys.format( TheGameText->fetch( "GUI:AddCash" ), value );
		dockerPos = docker->getPosition();
		pos.x = dockerPos->x;
		pos.y = dockerPos->y;
		pos.z = TheTerrainLogic->getGroundHeight(pos.x, pos.y);//dockerPos->z + docker->getGeometryInfo().getHeight();
		Color color = ownerPlayer->getPlayerColor() | GameMakeColor( 0, 0, 0, 230 );

		TheInGameUI->addFloatingText(moneys, &pos, color);
	}


	return FALSE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime SupplyCenterDockUpdate::update()
{
	//extend
	UpdateSleepTime result = DockUpdate::update();

#ifdef _DEBUG_ECONOMY
	static const NameKeyType key_SupplyCenterCreate = NAMEKEY("SupplyCenterCreate");
	SupplyCenterCreate* create = (SupplyCenterCreate*)getObject()->findCreateModule(key_SupplyCenterCreate);
	DEBUG_ASSERTCRASH( create && ! create->shouldDoOnBuildComplete(), ("A Supply center did not call onBuildComplete.") );
#endif

	return result;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SupplyCenterDockUpdate::crc( Xfer *xfer )
{

	// extend base class
	DockUpdate::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SupplyCenterDockUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DockUpdate::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SupplyCenterDockUpdate::loadPostProcess( void )
{

	// extend base class
	DockUpdate::loadPostProcess();

}
