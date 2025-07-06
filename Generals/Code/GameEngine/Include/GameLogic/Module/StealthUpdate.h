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

// FILE: StealthUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, May 2002
// Desc:	 An update that checks for a status bit to stealth the owning object
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __STEALTH_UPDATE_H_
#define __STEALTH_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
enum StealthLookType CPP_11(: Int);
class FXList;

enum
{
	STEALTH_NOT_WHILE_ATTACKING					= 0x00000001,
	STEALTH_NOT_WHILE_MOVING						= 0x00000002,
	STEALTH_NOT_WHILE_USING_ABILITY			= 0x00000004,
	STEALTH_NOT_WHILE_FIRING_PRIMARY		= 0x00000008,
	STEALTH_NOT_WHILE_FIRING_SECONDARY	= 0x00000010,
	STEALTH_NOT_WHILE_FIRING_TERTIARY		= 0x00000020,
	STEALTH_NOT_WHILE_FIRING_WEAPON			= (STEALTH_NOT_WHILE_FIRING_PRIMARY | STEALTH_NOT_WHILE_FIRING_SECONDARY | STEALTH_NOT_WHILE_FIRING_TERTIARY),
};

#ifdef DEFINE_STEALTHLEVEL_NAMES
static const char *TheStealthLevelNames[] = 
{
	"ATTACKING",
	"MOVING",
	"USING_ABILITY",
	"FIRING_PRIMARY",
	"FIRING_SECONDARY",
	"FIRING_TERTIARY",
	NULL
};
#endif

#define INVALID_OPACITY -1.0f

//-------------------------------------------------------------------------------------------------
class StealthUpdateModuleData : public UpdateModuleData
{
public:
	UnsignedInt		m_stealthDelay;
	UnsignedInt		m_stealthLevel;
	ObjectStatusMaskType	m_hintDetectableStates;
	Real					m_stealthSpeed;
	Real					m_friendlyOpacityMin;
	Real					m_friendlyOpacityMax;
	UnsignedInt		m_pulseFrames;
	Bool					m_teamDisguised;
	Real					m_revealDistanceFromTarget;
	Bool					m_orderIdleEnemiesToAttackMeUponReveal;
	FXList				*m_disguiseRevealFX;
	FXList				*m_disguiseFX;
	Bool					m_innateStealth;
	UnsignedInt		m_disguiseTransitionFrames;
	UnsignedInt		m_disguiseRevealTransitionFrames;

	StealthUpdateModuleData()
	{
		//Added By Sadullah Nader
		//Initialization(s) inserted
		m_disguiseFX = NULL;
		m_disguiseRevealFX = NULL;
		//
		m_stealthDelay		= UINT_MAX;
		m_stealthLevel		= 0;
		m_stealthSpeed		= 0.0f;
		m_friendlyOpacityMin = 0.5f;
		m_friendlyOpacityMax = 1.0f;
		m_pulseFrames = 30;
		m_teamDisguised		= false;
		m_revealDistanceFromTarget = 0.0f;
		m_orderIdleEnemiesToAttackMeUponReveal = false;
		m_innateStealth   = true;
		m_disguiseTransitionFrames = 0;
		m_disguiseRevealTransitionFrames = 0;
	}

	static void buildFieldParse(MultiIniFieldParse& p);

};

//-------------------------------------------------------------------------------------------------
class StealthUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( StealthUpdate, "StealthUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( StealthUpdate, StealthUpdateModuleData );

public:

	StealthUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual StealthUpdate* getStealth() { return this; }

	virtual UpdateSleepTime update();

	//Still gets called, even if held -ML
	virtual DisabledMaskType getDisabledTypesToProcess() const { return MAKE_DISABLED_MASK( DISABLED_HELD ); }

	// ??? ugh
	Bool isDisguised() const { return m_disguiseAsTemplate != NULL; }
	Int getDisguisedPlayerIndex() const { return m_disguiseAsPlayerIndex; }
	const ThingTemplate *getDisguisedTemplate() { return m_disguiseAsTemplate; }
	void markAsDetected( UnsignedInt numFrames = 0 );
	void disguiseAsObject( const Object *target ); //wrapper function for ease.
	Real getFriendlyOpacity() const;
	UnsignedInt getStealthDelay() const { return getStealthUpdateModuleData()->m_stealthDelay; }

protected:

	StealthLookType calcStealthedStatusForPlayer(const Object* obj, const Player* player);
	Bool canDisguise() const { return getStealthUpdateModuleData()->m_teamDisguised; }
	Real getRevealDistanceFromTarget() const { return getStealthUpdateModuleData()->m_revealDistanceFromTarget; }
	Bool allowedToStealth() const;
	void hintDetectableWhileUnstealthed( void ) ;

	void changeVisualDisguise();

	UpdateSleepTime calcSleepTime() const;

private:
	UnsignedInt						m_stealthAllowedFrame;
	UnsignedInt						m_detectionExpiresFrame;
	Bool									m_enabled;
	
	Real                  m_pulsePhaseRate;
	Real                  m_pulsePhase;

	//Disguise only members
	Int										m_disguiseAsPlayerIndex;		//The player team we are wanting to disguise as (might not actually be disguised yet).
	const ThingTemplate  *m_disguiseAsTemplate;				//The disguise template (might not actually be using it yet)
	UnsignedInt						m_disguiseTransitionFrames;	//How many frames are left before transition is complete.
	Bool									m_disguiseHalfpointReached;	//In the middle of the transition, we will switch drawables!
	Bool									m_transitioningToDisguise;	//Set when we are disguising -- clear when we're transitioning out of.
	Bool									m_disguised;								//We're disguised as far as other players are concerned.

	// runtime xfer members (does not need saving)
	Bool									m_xferRestoreDisguise;			//Tells us we need to restore our disguise

};


#endif 

