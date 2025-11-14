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

// FILE: ScriptActions.h ///////////////////////////////////////////////////////////////////////////
// Executes script actions for the scripting engine.
// Author: John Ahlquist, Nov 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class ScriptAction;
class GameWindow;
class	Team;
class View;

enum AudioAffect CPP_11(: Int);

//-----------------------------------------------------------------------------
// ScriptActionsInterface
//-----------------------------------------------------------------------------
/** Pure virtual class for Script Actions interface format */
//-----------------------------------------------------------------------------
class ScriptActionsInterface : public SubsystemInterface
{

public:

	virtual ~ScriptActionsInterface() { };

	virtual void init( void ) = 0;		///< Init
	virtual void reset( void ) = 0;		///< Reset
	virtual void update( void ) = 0;	///< Update

	virtual void executeAction( ScriptAction *pAction ) = 0; ///< execute a script action.
	virtual void closeWindows( Bool suppressNewWindows ) = 0;

	// Called by the script engine in postProcessLoad()
	virtual void doEnableOrDisableObjectDifficultyBonuses(Bool enableBonuses) = 0;
};
extern ScriptActionsInterface *TheScriptActions;   ///< singleton definition


//-----------------------------------------------------------------------------
// ScriptActions
//-----------------------------------------------------------------------------
/** Implementation for the Script Engine singleton */
//-----------------------------------------------------------------------------
class ScriptActions : public ScriptActionsInterface
{

public:
	ScriptActions();
	~ScriptActions();

public:
	virtual void init( void );		///< Init
	virtual void reset( void );		///< Reset
	virtual void update( void );	///< Update

	void executeAction( ScriptAction *pAction );
	void closeWindows( Bool suppressNewWindows );

	void doEnableOrDisableObjectDifficultyBonuses(Bool enableBonuses);

protected:

	static GameWindow *m_messageWindow;
	static void clearWindow(void) {m_messageWindow=NULL;};

	Bool m_suppressNewWindows;
	AsciiString m_unnamedUnit;

protected: // helper functions
	void changeObjectPanelFlagForSingleObject(Object *obj, const AsciiString& flagToChange, Bool newVal );

protected:
	void doChooseVictimAlwaysUsesNormal(Bool enable);
	void doDebugMessage(const AsciiString& msg, Bool pause);
	void doPlaySoundEffect(const AsciiString& sound);
	void doMoveCameraTo(const AsciiString& waypoint, Real sec, Real cameraStutterSec);
	void doSetupCamera(const AsciiString& waypoint, Real zoom, Real pitch, const AsciiString& lookAtWaypoint);
	void doRotateCamera(Real rotations, Real sec);
	void doRotateCameraTowardObject(const AsciiString& unitName, Real sec, Real holdSec);
	void doRotateCameraTowardWaypoint(const AsciiString& unitName, Real sec);
	void doPitchCamera(Real pitch, Real sec);
	void doZoomCamera(Real zoom, Real sec);
	void doResetCamera(const AsciiString& waypoint, Real sec);
	void doCameraFollowNamed(const AsciiString& unit, Bool snapToUnit);
	void doStopCameraFollowUnit(void);

	void doCameraTetherNamed(const AsciiString& unit, Bool snapToUnit, Real play);
	void doCameraStopTetherNamed(void);
	void doCameraSetDefault(Real pitch, Real angle, Real maxHeight);

