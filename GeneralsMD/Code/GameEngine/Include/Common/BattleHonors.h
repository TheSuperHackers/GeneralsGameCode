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

// FILE: BattleHonors.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Sep 2002
//
//	Filename: 	BattleHonors.h
//
//	author:		Chris Huybregts
//
//	purpose:
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
enum
{
  BATTLE_HONOR_LADDER_CHAMP				= 0x00000001,
	BATTLE_HONOR_STREAK							= 0x00000002,
//	BATTLE_HONOR_STREAK_5					= 0x00000004, // NOT TO BE REUSED
//	BATTLE_HONOR_STREAK_10				= 0x00000008, // NOT TO BE REUSED
//	BATTLE_HONOR_STREAK_25				= 0x00000010, // NOT TO BE REUSED
	BATTLE_HONOR_LOYALTY_USA				= 0x00000020,
	BATTLE_HONOR_LOYALTY_CHINA			= 0x00000040,
	BATTLE_HONOR_BATTLE_TANK				= 0x00000080,
	BATTLE_HONOR_AIR_WING						= 0x00000100,
	BATTLE_HONOR_LOYALTY_GLA				= 0x00000200, // was 0x60 - not actually gonna work that way, eh?
	BATTLE_HONOR_ENDURANCE					= 0x00000400,
	BATTLE_HONOR_CAMPAIGN_USA				= 0x00000800, // not set in battle honors field in persistent storage
	BATTLE_HONOR_CAMPAIGN_CHINA			= 0x00001000, // not set in battle honors field in persistent storage
	BATTLE_HONOR_CAMPAIGN_GLA  			= 0x00002000, // not set in battle honors field in persistent storage
	BATTLE_HONOR_BLITZ5							= 0x00004000,
	BATTLE_HONOR_BLITZ10						= 0x00008000,
	BATTLE_HONOR_FAIR_PLAY					= 0x00010000,
	BATTLE_HONOR_APOCALYPSE					= 0x00020000,
	BATTLE_HONOR_OFFICERSCLUB				= 0x00040000,
	BATTLE_HONOR_DOMINATION					= 0x00080000,
	BATTLE_HONOR_CHALLENGE_MODE			= 0x00100000,
	BATTLE_HONOR_ULTIMATE						= 0x00200000,
	BATTLE_HONOR_GLOBAL_GENERAL			= 0x00400000,
	BATTLE_HONOR_DOMINATION_ONLINE	= 0x00800000,
	BATTLE_HONOR_STREAK_ONLINE			= 0x01000000,
	BATTLE_HONOR_CHALLENGE					= 0x02000000, // not set in battle honors field in persistent storage
//BATTLE_HONOR_FREE								= 0x04000000,
	BATTLE_HONOR_NOT_GAINED					= 0x08000000  // This is set in tooltip item data to indicate that the honor isn't actually gained.
};

enum
{
	BH_CHALLENGE_MASK_1 = 0x0001,
	BH_CHALLENGE_MASK_2 = 0x0002,
	BH_CHALLENGE_MASK_3 = 0x0004,
	BH_CHALLENGE_MASK_4 = 0x0008,
	BH_CHALLENGE_MASK_5 = 0x0010,
	BH_CHALLENGE_MASK_6 = 0x0020,
	BH_CHALLENGE_MASK_7 = 0x0040,
};

enum
{
	MAX_BATTLE_HONOR_COLUMNS = 4,
	MAX_BATTLE_HONOR_IMAGE_WIDTH = 40,
	MAX_BATTLE_HONOR_IMAGE_HEIGHT = 41,
};

//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
