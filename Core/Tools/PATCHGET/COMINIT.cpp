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

//
// If you link with this it will automatically call the COM initialization stuff
//
#include "COMINIT.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <objbase.h>

namespace patchget
{

ComInit::ComInit()
{
  HRESULT hRes = CoInitialize(nullptr);
  if (SUCCEEDED(hRes)==FALSE)
  {
    MessageBox(nullptr,"Can't initialize COM?!?!","Error:",MB_OK);
    exit(0);
  }
}


ComInit::~ComInit()
{
  CoUninitialize();
}

// Creating this instance will setup all COM stuff & do cleanup on program exit
ComInit  Global_COM_Initializer;

} // namespace patchget
