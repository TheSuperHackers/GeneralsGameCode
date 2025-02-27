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

// FILE: WellKnownKeys.h ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  WellKnownKeys.h
//
// Created:    Steven Johnson, November 2001
//
// Desc:       Central repository for "well-known" Dict key definitions and descriptions.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/NameKeyGenerator.h"

#ifdef INSTANTIATE_WELL_KNOWN_KEYS
	#define DEFINE_KEY(NAME) 	extern const StaticNameKey TheKey_##NAME; const StaticNameKey TheKey_##NAME(#NAME);
#else
	#define DEFINE_KEY(NAME) 	extern const StaticNameKey TheKey_##NAME;
#endif

// ---------------------------------------------------------------------------------------
// well-known keys in Team dicts.
// ---------------------------------------------------------------------------------------

/**
	Which: Team
	Type: AsciiString
	Usage: name of team.
*/
DEFINE_KEY(teamName)

/**
	Which: Team
	Type: AsciiString
	Usage: name of the player-or-team that owns this team.
*/
DEFINE_KEY(teamOwner)

/**
	Which: Team
	Type: Bool
	Usage: if true, only one instance of this team can be made.
*/
DEFINE_KEY(teamIsSingleton)

/**
	Which: Team
	Type: AsciiString
	Usage: Name of waypoint where team spawns.
*/
DEFINE_KEY(teamHome)

/**
	Which: Team
	Type: AsciiString
	Usage: Thing name of first batch of units.
*/
DEFINE_KEY(teamUnitType1)

/**
	Which: Team
	Type: Int
	Usage: Minimum number of units of type 1 to build.
*/
DEFINE_KEY(teamUnitMinCount1)

/**
	Which: Team
	Type: Int
	Usage: Maximum number of units of type 1 to build.
*/
DEFINE_KEY(teamUnitMaxCount1)

/**
	Which: Team
	Type: AsciiString
	Usage: Thing name of second batch of units.
*/
DEFINE_KEY(teamUnitType2)

/**
	Which: Team
	Type: Int
	Usage: Minimum number of units of type 2 to build.
*/
DEFINE_KEY(teamUnitMinCount2)

/**
	Which: Team
	Type: Int
	Usage: Maximum number of units of type 2 to build.
*/
DEFINE_KEY(teamUnitMaxCount2)

/**
	Which: Team
	Type: AsciiString
	Usage: Thing name of third batch of units.
*/
DEFINE_KEY(teamUnitType3)

/**
	Which: Team
	Type: Int
	Usage: Minimum number of units of type 3 to build.
*/
DEFINE_KEY(teamUnitMinCount3)

/**
	Which: Team
	Type: Int
	Usage: Maximum number of units of type 3 to build.
*/
DEFINE_KEY(teamUnitMaxCount3)


/**
	Which: Team
	Type: AsciiString
	Usage: Thing name of fourth batch of units.
*/
DEFINE_KEY(teamUnitType4)

/**
	Which: Team
	Type: Int
	Usage: Minimum number of units of type 4 to build.
*/
DEFINE_KEY(teamUnitMinCount4)

/**
	Which: Team
	Type: Int
	Usage: Maximum number of units of type 4 to build.
*/
DEFINE_KEY(teamUnitMaxCount4)

/**
	Which: Team
	Type: AsciiString
	Usage: Thing name of fifth batch of units.
*/
DEFINE_KEY(teamUnitType5)

/**
	Which: Team
	Type: Int
	Usage: Minimum number of units of type 5 to build.
*/
DEFINE_KEY(teamUnitMinCount5)

/**
	Which: Team
	Type: Int
	Usage: Maximum number of units of type 5 to build.
*/
DEFINE_KEY(teamUnitMaxCount5)

/**
	Which: Team
	Type: AsciiString
	Usage: Thing name of sixth batch of units.
*/
DEFINE_KEY(teamUnitType6)

/**
	Which: Team
	Type: Int
	Usage: Minimum number of units of type 6 to build.
*/
DEFINE_KEY(teamUnitMinCount6)

/**
	Which: Team
	Type: Int
	Usage: Maximum number of units of type 6 to build.
*/
DEFINE_KEY(teamUnitMaxCount6)

/**
	Which: Team
	Type: AsciiString
	Usage: Thing name of seventh batch of units.
*/
DEFINE_KEY(teamUnitType7)

