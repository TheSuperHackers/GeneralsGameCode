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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/test1/test1.cpp $
// $Author: mhoffe $
// $Revision: #3 $
// $DateTime: 2003/07/09 10:57:23 $
//
// (c) 2003 Electronic Arts
//
// Profile module - Test 1 (basic testing)
//////////////////////////////////////////////////////////////////////////////
#include "../profile.h"
#include "../../debug/debug.h"
#include <stdio.h>

const char *DebugGetDefaultCommands(void)
{
  return "!debug.io con add\ndebug.add l + *\nprofile.result";
}

extern int q;

void calcThis(void)
{
  q++;
}

void calcThat(void)
{
  calcThis();
  q--;
}

// it must be done this "complicated" because
// otherwise VC does not generate a real recursive
// function call for this simple function...
void recursion2(int level);

void recursion(int level)
{
  q+=level;
  if (level<5000)
    recursion2(level+1);
}

void recursion2(int level)
{
  recursion(level);
}

void recursionShell(void)
{
  ProfileHighLevel::Block b("Test block");
  recursion(0);
}

void showResults(void)
{
  ProfileHighLevel::Id id;
  for (unsigned index=0;ProfileHighLevel::EnumProfile(index,id);index++)
    printf("%-16s%-6s %s\n",id.GetName(),id.GetTotalValue(),id.GetUnit());
}

void main(void)
{
  for (int k=0;k<100;k++)
    if (k%2&&k>80)
      calcThat();
    else
      calcThis();

  recursionShell();
  
  showResults();
}

int q;
