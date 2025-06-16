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

// Overridable.h //////////////////////////////////////////////////////////////////////////////////
// Electronic Arts Pacific
// Do Not Distribute

#pragma once

#ifndef __OSDISPLAY_H__
#define __OSDISPLAY_H__

#include "Lib/BaseType.h"

class AsciiString;

enum OSDisplayButtonType CPP_11(: UnsignedInt)
{
	OSDBT_OK										= 0x00000001,
	OSDBT_CANCEL								= 0x00000002,
															
															
	OSDBT_ERROR									= 0x80000000
};

enum OSDisplayOtherFlags CPP_11(: UnsignedInt)
{
	OSDOF_SYSTEMMODAL						= 0x00000001,
	OSDOF_APPLICATIONMODAL			= 0x00000002,
	OSDOF_TASKMODAL							= 0x00000004,
	
	OSDOF_EXCLAMATIONICON				= 0x00000008,
	OSDOF_INFORMATIONICON				= 0x00000010,
	OSDOF_ERRORICON							= 0x00000011,
	OSDOF_STOPICON							= 0x00000012,

	ODDOF_ERROR									= 0x80000000
};

// Display a warning box to the user with the specified localized prompt, message, and
// buttons. (Feel free to add buttons as appropriate to the enum above). 
// This function will return the button pressed to close the dialog.
OSDisplayButtonType OSDisplayWarningBox(AsciiString p, AsciiString m, UnsignedInt buttonFlags, UnsignedInt otherFlags);

#endif /* __OSDISPLAY_H__ */