/**
	Which: Team
	Type: Int
	Usage: Minimum number of units of type 7 to build.
*/
DEFINE_KEY(teamUnitMinCount7)

/**
	Which: Team
	Type: Int
	Usage: Maximum number of units of type 7 to build.
*/
DEFINE_KEY(teamUnitMaxCount7)

/**
	Which: Team
	Type: AsciiString
	Usage: Name of script to execute when team is created for AI.
*/
DEFINE_KEY(teamOnCreateScript)

/**
	Which: Team
	Type: AsciiString
	Usage: Name of script to execute when AI team is in idle state.
*/
DEFINE_KEY(teamOnIdleScript)

/**
	Which: Team
	Type: Int
	Usage: Number of frames to wait after achieving the minimum number of units
		to try to recruit up to the maximum number of units.
*/
DEFINE_KEY(teamInitialIdleFrames)

/**
	Which: Team
	Type: AsciiString
	Usage: Name of a script to run each time a member on this team is destroyed.
*/
DEFINE_KEY(teamOnUnitDestroyedScript)

/**
	Which: Team
	Type: AsciiString
	Usage: Name of script to execute when team is destroyed.
*/
DEFINE_KEY(teamOnDestroyedScript)

/**
	Which: Team
	Type: Real
	Usage: Percent destroyed to trigger OnDamagedScript.
		1.0 = 100% = trigger when team is all destroyed.
		0 = 0% = trigger when first team member is destroyed.
*/
DEFINE_KEY(teamDestroyedThreshold)

/**
	Which: Team
	Type: Real
	Usage: Name of script to execute when enemy is sighted.
*/
DEFINE_KEY(teamEnemySightedScript)

/**
	Which: Team
	Type: Real
	Usage: Name of script to execute when all clear.
*/
DEFINE_KEY(teamAllClearScript)

/**
	Which: Team
	Type: Bool
	Usage: True if team auto reinforces.
*/
DEFINE_KEY(teamAutoReinforce)

/**
	Which: Team
	Type: Bool
	Usage: True if team can be recruited from by other ai teams.
*/
DEFINE_KEY(teamIsAIRecruitable)

/**
	Which: Team
	Type: Bool
	Usage: True if team can be recruited from by other ai teams.
*/
DEFINE_KEY(teamIsBaseDefense)

/**
	Which: Team
	Type: Bool
	Usage: True if team can be recruited from by other ai teams.
*/
DEFINE_KEY(teamIsPerimeterDefense)

/**
	Which: Team
	Type: Int
	Usage: The aggressiveness of the team: -2 Sleep, -1 Passive, 0 Normal, 1 Alert, 2 Aggressive
*/
DEFINE_KEY(teamAggressiveness)

/**
	Which: Team
	Type: Bool
	Usage: True if transports return to base.
*/
DEFINE_KEY(teamTransportsReturn)

/**
	Which: Team
	Type: Bool
	Usage: True if the team avoids threats.
*/
DEFINE_KEY(teamAvoidThreats)

/**
	Which: Team
	Type: Bool
	Usage: True if the team attacks the same target when auto-acquiring targets.
*/
DEFINE_KEY(teamAttackCommonTarget)

/**
	Which: Team
	Type: Int
	Usage: If a team is not singleton, max number of this team at one time.
*/
DEFINE_KEY(teamMaxInstances)

/**
	Which: Team
	Type: AsciiString
	Usage: Description comment field.
*/
DEFINE_KEY(teamDescription)

/**
	Which: Team
	Type: AsciiString
	Usage: Script that contains the conditions for team production.
*/
DEFINE_KEY(teamProductionCondition)

/**
	Which: Team
	Type: Int
	Usage: Team's production priority.
*/
DEFINE_KEY(teamProductionPriority)

/**
	Which: Team
	Type: Int
	Usage: Team's production priority increase on success amount.
*/
DEFINE_KEY(teamProductionPrioritySuccessIncrease)

/**
	Which: Team
	Type: Int
	Usage: Team's production priority decrease on failure amount.
*/
DEFINE_KEY(teamProductionPriorityFailureDecrease)

/**
	Which: Team
	Type: AsciiString
	Usage: Team's transport unit for reinforcements.
*/
DEFINE_KEY(teamTransport)