	void doOversizeTheTerrain(Int amount);
	void doMoveCameraAlongWaypointPath(const AsciiString& waypoint, Real sec, Real cameraStutterSec);
	void doPlaySoundEffectAt(const AsciiString& sound, const AsciiString& waypoint);
	void doVictory(void);
	void doQuickVictory(void);
	void doSetInfantryLightingOverride(Real setting);
	void doDamageTeamMembers(const AsciiString& team, Real amount);
	void doModCameraMoveToSelection(void);
	void doDefeat(void);
	void doLocalDefeat(void);
	void doMoveToWaypoint(const AsciiString& team, const AsciiString& waypoint);
	void doNamedMoveToWaypoint(const AsciiString& unit, const AsciiString& waypoint);
	void doSetTeamState(const AsciiString& team, const AsciiString& state);
	void doCreateReinforcements(const AsciiString& team, const AsciiString& waypoint);
	void doModCameraLookToward(const AsciiString& waypoint);
	void doModCameraFinalLookToward(const AsciiString& waypoint);
	void doCreateObject(const AsciiString& objectName, const AsciiString& thingName, const AsciiString& team, Coord3D *pos, Real angle);
	void doAttack(const AsciiString& attackerName, const AsciiString& victimName);
	void doNamedAttack(const AsciiString& attackerName, const AsciiString& victimName);
	void doBuildBuilding(const AsciiString& buildingType);
	void doBuildSupplyCenter(const AsciiString& playerName, const AsciiString& buildingType, Int cash);
	void doBuildUpgrade(const AsciiString& playerName, const AsciiString& upgrade);
	void doBuildBaseDefense(Bool flank);
	void doBuildBaseStructure(const AsciiString& buildingType, Bool flank);
	void createUnitOnTeamAt(const AsciiString& unitName, const AsciiString& objType, const AsciiString& teamName, const AsciiString& waypoint);
	void doNamedAttackArea(const AsciiString& unitName, const AsciiString& areaName);
	void doNamedAttackTeam(const AsciiString& unitName, const AsciiString& teamName);
	void doTeamAttackArea(const AsciiString& teamName, const AsciiString& areaName);
	void doTeamAttackNamed(const AsciiString& teamName, const AsciiString& unitName);
	void doNamedEnterNamed(const AsciiString& unitSrcName, const AsciiString& unitDestName);
	void doTeamEnterNamed(const AsciiString& teamName, const AsciiString& unitDestName);
	void doNamedExitAll(const AsciiString& unitName);
	void doTeamExitAll(const AsciiString& teamName);
	void doNamedFollowWaypoints(const AsciiString& unitName, const AsciiString& waypointName);
	void doTeamFollowWaypoints(const AsciiString& teamName, const AsciiString& waypointName, Bool asTeam);
	void doTeamFollowWaypointsExact(const AsciiString& teamName, const AsciiString& waypointName, Bool asTeam);
	void doNamedFollowWaypointsExact(const AsciiString& unitName, const AsciiString& waypointName);
	void doTeamFollowSkirmishApproachPath(const AsciiString& teamName, const AsciiString& waypointName, Bool asTeam);
	void doTeamMoveToSkirmishApproachPath(const AsciiString& teamName, const AsciiString& waypointName);
	void doNamedHunt(const AsciiString& unitName);
	void doTeamHunt(const AsciiString& teamName);
	void doTeamHuntWithCommandButton(const AsciiString& teamName, const AsciiString& commandButton);
	void doPlayerHunt(const AsciiString& playerName);
	void doNamedDelete(const AsciiString& unitName);

	void doTeamGarrisonSpecificBuilding(const AsciiString& teamName, const AsciiString& buildingName);
	void doTeamGarrisonNearestBuilding(const AsciiString& teamName);
	void doTeamExitAllBuildings(const AsciiString& teamName);
	void doExitSpecificBuilding(const AsciiString& buildingName);

	void doUnitGarrisonSpecificBuilding(const AsciiString& unitName, const AsciiString& buildingName);
	void doUnitGarrisonNearestBuilding(const AsciiString& unitName);
	void doUnitExitBuilding(const AsciiString& unitName);

	void doPlayerGarrisonAllBuildings(const AsciiString& playerName);
	void doPlayerExitAllBuildings(const AsciiString& playerName);
	void doLetterBoxMode(Bool startLetterbox);	// if true, start it. If false, end it.
	void doBlackWhiteMode(Bool startBWMode, Int frames);	// if true, start it. If false, end it.
	void doSkyBox(Bool showSkyBox);	// if true, start it. If false, end it.

	void doFreezeTime( void );
	void doUnfreezeTime( void );

	void doMilitaryCaption(const AsciiString& briefing, Int duration);
	void doCameraSetAudibleDistance(Real audibleDistance);

	void doNamedSetHeld(const AsciiString& unit, Bool held);

