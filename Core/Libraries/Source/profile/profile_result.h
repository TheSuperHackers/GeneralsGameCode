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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/profile_result.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/09 10:57:23 $
//
// (c) 2003 Electronic Arts
//
// Result function interface and result functions
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  pragma once
#endif
#ifndef PROFILE_RESULT_H // Include guard
#define PROFILE_RESULT_H

/**
  \brief Result function class.

  Factories for instances of this class are registered using
  \ref Profile::AddResultFunction.
*/
class ProfileResultInterface
{
  // no copying
  ProfileResultInterface(const ProfileResultInterface&);
  ProfileResultInterface& operator=(const ProfileResultInterface&);

public:
  /**
    \brief Write out results.

    This function is called on program exit.
  */
  virtual void WriteResults(void)=0;

  /**
    \brief Destroys the current result function.

    Use this function instead of just delete'ing the instance.
  */
  virtual void Delete(void)=0;

protected:
  ProfileResultInterface(void) {}
};

#endif // PROFILE_RESULT_H