/**
	Which: Team
	Type: AsciiString
	Usage: Team's origin for reinforcements.
*/
DEFINE_KEY(teamReinforcementOrigin)

/**
	Which: Team
	Type: Bool
	Usage: Team's starts full flag (pack into transports) for reinforcement teams.
*/
DEFINE_KEY(teamStartsFull)

/**
	Which: Team
	Type: Bool
	Usage: Team's transports exit (leave the map) flag for reinforcement teams.
*/
DEFINE_KEY(teamTransportsExit)

/**
	Which: Team
	Type: Int
	Usage: What veterancy does this object have: 0 green, 1 normal, 2 veteran, 3 elite
*/
DEFINE_KEY(teamVeterancy)

/**
	Which: Team
	Type: Bool
	Usage: Does the team execute the actions in
*/
DEFINE_KEY(teamExecutesActionsOnCreate)

/**
	Which: Team
	Type: AsciiString
	Usage: This key is used in the same way that objectGrantUpgrades is used. See it for an explanation.

*/
DEFINE_KEY(teamGenericScriptHook)




// ---------------------------------------------------------------------------------------
// well-known keys in MapObject dicts.
// ---------------------------------------------------------------------------------------

/**
	Which: MapObject Properties
	Type: AsciiString
	Usage: name of player or team that is to have ownership. if missing or null, owned by neutral player's default team.
*/
DEFINE_KEY(originalOwner)

/**
	Which: MapObject Properties
	Type: Real
	Usage: If unit is a light, how high the light is above the terrain.
*/
DEFINE_KEY(lightHeightAboveTerrain)

/**
	Which: MapObject Properties
	Type: Real
	Usage: If unit is a light, Outer bounds of the light.  Reaches 0 intensity.
*/
DEFINE_KEY(lightOuterRadius)

/**
	Which: MapObject Properties
	Type: Real
	Usage: If unit is a light, Inside this radius the light is 100% intensity.
*/
DEFINE_KEY(lightInnerRadius)

/**
	Which: MapObject Properties
	Type: Int (RGB color in 0xAARRGGBB format)
	Usage: If unit is a light, Ambient color.
*/
DEFINE_KEY(lightAmbientColor)

/**
	Which: MapObject Properties
	Type: Int (RGB color in 0xAARRGGBB format)
	Usage: If unit is a light, Diffuse color.
*/
DEFINE_KEY(lightDiffuseColor)

/**
	Which: MapObject Properties
	Type: Ascii
	Usage: If unit is a waypoint, name of waypoint.
*/
DEFINE_KEY(waypointName)

/**
	Which: MapObject Properties
	Type: Int
	Usage: If unit is a waypoint, id of waypoint.
*/
DEFINE_KEY(waypointID)

/**
	Which: MapObject Properties
	Type: Int
	Usage: If unit is a waypoint, label for waypoint path.
*/
DEFINE_KEY(waypointPathLabel1)

/**
	Which: MapObject Properties
	Type: Int
	Usage: If unit is a waypoint, label for waypoint path.
*/
DEFINE_KEY(waypointPathLabel2)

/**
	Which: MapObject Properties
	Type: Int
	Usage: If unit is a waypoint, label for waypoint path.
*/
DEFINE_KEY(waypointPathLabel3)


/**
	Which: MapObject Properties
	Type: Bool
	Usage: If object is a waypoint, bi-directional flag.
*/
DEFINE_KEY(waypointPathBiDirectional)


/**
	Which: MapObject Properties
	Type: Ascii
	Usage: Name of unit.
*/
DEFINE_KEY(objectName)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Flag whether the object should be exported with a script list.
*/
DEFINE_KEY(exportWithScript)

/**
	Which: MapObject Properties
	Type: Int
	Usage: How much initial health this object has as percentage.
*/
DEFINE_KEY(objectInitialHealth)

/**
	Which: MapObject Properties
	Type: Int
	Usage: Override for default 'max' hitpoints for an object.
*/
DEFINE_KEY(objectMaxHPs)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Is the object enabled
*/
DEFINE_KEY(objectEnabled)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Can the object be destroyed
*/
DEFINE_KEY(objectIndestructible)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Can the AI sell this object for some loot
*/
DEFINE_KEY(objectUnsellable)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Can this object be acquired by the player (but not auto-acquired by AI).
*/
DEFINE_KEY(objectTargetable)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Does the object have power? It is thought this will not be used
*/
DEFINE_KEY(objectPowered)

