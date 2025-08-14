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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/internal_result.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/09 10:57:23 $
//
// (c) 2003 Electronic Arts
//
// Internal result functions
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  pragma once
#endif
#ifndef INTERNAL_RESULT_H // Include guard
#define INTERNAL_RESULT_H

/// \brief Simple CSV format flat file result function, for all threads.
class ProfileResultFileCSV: public ProfileResultInterface
{
  ProfileResultFileCSV(void) {}

  void WriteThread(ProfileFuncLevel::Thread &thread);

public:
  static ProfileResultInterface *Create(int argn, const char * const *);
  virtual const char *GetName(void) const { return "file_csv"; }
  virtual void WriteResults(void);
  virtual void Delete(void);
};

/**
  \brief Write out DOT file for calling hierarchy.

  The frame name and the file name must be specified when creating an
  instance of this result function. The result function will always pick
  the thread with the highest function count (which is usually the
  main thread).

  \note A DOT file is used with the DOT tool from the GraphViz package
  for generating directed graphs, e.g. by issuing dot -Tgif -ograph.gif profile.dot
*/
class ProfileResultFileDOT: public ProfileResultInterface
{
public:
  enum
  {
    MAX_FUNCTIONS_PER_FILE = 200
  };

  /**
    \brief Creates a class instance.

    \param fileName name of DOT file to generate (defaults to profile.dot)
    \param frameName name of frame to use (NULL for global)
    \param foldThreshold if the number of functions exceeds the given threshold
                         then all functions belonging to the same source file
                         will be folded into a single entry
    \return new instance
  */
  static ProfileResultInterface *Create(int argn, const char * const *);

  virtual const char *GetName(void) const { return "file_dot"; }
  virtual void WriteResults(void);
  virtual void Delete(void);

private:

  ProfileResultFileDOT(const char *fileName, const char *frameName, int foldThreshold);

  struct FoldHelper
  {
    FoldHelper *next;
    const char *source;
    ProfileFuncLevel::Id id[MAX_FUNCTIONS_PER_FILE];
    unsigned numId;
    bool mark;
  };

  char *m_fileName;
  char *m_frameName;
  int m_foldThreshold;
};

#endif // INTERNAL_RESULT_H
