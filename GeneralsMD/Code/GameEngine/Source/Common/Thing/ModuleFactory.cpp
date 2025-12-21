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

// FILE: ModuleFactory.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2001
// Desc:	 TheModuleFactory is where we actually instance modules for objects
//				 and drawbles.  Those modules are things such as an UpdateModule
//			   or DamageModule or DrawModule etc.
//
//				 TheModuleFactory will contain a list of ModuleTemplates, when we
//				 request a new module, we will look for that template in our
//				 list and create it
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Module.h"
#include "Common/ModuleFactory.h"
#include "Common/NameKeyGenerator.h"

// behavior includes
#include "GameLogic/Module/AutoHealBehavior.h"
#include "GameLogic/Module/GrantStealthBehavior.h"
#include "GameLogic/Module/NeutronBlastBehavior.h"
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/BridgeBehavior.h"
#include "GameLogic/Module/BridgeScaffoldBehavior.h"
#include "GameLogic/Module/BridgeTowerBehavior.h"
#include "GameLogic/Module/CountermeasuresBehavior.h"
#include "GameLogic/Module/DumbProjectileBehavior.h"
#include "GameLogic/Module/InstantDeathBehavior.h"
#include "GameLogic/Module/SlowDeathBehavior.h"
#include "GameLogic/Module/HelicopterSlowDeathUpdate.h"
#include "GameLogic/Module/NeutronMissileSlowDeathUpdate.h"
#include "GameLogic/Module/CaveContain.h"
#include "GameLogic/Module/OpenContain.h"
#include "GameLogic/Module/OverchargeBehavior.h"
#include "GameLogic/Module/HealContain.h"
#include "GameLogic/Module/GarrisonContain.h"
#include "GameLogic/Module/InternetHackContain.h"
#include "GameLogic/Module/RailedTransportContain.h"
#include "GameLogic/Module/RiderChangeContain.h"
#include "GameLogic/Module/TransportContain.h"
#include "GameLogic/Module/MobNexusContain.h"
#include "GameLogic/Module/TunnelContain.h"
#include "GameLogic/Module/OverlordContain.h"
#include "GameLogic/Module/HelixContain.h"
#include "GameLogic/Module/ParachuteContain.h"
#ifdef ALLOW_SURRENDER
#include "GameLogic/Module/POWTruckBehavior.h"
#include "GameLogic/Module/PrisonBehavior.h"
#include "GameLogic/Module/PropagandaCenterBehavior.h"
#endif
#include "GameLogic/Module/PropagandaTowerBehavior.h"
#include "GameLogic/Module/BunkerBusterBehavior.h"
#include "GameLogic/Module/FireWeaponWhenDamagedBehavior.h"
#include "GameLogic/Module/FireWeaponWhenDeadBehavior.h"
#include "GameLogic/Module/GenerateMinefieldBehavior.h"
#include "GameLogic/Module/ParkingPlaceBehavior.h"
#include "GameLogic/Module/FlightDeckBehavior.h"
#include "GameLogic/Module/PoisonedBehavior.h"
#include "GameLogic/Module/RebuildHoleBehavior.h"
#include "GameLogic/Module/SupplyWarehouseCripplingBehavior.h"
#include "GameLogic/Module/TechBuildingBehavior.h"
#include "GameLogic/Module/MinefieldBehavior.h"
#include "GameLogic/Module/BattleBusSlowDeathBehavior.h"
#include "GameLogic/Module/JetSlowDeathBehavior.h"

// die includes
#include "GameLogic/Module/CreateCrateDie.h"
#include "GameLogic/Module/CreateObjectDie.h"
#include "GameLogic/Module/CrushDie.h"
#include "GameLogic/Module/DamDie.h"
#include "GameLogic/Module/DestroyDie.h"
#include "GameLogic/Module/EjectPilotDie.h"
#include "GameLogic/Module/FXListDie.h"
#include "GameLogic/Module/RebuildHoleExposeDie.h"
#include "GameLogic/Module/SpecialPowerCompletionDie.h"
#include "GameLogic/Module/UpgradeDie.h"
#include "GameLogic/Module/KeepObjectDie.h"

