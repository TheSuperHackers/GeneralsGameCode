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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/profile_highlevel.h $
// $Author: mhoffe $
// $Revision: #2 $
// $DateTime: 2003/07/09 10:57:23 $
//
// (c) 2003 Electronic Arts
//
// High level profiling
//////////////////////////////////////////////////////////////////////////////

#pragma once

/// \internal internal Id representation
class ProfileId;

/**
  \brief The high level profiler.
*/
class ProfileHighLevel
{
  friend class Profile;

  // no, no copying allowed!
  ProfileHighLevel(const ProfileHighLevel&);
  ProfileHighLevel& operator=(const ProfileHighLevel&);

public:

  /// \brief A high level profile ID.
  class Id
  {
    friend ProfileHighLevel;

  public:
    Id(void): m_idPtr(0) {}

    /**
      \brief Increment the internal profile value.

      \note Do not mix with SetMax.

      \param add amount to add to internal profile value
    */
    void Increment(double add=1.0);

    /**
      \brief Set a new maximum value.

      \note Do not mix with Increment.

      This function sets a new maximum value (if the value
      passed in is actually larger than the current max value).

      \param max new maximum value (if larger than current max value,
                 otherwise current max value is left unchanged)
    */
    void SetMax(double max);

    /**
      \brief Returns the internal Id name.

      \return internal Id name, e.g. 'render.texture.count.512x512'
    */
    const char *GetName(void) const;

    /**
      \brief Returns the descriptive name.

      \return descriptive name, e.g. '# of 512x512 textures'
    */
    const char *GetDescr(void) const;

    /**
      \brief Returns the value's unit text.

      \return unit text, e.g. 'bytes'
    */
    const char *GetUnit(void) const;

    /**
      \brief Returns the current value.

      'Current' means the value since the last call to this function for
      the same Id.

      This function is intended for displaying profile data while the
      application is running.

      \note The contents of the buffer returned may be overwritten by
      any consecutive call to any profile module function.

      \return current value
    */
    const char *GetCurrentValue(void) const;

    /**
      \brief Returns the value for the given recorded frame/range.

      \note The contents of the buffer returned may be overwritten by
      any consecutive call to any profile module function.

      \param frame number of recorded frame/range
      \return value at given frame, NULL if frame not found
    */
    const char *GetValue(unsigned frame) const;

    /**
      \brief Returns the total value for all frames.

      This even includes data collected while no frames have been
      recorded.

      \note A call to ProfileHighLevel::ClearTotals() resets this value.

      \return total value
    */
    const char *GetTotalValue(void) const;

  private:

    /// internal pointer
    ProfileId *m_idPtr;
  };

  /// \brief Timer based function block profile
  class Block
  {
    friend ProfileHighLevel;

    // no copying
    Block(const Block&);
    Block& operator=(const Block&);

  public:
    /**
      \brief Instructs high level profiler to start a new timer
             based function block (or update if it already exists)

      \note These function blocks are in the same name space as
      high level profile values registered with ProfileHighLevel::AddProfile()

      \param name name of function block
    */
    explicit Block(const char *name);

    /// \brief Updates timer based function block
    ~Block();

  private:
    /// internal id (time)
    Id m_idTime;

    /// start time
    _int64 m_start;
  };

  /**
    \brief Registers a new high level profile value.

    If there is already a high level profile with the given name the
    Id of that profile is returned instead.

    High level profiles can only be added, never removed.

    \note Important: This function can (and should) be used in static
    initializers!

    \note This function may be slow so don't use it too often.

    \param name value name, e.g. "render.texture.count.512x512"
    \param descr descriptive name, e.g. "# of 512x512 textures"
    \param unit unit name, e.g. "byte" or "sec"
    \param precision number of decimal places to show
    \param exp10 10 base exponent (used for scaleing)
    \return internal profile ID value
  */
  static Id AddProfile(const char *name, const char *descr, const char *unit, int precision, int exp10=0);

  /**
    \brief Enumerates the list of known high level profile values.

    \note Profiles are always sorted ascending by profile name.

    \param index index value, >=0
    \param id return buffer for ID value
    \return true if ID found at given index, false if not
  */
  static bool EnumProfile(unsigned index, Id &id);

  /**
    \brief Searches for the given high level profile.

    \note Actually the ID returned belongs to the first profile
    which has a name that is equal to or larger than the name
    searched for.

    \param name profile name to search for
    \param id return buffer for ID value
    \return true if ID found, false if not
  */
  static bool FindProfile(const char *name, Id &id);

private:

  /** \internal

    Undocumented default constructor. Initializes high level profiler.
    We can make this private as well so nobody accidently tries to create
    another instance.
  */
  ProfileHighLevel(void);

  /**
    \brief The only high level profiler instance.
  */
  static ProfileHighLevel Instance;
};
