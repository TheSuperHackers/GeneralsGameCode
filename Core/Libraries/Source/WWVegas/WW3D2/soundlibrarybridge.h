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
 *                                                                                             
 *                 Project name : Earth and Beyond                                             
 *                                                                                             
 *                    File name : soundlibrarybridge.h                                         
 *                                                                                             
 *                   Programmer : Mike Lytle                                                   
 *                                                                                             
 *                   Start date : 6/21/2002                                                    
 *                                                                                             
 *                  Last update : 10/23/2002 MBL                                                    
 *                                                                                             
 *---------------------------------------------------------------------------------------------
 * Functions:                                                                                  
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef SOUNDLIBRARYBRIDGE_H
#define SOUNDLIBRARYBRIDGE_H

// Forward declarations.
class		Matrix3D;

class SoundLibraryBridgeClass {
	public:
		virtual	void			Play_3D_Audio(const char * name, const Matrix3D & tm) = 0; 
		virtual	void			Play_2D_Audio(const char * name) = 0; 
		virtual	void			Stop_Playing_Audio(const char * name) = 0;
};

#endif //SOUNDLIBRARYBRIDGE_H
