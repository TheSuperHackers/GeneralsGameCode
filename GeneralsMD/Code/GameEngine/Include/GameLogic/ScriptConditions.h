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

// FILE: ScriptConditions.h ///////////////////////////////////////////////////////////////////////////
// Script conditions evaluator for the scripting engine.
// Author: John Ahlquist, Nov 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class Condition;
class ObjectTypes;
class Parameter;
class Player;
//-----------------------------------------------------------------------------
// ScriptConditionsInterface
//-----------------------------------------------------------------------------
/** Pure virtual class for Script Conditions interface format */
//-----------------------------------------------------------------------------
class ScriptConditionsInterface : public SubsystemInterface
{

public:

	virtual ~ScriptConditionsInterface() { };

	virtual void init( void ) = 0;		///< Init
	virtual void reset( void ) = 0;		///< Reset
	virtual void update( void ) = 0;	///< Update

	virtual Bool evaluateCondition( Condition *pCondition ) = 0; ///< evaluate a a script condition.

	virtual Bool evaluateSkirmishCommandButtonIsReady( Parameter *pSkirmishPlayerParm, Parameter *pTeamParm, Parameter *pCommandButtonParm, Bool allReady ) = 0;
	virtual Bool evaluateTeamIsContained(Parameter *pTeamParm, Bool allContained) = 0;

};
extern ScriptConditionsInterface *TheScriptConditions;   ///< singleton definition


//-----------------------------------------------------------------------------
// ScriptConditions
//-----------------------------------------------------------------------------
/** Implementation for the Script Conditions singleton */
//-----------------------------------------------------------------------------
class ScriptConditions : public ScriptConditionsInterface
{

public:
	ScriptConditions();
	~ScriptConditions();

public:

	virtual void init( void );		///< Init
	virtual void reset( void );		///< Reset
	virtual void update( void );	///< Update

	Bool evaluateCondition( Condition *pCondition );

protected:
	Player *playerFromParam(Parameter *pSideParm);			// Gets a player from a parameter.
	void objectTypesFromParam(Parameter *pTypeParm, ObjectTypes *outObjectTypes);		// Must pass in a valid objectTypes for outObjectTypes

	Bool evaluateAllDestroyed(Parameter *pSideParm);
	Bool evaluateAllBuildFacilitiesDestroyed(Parameter *pSideParm);
	Bool evaluateIsDestroyed(Parameter *pTeamParm);
	Bool evaluateBridgeBroken(Parameter *pBridgeParm);
	Bool evaluateBridgeRepaired(Parameter *pBridgeParm);
	Bool evaluateNamedUnitDestroyed(Parameter *pUnitParm);
	Bool evaluateNamedUnitExists(Parameter *pUnitParm);
	Bool evaluateNamedUnitDying(Parameter *pUnitParm);
	Bool evaluateNamedUnitTotallyDead(Parameter *pUnitParm);
	Bool evaluateHasUnits(Parameter *pTeamParm);

	Bool evaluateTeamEnteredAreaEntirely(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm);
	Bool evaluateTeamEnteredAreaPartially(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm);
	Bool evaluateTeamExitedAreaEntirely(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm);
	Bool evaluateTeamExitedAreaPartially(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm);
	Bool evaluateTeamInsideAreaEntirely(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm);
	Bool evaluateTeamInsideAreaPartially(Parameter *pUnitParm, Parameter *pTriggerParm, Parameter *pTypeParm);
	Bool evaluateTeamOutsideAreaEntirely(Parameter *pTeamParm, Parameter *pTriggerParm, Parameter *pTypeParm);

	Bool evaluateNamedEnteredArea(Parameter *pUnitParm, Parameter *pTriggerParm);
	Bool evaluateNamedExitedArea(Parameter *pUnitParm, Parameter *pTriggerParm);
	Bool evaluateNamedInsideArea(Parameter *pUnitParm, Parameter *pTriggerParm);
	Bool evaluateNamedOutsideArea(Parameter *pUnitParm, Parameter *pTriggerParm);

