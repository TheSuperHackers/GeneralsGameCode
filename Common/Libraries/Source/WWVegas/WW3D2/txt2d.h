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

/*************************************************************************** 
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     *** 
 *************************************************************************** 
 *                                                                         * 
 *                 Project Name : Commando/G                               * 
 *                                                                         * 
 *                     $Archive:: /Commando/Code/ww3d2/txt2d.h            $* 
 *                                                                         * 
 *                      $Author:: Greg_h                                  $* 
 *                                                                         * 
 *                     $Modtime:: 1/23/01 10:37a                          $* 
 *                                                                         * 
 *                    $Revision:: 2                                       $* 
 *                                                                         * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef TXT2D_H
#define TXT2D_H

#include "dynamesh.h"
#include "txt.h"

class FontClass;
class ConvertClass;

#ifdef WW3D_DX8
class Text2DObjClass : public DynamicScreenMeshClass
{
	public:
		Text2DObjClass(FontClass &font, const char *str, float screen_x, float screen_y, int fore, int back, ConvertClass &conv, bool center, bool clamp, ...);
		void Set_Text(FontClass &font, const char *str, float screen_x, float screen_y, int fore, int back, ConvertClass &conv, bool center, bool clamp, ...);

		// class id of this render object
		virtual int Class_ID(void) const { return CLASSID_TEXT2D; }
		
		static float		_LastWidth;
		static float		_LastHeight;

	private:
		TextTextureClass	TextTexture;
};
#endif //WW3D_DX8

#endif