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

// FILE: Errors.h
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
// File name:  Errors.h
//
// Created:    Steven Johnson, August 2001
//
// Desc:       Error codes
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

/**
	An ErrorCode is the repository for failure modes. In almost all situations,
	these values will  be THROWN, not returned as error codes. Feel free
	to add to this list as necessary; however, there should generally be very
	few codes needed.
*/
enum ErrorCode CPP_11(: UnsignedInt)
{
	ERROR_BASE									= 0xdead0001,								// a nice, distinctive value

	ERROR_BUG										= (ERROR_BASE + 0x0000),		///< should not be possible under normal operation
	ERROR_OUT_OF_MEMORY					= (ERROR_BASE + 0x0001),		///< unable to allocate memory.
	ERROR_BAD_ARG								= (ERROR_BASE + 0x0002),		///< generic "bad argument".
	ERROR_INVALID_FILE_VERSION	= (ERROR_BASE + 0x0003),		///< Unrecognized file version.
	ERROR_CORRUPT_FILE_FORMAT		= (ERROR_BASE + 0x0004),		///< Invalid file format.
	ERROR_BAD_INI								= (ERROR_BASE + 0x0005),		///< Bad INI data.
	ERROR_INVALID_D3D						= (ERROR_BASE + 0x0006),    ///< Error initing D3D

	ERROR_LAST
};