	Bool evaluateTeamStateIs(Parameter *pTeamParm, Parameter *pStateParm);
	Bool evaluateTeamStateIsNot(Parameter *pTeamParm, Parameter *pStateParm);
	Bool evaluatePlayerHasCredits(Parameter *pCreditsParm, Parameter* pComparisonParm, Parameter *pPlayerParm);
	Bool evaluateNamedCreated(Parameter* pUnitParm);	///< Implemented as evaluateNamedExists(...)
	Bool evaluateTeamCreated(Parameter* pTeamParm);		///< Implemented as evaluateTeamExists(...)
	Bool evaluateNamedOwnedByPlayer(Parameter *pUnitParm, Parameter *pPlayerParm);
	Bool evaluateTeamOwnedByPlayer(Parameter *pTeamParm, Parameter *pPlayerParm);
	Bool evaluateMultiplayerAlliedVictory(void);
	Bool evaluateMultiplayerAlliedDefeat(void);
	Bool evaluateMultiplayerPlayerDefeat(void);
	Bool evaluateNamedAttackedByType(Parameter *pUnitParm, Parameter *pTypeParm);
	Bool evaluateTeamAttackedByType(Parameter *pTeamParm, Parameter *pTypeParm);
	Bool evaluateNamedAttackedByPlayer(Parameter *pUnitParm, Parameter *pPlayerParm);
	Bool evaluateTeamAttackedByPlayer(Parameter *pTeamParm, Parameter *pPlayerParm);
	Bool evaluateBuiltByPlayer(Condition *pCondition, Parameter* pTypeParm, Parameter* pPlayerParm);
	Bool evaluatePlayerHasNOrFewerBuildings(Parameter *pBuildingCountParm, Parameter *pPlayerParm);
	Bool evaluatePlayerHasNOrFewerFactionBuildings(Parameter *pBuildingCountParm, Parameter *pPlayerParm);
	Bool evaluatePlayerHasPower(Parameter *pPlayerParm);
	Bool evaluateNamedReachedWaypointsEnd(Parameter *pUnitParm, Parameter* pWaypointPathParm);
	Bool evaluateTeamReachedWaypointsEnd(Parameter *pTeamParm, Parameter* pWaypointPathParm);
	Bool evaluateNamedSelected(Condition *pCondition, Parameter *pUnitParm);
	Bool evaluateVideoHasCompleted(Parameter *pVideoParm);
	Bool evaluateSpeechHasCompleted(Parameter *pSpeechParm);
	Bool evaluateAudioHasCompleted(Parameter *pAudioParm);
	Bool evaluateNamedDiscovered(Parameter *pItemParm, Parameter* pPlayerParm);
	Bool evaluateTeamDiscovered(Parameter *pTeamParm, Parameter *pPlayerParm);
	Bool evaluateBuildingEntered(Parameter *pItemParm, Parameter *pUnitParm );
	Bool evaluateIsBuildingEmpty(Parameter *pItemParm);
	Bool evaluateEnemySighted(Parameter *pItemParm, Parameter *pAllianceParm, Parameter* pPlayerParm);
	Bool evaluateTypeSighted(Parameter *pItemParm, Parameter *pTypeParm, Parameter* pPlayerParm);
	Bool evaluateUnitHealth(Parameter *pItemParm, Parameter* pComparisonParm, Parameter *pHealthPercent);
	Bool evaluatePlayerUnitCondition(Condition *pCondition, Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm, Parameter *pUnitTypeParm);
	Bool evaluatePlayerSpecialPowerFromUnitTriggered(Parameter *pPlayerParm, Parameter *pSpecialPowerParm, Parameter* pUnitParm);
	Bool evaluatePlayerSpecialPowerFromUnitMidway		(Parameter *pPlayerParm, Parameter *pSpecialPowerParm, Parameter* pUnitParm);
	Bool evaluatePlayerSpecialPowerFromUnitComplete	(Parameter *pPlayerParm, Parameter *pSpecialPowerParm, Parameter* pUnitParm);
	Bool evaluateUpgradeFromUnitComplete						(Parameter *pPlayerParm, Parameter *pUpgradeParm,			 Parameter* pUnitParm);
	Bool evaluateScienceAcquired										(Parameter *pPlayerParm, Parameter *pScienceParm);
	Bool evaluateCanPurchaseScience									(Parameter *pPlayerParm, Parameter *pScienceParm);
	Bool evaluateSciencePurchasePoints							(Parameter *pPlayerParm, Parameter *pSciencePointParm);
	Bool evaluateNamedHasFreeContainerSlots(Parameter *pUnitParm);
	Bool evaluatePlayerDestroyedNOrMoreBuildings(Parameter *pPlayerParm, Parameter *pNumParm, Parameter *pOppenentParm);
	Bool evaluateUnitHasObjectStatus(Parameter *pUnitParm, Parameter *pObjectStatus);
	Bool evaluateTeamHasObjectStatus(Parameter *pTeamParm, Parameter *pObjectStatus, Bool entireTeam);
	Bool evaluatePlayerHasComparisonPercentPower(Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pPercent);
	Bool evaluatePlayerHasComparisonValueExcessPower(Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pKWHParm);
	Bool evaluatePlayerHasUnitTypeInArea(Condition *pCondition, Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm, Parameter *pTypeParm, Parameter *pTriggerParm);
	Bool evaluatePlayerHasUnitKindInArea(Condition *pCondition, Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm,Parameter *pKindParm, Parameter *pTriggerParm);
	Bool evaluateUnitHasEmptied(Parameter *pUnitParm);
	Bool evaluateTeamIsContained(Parameter *pTeamParm, Bool allContained);
	Bool evaluateMusicHasCompleted(Parameter *pMusicParm, Parameter *pIntParm);
	Bool evaluatePlayerLostObjectType(Parameter *pPlayerParm, Parameter *pTypeParm);

