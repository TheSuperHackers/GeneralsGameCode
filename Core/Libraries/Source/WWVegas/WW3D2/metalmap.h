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
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/metalmap.h                             $*
 *                                                                                             *
 *                      $Author:: Hector_y                                                    $*
 *                                                                                             *
 *                     $Modtime:: 6/27/01 4:30p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef METALMAP_H
#define METALMAP_H

#include <vector3.h>

class TextureClass;
class INIClass;

/*
** MetalMapManagerClass: This class updates the procedural environment maps
** (aka metal maps) based on the global lighting environment.
** At the moment the metal maps are shaded using a simple Phong model - in
** future this may be extended to more complex and realistic shading
** (Cook-Torrance, etc.) which would require changing the metal parameters.
** Another possible extension would be to support a large dynamic range for
** the main light source (currently its color is treated as an RGB triple
** which is bound to 0,1).
*/

// If this is increased too much the metal map updates will be too expensive
#define METALMAP_SIZE 16
#define METALMAP_SIZE_2 (METALMAP_SIZE * METALMAP_SIZE)

class MetalMapManagerClass {

public:

	// These parameters are for a simple Phong reflectance model
	struct MetalParams {
		Vector3	AmbientColor;
		Vector3	DiffuseColor;
		Vector3	SpecularColor;
		float		Shininess;
	};

	// Create metal map manager with maps specified by INI file
	MetalMapManagerClass(INIClass &ini);
	~MetalMapManagerClass(void);

	// Get the texture for a metal map by id number
	TextureClass *	Get_Metal_Map(int id);

	// Get the number of metal maps in the manager
	int				Metal_Map_Count(void);

	// Update the lighting parameters used for generating the maps
	void				Update_Lighting(const Vector3& ambient, const Vector3& main_light_color,
							const Vector3& main_light_dir, const Vector3& camera_dir);

	// Update the metal map textures (should call once/frame before rendering)
	void				Update_Textures(void);

private:

	// 16 x 16 table of cameraspace normals for the environment maps
	static Vector3 *	_NormalTable;
	void					initialize_normal_table(void);	// Utility function

	// Utility function - shared CTor code
	void					initialize_metal_params(int map_count, MetalParams *metal_params);

	// Number of metal maps
	int					MapCount;

	// Array of metal map texture pointers
	TextureClass **	Textures;

	// Array of metal parameters
	MetalParams *		MetalParameters;

	// Current lighting parameters
	Vector3				CurrentAmbient;
	Vector3				CurrentMainLightColor;
	Vector3				CurrentMainLightDir;
	Vector3				CurrentCameraDir;

	// Use 16-bit metal maps or not
	bool					Use16Bit;
};

#endif
