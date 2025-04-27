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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/internal_cmd.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/09 10:57:23 $
//
// (c) 2003 Electronic Arts
//
// Profile module command interface
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#  pragma once
#endif
#ifndef INTERNAL_CMD_H // Include guard
#define INTERNAL_CMD_H

class ProfileCmdInterface: public DebugCmdInterface
{
  struct Factory
  {
    ProfileResultInterface* (*func)(int, const char * const *);
    const char *name,*arg;
  };

  static unsigned numResIf;
  static Factory *resIf;

  unsigned numResFunc; // optimizer bug: must be declared volatile!
  ProfileResultInterface **resFunc;

public:
  ProfileCmdInterface(void): numResFunc(0), resFunc(0) {}
  
  static void AddResultFunction(ProfileResultInterface* (*func)(int, const char * const *),
                                const char *name, const char *arg);
  void RunResultFunctions(void);

  virtual bool Execute(class Debug& dbg, const char *cmd, CommandMode cmdmode,
                       unsigned argn, const char * const * argv);
  virtual void Delete(void) {}
};

#endif // INTERNAL_CMD_H
