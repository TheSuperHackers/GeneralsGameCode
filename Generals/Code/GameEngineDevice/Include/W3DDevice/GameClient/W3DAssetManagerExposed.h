/*
**	Command & Conquer Generals(tm)
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DAssetManagerExposed.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  W3DAssetManagerExposed.h                                      */
/* Created:    John K. McDonald, Jr., 4/27/2002                              */
/* Desc:       A hack to get around our build structure.                     */
/* Revision History:                                                         */
/*		4/27/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/

#pragma once
#ifndef _H_W3DASSETMANAGEREXPOSED_
#define _H_W3DASSETMANAGEREXPOSED_

// INCLUDES ///////////////////////////////////////////////////////////////////
// DEFINES ////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

// This function is here because the particle editor needs to be able to force
// the asset manager to release all his textures and then reload them on demand.
// Unfortunately, the asset manager can't be called directly from the gamelogic,
// so this function is here. It should only be called by the particle editor,
// @todo Remove this function when we are no longer editing particles.
void ReloadAllTextures(void);

#endif /* _H_W3DASSETMANAGEREXPOSED_ */
