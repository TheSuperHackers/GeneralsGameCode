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

/****************************************************************************\
timezone                      Matthew D. Campbell

This is just a couple of convenience functions for determining what timezone
we are now in.  It even accounts for daylight savings!  One caveat is that it
only tells you info about what the daylight savings info is now, not 5 minutes
from now, not 2 hours ago.  Oh well.
\****************************************************************************/

#pragma once

// Just fill in both the timezone description and its offset from GMT
void GetTimezoneInfo(const char * &timezone_str, int &timezone_offset);

// Returns the description of the current timezone (daylight savings included)
const char * TimezoneString(void);

// Returns the offset from GMT of the current timezone
int TimezoneOffset(void);
