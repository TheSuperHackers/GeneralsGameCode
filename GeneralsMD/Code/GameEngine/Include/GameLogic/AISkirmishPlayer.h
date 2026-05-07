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

// AISkirmishPlayer.h
// Computerized opponent
// Author: Michael S. Booth, January 2002

#pragma once

#include "Common/GameMemory.h"
#include "GameLogic/AIPlayer.h"

class BuildListInfo;
class SpecialPowerTemplate;


/**
 * The computer-controlled opponent.
 */
class AISkirmishPlayer : public AIPlayer
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AISkirmishPlayer, "AISkirmishPlayer"  )

public:	 // AISkirmish specific methods.

	AISkirmishPlayer( Player *p );							///< constructor
	virtual Bool computeSuperweaponTarget(const SpecialPowerTemplate *power, Coord3D *pos, Int playerNdx, Real weaponRadius) override; ///< Calculates best pos for weapon given radius.

public:	// AIPlayer interface methods.

	virtual void update() override;											///< simulates the behavior of a player

	virtual void newMap() override;											///< New map loaded call.

	/// Invoked when a unit I am training comes into existence
	virtual void onUnitProduced( Object *factory, Object *unit ) override;

	virtual void buildSpecificAITeam(TeamPrototype *teamProto, Bool priorityBuild) override; ///< Builds this team immediately.

	virtual void buildSpecificAIBuilding(const AsciiString &thingName) override; ///< Builds this building as soon as possible.

	virtual void buildAIBaseDefense(Bool flank) override; ///< Builds base defense on front or flank of base.

	virtual void buildAIBaseDefenseStructure(const AsciiString &thingName, Bool flank) override; ///< Builds base defense on front or flank of base.

	virtual void recruitSpecificAITeam(TeamPrototype *teamProto, Real recruitRadius) override; ///< Builds this team immediately.

	virtual Bool isSkirmishAI() override {return true;}

	virtual Bool checkBridges(Object *unit, Waypoint *way) override;

	virtual Player *getAiEnemy() override;	///< Solo AI attacks based on scripting.  Only skirmish auto-acquires an enemy at this point.  jba.

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer ) override;
	virtual void xfer( Xfer *xfer ) override;
	virtual void loadPostProcess() override;

	virtual void doBaseBuilding() override;
	virtual void checkReadyTeams() override;
	virtual void checkQueuedTeams() override;
	virtual void doTeamBuilding() override;
	virtual Object *findDozer(const Coord3D *pos) override;
	virtual void queueDozer() override;

protected:

	virtual Bool selectTeamToBuild() override;			///< determine the next team to build
	virtual Bool selectTeamToReinforce( Int minPriority ) override;			///< determine the next team to reinforce
	virtual Bool startTraining( WorkOrder *order, Bool busyOK, AsciiString teamName) override;	///< find a production building that can handle the order, and start building

	virtual Bool isAGoodIdeaToBuildTeam( TeamPrototype *proto ) override;		///< return true if team should be built
	virtual void processBaseBuilding() override;		///< do base-building behaviors
	virtual void processTeamBuilding() override;		///< do team-building behaviors

protected:
	void adjustBuildList(BuildListInfo *list);
	Int getMyEnemyPlayerIndex();
	void acquireEnemy();
	// Periodically sweeps the AI's idle combat units into a group attack on the
	// current enemy. Backstops the script-driven team flow so accumulated idle
	// units don't sit in base late game.
	void commitIdleArmy();
	// Scan ally beacons for the directive prefix; track the active directive
	// beacon ID; announce on switch and on minute boundaries.
	void processBeaconDirective();
	// Once-per-second phase milestone announcements (mid-game at 5min, late-game at 15min)
	// plus the Syn/Dan broadcast easter egg fired at the late-game milestone.
	void announceMilestones();
	// Sample owned-structure HP on a 2s cadence; trigger an ally-only distress
	// message when HP drops sharply or a structure is destroyed within the window.
	void processDistressSignal();
	// Local helper invoked from commitIdleArmy() after a force is dispatched
	// against an enemy structure (skipped when the dispatch was a beacon directive).
	void announceAttackCommit(const Coord3D *target, Player *enemyPlayer);

protected:
	Int m_curFrontBaseDefense; // First is 0.
	Int m_curFlankBaseDefense; // First is 0.
	Real m_curFrontLeftDefenseAngle;
	Real m_curFrontRightDefenseAngle;
	Real m_curLeftFlankLeftDefenseAngle;
	Real m_curLeftFlankRightDefenseAngle;
	Real m_curRightFlankLeftDefenseAngle;
	Real m_curRightFlankRightDefenseAngle;

	UnsignedInt m_frameToCheckEnemy;
	Player			*m_currentEnemy;

	UnsignedInt m_nextIdleSweepFrame;

	// Beacon directive ("!attack" prefix in an ally beacon's caption).
	ObjectID    m_directiveBeaconID;
	UnsignedInt m_nextDirectiveScanFrame;
	UnsignedInt m_nextDirectiveAnnounceFrame;

	// Ally communication: phase milestones, attack-commit, and distress signals.
	UnsignedInt m_nextMilestoneCheckFrame;
	Int         m_milestoneAnnouncedMask; // bit 0 = mid-game, bit 1 = late-game
	Bool        m_synAnnounced;
	UnsignedInt m_nextAttackAnnounceFrame;
	UnsignedInt m_nextDistressSampleFrame;
	UnsignedInt m_nextDistressAnnounceFrame;
	Real        m_distressHPRing[5];
	Int         m_distressCountRing[5];
	Int         m_distressRingHead;
	Int         m_distressRingFilled;

};
