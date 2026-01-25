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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/test3/test3.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debug module - Test 3 (Checking FLAT I/O, logging)
//////////////////////////////////////////////////////////////////////////////

#include "../debug.h"

unsigned divByNull;

void main(void)
{
  // switch to debug group
  Debug::Command("debug.");

  Debug::Command("alwaysflush +");

  // we want our files copied
  Debug::Command("io flat copy ..\\");

  // split 'split_me' log group to separate file (with size limit)
  Debug::Command("io flat splitadd l split_me splitted 1");

  // turn all logs on
  Debug::Command("add l + *");

  // disable no_log log
  Debug::Command("add l - no_log");

  // now log something...
  DLOG("This should be visible.\n");
  DLOG_GROUP(no_log,"This should *NOT* be visible.\n");
  for (int k=0;k<200;k++)
    DLOG_GROUP(split_me,"Log line " << k << "\n");
  DLOG_GROUP(split_me,"Final line in separate log file.\n");

  // and now let's crash!
  divByNull/=divByNull;
}
