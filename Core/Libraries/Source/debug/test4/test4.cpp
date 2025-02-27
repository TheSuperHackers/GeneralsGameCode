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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/test4/test4.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debug module - Test 4 (Multiple DASSERTs, high-count DCHECKs)
//////////////////////////////////////////////////////////////////////////////
#include "../debug.h"

void main(void)
{
  for (int i=0;i<30;i++)
    DCHECK_MSG(i>100,"run#" << i);
  Debug::Command("list c");
  for (int k=0;k<3;k++)
  {
    DASSERT(k>4);
    DASSERT_MSG(k<1,"k must be less than 1...");
    Debug::Command("list a");
  }
}
