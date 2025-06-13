/*
**	Command & Conquer Renegade(tm)
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

/////////////////////////////////////////////////////////////////////
//
//	Globals.CPP
//
//	Module containing global variable initialization.
//
//

#include "StdAfx.h"

#include "Globals.h"
#include "assetmgr.h"
#include "ViewerAssetMgr.h"

// Main asset manager for the application.
ViewerAssetMgrClass *_TheAssetMgr = NULL;


int g_iDeviceIndex      = -1;//DEFAULT_DEVICEINDEX;
int g_iBitsPerPixel     = -1;//DEFAULT_BITSPERPIX;
int g_iWidth				= 640;
int g_iHeight				= 480;
