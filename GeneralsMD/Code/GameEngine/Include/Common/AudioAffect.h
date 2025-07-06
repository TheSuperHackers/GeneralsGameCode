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

// EA Pacific
// John McDonald, Jr
// Do not distribute
#pragma once

#ifndef _AUDIOAFFECT_H_
#define _AUDIOAFFECT_H_

#include <Lib/BaseType.h>

// if it is set by the options panel, use the system setting parameter. Otherwise, this will be 
// appended to whatever the current system volume is.
enum AudioAffect CPP_11(: Int)
{
	AudioAffect_Music		= 0x01,
	AudioAffect_Sound		= 0x02,
	AudioAffect_Sound3D	= 0x04,
	AudioAffect_Speech	= 0x08,
	AudioAffect_All			= (AudioAffect_Music | AudioAffect_Sound | AudioAffect_Sound3D | AudioAffect_Speech),

	AudioAffect_SystemSetting = 0x10,
};

#endif // _AUDIOAFFECT_H_
