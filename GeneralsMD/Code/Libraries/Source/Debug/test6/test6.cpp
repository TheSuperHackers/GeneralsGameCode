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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/test6/test6.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debug module - Test 6 (SEH, FPO test)
//////////////////////////////////////////////////////////////////////////////
#include "../debug.h"
#include <stdio.h>

int test,divByZero;

void func1(void)
{
  test/=divByZero;
}

void func2(void)
{
  func1();
}

void func3(void)
{
  func2();
}

void main(void)
{
  try
  {
    func3();
  }
  catch (...) 
  {
    printf("This catch clause should not be executed.\n");
  }
}
