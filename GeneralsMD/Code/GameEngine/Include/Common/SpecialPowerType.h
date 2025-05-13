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

// SpecialPowerType.h /////////////////////////////////////////////////////////////////////////////
// Part of header detangling
// JKMCD Aug 2002

#pragma once
#ifndef __SPECIALPOWERTYPE_H__
#define __SPECIALPOWERTYPE_H__

// ------------------------------------------------------------------------------------------------
// don't forget to add new strings to SpecialPowerMaskType::s_bitNameList[]
// ------------------------------------------------------------------------------------------------
//
// Note: these values are saved in save files, so you MUST NOT REMOVE OR CHANGE
// existing values!
//
enum SpecialPowerType CPP_11(: Int)
{
	SPECIAL_INVALID,
	// don't forget to add new strings to SpecialPowerMaskType::s_bitNameList[]

	//Superweapons
	SPECIAL_DAISY_CUTTER,
	SPECIAL_PARADROP_AMERICA,
	SPECIAL_CARPET_BOMB,
	SPECIAL_CLUSTER_MINES,
	SPECIAL_EMP_PULSE,
	SPECIAL_NAPALM_STRIKE,
	SPECIAL_CASH_HACK,
	SPECIAL_NEUTRON_MISSILE,
	SPECIAL_SPY_SATELLITE,
	SPECIAL_DEFECTOR,
	SPECIAL_TERROR_CELL,
	SPECIAL_AMBUSH,
	SPECIAL_BLACK_MARKET_NUKE,
	SPECIAL_ANTHRAX_BOMB,
	SPECIAL_SCUD_STORM,
#ifdef ALLOW_DEMORALIZE
	SPECIAL_DEMORALIZE,
#else
	SPECIAL_DEMORALIZE_OBSOLETE,
#endif
	SPECIAL_CRATE_DROP,
	SPECIAL_A10_THUNDERBOLT_STRIKE,
	SPECIAL_DETONATE_DIRTY_NUKE,
	SPECIAL_ARTILLERY_BARRAGE,
	// don't forget to add new strings to SpecialPowerMaskType::s_bitNameList[]

	//Special abilities
	SPECIAL_MISSILE_DEFENDER_LASER_GUIDED_MISSILES,
	SPECIAL_REMOTE_CHARGES,
	SPECIAL_TIMED_CHARGES, 
	SPECIAL_HELIX_NAPALM_BOMB,
	SPECIAL_HACKER_DISABLE_BUILDING,
	SPECIAL_TANKHUNTER_TNT_ATTACK,
	SPECIAL_BLACKLOTUS_CAPTURE_BUILDING,
	SPECIAL_BLACKLOTUS_DISABLE_VEHICLE_HACK,
	SPECIAL_BLACKLOTUS_STEAL_CASH_HACK,
	SPECIAL_INFANTRY_CAPTURE_BUILDING,
	SPECIAL_RADAR_VAN_SCAN,
	SPECIAL_SPY_DRONE,
	SPECIAL_DISGUISE_AS_VEHICLE,
	SPECIAL_BOOBY_TRAP,
	// don't forget to add new strings to SpecialPowerMaskType::s_bitNameList[]
	SPECIAL_REPAIR_VEHICLES,
	SPECIAL_PARTICLE_UPLINK_CANNON,
	SPECIAL_CASH_BOUNTY,
	SPECIAL_CHANGE_BATTLE_PLANS,
	SPECIAL_CIA_INTELLIGENCE,
	SPECIAL_CLEANUP_AREA,
	// don't forget to add new strings to SpecialPowerMaskType::s_bitNameList[]
	SPECIAL_LAUNCH_BAIKONUR_ROCKET,

  SPECIAL_SPECTRE_GUNSHIP,
  SPECIAL_GPS_SCRAMBLER,
	
	SPECIAL_FRENZY,
	SPECIAL_SNEAK_ATTACK,

	//Ack, this is ass. These enums fix a bug where new enums were missing for 
	//shortcut powers... but the real clincher was that if you were say USA and
	//captured a Tank China command center, your US paradrop would be assigned
	//to the china tank drop and when you tried to fire it from the shortcut
	//it could pick the china one and not fire it because it didn't have
	//complete connection... ugh!!!
	SPECIAL_CHINA_CARPET_BOMB,
	EARLY_SPECIAL_CHINA_CARPET_BOMB,
	SPECIAL_LEAFLET_DROP,
	EARLY_SPECIAL_LEAFLET_DROP,
	EARLY_SPECIAL_FRENZY,
	SPECIAL_COMMUNICATIONS_DOWNLOAD,
	EARLY_SPECIAL_REPAIR_VEHICLES,
	SPECIAL_TANK_PARADROP,
	SUPW_SPECIAL_PARTICLE_UPLINK_CANNON,
	AIRF_SPECIAL_DAISY_CUTTER,
	NUKE_SPECIAL_CLUSTER_MINES,
	NUKE_SPECIAL_NEUTRON_MISSILE,
	AIRF_SPECIAL_A10_THUNDERBOLT_STRIKE,
	AIRF_SPECIAL_SPECTRE_GUNSHIP,
	INFA_SPECIAL_PARADROP_AMERICA,
	SLTH_SPECIAL_GPS_SCRAMBLER,
	AIRF_SPECIAL_CARPET_BOMB,
	SUPR_SPECIAL_CRUISE_MISSILE,
	LAZR_SPECIAL_PARTICLE_UPLINK_CANNON,
	SUPW_SPECIAL_NEUTRON_MISSILE,

	SPECIAL_BATTLESHIP_BOMBARDMENT,
		
	SPECIALPOWER_COUNT,
	// don't forget to add new strings to SpecialPowerMaskType::s_bitNameList[]
};

	// Definition of these names is located in SpecialPower.cpp

#endif /* __SPECIALPOWERTYPE_H__ */