// logic update includes
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/AnimationSteeringUpdate.h"
#include "GameLogic/Module/AssistedTargetingUpdate.h"
#include "GameLogic/Module/BaseRegenerateUpdate.h"
#include "GameLogic/Module/BoneFXUpdate.h"
#include "GameLogic/Module/ChinookAIUpdate.h"
#include "GameLogic/Module/DefaultProductionExitUpdate.h"
#include "GameLogic/Module/DeletionUpdate.h"
#include "GameLogic/Module/SmartBombTargetHomingUpdate.h"
#include "GameLogic/Module/DeliverPayloadAIUpdate.h"
#include "GameLogic/Module/DozerAIUpdate.h"
#include "GameLogic/Module/DynamicGeometryInfoUpdate.h"
#include "GameLogic/Module/DynamicShroudClearingRangeUpdate.h"
#include "GameLogic/Module/EnemyNearUpdate.h"
#include "GameLogic/Module/FireSpreadUpdate.h"
#include "GameLogic/Module/FirestormDynamicGeometryInfoUpdate.h"
#include "GameLogic/Module/FireWeaponUpdate.h"
#include "GameLogic/Module/FlammableUpdate.h"
#include "GameLogic/Module/FloatUpdate.h"
#include "GameLogic/Module/TensileFormationUpdate.h"
#include "GameLogic/Module/HackInternetAIUpdate.h"
#include "GameLogic/Module/DeployStyleAIUpdate.h"
#include "GameLogic/Module/AssaultTransportAIUpdate.h"
#include "GameLogic/Module/HeightDieUpdate.h"
#include "GameLogic/Module/HordeUpdate.h"
#include "GameLogic/Module/JetAIUpdate.h"
#include "GameLogic/Module/LaserUpdate.h"
#include "GameLogic/Module/PointDefenseLaserUpdate.h"
#include "GameLogic/Module/CleanupHazardUpdate.h"
#include "GameLogic/Module/AutoFindHealingUpdate.h"
#include "GameLogic/Module/CommandButtonHuntUpdate.h"
#include "GameLogic/Module/PilotFindVehicleUpdate.h"
#include "GameLogic/Module/DemoTrapUpdate.h"
#include "GameLogic/Module/ParticleUplinkCannonUpdate.h"
#include "GameLogic/Module/SpectreGunshipUpdate.h"
#include "GameLogic/Module/SpectreGunshipDeploymentUpdate.h"
#include "GameLogic/Module/BaikonurLaunchPower.h"
#include "GameLogic/Module/BattlePlanUpdate.h"
#include "GameLogic/Module/LifetimeUpdate.h"
#include "GameLogic/Module/RadiusDecalUpdate.h"
#include "GameLogic/Module/AutoDepositUpdate.h"
#include "GameLogic/Module/MissileAIUpdate.h"
#include "GameLogic/Module/NeutronMissileUpdate.h"
#include "GameLogic/Module/OCLUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#ifdef ALLOW_SURRENDER
#include "GameLogic/Module/POWTruckAIUpdate.h"
#endif
#include "GameLogic/Module/ProductionUpdate.h"
#include "GameLogic/Module/ProjectileStreamUpdate.h"
#include "GameLogic/Module/ProneUpdate.h"
#include "GameLogic/Module/QueueProductionExitUpdate.h"
#include "GameLogic/Module/RadarUpdate.h"
#include "GameLogic/Module/RepairDockUpdate.h"
#ifdef ALLOW_SURRENDER
#include "GameLogic/Module/PrisonDockUpdate.h"
#endif
#include "GameLogic/Module/RailedTransportDockUpdate.h"
#include "GameLogic/Module/RailedTransportAIUpdate.h"
#include "GameLogic/Module/RailroadGuideAIUpdate.h"
#include "GameLogic/Module/SlavedUpdate.h"
#include "GameLogic/Module/MobMemberSlavedUpdate.h"
#include "GameLogic/Module/SpecialAbilityUpdate.h"
#include "GameLogic/Module/MissileLauncherBuildingUpdate.h"
#include "GameLogic/Module/StealthDetectorUpdate.h"
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/Module/SpawnPointProductionExitUpdate.h"
#include "GameLogic/Module/SpawnBehavior.h"
#include "GameLogic/Module/SpyVisionUpdate.h"
#include "GameLogic/Module/StickyBombUpdate.h"
#include "GameLogic/Module/FireOCLAfterWeaponCooldownUpdate.h"
#include "GameLogic/Module/HijackerUpdate.h"
#include "GameLogic/Module/StructureCollapseUpdate.h"
#include "GameLogic/Module/StructureToppleUpdate.h"
#include "GameLogic/Module/SupplyCenterDockUpdate.h"
#include "GameLogic/Module/SupplyCenterProductionExitUpdate.h"
#include "GameLogic/Module/SupplyTruckAIUpdate.h"
#include "GameLogic/Module/SupplyWarehouseDockUpdate.h"
#include "GameLogic/Module/ToppleUpdate.h"
#include "GameLogic/Module/TransportAIUpdate.h"
#include "GameLogic/Module/WanderAIUpdate.h"
#include "GameLogic/Module/WaveGuideUpdate.h"
#include "GameLogic/Module/WeaponBonusUpdate.h"
#include "GameLogic/Module/WorkerAIUpdate.h"
#include "GameLogic/Module/PowerPlantUpdate.h"
#include "GameLogic/Module/CheckpointUpdate.h"
#include "GameLogic/Module/EMPUpdate.h"

