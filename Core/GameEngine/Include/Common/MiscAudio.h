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

///MiscAudio.h/////////////////////////////////////////////////////////////////////////////////////
// This file is for miscellaneous sound hooks that don't have another happy home.

#pragma once

#include "Common/AudioEventRTS.h"

struct MiscAudio
{
	static const FieldParse m_fieldParseTable[];

	AudioEventRTS m_radarUnitUnderAttackSound;						///< radar sounds to play when unit under attack
	AudioEventRTS m_radarHarvesterUnderAttackSound;				///< radar sounds to play when harvester under attack
	AudioEventRTS m_radarStructureUnderAttackSound;				///< radar sounds to play when structure under attack
	AudioEventRTS m_radarUnderAttackSound;								///< radar sounds to play when ? under attack
	AudioEventRTS m_radarInfiltrationSound;								///< radar sounds to play when something is infiltrated
	AudioEventRTS m_radarOnlineSound;											///< radar sounds to play when radar goes online
	AudioEventRTS m_radarOfflineSound;										///< radar sounds to play when radar goes offline
	AudioEventRTS m_defectorTimerTickSound;								///< snd to play during transient invulnerability while defecting // lorenzen
	AudioEventRTS m_defectorTimerDingSound;								///< snd to play when you become vulnerable again // lorenzen
	AudioEventRTS m_lockonTickSound;											///< snd to play during stealth-fighter-lockon period
	AudioEventRTS m_allCheerSound;												///< snd to play when user presses 'cheer' key
	AudioEventRTS m_battleCrySound;												///< snd to play when user presses 'battlecry' key
	AudioEventRTS m_guiClickSound;												///< snd to play when user presses button in GUI
	AudioEventRTS m_noCanDoSound;													///< Global "No Can Do" sound
	AudioEventRTS	m_stealthDiscoveredSound;								///< I have just discovered an enemy stealth unit
	AudioEventRTS	m_stealthNeutralizedSound;							///< One of my stealthed units has just been discovered by the enemy
	AudioEventRTS m_moneyDepositSound;										///< Money was deposited in my bank
	AudioEventRTS m_moneyWithdrawSound;										///< Money was withdrawn from my bank
	AudioEventRTS m_buildingDisabled;											///< Building has lost power, been hit with an EMP, or disable hacked.
	AudioEventRTS m_buildingReenabled;										///< Building has recovered from being disabled.
	AudioEventRTS m_vehicleDisabled;											///< Vehicle has been disabled via EMP or hacker attack.
	AudioEventRTS m_vehicleReenabled;											///< Vehicle has recovered from being disabled.
	AudioEventRTS m_splatterVehiclePilotsBrain;						///< Pilot has been sniped by Jarmen Kell.
	AudioEventRTS m_terroristInCarMoveVoice;							///< Terrorist issues a move order while in a car.
	AudioEventRTS m_terroristInCarAttackVoice;						///< Terrorist issues attack order while in a car.
	AudioEventRTS m_terroristInCarSelectVoice;						///< Terrorist is selected while in a car.
	AudioEventRTS m_crateHeal;														///< When heal crate is picked up.
	AudioEventRTS m_crateShroud;													///< When shroud crate is picked up.
	AudioEventRTS m_crateSalvage;													///< When salvage crate is picked up.
	AudioEventRTS m_crateFreeUnit;												///< When free unit crate is picked up.
	AudioEventRTS m_crateMoney;														///< When money crate is picked up.
	AudioEventRTS m_unitPromoted;													///< Unit is promoted.
	AudioEventRTS m_repairSparks;													///< Battle drone repairs unit.
	AudioEventRTS m_sabotageShutDownBuilding;											///< When Saboteur hits a building
	AudioEventRTS m_sabotageResetTimerBuilding;											///< When Saboteur hits a building
	AudioEventRTS m_aircraftWheelScreech;									///< When a jet lands on a runway.
};
