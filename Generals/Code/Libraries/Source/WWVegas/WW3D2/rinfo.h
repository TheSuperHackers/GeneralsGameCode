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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/rinfo.h                                $*
 *                                                                                             *
 *                   Org Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/27/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 * 06/27/02 KM Render to shadow buffer texture support														*
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "bittype.h"
#include "ww3d.h"
#include "wwdebug.h"
#include "shader.h"
#include "Vector.H"
#include "matrix3d.h"
#include "matrix4.h"


class MaterialPassClass;
class LightEnvironmentClass;
class VisRasterizerClass;
class BWRenderClass;
class TexProjectClass;

const unsigned MAX_ADDITIONAL_MATERIAL_PASSES=32;
const unsigned MAX_OVERRIDE_FLAG_LEVEL=32;

/**
** RenderInfoClass
** This class contains all of the data needed for the scene to render
** itself.  It will be passed on to the scene from a WW3D::Render(scene)
** call.
**
** Camera - The camera being used to render the scene, contains culling code, etc
*/
class RenderInfoClass
{
public:
	RenderInfoClass(CameraClass & cam);
	~RenderInfoClass(void);

	enum RINFO_OVERRIDE_FLAGS {
		RINFO_OVERRIDE_DEFAULT						= 0x0000,	// No overrides
		RINFO_OVERRIDE_FORCE_TWO_SIDED			= 0x0001,	// Override mesh settings to force no backface culling
		RINFO_OVERRIDE_FORCE_SORTING				= 0x0002,	// Override mesh settings to force sorting
		RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY	= 0x0004,	// Do not render base passes (only additional passes)
		RINFO_OVERRIDE_SHADOW_RENDERING			= 0x0008		// Hint: we are rendering a shadow
	};

	void								Push_Material_Pass(MaterialPassClass * matpass);
	void								Pop_Material_Pass(void);

	int								Additional_Pass_Count(void);
	MaterialPassClass *			Peek_Additional_Pass(int i);

	void								Push_Override_Flags(RINFO_OVERRIDE_FLAGS flg);	// Saves current override flags on stack and installs a new one
	void								Pop_Override_Flags(void);								// Restores previous override flags from stack
	RINFO_OVERRIDE_FLAGS &		Current_Override_Flags(void);							// Access to current override flags

	CameraClass &					Camera;

	float								fog_scale;
	float								fog_start;
	float								fog_end;
	float								alphaOverride;	//added for 'Generals' to allow variable alpha -MW
	float								materialPassAlphaOverride;	////added for 'Generals' to allow variable alpha on additional render passes.-MW
	float								materialPassEmissiveOverride;	////added for 'Generals' to allow variable emissive on additional render passes.-MW

	LightEnvironmentClass*		light_environment;

	TexProjectClass*				Texture_Projector;

protected:
	MaterialPassClass*			AdditionalMaterialPassArray[MAX_ADDITIONAL_MATERIAL_PASSES];
	unsigned							AdditionalMaterialPassCount;
	unsigned							RejectedMaterialPasses;
	RINFO_OVERRIDE_FLAGS			OverrideFlag[MAX_OVERRIDE_FLAG_LEVEL];
	unsigned							OverrideFlagLevel;

};


/**
** SpecialRenderInfoClass
** This structure also contains a "grab-bag" of junk for use by the Special_Render
** function.  The first use that I have for Special_Render is to implement the
** visibility detection algorithm where each object is rendered in such a way
** that I can get the 'id' of the object which generated each pixel on the screen.
** Another use I have planned for Special_Render is a shadow rendering mode that
** just draws an object in solid black from the point of view of a light source.
** This would just need another enum for the RenderType...
**
** The reason for a Special_Render function is that I didn't want to pollute
** the main rendering pipeline with checks for these alternate rendering operations.
*/
class SpecialRenderInfoClass : public RenderInfoClass
{

public:
	SpecialRenderInfoClass(CameraClass & cam,int render_type);
	~SpecialRenderInfoClass(void);

	// The following fields are only used by the Special_Render function.
	// this is basically just a place to stick whatever information you need.
	enum
	{
		RENDER_VIS,
		RENDER_SHADOW
	};
	int								RenderType;

	// RENDER_VIS variables and methods:
	VisRasterizerClass *			VisRasterizer;

	// RENDER_SHADOW variables and methods:
	// NOTE: this is somewhat obsolete now that we have hardware render-to-texture.
	BWRenderClass *				BWRenderer;					// Black & white non-textured renderer

private:

	// Not implemented...
	SpecialRenderInfoClass(const RenderInfoClass &);
	SpecialRenderInfoClass & operator = (const RenderInfoClass &);

};