// upgrade includes
#include "GameLogic/Module/ActiveShroudUpgrade.h"
#include "GameLogic/Module/ArmorUpgrade.h"
#include "GameLogic/Module/CommandSetUpgrade.h"
#include "GameLogic/Module/GrantScienceUpgrade.h"
#include "GameLogic/Module/PassengersFireUpgrade.h"
#include "GameLogic/Module/LocomotorSetUpgrade.h"
#include "GameLogic/Module/ObjectCreationUpgrade.h"
#include "GameLogic/Module/RadarUpgrade.h"
#include "GameLogic/Module/PowerPlantUpgrade.h"
#include "GameLogic/Module/ReplaceObjectUpgrade.h"
#include "GameLogic/Module/ModelConditionUpgrade.h"
#include "GameLogic/Module/StatusBitsUpgrade.h"
#include "GameLogic/Module/SubObjectsUpgrade.h"
#include "GameLogic/Module/StealthUpgrade.h"
#include "GameLogic/Module/UnpauseSpecialPowerUpgrade.h"
#include "GameLogic/Module/WeaponBonusUpgrade.h"
#include "GameLogic/Module/WeaponSetUpgrade.h"
#include "GameLogic/Module/CostModifierUpgrade.h"
#include "GameLogic/Module/ExperienceScalarUpgrade.h"
#include "GameLogic/Module/MaxHealthUpgrade.h"

// create includes
#include "GameLogic/Module/LockWeaponCreate.h"
#include "GameLogic/Module/SupplyCenterCreate.h"
#include "GameLogic/Module/SupplyWarehouseCreate.h"
#include "GameLogic/Module/GrantUpgradeCreate.h"
#include "GameLogic/Module/PreorderCreate.h"
#include "GameLogic/Module/SpecialPowerCreate.h"
#include "GameLogic/Module/VeterancyGainCreate.h"

// damage includes
#include "GameLogic/Module/BoneFXDamage.h"
#include "GameLogic/Module/TransitionDamageFX.h"

// collide includes
#include "GameLogic/Module/FireWeaponCollide.h"
#include "GameLogic/Module/SquishCollide.h"