/**
	Which: MapObject Properties
	Type: AsciiString
	Usage: The script attached to the object
*/
DEFINE_KEY(objectScriptAttachment)

/**
	Which: MapObject Properties
	Type: Int
	Usage: The aggressiveness of the object: -2 Sleep, -1 Passive, 0 Normal, 1 Alert, 2 Aggressive
*/
DEFINE_KEY(objectAggressiveness)

/**
	Which: MapObject Properties
	Type: Int
	Usage: How far does this object see? (This is actually how far out it will scan for targets)
*/
DEFINE_KEY(objectVisualRange)

/**
	Which: MapObject Properties
	Type: Int
	Usage: How far does this object clear the shroud/fog out to?
*/
DEFINE_KEY(objectShroudClearingDistance)

/**
	Which: MapObject Properties
	Type: Int
	Usage: Does this object belong to a control group: 0 no, 1-X That group number
*/
DEFINE_KEY(objectGroupNumber)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Can this object be recruited by the AI?
*/
DEFINE_KEY(objectRecruitableAI)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Can this object be selected?
*/
DEFINE_KEY(objectSelectable)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: What veterancy does this object have: 0 green, 1 normal, 2 veteran, 3 elite
*/
DEFINE_KEY(objectVeterancy)

/**
	Which: MapObject Properties
	Type: Int
	Usage: 0 map default, 1 daytime, 2 nighttime
*/
DEFINE_KEY(objectTime)


/**
	Which: MapObject Properties
	Type: Int
	Usage: 0 map default, 1 normal, 2 snow
*/
DEFINE_KEY(objectWeather)

/**
	Which: MapObject Properties
	Type: Real
	Usage: What is the radius of this thing? (for Scorch Marks and the like)
*/
DEFINE_KEY(objectRadius)

/**
	Which: MapObject Properties
	Type: Int
	Usage: What is the type of scorch? (for Scorch Marks)
*/
DEFINE_KEY(scorchType)

/**
	Which: MapObject Properties
	Type: Real
	Usage: What is the stopping distance for this vehicle?  (Anything with a locomotor)
*/
DEFINE_KEY(objectStoppingDistance)

/**
	Which: MapObject Properties
	Type: AsciiString
	Usage: What is the name of the layer that this thing lives on? (Empty means default)
*/
DEFINE_KEY(objectLayer)

/**
	Which: MapObject Properties
	Type: AsciiString
	Usage: This is the basis for many keys that are less well known, namely 0..(N - 1), where N
				 is the number of upgrades that this unit gets on creation. These are done by taking this
				 key name and appending a number on it. The first key that doesn't exist in this sequence
				 signifies the end of the "You get this upgrade list."
*/
DEFINE_KEY(objectGrantUpgrade)

/**
	Which: MapObject Properties
	Type: AsciiString
	Usage: What is the unique ID (in the form of "templatename #") for this map object?
*/
DEFINE_KEY(uniqueID)

/**
	Which: MapObject Properties
	Type: AsciiString
	Usage: What ambient sound does this object have attached to it?
         Missing means "Use the default sound for object type from INI"
         Blank means "No ambient sound"
*/
DEFINE_KEY(objectSoundAmbient)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Does the ambient sound have customized flags & properties? Blank or false - use INI parameters for sound
*/
DEFINE_KEY(objectSoundAmbientCustomized)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Does the ambient sound start off playing?
         Blank -- use default of true for looping sounds, false for non-looping sounds
*/
DEFINE_KEY(objectSoundAmbientEnabled)

/**
	Which: MapObject Properties
	Type: Bool
	Usage: Does the ambient sound loop? Blank -- use default for sound
*/
DEFINE_KEY(objectSoundAmbientLooping)

/**
	Which: MapObject Properties
	Type: Int
	Usage: How many times does the sound loop (0 = forever)? Blank - use default for sound
*/
DEFINE_KEY(objectSoundAmbientLoopCount)

/**
	Which: MapObject Properties
	Type: Real
	Usage: Minimum volume of sound. Blank - use default for sound
*/
DEFINE_KEY(objectSoundAmbientMinVolume)

