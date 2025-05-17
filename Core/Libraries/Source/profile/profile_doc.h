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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/profile_doc.h $
// $Author: mhoffe $
// $Revision: #3 $
// $DateTime: 2003/07/09 10:57:23 $
//
// (c) 2003 Electronic Arts
//
// additional Doxygen module documentation
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  pragma once
#endif
#ifndef PROFILE_DOC_H // Include guard
#define PROFILE_DOC_H

// This generates a small main page for Doxygen if a module only
// documentation is built. 
#ifndef DOXYGEN_GLOBAL_DOC
/**
  \mainpage %Profile module

  The following pages contain the most useful information:
  - \ref module_profile
  - \ref profile_cmd

  \internal This is the internal module documentation meant only for
            programmers who are working on the module internals.
*/
#endif

/**
  \page module_profile Profile module overview
  
  \section overview Overview

  The profile module contains the following logical groups:
  - high level hierarchical timer and logical profiling (frame based and global)
  - function level hierarchical timer based profiling (frame based and global)

  Profiling data can be accessed in different ways:
  - using debug commands to query for profiling data
  - using debug commands to set up a write-to-file mode for profile data
  - using the C++ interface herein

  \section highlevel High Level Profiling

  High level profiling can be done both timer based and logical. An example
  for a logical profile would be the number of texture changes per frame
  or the number of triangles rendered. 
  
  The hierarchy is enforced by using a hierarchical naming scheme. For
  timer based profiles this hierachy is build automatically, for 
  logical profiles each profile must be named accordingly when being created.

  High level profiling is available in all build configurations. All
  high level profile functions are optimized for speed (at least while
  profiling is disabled) so that there is an almost zero cost for 
  having profiling in all configurations.

  \section funclevel Function Level Profiling

  Function level profiling determines general call statistics, e.g. call
  count per frame/total, time spent in function, time per call, time
  spend in function and children, etc.

  Function level profiling is available in the 'profile' build configuration
  only since it relies on the fact that the compiler generates '_penter'
  function calls at the beginning of each function.

  \section frames Profile frames

  Instead of just providing frame based profiles this module has the concept
  of profile ranges. A range is a period of time specified by a Begin and
  and End function call. All data within this range is recorded as a range.

  It is possible to capture to more than one active range at the same time.

  Ordinary frame capturing can be achieved by recording a range back-to-back.

  Using ranges makes it very easy to profile logically connected events, e.g.
  profiling a level load time.

  \section cmdif Command interface

  This module provides a Debug Module command interface called 'profile'.
  The commands in this command interface are basically:
  - enabling/disabling profiles, either for frame counts or ranges
  - querying of current profile data
  - enabling write-to-file mode for profile data
*/

/**
  \page profile_cmd Profile commands

  Notation used:
  - [ abc ] means that 'abc' may or may not be specified
  - { abc } means that 'abc' may be specified any number of times (or not at all)
  - (a|b|c) means that either 'a', 'b' or 'c' must be specified once

  The following commands exist (group: profile):

  <table><tr>
    <td><b>Command</b></td>
    <td><b>Parameters</b></td>
    <td><b>Description</b></td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>result</td>
    <td valign=top>[ \<res_func_name\> [ \<arg1\> .. \<argN\> ] ]</td>
    <td>
       Without parameters this command shows the list of available 
       result functions and their optional parameters.

       If however a result function name is given then this result
       function (and the given parameters) are added to the list of
       active result functions. They are executed on program exit and
       used to generate e.g. a CSV file containing the profile results.
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>caller</td>
    <td valign=top>[ (+|-) ]</td>
    <td>
      Enables/disables recording of caller information while
      performing function level profiling. Turned off by default
      since CPU hit is non-zero.
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>clear</td>
    <td valign=top></td>
    <td>
      Clears the profile inclusion/exclusion list.
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>add</td>
    <td valign=top>(+|-) \<pattern\></td>
    <td>
      Adds a pattern to the profile list. By default all
      profile ranges are disabled. Each new range is then checked
      against all pattern in this list. If a match is found the
      active/inactive state is modified accordingly (+ for active,
      - for inactive). The final state is always the last match.
    </td>

  </tr><tr>

    <!---------------------------------->
    <td valign=top>view</td>
    <td valign=top></td>
    <td>
      Shows the active pattern list.
    </td>

  <!-- keep this as template for new commands -->
  </tr><tr>

    <!---------------------------------->
    <td valign=top></td>
    <td valign=top></td>
    <td>
    </td>

  </tr></table>

*/

#endif // PROFILE_DOC_H
