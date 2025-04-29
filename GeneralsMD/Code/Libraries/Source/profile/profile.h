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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/profile.h $
// $Author: mhoffe $
// $Revision: #4 $
// $DateTime: 2003/08/14 13:43:29 $
//
// (c) 2003 Electronic Arts
//
// Profiling module
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  pragma once
#endif
#ifndef PROFILE_H // Include guard
#define PROFILE_H

#if defined(RTS_DEBUG) && defined(_INTERNAL)
	#error "Only either RTS_DEBUG or _INTERNAL should ever be defined"
#endif

// include all our public header files (use double quotes here)
#include "profile_doc.h"
#include "profile_highlevel.h"
#include "profile_funclevel.h"
#include "profile_result.h"

/**
  \brief Functions common to both profilers.
*/
class Profile
{
  friend class ProfileCmdInterface;
  
  // nobody can construct this class
  Profile();

public:

  /**
    \brief Starts range recording.

    \param range name of range to record, ==NULL for "frame"
  */
  static void StartRange(const char *range=0);

  /**
    \brief Appends profile data to the last recorded frame
    of the given range.

    \param range name of range to record, ==NULL for "frame"
  */
  static void AppendRange(const char *range=0);
  
  /**
    \brief Stops range recording.

    \note After this call the recorded range data will be available
    as a new range frame.

    \param range name of range to record, ==NULL for "frame"
  */
  static void StopRange(const char *range=0);

  /**
    \brief Determines if any range recording is enabled or not.

    \return true if range profiling is enabled, false if not
  */
  static bool IsEnabled(void);

  /**
    \brief Determines the number of known (recorded) range frames.

    Note that if function level profiling is enabled then the number
    of recorded high level frames is the same as the number of recorded
    function level frames.

    \return number of recorded range frames
  */
  static unsigned GetFrameCount(void);

  /**
    \brief Determines the range name of a recorded range frame.

    \note A unique number will be added to the frame name, separated by
    a ':', e.g. 'frame:3'

    \param frame number of recorded frame
    \return range name
  */
  static const char *GetFrameName(unsigned frame);

  /**
    \brief Resets all 'total' counter values to 0.

    This function does not change any recorded frames.
  */
  static void ClearTotals(void);

  /**
    \brief Determines number of CPU clock cycles per second.

    \note This value is cached internally so this function is
    quite fast.

    \return number of CPU clock cycles per second
  */
  static _int64 GetClockCyclesPerSecond(void);
  
  /**
    \brief Add the given result function interface.

    \param func factory function
    \param name factory name
    \param arg description of optional parameters the factory function recognizes
  */
  static void AddResultFunction(ProfileResultInterface* (*func)(int, const char * const *),
                                const char *name, const char *arg);

private:
  /** \internal

    \brief Simple recursive pattern matcher.

    \param str string to match
    \param pattern pattern, only wildcard valid is '*'
    \return true if string matches pattern, false if not
  */
  static bool SimpleMatch(const char *str, const char *pattern);
  
  /// known frame names
  struct FrameName
  {
    /// frame name
    char *name;

    /// number of recorded frames for this name
    unsigned frames;

    /// are we currently recording?
    bool isRecording;

    /// should current frame be appended to last frame with same name
    bool doAppend;

    /// internal index for function level profiler
    int funcIndex;

    /// internal index for high level profiler
    int highIndex;

    /// global frame number of last recorded frame for this range, -1 if none
    int lastGlobalIndex;
  };

  /// \internal pattern list entry
  struct PatternListEntry
  {
    /// next entry
    PatternListEntry *next;

    /// active (true) or inactive (false)?
    bool isActive;

    /// pattern itself (dynamic allocated memory)
    char *pattern;
  };

  /** \internal

    First pattern list list entry. A singly linked list is
    okay for this because checking patterns is a costly 
    operation anyway and is therefore cached.
  */
  static PatternListEntry *firstPatternEntry;

  /// \internal last pattern list entry for fast additions to list at end
  static PatternListEntry *lastPatternEntry;

  /// number of recorded frames
  static unsigned m_rec;

  /// names of recorded frames
  static char **m_recNames;

  /// number of known frame names
  static unsigned m_names;

  /// list of known frame names
  static FrameName *m_frameNames;

  /// CPU clock cycles/second
  static _int64 m_clockCycles;
};

#endif // PROFILE_H