/**
	Which: MapObject Properties
	Type: Real
	Usage: Base volume of sound. Blank - use default for sound
*/
DEFINE_KEY(objectSoundAmbientVolume)

/**
	Which: MapObject Properties
	Type: Real
	Usage: Minimum range of sound. Within this area, sound plays at full volume. Blank - use default for sound
*/
DEFINE_KEY(objectSoundAmbientMinRange)

/**
	Which: MapObject Properties
	Type: Real
	Usage: Maximum range of sound. Sound drops to zero at this range Blank - use default for sound
*/
DEFINE_KEY(objectSoundAmbientMaxRange)

/**
	Which: MapObject Properties
	Type: Int
	Usage: Priority of sound using the enum AudioPriority Blank - use default for sound
*/
DEFINE_KEY(objectSoundAmbientPriority)



// ---------------------------------------------------------------------------------------
// well-known keys in Player dicts.
// ---------------------------------------------------------------------------------------

/**
	Which: Player Properties
	Type: ascii
	Usage: internal identifier for player.
				NOTE: an empty string for playerName is reserved to denote the Neutral player.
*/
DEFINE_KEY(playerName)

/**
	Which: Player Properties
	Type: Bool
	Usage: true if this player is to be human-controlled. false if computer-controlled.
*/
DEFINE_KEY(playerIsHuman)

/**
	Which: Player Properties
	Type: Bool
	Usage: true if this player is in skirmish or multiplayer, rather than solo.
*/
DEFINE_KEY(playerIsSkirmish)

/**
	Which: Player Properties
	Type: unicode
	Usage: displayable name for player.
*/
DEFINE_KEY(playerDisplayName)

/**
	Which: Player Properties
	Type: ascii
	Usage: playertemplate to use to construct the player
*/
DEFINE_KEY(playerFaction)

/**
	Which: Player Properties
	Type: ascii
	Usage: whitespace-separated list of player(s) we start as enemies with
*/
DEFINE_KEY(playerEnemies)

/**
	Which: Player Properties
	Type: ascii
	Usage: whitespace-separated list of player(s) we start as allies with
*/
DEFINE_KEY(playerAllies)

/**
	Which: Player Properties
	Type: Int
	Usage: (optional) if present, amount of money the player starts with
*/
DEFINE_KEY(playerStartMoney)

/**
	Which: Player Properties
	Type: Int (color)
	Usage: (optional) if present, color to use for player (overrides player color)
*/
DEFINE_KEY(playerColor)

/**
	Which: Player Night Properties
	Type: Int (color)
	Usage: (optional) if present, color to use for player at night (overrides player color)
*/
DEFINE_KEY(playerNightColor)

/**
	Which: Player Properties
	Type: Int
	Usage: (optional) if present, Player_*_Start waypoint to use for player (overrides InitialCameraPosition)
*/
DEFINE_KEY(multiplayerStartIndex)

/**
	Which: Player Properties
	Type: Int
	Usage: (optional) if present, Difficulty level to use for player (overrides global difficulty)
*/
DEFINE_KEY(skirmishDifficulty)

/**
	Which: Player Properties
	Type: Bool
	Usage: (optional) if present, signifies if the player is the local player
*/
DEFINE_KEY(multiplayerIsLocal)

/**
	Which: Player Properties
	Type: Bool
	Usage: (optional) if present, signifies if the player has preordered
*/
DEFINE_KEY(playerIsPreorder)

// ---------------------------------------------------------------------------------------
// well-known keys in the World dict.
// ---------------------------------------------------------------------------------------
/**
	Which: World Properties
	Type: Int
	Usage: (optional) if present, describes the weather. If not present, weather is "Normal"
*/
DEFINE_KEY(weather)
DEFINE_KEY(mapName)
DEFINE_KEY(compression)


// ---------------------------------------------------------------------------------------
// well-known Waypoints.
// ---------------------------------------------------------------------------------------
DEFINE_KEY(InitialCameraPosition)
DEFINE_KEY(Player_1_Start)
DEFINE_KEY(Player_2_Start)
DEFINE_KEY(Player_3_Start)
DEFINE_KEY(Player_4_Start)
DEFINE_KEY(Player_5_Start)
DEFINE_KEY(Player_6_Start)
DEFINE_KEY(Player_7_Start)
DEFINE_KEY(Player_8_Start)

// ---------------------------------------------------------------------------------------
