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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWShade                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwshade/shdlib.cpp									$*
 *                                                                                             *
 *                   Org Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 07/01/02 10:31a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * WWShade.lib library interface                                                               *
 *---------------------------------------------------------------------------------------------*/


#include "shdlib.h"
#include "assetmgr.h"
#include "shdloader.h"
#include "shdrenderer.h"

void SHD_Init()
{
	ShdRendererClass::Peek_Instance()->Init();
}

void SHD_Shutdown()
{
	ShdRendererClass::Peek_Instance()->Shutdown();
}

void SHD_Init_Shaders()
{
	ShdRendererClass::Init_Shaders();
}

void SHD_Shutdown_Shaders()
{
	ShdRendererClass::Shutdown_Shaders();
}

void SHD_Flush()
{
	ShdRendererClass::Peek_Instance()->Flush();
}

void SHD_Register_Loader()
{
	WW3DAssetManager::Get_Instance()->Register_Prototype_Loader(&_ShdMeshLoader);
//	WW3DAssetManager::Get_Instance()->Register_Prototype_Loader(&_ShdMeshLegacyLoader);
	WW3DAssetManager::Get_Instance()->Register_Prototype_Loader(&_MeshLoader);
}