#include "GameLogic/Module/ConvertToCarBombCrateCollide.h"
#include "GameLogic/Module/ConvertToHijackedVehicleCrateCollide.h"
#include "GameLogic/Module/HealCrateCollide.h"
#include "GameLogic/Module/MoneyCrateCollide.h"
#include "GameLogic/Module/SabotageCommandCenterCrateCollide.h"
#include "GameLogic/Module/SabotageFakeBuildingCrateCollide.h"
#include "GameLogic/Module/SabotageInternetCenterCrateCollide.h"
#include "GameLogic/Module/SabotageMilitaryFactoryCrateCollide.h"
#include "GameLogic/Module/SabotagePowerPlantCrateCollide.h"
#include "GameLogic/Module/SabotageSuperweaponCrateCollide.h"
#include "GameLogic/Module/SabotageSupplyCenterCrateCollide.h"
#include "GameLogic/Module/SabotageSupplyDropzoneCrateCollide.h"
#include "GameLogic/Module/SalvageCrateCollide.h"
#include "GameLogic/Module/ShroudCrateCollide.h"
#include "GameLogic/Module/UnitCrateCollide.h"
#include "GameLogic/Module/VeterancyCrateCollide.h"

// body includes
#include "GameLogic/Module/InactiveBody.h"
#include "GameLogic/Module/ActiveBody.h"
#include "GameLogic/Module/HighlanderBody.h"
#include "GameLogic/Module/ImmortalBody.h"
#include "GameLogic/Module/StructureBody.h"
#include "GameLogic/Module/HiveStructureBody.h"
#include "GameLogic/Module/UndeadBody.h"

// contain includes
// (none)

// special power modules
#include "GameLogic/Module/CashHackSpecialPower.h"
#include "GameLogic/Module/DefectorSpecialPower.h"
#ifdef ALLOW_DEMORALIZE
#include "GameLogic/Module/DemoralizeSpecialPower.h"
#endif
#include "GameLogic/Module/OCLSpecialPower.h"
#include "GameLogic/Module/SpecialAbility.h"
#include "GameLogic/Module/SpyVisionSpecialPower.h"
#include "GameLogic/Module/CashBountyPower.h"
#include "GameLogic/Module/CleanupAreaPower.h"
#include "GameLogic/Module/FireWeaponPower.h"

// destroy includes
// (none)

