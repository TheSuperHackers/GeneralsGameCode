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

/////////////////////////////////////////////////////////////////////////EA-V1
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debugging module
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  pragma once
#endif
#ifndef DEBUG_H // Include guard
#define DEBUG_H

/** 
  \page lib_var Library variants

  Generally speaking there are four different library variants:
  - Internal: all asserts/checks/logs, full optimizations (RTS_INTERNAL macro defined)
  - %Debug: all asserts/checks/logs, no optimizations (RTS_DEBUG macro defined)
  - Profile: all asserts/checks/logs, full optimizations, profiling active (RTS_PROFILE macro defined)
  - Release: no asserts/checks/logs, full optimizations

  These variants will be broken down into separate features which
  can be queried for by #ifdef(HAS_FEATURE):

  <table><tr>
    <td><b>Variant</b></td>
    <td><b>HAS_ASSERTS</b></td>
    <td><b>HAS_LOGS</b></td>
    <td><b>HAS_OPT</b></td>
    <td><b>HAS_PROFILE</b></td>
  </tr><tr>
    <td>Internal</td>
    <td><center>Y</center></td>
    <td><center>Y</center></td>
    <td><center>Y</center></td>
    <td><center></center></td>
  </tr><tr>
    <td>%Debug</td>
    <td><center>Y</center></td>
    <td><center>Y</center></td>
    <td><center></center></td>
    <td><center></center></td>
  </tr><tr>
    <td>Profile</td>
    <td><center>Y</center></td>
    <td><center>Y</center></td>
    <td><center>Y</center></td>
    <td><center>Y</center></td>
  </tr><tr>
    <td>Release</td>
    <td><center></center></td>
    <td><center></center></td>
    <td><center>Y</center></td>
    <td><center></center></td>
  </tr></table>

  Library files have a suffix appended that depends on the
  library variant:
  - Internal: XXXInternal.lib
  - %Debug: XXXDebug.lib
  - Profile: XXXProfile.lib
  - Release: XXX.lib
*/

#if defined(RTS_DEBUG) && defined(RTS_INTERNAL)
	#error "Only either RTS_DEBUG or RTS_INTERNAL should ever be defined"
#endif

// Define which libraries to use. 
#if defined(RTS_INTERNAL) || defined(RTS_DEBUG) || defined(RTS_PROFILE)
#  define HAS_ASSERTS
#  define HAS_LOGS
#endif

#if !defined(RTS_DEBUG)
#  define HAS_OPT
#endif

#if defined(RTS_PROFILE)
#  define HAS_PROFILE
#endif

// include all our public header files (use double quotes here)
#include "debug_doc.h"
#include "debug_macro.h"
#include "debug_io.h"
#include "debug_cmd.h"
#include "debug_stack.h"
#include "debug_debug.h"

#endif // DEBUG_H
