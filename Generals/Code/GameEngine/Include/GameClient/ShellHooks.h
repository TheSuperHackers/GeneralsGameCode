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


// ShellHooks.h
// Author: Matthew D. Campbell, October 2002

#pragma once

#ifndef SHELLHOOKS_H
#define SHELLHOOKS_H

//
// This enumeration holds all the shell script hooks that we currently have, If you are going to
// add more, it's important to keep the enum lined up with the names in TheShellHookNames located
// in Scripts.cpp.
//
enum
{
	SHELL_SCRIPT_HOOK_MAIN_MENU_CAMPAIGN_SELECTED = 0,
	SHELL_SCRIPT_HOOK_MAIN_MENU_CAMPAIGN_HIGHLIGHTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_CAMPAIGN_UNHIGHLIGHTED,

	SHELL_SCRIPT_HOOK_MAIN_MENU_SKIRMISH_SELECTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_SKIRMISH_HIGHLIGHTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_SKIRMISH_UNHIGHLIGHTED,

	SHELL_SCRIPT_HOOK_MAIN_MENU_OPTIONS_SELECTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_OPTIONS_HIGHLIGHTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_OPTIONS_UNHIGHLIGHTED,

	SHELL_SCRIPT_HOOK_MAIN_MENU_ONLINE_SELECTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_ONLINE_HIGHLIGHTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_ONLINE_UNHIGHLIGHTED,

	SHELL_SCRIPT_HOOK_MAIN_MENU_NETWORK_SELECTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_NETWORK_HIGHLIGHTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_NETWORK_UNHIGHLIGHTED,

	SHELL_SCRIPT_HOOK_MAIN_MENU_EXIT_SELECTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_EXIT_HIGHLIGHTED,
	SHELL_SCRIPT_HOOK_MAIN_MENU_EXIT_UNHIGHLIGHTED,

	SHELL_SCRIPT_HOOK_GENERALS_ONLINE_LOGIN,
	SHELL_SCRIPT_HOOK_GENERALS_ONLINE_LOGOUT,
	SHELL_SCRIPT_HOOK_GENERALS_ONLINE_ENTERED_FROM_GAME,

	SHELL_SCRIPT_HOOK_OPTIONS_OPENED,
	SHELL_SCRIPT_HOOK_OPTIONS_CLOSED,

	SHELL_SCRIPT_HOOK_SKIRMISH_OPENED,
	SHELL_SCRIPT_HOOK_SKIRMISH_CLOSED,
	SHELL_SCRIPT_HOOK_SKIRMISH_ENTERED_FROM_GAME,

	SHELL_SCRIPT_HOOK_LAN_OPENED,
	SHELL_SCRIPT_HOOK_LAN_CLOSED,
	SHELL_SCRIPT_HOOK_LAN_ENTERED_FROM_GAME,

	SHELL_SCRIPT_HOOK_TOTAL
};

extern const char *const TheShellHookNames[];				///< Contains a list of the text representation of the shell hooks Used in WorldBuilder and in the shell.
void SignalUIInteraction(Int interaction);

#endif // SHELLHOOKS_H

