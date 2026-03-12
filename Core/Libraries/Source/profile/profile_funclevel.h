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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/profile_funclevel.h $
// $Author: mhoffe $
// $Revision: #3 $
// $DateTime: 2003/07/09 10:57:23 $
//
// (c) 2003 Electronic Arts
//
// Function level profiling
//////////////////////////////////////////////////////////////////////////////

#pragma once

// Platform-specific 64-bit type compatibility
#ifdef _WIN32
    // Windows: _int64 is native MSVC type
    typedef unsigned _int64 u64;
    typedef _int64 i64;
#else
    // Linux: Use C++11 standard types
    #include <cstdint>
    typedef uint64_t u64;
    typedef int64_t i64;
    // Define _int64 for use in (unsigned _int64) patterns
    // Only define if not already defined by types_compat.h
    #ifndef _int64
    typedef int64_t _int64;
    #endif
#endif
#include <Utility/intrin_compat.h>
#include <cstdint>

// GeneralsX @bugfix fbraz 03/02/2026 Use guard macro to prevent typedef conflicts
#ifndef _INT64_TYPES_DEFINED
	#define _INT64_TYPES_DEFINED
	typedef int64_t __int64;
	typedef int64_t _int64;
#endif

/**
  \brief The function level profiler.

  Note that this class exists even if the current build configuration
  is not RTS_PROFILE. In these cases all calls will simply return
  empty data.
*/
class ProfileFuncLevel
{
  friend class Profile;

  // no, no copying allowed!
  ProfileFuncLevel(const ProfileFuncLevel&);
  ProfileFuncLevel& operator=(const ProfileFuncLevel&);

public:
  class Id;
  class Thread;

  /// \brief A list of function level profile IDs
  class IdList
  {
    friend Id;

  public:
    IdList(): m_ptr(0) {}

    /**
      \brief Enumerates the list of IDs.

      \note These values are not sorted in any way.

      \param index index value, >=0
      \param id return buffer for ID value
      \param countPtr return buffer for count, if given
      \return true if ID found at given index, false if not
    */
    bool Enum(unsigned index, Id &id, unsigned *countPtr=0) const;

  private:

    /// internal value
    void *m_ptr;
  };

  /// \brief A function level profile ID.
  class Id
  {
    friend IdList;
    friend Thread;

  public:
    Id(): m_funcPtr(0) {}

    /// special 'frame' numbers
    enum
    {
      /// return the total value/count
      Total = 0xffffffff
    };

    /**
      \brief Returns the source file this Id is in.

      \return source file name, may be nullptr
    */
    const char *GetSource() const;

    /**
      \brief Returns the function name for this Id.

      \return function name, may be nullptr
    */
    const char *GetFunction() const;

    /**
      \brief Returns function address.

      \return function address
    */
    unsigned GetAddress() const;

    /**
      \brief Returns the line number for this Id.

      \return line number, 0 if unknown
    */
    unsigned GetLine() const;

    /**
      \brief Determine call counts.

      \param frame number of recorded frame, or Total
      \return number of calls
    */
    // GeneralsX @refactor BenderAI 10/02/2026
    // Changed from unsigned _int64 (MSVC-specific) to u64 (platform typedef)
    u64 GetCalls(unsigned frame) const;

    /**
      \brief Determine time spend in this function and its children.

      \param frame number of recorded frame, or Total
      \return time spend (in CPU ticks)
    */
    u64 GetTime(unsigned frame) const;

    /**
      \brief Determine time spend in this function only (exclude
      any time spend in child functions).

      \param frame number of recorded frame, or Total
      \return time spend in this function alone (in CPU ticks)
    */
    u64 GetFunctionTime(unsigned frame) const;

    /**
      \brief Determine the list of caller Ids.

      \param frame number of recorded frame, or Total
      \return Caller Id list (actually just a handle value)
    */
    IdList GetCaller(unsigned frame) const;

  private:
    /// internal function pointer
    void *m_funcPtr;
  };

  /// \brief a profiled thread
  class Thread
  {
    friend ProfileFuncLevel;

  public:
    Thread(): m_threadID(0) {}

    /**
      \brief Enumerates the list of known function level profile values.

      \note These values are not sorted in any way.

      \param index index value, >=0
      \param id return buffer for ID value
      \return true if ID found at given index, false if not
    */
    bool EnumProfile(unsigned index, Id &id) const;

    /**
      \brief Returns a unique thread ID (not related to Windows thread ID)

      \return profile thread ID
    */
  // GeneralsX @refactor BenderAI 10/02/2026 Use uintptr_t to avoid pointer precision loss on 64-bit
  uintptr_t GetId() const
    {
      return (uintptr_t)m_threadID;
    }

  private:
    /// internal thread ID
    class ProfileFuncLevelTracer *m_threadID;
  };

  /**
    \brief Enumerates the list of known and profiled threads.

    \note These values are not sorted in any way.

    \param index index value, >=0
    \param thread return buffer for thread handle
    \return true if Thread found, false if not
  */
  static bool EnumThreads(unsigned index, Thread &thread);

private:

  /** \internal

    Undocumented default constructor. Initializes function-level profiler.
    We can make this private as well so nobody accidentally tries to create
    another instance.
  */
  ProfileFuncLevel();

  /**
    \brief The only function level profiler instance.
  */
  static ProfileFuncLevel Instance;
};
