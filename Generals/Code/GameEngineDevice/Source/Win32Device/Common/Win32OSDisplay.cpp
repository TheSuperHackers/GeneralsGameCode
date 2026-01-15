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

// Win32OSDisplay.cpp //////////////////////////////////////
// John McDonald, December 2002
////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Common/OSDisplay.h"

#include "Common/SubsystemInterface.h"
#include "Common/STLTypedefs.h"
#include "Common/AsciiString.h"
#include "Common/SystemInfo.h"
#include "Common/UnicodeString.h"
#include "GameClient/GameText.h"



extern HWND ApplicationHWnd;

//-------------------------------------------------------------------------------------------------
static void RTSFlagsToOSFlags(UnsignedInt buttonFlags, UnsignedInt otherFlags, UnsignedInt& outWindowsFlags)
{
	outWindowsFlags = 0;

	if (BitIsSet(buttonFlags, OSDBT_OK)) {
		outWindowsFlags |= MB_OK;
	}

	if (BitIsSet(buttonFlags, OSDBT_CANCEL)) {
		outWindowsFlags |= MB_OKCANCEL;
	}

	//-----------------------------------------------------------------------------------------------
	if (BitIsSet(otherFlags, OSDOF_SYSTEMMODAL)) {
		outWindowsFlags |= MB_SYSTEMMODAL;
	}

	if (BitIsSet(otherFlags, OSDOF_APPLICATIONMODAL)) {
		outWindowsFlags |= MB_APPLMODAL;
	}

	if (BitIsSet(otherFlags, OSDOF_TASKMODAL)) {
		outWindowsFlags |= MB_TASKMODAL;
	}

	if (BitIsSet(otherFlags, OSDOF_EXCLAMATIONICON)) {
		outWindowsFlags |= MB_ICONEXCLAMATION;
	}

	if (BitIsSet(otherFlags, OSDOF_INFORMATIONICON)) {
		outWindowsFlags |= MB_ICONINFORMATION;
	}

	if (BitIsSet(otherFlags, OSDOF_ERRORICON)) {
		outWindowsFlags |= MB_ICONERROR;
	}

	if (BitIsSet(otherFlags, OSDOF_STOPICON)) {
		outWindowsFlags |= MB_ICONSTOP;
	}

}

//-------------------------------------------------------------------------------------------------
OSDisplayButtonType OSDisplayWarningBox(AsciiString p, AsciiString m, UnsignedInt buttonFlags, UnsignedInt otherFlags)
{
	if (!TheGameText) {
		return OSDBT_ERROR;
	}

	UnicodeString promptStr = TheGameText->fetch(p);
	UnicodeString mesgStr = TheGameText->fetch(m);

	UnsignedInt windowsOptionsFlags = 0;
	RTSFlagsToOSFlags(buttonFlags, otherFlags, windowsOptionsFlags);

	// @todo Make this return more than just ok/cancel - jkmcd
	// (we need a function to translate back the other way.)
	Int returnResult = 0;
	if (TheSystemIsUnicode)
	{
		returnResult = ::MessageBoxW(nullptr, mesgStr.str(), promptStr.str(), windowsOptionsFlags);
	}
	else
	{
		// However, if we're using the default version of the message box, we need to
		// translate the string into an AsciiString
		AsciiString promptA, mesgA;
		promptA.translate(promptStr);
		mesgA.translate(mesgStr);
		//Make sure main window is not TOP_MOST
		::SetWindowPos(ApplicationHWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		returnResult = ::MessageBoxA(nullptr, mesgA.str(), promptA.str(), windowsOptionsFlags);
	}

	if (returnResult == IDOK) {
		return OSDBT_OK;
	}

	return OSDBT_CANCEL;
}

//-------------------------------------------------------------------------------------------------
void OSDisplaySetBusyState(Bool busyDisplay, Bool busySystem)
{
	EXECUTION_STATE state = ES_CONTINUOUS;
	state |= busyDisplay ? ES_DISPLAY_REQUIRED : 0;
	state |= busySystem ? ES_SYSTEM_REQUIRED : 0;

	::SetThreadExecutionState(state);
}
