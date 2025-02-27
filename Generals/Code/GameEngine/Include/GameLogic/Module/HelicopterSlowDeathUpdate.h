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

// FILE: HelicopterSlowDeathUpdate.h //////////////////////////////////////////////////////////////
// Author: Colin Day, March 2002
// Desc:   Helicoptor slow deaths
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/AudioEventRTS.h"
#include "GameLogic/Module/SlowDeathBehavior.h"

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////////////////////////
class ParticleSystemTemplate;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class HelicopterSlowDeathBehaviorModuleData : public SlowDeathBehaviorModuleData
{

public:

	HelicopterSlowDeathBehaviorModuleData( void );

	static void buildFieldParse(MultiIniFieldParse &p );

	Real m_spiralOrbitTurnRate;						///< (rads per frame) rate at which we do big circles down toward the ground
	Real m_spiralOrbitForwardSpeed;				///< (dist per frame) speed at which we move "forward" in the downward spiral
	Real m_spiralOrbitForwardSpeedDamping;///< every frame our forward speed in the orbit is adjusted by this amount
	Real m_minSelfSpin;										///< (rads per frame) min turning rate at which we spin around our center of gravity
	Real m_maxSelfSpin;										///< (rads per frame) max turning rate at which we spin around our center of gravity
	Real m_selfSpinUpdateDelay;						///< (frames) every this many frames we will update the self spin angle, but we'll keep it inbetween the min and max self spin
	Real m_selfSpinUpdateAmount;					///< (radian) when we update the self spin every SelfSpinUpdateDelay frames, we change it this much, but keep it between min and max self spin
	Real m_fallHowFast;										///< a fraction of gravity we use to modify the helicopert locmotor lift
	Real m_minBladeFlyOffDelay;						///< (frames) min frame that the blade will fly off at
	Real m_maxBladeFlyOffDelay;						///< (frames) max frame that the blade will fly off at
	const ParticleSystemTemplate *m_attachParticleSystem;		///< particle system to attach
	AsciiString m_attachParticleBone;			///< bone to attach particle system to
	Coord3D m_attachParticleLoc;					///< loc attach particle system to if bone not present
	AsciiString m_bladeObjectName;				///< object name of the blade piece
	AsciiString m_bladeBone;							///< bone name of main propeller blade
	const ObjectCreationList *m_oclEjectPilot;	///< OCL for ejecting pilot
	const FXList *m_fxBlade;							///< blade fly off event fx list
	const ObjectCreationList *m_oclBlade;	///< OCL at blade fly off event
	const FXList *m_fxHitGround;					///< fx for hitting the ground
	const ObjectCreationList *m_oclHitGround;  ///< OCL at hit ground event
	const FXList *m_fxFinalBlowUp;				///< the final blow up
	const ObjectCreationList *m_oclFinalBlowUp;///< OCL at final blow up event
	Real m_delayFromGroundToFinalDeath;		///< (frames) delay from when we hit the ground to final BOOM!
	AsciiString m_finalRubbleObject;			///< final rubble object to create after it's ALL over
	Real m_maxBraking;										///< max braking we may use during death spiral

	// @todo propagate this up to SlowDeathBehaviorModuleData. I don't wanna do it today, cause its 4/3. jkmcd
	AudioEventRTS m_deathSound;						///< Sound played during death sequence.

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class HelicopterSlowDeathBehavior : public SlowDeathBehavior
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( HelicopterSlowDeathBehavior, "HelicopterSlowDeathBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( HelicopterSlowDeathBehavior, HelicopterSlowDeathBehaviorModuleData )

public:

	HelicopterSlowDeathBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void beginSlowDeath( const DamageInfo *damageInfo );	///< begin the slow death cycle
	virtual UpdateSleepTime update();

protected:

	Int m_orbitDirection;			 ///< -1 or +1 ... use ORBIT_DIRECTION_LEFT/RIGHT
	Real m_forwardAngle;			 /**< angle of the direction "forward" we want to move in our downspin
																	which is independent of the actual direction angle of the object */
	Real m_forwardSpeed;			 ///< the speed we're travelling forward in our spiral orbit
	Real m_selfSpin;					 ///< rads per frame that we change our facing direction
	Bool m_selfSpinTowardsMax; ///< TRUE when our self spin rate is increasing towards the MaxSelfSpin
	UnsignedInt m_lastSelfSpinUpdateFrame;	///< frame we last updated the self spin on
	UnsignedInt m_bladeFlyOffFrame;		///< frame we throw the blade off at
	UnsignedInt m_hitGroundFrame;			///< frame we hit the ground on

	// @todo propagate this up to SlowDeathBehavior. I don't wanna do it today, cause its 4/3. jkmcd
	AudioEventRTS m_deathSound;						///< Sound played during death sequence.

};
