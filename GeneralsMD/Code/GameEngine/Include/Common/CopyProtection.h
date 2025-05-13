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

// FILE: CopyProtection.h ////////////////////////////////////////////////////
// Author: Matthew D. Campbell
// Taken From: Denzil Long's code in Tiberian Sun, by way of Yuri's Revenge
//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef COPYPROTECTION_H
#define COPYPROTECTION_H

// Comment out the following line to disable copy protection checks
//#define DO_COPY_PROTECTION

#ifdef DO_COPY_PROTECTION

class CopyProtect
	{
	public:
		static Bool isLauncherRunning(void);
		static Bool notifyLauncher(void);
		static void checkForMessage(UINT message, LPARAM lParam);
		static Bool validate(void);
		static void shutdown(void);

	private:	
		static LPVOID s_protectedData;
	};

#endif // DO_COPY_PROTECTION

#endif // COPYPROTECTION_H
