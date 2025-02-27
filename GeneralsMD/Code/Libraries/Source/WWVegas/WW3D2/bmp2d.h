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
 *                     $Archive:: /Commando/Code/ww3d2/bmp2d.h            $*
 *                                                                         *
 *                      $Author:: Hector_y                                $*
 *                                                                         *
 *                     $Modtime:: 2/21/01 1:31p                           $*
 *                                                                         *
 *                    $Revision:: 3                                       $*
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "dynamesh.h"

class Bitmap2DObjClass : public DynamicScreenMeshClass
{
	public:
		Bitmap2DObjClass(const char *filename, float norm_x, float norm_y,
			bool center, bool additive, bool colorizable = false, int width = -1, int height = -1, bool ignore_alpha = false);
		Bitmap2DObjClass(TextureClass *texture, float norm_x, float norm_y,
			bool center, bool additive, bool colorizable = false, bool ignore_alpha = false);
		Bitmap2DObjClass( const Bitmap2DObjClass & src) : DynamicScreenMeshClass(src) {}

		virtual RenderObjClass * 	Clone(void) const;
		virtual int						Class_ID(void) const { return CLASSID_BITMAP2D; }
};
