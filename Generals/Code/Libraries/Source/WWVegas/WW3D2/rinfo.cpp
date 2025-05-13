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
 *                     $Archive:: /Commando/Code/ww3d2/rinfo.cpp                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/24/01 3:31p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "rinfo.h"
#include "camera.h"
#include "matpass.h"


/***********************************************************************************************
**
** RenderInfoClass Implementation
**
***********************************************************************************************/
RenderInfoClass::RenderInfoClass(CameraClass & cam) :
	Camera(cam), 
	fog_start(0.0f),
	fog_end(0.0f),
	fog_scale(0.0f),
	light_environment(0),
	AdditionalMaterialPassCount(0),
	RejectedMaterialPasses(0),
	OverrideFlagLevel(0),
	Texture_Projector(NULL),
	alphaOverride(1.0f),
	materialPassAlphaOverride(1.0f),
	materialPassEmissiveOverride(1.0f)
{ 
	// Need to have one entry in the override flags stack, initialize it to default values.
	OverrideFlag[OverrideFlagLevel]=RINFO_OVERRIDE_DEFAULT;
}

RenderInfoClass::~RenderInfoClass(void)
{
}

void RenderInfoClass::Push_Material_Pass(MaterialPassClass * matpass)
{
	// add to the end of the array
	if (matpass) {
		matpass->Add_Ref();
	}
	WWASSERT(AdditionalMaterialPassCount<MAX_ADDITIONAL_MATERIAL_PASSES);
	AdditionalMaterialPassArray[AdditionalMaterialPassCount++]=matpass;
}

void RenderInfoClass::Pop_Material_Pass(void)
{
	// remove from the end of the array
	WWASSERT(AdditionalMaterialPassCount>0);
	AdditionalMaterialPassCount--;
	MaterialPassClass * mpass = AdditionalMaterialPassArray[AdditionalMaterialPassCount];
	if (mpass != NULL) {
		mpass->Release_Ref();
	}
}

int RenderInfoClass::Additional_Pass_Count(void)
{
	return AdditionalMaterialPassCount;
}

MaterialPassClass * RenderInfoClass::Peek_Additional_Pass(int i)
{
	return AdditionalMaterialPassArray[i];
}

void RenderInfoClass::Push_Override_Flags(RINFO_OVERRIDE_FLAGS flg)
{
	// copy to the end of the array
	WWASSERT(OverrideFlagLevel<MAX_OVERRIDE_FLAG_LEVEL);
	OverrideFlagLevel++;
	OverrideFlag[OverrideFlagLevel]=flg;
}

void RenderInfoClass::Pop_Override_Flags(void)
{
	WWASSERT(OverrideFlagLevel>0);
	OverrideFlagLevel--;
}

RenderInfoClass::RINFO_OVERRIDE_FLAGS & RenderInfoClass::Current_Override_Flags(void)
{
	return OverrideFlag[OverrideFlagLevel];
}



/***********************************************************************************************
**
** SpecialRenderInfoClass Implementation
**
***********************************************************************************************/

SpecialRenderInfoClass::SpecialRenderInfoClass(CameraClass & cam,int render_type) :
	RenderInfoClass(cam),
	RenderType(render_type),
	VisRasterizer(NULL),
	BWRenderer(NULL)
{
}

SpecialRenderInfoClass::~SpecialRenderInfoClass(void)
{
}

