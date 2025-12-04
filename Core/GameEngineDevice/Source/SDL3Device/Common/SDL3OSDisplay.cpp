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

// SDL3OSDisplay.cpp
// TheSuperHackers @feature denysmitin 03/12/2025 SDL3 implementation of OS display functions.

#include "Common/OSDisplay.h"

#include "Common/AsciiString.h"
#include "Common/STLTypedefs.h"
#include "Common/SubsystemInterface.h"
#include "Common/SystemInfo.h"
#include "Common/UnicodeString.h"
#include "GameClient/GameText.h"

#include <SDL3/SDL.h>

//-------------------------------------------------------------------------------------------------
static SDL_MessageBoxFlags convertButtonFlags(UnsignedInt buttonFlags, UnsignedInt otherFlags)
{
	SDL_MessageBoxFlags flags = SDL_MESSAGEBOX_INFORMATION;

	if (BitIsSet(otherFlags, OSDOF_EXCLAMATIONICON))
	{
		flags = SDL_MESSAGEBOX_WARNING;
	}
	else if (BitIsSet(otherFlags, OSDOF_ERRORICON) || BitIsSet(otherFlags, OSDOF_STOPICON))
	{
		flags = SDL_MESSAGEBOX_ERROR;
	}

	return flags;
}

//-------------------------------------------------------------------------------------------------
OSDisplayButtonType OSDisplayWarningBox(AsciiString p, AsciiString m, UnsignedInt buttonFlags, UnsignedInt otherFlags)
{
	if (!TheGameText)
	{
		return OSDBT_ERROR;
	}

	UnicodeString promptStr = TheGameText->fetch(p);
	UnicodeString mesgStr = TheGameText->fetch(m);

	// SDL expects UTF-8 encoded strings
	AsciiString promptA, mesgA;
	promptA.translate(promptStr);
	mesgA.translate(mesgStr);

	SDL_MessageBoxFlags sdlFlags = convertButtonFlags(buttonFlags, otherFlags);

	// Determine which buttons to show
	SDL_MessageBoxButtonData buttons[2] = {};
	int buttonCount = 0;

	if (BitIsSet(buttonFlags, OSDBT_OK))
	{
		buttons[buttonCount].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
		buttons[buttonCount].buttonID = OSDBT_OK;
		buttons[buttonCount].text = "OK";
		buttonCount++;
	}

	if (BitIsSet(buttonFlags, OSDBT_CANCEL))
	{
		buttons[buttonCount].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
		buttons[buttonCount].buttonID = OSDBT_CANCEL;
		buttons[buttonCount].text = "Cancel";
		buttonCount++;
	}

	// If no buttons specified, default to OK
	if (buttonCount == 0)
	{
		buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
		buttons[0].buttonID = OSDBT_OK;
		buttons[0].text = "OK";
		buttonCount = 1;
	}

	SDL_MessageBoxData messageboxdata = {};
	messageboxdata.flags = sdlFlags;
	messageboxdata.window = nullptr; // No parent window
	messageboxdata.title = promptA.str();
	messageboxdata.message = mesgA.str();
	messageboxdata.numbuttons = buttonCount;
	messageboxdata.buttons = buttons;

	int buttonid = 0;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonid) == 0)
	{
		return static_cast<OSDisplayButtonType>(buttonid);
	}

	return OSDBT_ERROR;
}

//-------------------------------------------------------------------------------------------------
void OSDisplaySetBusyState(Bool busyDisplay, Bool busySystem)
{
	// TheSuperHackers @feature denysmitin 03/12/2025 Prevent screen saver and display sleep during gameplay.
	// SDL3 doesn't have direct API for system sleep control, only screen saver disable.
	if (busyDisplay)
	{
		SDL_DisableScreenSaver();
	}
	else
	{
		SDL_EnableScreenSaver();
	}

	// Note: SDL3 doesn't provide system sleep prevention (ES_SYSTEM_REQUIRED equivalent).
	// On platforms where this is critical, platform-specific code would be needed.
	(void)busySystem;
}
