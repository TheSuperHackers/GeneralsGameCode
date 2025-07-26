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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwshade/shdlib.h                           $*
 *
 *                    Org Author:: Kenny Mitchell
 *                                                                                             *
 *                       $Author:: Kenny_m
 *																																	
 *								$Modtime:: 07/01/02 9:58p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SHDLIB_H
#define SHDLIB_H

// TheSuperHackers @build Removes the unused WWShade code. (#596)
// If there is a desire to revive this, then the WWShade library needs to be re-added.

#ifdef USE_WWSHADE

extern void SHD_Init();
extern void SHD_Shutdown();
extern void SHD_Init_Shaders();
extern void SHD_Shutdown_Shaders();
extern void SHD_Flush();
extern void SHD_Register_Loader();

#define SHD_INIT					SHD_Init()
#define SHD_SHUTDOWN				SHD_Shutdown()
#define SHD_INIT_SHADERS		SHD_Init_Shaders()
#define SHD_SHUTDOWN_SHADERS	SHD_Shutdown_Shaders()
#define SHD_FLUSH					SHD_Flush()
#define SHD_REG_LOADER			SHD_Register_Loader()

#else // USE_WWSHADE

#define SHD_INIT					
#define SHD_SHUTDOWN				
#define SHD_INIT_SHADERS		
#define SHD_SHUTDOWN_SHADERS	
#define SHD_FLUSH					
#define SHD_REG_LOADER			

#endif // USE_WWSHADE


#endif // SHDLIB_H