	void doNamedSetStoppingDistance(const AsciiString& unit, Real stoppingDistance);
	void doSetStoppingDistance(const AsciiString& team, Real stoppingDistance);

	void doDisableSpecialPowerDisplay( void );
	void doEnableSpecialPowerDisplay( void );
	void doNamedHideSpecialPowerDisplay( const AsciiString& unit );
	void doNamedShowSpecialPowerDisplay( const AsciiString& unit );

	void doNamedStopSpecialPowerCountdown( const AsciiString& unit, const AsciiString& specialPower, Bool stop );
	void doNamedSetSpecialPowerCountdown( const AsciiString& unit, const AsciiString& specialPower, Int frames );
	void doNamedAddSpecialPowerCountdown( const AsciiString& unit, const AsciiString& specialPower, Int frames );
	void doNamedFireSpecialPowerAtWaypoint( const AsciiString& unit, const AsciiString& specialPower, const AsciiString& waypoint );
	void doNamedFireSpecialPowerAtNamed( const AsciiString& unit, const AsciiString& specialPower, const AsciiString& target );
	void doSkirmishFireSpecialPowerAtMostCost( const AsciiString& player, const AsciiString& specialPower );
	void doNamedFireWeaponFollowingWaypointPath( const AsciiString& unit, const AsciiString& waypointPath );
	void doNamedUseCommandButtonAbility( const AsciiString& unit, const AsciiString& ability );
	void doNamedUseCommandButtonAbilityOnNamed( const AsciiString& unit, const AsciiString& ability, const AsciiString& target );
	void doNamedUseCommandButtonAbilityAtWaypoint( const AsciiString& unit, const AsciiString& ability, const AsciiString& waypoint );
	void doTeamUseCommandButtonAbility( const AsciiString& team, const AsciiString& ability );
	void doTeamUseCommandButtonAbilityOnNamed( const AsciiString& team, const AsciiString& ability, const AsciiString& target );
	void doTeamUseCommandButtonAbilityAtWaypoint( const AsciiString& team, const AsciiString& ability, const AsciiString& waypoint );

	void doDisplayCountdownTimer(const AsciiString& timerName, const AsciiString& timerText);
	void doHideCountdownTimer(const AsciiString& timerName);
	void doDisableCountdownTimerDisplay(void);
	void doEnableCountdownTimerDisplay(void);

	void doDisplayCounter(const AsciiString& counterName, const AsciiString& counterText);
	void doHideCounter(const AsciiString& counterName);

	void doAudioSetVolume(AudioAffect whichToAffect, Real newVolumeLevel);

	void doTransferTeamToPlayer(const AsciiString& teamName, const AsciiString& playerName);

	void doSetMoney(const AsciiString& playerName, Int money);		// Set a player's cash reserves to a specific value.
	void doGiveMoney(const AsciiString& playerName, Int money);	// Add/subtract cash from a player's reserves.