	// Skirmish Scripts. Please note that ALL Skirmish conditions should first pass a pSkirmishPlayerParm to
	// prevent the necessity of having to write additional scripts for other players / skirmish types later.
	Bool evaluateSkirmishSpecialPowerIsReady(Parameter *pSkirmishPlayerParm, Parameter *pPower);
	Bool evaluateSkirmishValueInArea(Condition *pCondition, Parameter *pSkirmishPlayerParm, Parameter *pComparisonParm, Parameter *pMoneyParm, Parameter *pTriggerParm);
	Bool evaluateSkirmishPlayerIsFaction(Parameter *pSkirmishPlayerParm, Parameter *pFactionParm);
	Bool evaluateSkirmishSuppliesWithinDistancePerimeter(Parameter *pSkirmishPlayerParm, Parameter *pDistanceParm, Parameter *pLocationParm, Parameter *pValueParm);
	Bool evaluateSkirmishPlayerTechBuildingWithinDistancePerimeter(Condition *pCondition, Parameter *pSkirmishPlayerParm, Parameter *pDistanceParm, Parameter *pLocationParm);
	Bool evaluateSkirmishCommandButtonIsReady( Parameter *pSkirmishPlayerParm, Parameter *pTeamParm, Parameter *pCommandButtonParm, Bool allReady );
	Bool evaluateSkirmishUnownedFactionUnitComparison( Parameter *pSkirmishPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm );
	Bool evaluateSkirmishPlayerHasPrereqsToBuild( Parameter *pSkirmishPlayerParm, Parameter *pObjectTypeParm );
	Bool evaluateSkirmishPlayerHasComparisonGarrisoned(Parameter *pSkirmishPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm );
	Bool evaluateSkirmishPlayerHasComparisonCapturedUnits(Parameter *pSkirmishPlayerParm, Parameter *pComparisonParm, Parameter *pCountParm );
	Bool evaluateSkirmishNamedAreaExists(Parameter *pSkirmishPlayerParm, Parameter *pTriggerParm);
	Bool evaluateSkirmishPlayerHasUnitsInArea(Condition *pCondition, Parameter *pSkirmishPlayerParm, Parameter *pTriggerParm );
	Bool evaluateSkirmishPlayerHasBeenAttackedByPlayer(Parameter *pSkirmishPlayerParm, Parameter *pAttackedByParm );
	Bool evaluateSkirmishPlayerIsOutsideArea(Condition *pCondition, Parameter *pSkirmishPlayerParm, Parameter *pTriggerParm );
	Bool evaluateSkirmishPlayerHasDiscoveredPlayer(Parameter *pSkirmishPlayerParm, Parameter *pDiscoveredByParm );
	Bool evaluateSkirmishSupplySourceSafe(Condition *pCondition, Parameter *pSkirmishPlayerParm, Parameter *pMinAmountOfSupplies );
	Bool evaluateSkirmishSupplySourceAttacked(Parameter *pSkirmishPlayerParm );
	Bool evaluateSkirmishStartPosition(Parameter *pSkirmishPlayerParm, Parameter *startNdx );


	// Stubs
	Bool evaluateMissionAttempts(Parameter *pPlayerParm, Parameter *pComparisonParm, Parameter *pAttemptsParm);


};
