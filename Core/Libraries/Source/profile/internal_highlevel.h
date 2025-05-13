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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/internal_highlevel.h $
// $Author: mhoffe $
// $Revision: #2 $
// $DateTime: 2003/08/14 13:43:29 $
//
// (c) 2003 Electronic Arts
//
// High level profiling (internal header)
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  pragma once
#endif
#ifndef INTERNAL_HIGHLEVEL_H // Include guard
#define INTERNAL_HIGHLEVEL_H

/// an internal high level profile ID
class ProfileId
{
  ProfileId(const ProfileId&);
  ProfileId& operator=(const ProfileId&);

public:
  /**
    Creates a new high level profile ID.

    \param name profile name
    \param descr descriptive name
    \param unit unit name
    \param precision number of decimal places to show
    \param exp10 10 base exponent (used for scaleing)
  */
  ProfileId(const char *name, const char *descr, const char *unit, int precision, int exp10);

  /**
    Retrieves the first profile ID.

    \return first profile ID
  */
  static ProfileId *GetFirst(void) { return first; }

  /**
    Retrieves next profile ID.

    \return next profile ID, NULL if none
  */
  ProfileId *GetNext(void) const { return m_next; }

  /**
    Retrieves name of the profile ID.

    \return profile ID name
  */
  const char *GetName(void) const { return m_name; }

  /**
    Retrieves unit name of the profile ID.

    \return profile ID unit name
  */
  const char *GetUnit(void) const { return m_unit?m_unit:""; }

  /**
    Retrieves description of the profile ID.

    \return profile description
  */
  const char *GetDescr(void) const { return m_descr?m_descr:""; }

  /**
    Increments the profile value.

    \param add add value
  */
  void Increment(double add);

  /**
    Sets a new maximum value.

    \param max new maximum value
  */
  void Maximum(double max);

  /**
    Returns current value, internally resetting it.

    \return current value
  */
  double GetCurrentValue(void)
  {
    double help=m_curVal;
    m_curVal=0.;
    return help;
  }

  /**
    Returns total value

    \return total value
  */
  double GetTotalValue(void) const
  {
    return m_totalVal;
  }

  /**
    Returns value at the given frame.

    \param frame frame number
    \param value value at frame
    \return true if frame found, false if not
  */
  bool GetFrameValue(unsigned frame, double &value) const
  {
    if (frame<(unsigned)m_firstFrame||
        frame>=(unsigned)curFrame)
      return false;
    value=m_recFrameVal[frame-m_firstFrame];
    return true;
  }

  /**
    Translate given numeric value into a string, using an internal temp buffer.

    \param v value
    \return given numeric value as string
  */
  const char *AsString(double v) const;

  /**
    Shutdown function.
  */
  static void Shutdown(void);

  /**
    Starts frame based profiling, starts a new frame.
  */
  static int FrameStart(void);

  /**
    Ends frame based profiling.
  */
  static void FrameEnd(int which, int mixIndex);

  /**
    Clears all total values.
  */
  static void ClearTotals(void)
  {
    for (ProfileId *cur=first;cur;cur=cur->m_next)
      cur->m_totalVal=0.;
  }

private:
  /**
    ProfileId's can't be destructed.
  */
  ~ProfileId() {}
  
  enum
  {
    /// # of simultaneous frame recordings
    MAX_FRAME_RECORDS = 4,

    /// size of internal string buffer
    STRING_BUFFER_SIZE = 1024
  };

  /// possible value modes
  enum ValueMode
  {
    Unknown,
    ModeIncrement,
    ModeMaximum
  };

  /// first profile ID
  static ProfileId *first;

  /// next profile ID
  ProfileId *m_next;

  /// profile name
  char *m_name;

  /// profile description
  char *m_descr;

  /// profile unit
  char *m_unit;

  /// number of decimal places to show
  int m_precision;

  /// 10 base exponent (used for scaleing)
  int m_exp10;

  /// current value
  double m_curVal;

  /// total value
  double m_totalVal;

  /// frame values
  double m_frameVal[MAX_FRAME_RECORDS];

  /// list of recorded frame values
  double *m_recFrameVal;

  /// index of first recorded frame
  int m_firstFrame;

  /// value mode
  ValueMode m_valueMode;

  /// current frame
  static int curFrame;

  /// bit mask, currently recording which cur[] entries?
  static unsigned frameRecordMask;

  /// internal string buffer
  static char stringBuf[STRING_BUFFER_SIZE];

  /// next unused char in string buffer
  static unsigned stringBufUnused;
};

#endif // INTERNAL_HIGHLEVEL_H