	void updateNamedAttackPrioritySet(const AsciiString& unitName, const AsciiString& attackPrioritySet);
	void updateTeamAttackPrioritySet(const AsciiString& teamName, const AsciiString& attackPrioritySet);
	void updateBaseConstructionSpeed(const AsciiString& playerName, Int speed);
	void updateNamedSetAttitude(const AsciiString& unitName, Int attitude);
	void updateTeamSetAttitude(const AsciiString& teamName, Int attitude);
	void doNamedSetRepulsor(const AsciiString& unitName, Bool repulsor);
	void doTeamSetRepulsor(const AsciiString& teamName, Bool repulsor);
	void doLoadAllTransports(const AsciiString& teamName);
	void doNamedGuard(const AsciiString& unitName);
	void doTeamGuard(const AsciiString& teamName);
	void doTeamGuardPosition(const AsciiString& teamName, const AsciiString& waypointName);
	void doTeamGuardObject(const AsciiString& teamName, const AsciiString& unitName);
	void doTeamGuardArea(const AsciiString& teamName, const AsciiString& areaName);
	void doPlayerSellEverything(const AsciiString& playerName);
	void doPlayerDisableBaseConstruction(const AsciiString& playerName);
	void doPlayerDisableFactories(const AsciiString& playerName, const AsciiString& objectName);
	void doPlayerDisableUnitConstruction(const AsciiString& playerName);
	void doPlayerEnableBaseConstruction(const AsciiString& playerName);
	void doPlayerEnableFactories(const AsciiString& playerName, const AsciiString& objectName);
	void doPlayerRepairStructure(const AsciiString& playerName, const AsciiString& objectName);
	void doPlayerEnableUnitConstruction(const AsciiString& playerName);
	void doCameraMoveHome(void);
	void doBuildTeam(const AsciiString& teamName);
	void doRecruitTeam(const AsciiString& teamName, Real recrutiRadius);
	void doNamedDamage(const AsciiString& unitName, Int damageAmt);
	void doTeamDelete(const AsciiString& teamName, Bool ignoreDead);
	void doTeamIncreasePriority(const AsciiString& teamName);
	void doTeamDecreasePriority(const AsciiString& teamName);
	void doTeamWander(const AsciiString& teamName, const AsciiString& waypointName);
	void doTeamPanic(const AsciiString& teamName, const AsciiString& waypointName);
	void doTeamWanderInPlace(const AsciiString& teamName);
	void doNamedKill(const AsciiString& unitName);
	void doTeamKill(const AsciiString& teamName);
	void doPlayerKill(const AsciiString& playerName);
	void doDisplayText(const AsciiString& displayText);
	void doDisplayCinematicText(const AsciiString& displayText, const AsciiString& fontType, Int timeInSeconds);
	void doCameoFlash(const AsciiString& cameoFlash, Int timeInSeconds);
	void doNamedFlash(const AsciiString& unitName, Int timeInSeconds, const RGBColor *color);
	void doNamedCustomColor(const AsciiString& unitName, Color c);
	void doTeamFlash(const AsciiString& teamName, Int timeInSeconds, const RGBColor *color);
	void doMoviePlayFullScreen(const AsciiString& movieName);
	void doMoviePlayRadar(const AsciiString& movieName);
	void doSoundPlayFromNamed(const AsciiString& soundName, const AsciiString& unitName);
	void doSpeechPlay(const AsciiString& speechName, Bool allowOverlap);
	void doPlayerTransferAssetsToPlayer(const AsciiString& playerSrcName, const AsciiString& playerDstName);
	void doNamedTransferAssetsToPlayer(const AsciiString& unitName, const AsciiString& playerDstName);
	void excludePlayerFromScoreScreen(const AsciiString& playerName);
	void enableScoring(Bool score);
	void updatePlayerRelationTowardPlayer(const AsciiString& playerSrcName, Int relationType, const AsciiString& playerDestPlayer);
	void doRadarCreateEvent(Coord3D *pos, Int eventType);
	void doRadarDisable(void);
	void doRadarEnable(void);
	void doNamedEnableStealth(const AsciiString& unitName, Bool enabled);
	void doTeamEnableStealth(const AsciiString& unitName, Bool enabled);
	void doRevealMapAtWaypoint(const AsciiString& waypointName, Real radiusToReveal, const AsciiString& playerName);
	void doShroudMapAtWaypoint(const AsciiString& waypointName, Real radiusToShroud, const AsciiString& playerName);
	void doTeamAvailableForRecruitment(const AsciiString& teamName, Bool availability);
	void doCollectNearbyForTeam(const AsciiString& teamName);
	void doMergeTeamIntoTeam(const AsciiString& teamSrcName, const AsciiString& teamDestName);
	void doIdleAllPlayerUnits(const AsciiString& playerName);
	void doResumeSupplyTruckingForIdleUnits(const AsciiString& playerName);
	void doDisableInput();
	void doEnableInput();
	void doSetBorderShroud( Bool setting );
	void doAmbientSoundsPause(Bool pausing);	// if true, then pause, if false then resume.
	void doMusicTrackChange(const AsciiString& newTrackName, Bool fadeout, Bool fadein);
	void doRevealMapEntire(const AsciiString& playerName);
	void doRevealMapEntirePermanently( Bool reveal, const AsciiString& playerName );
	void doShroudMapEntire(const AsciiString& playerName);
	void doCameraMotionBlur(Bool zoomIn, Bool saturate);
	void doCameraMotionBlurJump(const AsciiString& waypointName, Bool saturate);
	void doRadarRefresh( void );
	void doNamedStop(const AsciiString& unitName);
	void doTeamStop(const AsciiString& teamName, Bool shouldDisband);
	void doTeamSetOverrideRelationToTeam(const AsciiString& teamName, const AsciiString& otherTeam, Int relation);
	void doTeamRemoveOverrideRelationToTeam(const AsciiString& teamName, const AsciiString& otherTeam);
	void doTeamSetOverrideRelationToPlayer(const AsciiString& teamName, const AsciiString& otherPlayer, Int relation);
	void doTeamRemoveOverrideRelationToPlayer(const AsciiString& teamName, const AsciiString& otherPlayer);
	void doPlayerSetOverrideRelationToTeam(const AsciiString& playerName, const AsciiString& otherTeam, Int relation);
	void doPlayerRemoveOverrideRelationToTeam(const AsciiString& playerName, const AsciiString& otherTeam);
	void doTeamRemoveAllOverrideRelations(const AsciiString& teamName);
	void doUnitStartSequentialScript(const AsciiString& unitName, const AsciiString& scriptName, Int loopVal);
	void doUnitStopSequentialScript(const AsciiString& unitName);
	void doTeamStartSequentialScript(const AsciiString& teamName, const AsciiString& scriptName, Int loopVal);
	void doTeamStopSequentialScript(const AsciiString& teamName);
	void doUnitGuardForFramecount(const AsciiString& unitName, Int framecount);
	void doUnitIdleForFramecount(const AsciiString& unitName, Int framecount);
	void doTeamGuardForFramecount(const AsciiString& teamName, Int framecount);
	void doTeamIdleForFramecount(const AsciiString& teamName, Int framecount);
	void doWaterChangeHeight(const AsciiString& waterName, Real newHeight);
	void doWaterChangeHeightOverTime( const AsciiString& waterName, Real newHeight, Real time, Real damage );
	void doBorderSwitch(Int borderToUse);
	void doForceObjectSelection(const AsciiString& teamName, const AsciiString& objectType, Bool centerInView, const AsciiString& audioToPlay);
	void doDestroyAllContained(const AsciiString& unitName, Int damageType);
	void doRadarForceEnable(void);
	void doRadarRevertNormal(void);
	void doScreenShake( UnsignedInt intensity );
	void doModifyBuildableStatus( const AsciiString& objectType, Int buildableStatus );
	void doSetWarehouseValue( const AsciiString& warehouseName, Int cashValue );
	void doSetCaveIndex( const AsciiString& caveName, Int caveIndex );
	void doObjectRadarCreateEvent( const AsciiString& unitName, Int eventType );
	void doTeamRadarCreateEvent( const AsciiString& teamName, Int eventType );
	void doSoundEnableType( const AsciiString& soundEventName, Bool enable );
	void doSoundRemoveType( const AsciiString& soundEventName );
	void doSoundRemoveAllDisabled();
	void doSoundOverrideVolume( const AsciiString& soundEventName, Real newVolume );
	void doInGamePopupMessage( const AsciiString& message, Int x, Int y, Int width, Bool pause );
	void doSetToppleDirection( const AsciiString& unitName, const Coord3D* direction);
	void doMoveUnitTowardsNearest( const AsciiString& unitName, const AsciiString& objectType, AsciiString triggerName);
	void doMoveTeamTowardsNearest( const AsciiString& teamName, const AsciiString& objectType, AsciiString triggerName);
	void doUnitReceiveUpgrade( const AsciiString& unitName, const AsciiString& upgradeName );
	void doSkirmishAttackNearestGroupWithValue( const AsciiString& teamName, Int comparison, Int value );
	void doSkirmishCommandButtonOnMostValuable( const AsciiString& teamName, const AsciiString& commandButton, Real range, Bool allTeamMembers);
	void doTeamSpinForFramecount( const AsciiString& teamName, Int waitForFrames );
	void doTeamUseCommandButtonOnNamed( const AsciiString& teamName, const AsciiString& commandAbility, const AsciiString& unitName );
	void doTeamUseCommandButtonOnNearestEnemy( const AsciiString& teamName, const AsciiString& commandAbility );
	void doTeamUseCommandButtonOnNearestGarrisonedBuilding( const AsciiString& teamName, const AsciiString& commandAbility );
	void doTeamUseCommandButtonOnNearestKindof( const AsciiString& teamName, const AsciiString& commandAbility, Int kindofBit );
	void doTeamUseCommandButtonOnNearestBuilding( const AsciiString& teamName, const AsciiString& commandAbility );
	void doTeamUseCommandButtonOnNearestBuildingClass( const AsciiString& teamName, const AsciiString& commandAbility, Int kindofBit );
	void doTeamUseCommandButtonOnNearestObjectType( const AsciiString& teamName, const AsciiString& commandAbility, const AsciiString& objectType );
	void doTeamPartialUseCommandButton( Real percentage, const AsciiString& teamName, const AsciiString& commandAbility );
	void doTeamCaptureNearestUnownedFactionUnit( const AsciiString& teamName );
	void doCreateTeamFromCapturedUnits( const AsciiString& playerName, const AsciiString& teamName );
	void doPlayerAddSkillPoints(const AsciiString& playerName, Int delta);
	void doPlayerAddRankLevels(const AsciiString& playerName, Int delta);
	void doPlayerSetRankLevel(const AsciiString& playerName, Int level);
	void doMapSetRankLevelLimit(Int level);
	void doPlayerGrantScience(const AsciiString& playerName, const AsciiString& scienceName);
	void doPlayerPurchaseScience(const AsciiString& playerName, const AsciiString& scienceName);
	void doPlayerSetScienceAvailability( const AsciiString& playerName, const AsciiString& scienceName, const AsciiString& scienceAvailability );
	void doTeamEmoticon(const AsciiString& teamName, const AsciiString& emoticonName, Real duration);
	void doNamedEmoticon(const AsciiString& unitName, const AsciiString& emoticonName, Real duration);
	void doObjectTypeListMaintenance(const AsciiString& objectList, const AsciiString& objectType, Bool addObject);
	void doRevealMapAtWaypointPermanent(const AsciiString& waypointName, Real radiusToReveal, const AsciiString& playerName, const AsciiString& lookName);
	void doUndoRevealMapAtWaypointPermanent(const AsciiString& lookName);
	void doEvaEnabledDisabled(Bool setEnabled);
	void doSetOcclusionMode(Bool setEnabled);
	void doSetDrawIconUIMode(Bool setEnabled);
	void doSetDynamicLODMode(Bool setEnabled);
	void doAffectObjectPanelFlagsUnit(const AsciiString& unitName, const AsciiString& flagName, Bool enable);
	void doAffectObjectPanelFlagsTeam(const AsciiString& teamName, const AsciiString& flagName, Bool enable);
	void doGuardSupplyCenter(const AsciiString& teamName, Int supplies);
	void doTeamGuardInTunnelNetwork(const AsciiString& teamName);
	void doAffectPlayerSkillset(const AsciiString& playerName, Int skillset);
	void doOverrideHulkLifetime( Real seconds );
	void doNamedFaceNamed( const AsciiString &unitName, const AsciiString &faceUnitName );
	void doNamedFaceWaypoint( const AsciiString &unitName, const AsciiString &faceWaypointName );
	void doTeamFaceNamed( const AsciiString &teamName, const AsciiString &faceUnitName );
	void doTeamFaceWaypoint( const AsciiString &teamName, const AsciiString &faceWaypointName );
	void doRemoveCommandBarButton(const AsciiString& commandBarButton, const AsciiString& objectType);
	void doAddCommandBarButton(const AsciiString& commandBarButton, const AsciiString& objectType, Int slotNum);
	void doAffectSkillPointsModifier(const AsciiString& playerName, Real newModifier);
	void doResizeViewGuardband(const Real gbx, const Real gby );
	void deleteAllUnmanned();

};