// client update includes
#include "GameClient/Module/AnimatedParticleSysBoneClientUpdate.h"
#include "GameClient/Module/SwayClientUpdate.h"
#include "GameClient/Module/BeaconClientUpdate.h"

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
ModuleFactory *TheModuleFactory = nullptr;  ///< the module factory singleton

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ModuleFactory::ModuleFactory( void )
{
	m_moduleTemplateMap.clear();
	m_moduleDataList.clear();

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ModuleFactory::~ModuleFactory( void )
{
	m_moduleTemplateMap.clear();

	for (ModuleDataList::iterator i = m_moduleDataList.begin(); i != m_moduleDataList.end(); ++i)
	{
		const ModuleData* data = *i;
		delete data;
	}
	m_moduleDataList.clear();

}

//-------------------------------------------------------------------------------------------------
/** Initialize the module factory.  Any class that needs to be attached
	* to objects or drawables as modules needs to add a template
	* for that class here */
//-------------------------------------------------------------------------------------------------
void ModuleFactory::init( void )
{

	// behavior modules
	addModule( AutoHealBehavior );
	addModule( GrantStealthBehavior );
	addModule( NeutronBlastBehavior );
	addModule( BridgeBehavior );
	addModule( BridgeScaffoldBehavior );
	addModule( BridgeTowerBehavior );
	addModule( CountermeasuresBehavior );
	addModule( DumbProjectileBehavior );
	addModule( PhysicsBehavior );
	addModule( InstantDeathBehavior );
	addModule( SlowDeathBehavior );
	addModule( HelicopterSlowDeathBehavior );
	addModule( NeutronMissileSlowDeathBehavior );
	addModule( CaveContain );
	addModule( OpenContain );
	addModule( OverchargeBehavior );
	addModule( HealContain );
	addModule( GarrisonContain );
	addModule( InternetHackContain );
	addModule( TransportContain );
	addModule( RiderChangeContain );
	addModule( RailedTransportContain );
	addModule( MobNexusContain );
	addModule( TunnelContain );
	addModule( OverlordContain );
	addModule( HelixContain );
	addModule( ParachuteContain );
#ifdef ALLOW_SURRENDER
	addModule( POWTruckBehavior );
	addModule( PrisonBehavior );
	addModule( PropagandaCenterBehavior );
#endif
	addModule( PropagandaTowerBehavior );
	addModule( BunkerBusterBehavior );
	addModule( FireWeaponWhenDamagedBehavior );
	addModule( FireWeaponWhenDeadBehavior );
	addModule( GenerateMinefieldBehavior );
	addModule( ParkingPlaceBehavior );
	addModule( FlightDeckBehavior );
	addModule( PoisonedBehavior );
	addModule( RebuildHoleBehavior );
	addModule( SupplyWarehouseCripplingBehavior );
	addModule( TechBuildingBehavior );
	addModule( MinefieldBehavior );
	addModule( BattleBusSlowDeathBehavior );
	addModule( JetSlowDeathBehavior );
	addModule( RailroadBehavior );
	addModule( SpawnBehavior );

	// die modules
	addModule( DestroyDie );
	addModule( FXListDie );
	addModule( CrushDie );
	addModule( DamDie );
	addModule( CreateCrateDie );
	addModule( CreateObjectDie );
	addModule( EjectPilotDie );
	addModule( SpecialPowerCompletionDie );
	addModule( RebuildHoleExposeDie );
	addModule( UpgradeDie );
	addModule( KeepObjectDie );

	// update modules
	addModule( AssistedTargetingUpdate );
	addModule( AutoFindHealingUpdate );
	addModule( BaseRegenerateUpdate );
	addModule( StealthDetectorUpdate );
	addModule( StealthUpdate );
	addModule( DeletionUpdate );
	addModule( SmartBombTargetHomingUpdate );
	addModule( DynamicShroudClearingRangeUpdate );
	addModule( DeployStyleAIUpdate );
	addModule( AssaultTransportAIUpdate );
	addModule( HordeUpdate );
	addModule( ToppleUpdate );
	addModule( EnemyNearUpdate );
	addModule( LifetimeUpdate );
	addModule( RadiusDecalUpdate );
	addModule( EMPUpdate );
  addModule( LeafletDropBehavior );
	addModule( AutoDepositUpdate );
	addModule( WeaponBonusUpdate );
	addModule( MissileAIUpdate );
	addModule( NeutronMissileUpdate );
	addModule( FireSpreadUpdate );
	addModule( FireWeaponUpdate );
	addModule( FlammableUpdate );
	addModule( FloatUpdate );
	addModule( TensileFormationUpdate );
	addModule( HeightDieUpdate );
	addModule( ChinookAIUpdate );
	addModule( JetAIUpdate );
	addModule( AIUpdateInterface );
	addModule( SupplyTruckAIUpdate );
	addModule( DeliverPayloadAIUpdate );
	addModule( HackInternetAIUpdate );
	addModule( DynamicGeometryInfoUpdate );
	addModule( FirestormDynamicGeometryInfoUpdate );
	addModule( LaserUpdate );
	addModule( PointDefenseLaserUpdate );
	addModule( CleanupHazardUpdate );
	addModule( CommandButtonHuntUpdate );
	addModule( PilotFindVehicleUpdate );
	addModule( DemoTrapUpdate );
	addModule( ParticleUplinkCannonUpdate );
	addModule( SpectreGunshipUpdate );
	addModule( SpectreGunshipDeploymentUpdate );
	addModule( BaikonurLaunchPower );
	addModule( BattlePlanUpdate );
	addModule( ProjectileStreamUpdate );
	addModule( QueueProductionExitUpdate );
	addModule( RepairDockUpdate );
#ifdef ALLOW_SURRENDER
	addModule( PrisonDockUpdate );
#endif
	addModule( RailedTransportDockUpdate );
	addModule( DefaultProductionExitUpdate );
	addModule( SpawnPointProductionExitUpdate );
	addModule( SpyVisionUpdate );
	addModule( SlavedUpdate );
	addModule( MobMemberSlavedUpdate );
	addModule( OCLUpdate );
	addModule( SpecialAbilityUpdate );
	addModule( MissileLauncherBuildingUpdate );
	addModule( SupplyCenterProductionExitUpdate );
	addModule( SupplyCenterDockUpdate );
	addModule( SupplyWarehouseDockUpdate );
	addModule( DozerAIUpdate );
#ifdef ALLOW_SURRENDER
	addModule( POWTruckAIUpdate );
#endif
	addModule( RailedTransportAIUpdate );
	addModule( ProductionUpdate );
	addModule( ProneUpdate );
	addModule( StickyBombUpdate );
	addModule( FireOCLAfterWeaponCooldownUpdate );
	addModule( HijackerUpdate );
	addModule( StructureToppleUpdate );
	addModule( StructureCollapseUpdate );
	addModule( BoneFXUpdate );
	addModule( RadarUpdate );
	addModule( AnimationSteeringUpdate );
	addModule( TransportAIUpdate );
	addModule( WanderAIUpdate );
	addModule( WaveGuideUpdate );
	addModule( WorkerAIUpdate );
	addModule( PowerPlantUpdate );
	addModule( CheckpointUpdate );

	// upgrade modules
	addModule( CostModifierUpgrade );
	addModule( ActiveShroudUpgrade );
	addModule( ArmorUpgrade );
	addModule( CommandSetUpgrade );
	addModule( GrantScienceUpgrade );
	addModule( PassengersFireUpgrade );
	addModule( StatusBitsUpgrade );
	addModule( SubObjectsUpgrade );
	addModule( StealthUpgrade );
	addModule( RadarUpgrade );
	addModule( PowerPlantUpgrade );
	addModule( LocomotorSetUpgrade );
	addModule( ObjectCreationUpgrade );
	addModule( ReplaceObjectUpgrade );
	addModule( ModelConditionUpgrade );
	addModule( UnpauseSpecialPowerUpgrade );
	addModule( WeaponBonusUpgrade );
	addModule( WeaponSetUpgrade );
	addModule( WeaponBonusUpgrade );
	addModule( ExperienceScalarUpgrade );
	addModule( MaxHealthUpgrade );

	// create modules
	addModule( LockWeaponCreate );
	addModule( PreorderCreate );
	addModule( SupplyCenterCreate );
	addModule( SupplyWarehouseCreate );
	addModule( SpecialPowerCreate );
	addModule( GrantUpgradeCreate );
	addModule( VeterancyGainCreate );

	// damage modules
	addModule( BoneFXDamage );
	addModule( TransitionDamageFX );

	// collide modules
	addModule( FireWeaponCollide );
	addModule( SquishCollide );

	addModule( HealCrateCollide );
	addModule( MoneyCrateCollide );
	addModule( ShroudCrateCollide );
	addModule( UnitCrateCollide );
	addModule( VeterancyCrateCollide );
	addModule( ConvertToCarBombCrateCollide );
	addModule( ConvertToHijackedVehicleCrateCollide );
	addModule( SabotageCommandCenterCrateCollide );
	addModule( SabotageFakeBuildingCrateCollide );
	addModule( SabotageInternetCenterCrateCollide );
	addModule( SabotageMilitaryFactoryCrateCollide );
	addModule( SabotagePowerPlantCrateCollide );
	addModule( SabotageSuperweaponCrateCollide );
	addModule( SabotageSupplyCenterCrateCollide );
	addModule( SabotageSupplyDropzoneCrateCollide );
	addModule( SalvageCrateCollide );

	// body modules
	addModule( InactiveBody );
	addModule( ActiveBody );
	addModule( HighlanderBody );
	addModule( ImmortalBody );
	addModule( StructureBody );
	addModule( HiveStructureBody );
	addModule( UndeadBody );

	// contain modules
	// (none)

	// special power modules
	addModule( CashHackSpecialPower );
	addModule( DefectorSpecialPower );
#ifdef ALLOW_DEMORALIZE
	addModule( DemoralizeSpecialPower );
#endif
	addModule( OCLSpecialPower );
	addModule( FireWeaponPower );
	addModule( SpecialAbility );
	addModule( SpyVisionSpecialPower );
	addModule( CashBountyPower );
	addModule( CleanupAreaPower );

	// destroy modules
	// (none)

	// client update modules
	addModule( AnimatedParticleSysBoneClientUpdate );
	addModule( SwayClientUpdate );
	addModule( BeaconClientUpdate );

}

//-------------------------------------------------------------------------------------------------
Int ModuleFactory::findModuleInterfaceMask(const AsciiString& name, ModuleType type)
{
	if (name.isEmpty())
		return 0;

	const ModuleTemplate* moduleTemplate = findModuleTemplate(name, type);
	if (moduleTemplate)
	{
		return moduleTemplate->m_whichInterfaces;
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
ModuleData* ModuleFactory::newModuleDataFromINI(INI* ini, const AsciiString& name, ModuleType type,
																								const AsciiString& moduleTag)
{
	if (name.isEmpty())
		return nullptr;

	const ModuleTemplate* moduleTemplate = findModuleTemplate(name, type);
	if (moduleTemplate)
	{
		ModuleData* md = (*moduleTemplate->m_createDataProc)(ini);
		md->setModuleTagNameKey( NAMEKEY( moduleTag ) );
		m_moduleDataList.push_back(md);
		return md;
	}

	return nullptr;
}

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/*static*/ NameKeyType ModuleFactory::makeDecoratedNameKey(const AsciiString& name, ModuleType type)
{
	char tmp[256];
	tmp[0] = '0' + (int)type;
	strcpy(&tmp[1], name.str());
	return TheNameKeyGenerator->nameToKey(tmp);
}

//-------------------------------------------------------------------------------------------------
const ModuleFactory::ModuleTemplate* ModuleFactory::findModuleTemplate(const AsciiString& name, ModuleType type)
{
	NameKeyType namekey = makeDecoratedNameKey(name, type);

  ModuleTemplateMap::const_iterator it = m_moduleTemplateMap.find(namekey);
  if (it == m_moduleTemplateMap.end())
	{
		DEBUG_CRASH(( "Module name '%s' not found", name.str() ));
		return nullptr;
	}
	else
	{
		return &(*it).second;
	}
}

//-------------------------------------------------------------------------------------------------
/** Allocate a new acton class istance given the name */
//-------------------------------------------------------------------------------------------------
Module *ModuleFactory::newModule( Thing *thing, const AsciiString& name, const ModuleData* moduleData, ModuleType type )
{
	// sanity
	if( name.isEmpty() )
	{
		DEBUG_CRASH(("attempting to create module with empty name"));
		return nullptr;
	}
	const ModuleTemplate* mt = findModuleTemplate(name, type);
	if (mt)
	{
		Module* mod = (*mt->m_createProc)( thing, moduleData );

#ifdef DEBUG_CRASHING
		if (type == MODULETYPE_BEHAVIOR)
		{
			BehaviorModule* bm = (BehaviorModule*)mod;

			DEBUG_ASSERTCRASH(
				((mt->m_whichInterfaces & (MODULEINTERFACE_BODY)) != 0) == (bm->getBody() != nullptr),
				("getInterfaceMask bad for MODULE_BODY (%s)",name.str()));
			DEBUG_ASSERTCRASH(
				((mt->m_whichInterfaces & (MODULEINTERFACE_COLLIDE)) != 0) == (bm->getCollide() != nullptr),
				("getInterfaceMask bad for MODULE_COLLIDE (%s)",name.str()));
			DEBUG_ASSERTCRASH(
				((mt->m_whichInterfaces & (MODULEINTERFACE_CONTAIN)) != 0) == (bm->getContain() != nullptr),
				("getInterfaceMask bad for MODULE_CONTAIN (%s)",name.str()));
			DEBUG_ASSERTCRASH(
				((mt->m_whichInterfaces & (MODULEINTERFACE_CREATE)) != 0) == (bm->getCreate() != nullptr),
				("getInterfaceMask bad for MODULE_CREATE (%s)",name.str()));
			DEBUG_ASSERTCRASH(
				((mt->m_whichInterfaces & (MODULEINTERFACE_DAMAGE)) != 0) == (bm->getDamage() != nullptr),
				("getInterfaceMask bad for MODULE_DAMAGE (%s)",name.str()));
			DEBUG_ASSERTCRASH(
				((mt->m_whichInterfaces & (MODULEINTERFACE_DESTROY)) != 0) == (bm->getDestroy() != nullptr),
				("getInterfaceMask bad for MODULE_DESTROY (%s)",name.str()));
			DEBUG_ASSERTCRASH(
				((mt->m_whichInterfaces & (MODULEINTERFACE_DIE)) != 0) == (bm->getDie() != nullptr),
				("getInterfaceMask bad for MODULE_DIE (%s)",name.str()));
			DEBUG_ASSERTCRASH(
				((mt->m_whichInterfaces & (MODULEINTERFACE_SPECIAL_POWER)) != 0) == (bm->getSpecialPower() != nullptr),
				("getInterfaceMask bad for MODULE_SPECIAL_POWER (%s)",name.str()));
			DEBUG_ASSERTCRASH(
				((mt->m_whichInterfaces & (MODULEINTERFACE_UPDATE)) != 0) == (bm->getUpdate() != nullptr),
				("getInterfaceMask bad for MODULE_UPDATE (%s)",name.str()));
			DEBUG_ASSERTCRASH(
				((mt->m_whichInterfaces & (MODULEINTERFACE_UPGRADE)) != 0) == (bm->getUpgrade() != nullptr),
				("getInterfaceMask bad for MODULE_UPGRADE (%s)",name.str()));
		}
#endif

		return mod;
	}

	return nullptr;

}

//-------------------------------------------------------------------------------------------------
/** Add a module template to our list of templates */
//-------------------------------------------------------------------------------------------------
void ModuleFactory::addModuleInternal( NewModuleProc proc, NewModuleDataProc dataproc, ModuleType type, const AsciiString& name, Int whichIntf )
{
	NameKeyType namekey = makeDecoratedNameKey(name, type);
	ModuleTemplate& mtm = m_moduleTemplateMap[namekey];	// this creates it if it does not exist already
	mtm.m_createProc = proc;
	mtm.m_createDataProc = dataproc;
	mtm.m_whichInterfaces = whichIntf;
}

//-------------------------------------------------------------------------------------------------
void ModuleFactory::crc( Xfer *xfer )
{
	for (ModuleDataList::iterator mdIt = m_moduleDataList.begin(); mdIt != m_moduleDataList.end(); ++mdIt)
	{
		((ModuleData *)(*mdIt))->crc(xfer);
	}
}

//-------------------------------------------------------------------------------------------------
void ModuleFactory::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	for (ModuleDataList::iterator mdIt = m_moduleDataList.begin(); mdIt != m_moduleDataList.end(); ++mdIt)
	{
		((ModuleData *)(*mdIt))->xfer(xfer);
	}
}

//-------------------------------------------------------------------------------------------------
void ModuleFactory::loadPostProcess( void )
{
}
