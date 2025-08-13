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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:    Generals
//
// Module:     Game Engine DEvice Win32 Common
//
// File name:  Win32Device/Common/Win32CDManager.h
//
// Created:    11/26/01 TR
//
//----------------------------------------------------------------------------

#pragma once

#ifndef _WIN32DEVICE_COMMON_WIN32CDMANAGER_H_
#define _WIN32DEVICE_COMMON_WIN32CDMANAGER_H_


//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "Common/CDManager.h"


//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// Win32CDDrive
//===============================

class Win32CDDrive : public CDDrive
{
	public:

	Win32CDDrive();
	virtual ~Win32CDDrive();

	virtual void refreshInfo( void );					///< Update drive with least

};

//===============================
// Win32CDManager
//===============================

class Win32CDManager : public CDManager
{
	public:

		Win32CDManager();
		virtual ~Win32CDManager();

		// sub system operations
		virtual void init( void );
		virtual void update( void );
		virtual void reset( void );
		virtual void refreshDrives( void );				///< Refresh drive info

	protected:

		virtual CDDriveInterface* createDrive( void );
};

//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------


#endif // _WIN32DEVICE_COMMON_WIN32CDMANAGER_H_
