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
 *                     $Archive:: /Commando/Code/ww3d2/texproject.cpp                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *								$Modtime:: 08/05/02 10:03a                                             $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 * 06/27/02 KM Render to shadow buffer texture support														*
 * 08/05/02 KM Texture class redesign
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   TexProjectClass::TexProjectClass -- Constructor                                           *
 *   TexProjectClass::~TexProjectClass -- Destructor                                           *
 *   TexProjectClass::Set_Texture_Size -- Set the size of texture to use                       *
 *   TexProjectClass::Get_Texture_Size -- Returns the stored texture size                      *
 *   TexProjectClass::Set_Flag -- Turn specified flag on or off                                *
 *   TexProjectClass::Get_Flag -- Get the current state of specified flag                      *
 *   TexProjectClass::Set_Intensity -- Set the intensity of this projector                     *
 *   TexProjectClass::Get_Intensity -- returns the current "desired" intensity                 *
 *   TexProjectClass::Set_Attenuation -- Set the attenuation factor                            *
 *   TexProjectClass::Get_Attenuation -- Returns the attenuation value                         *
 *   TexProjectClass::Enable_Attenuation -- Set the state of the ATTENUATE flag                *
 *   TexProjectClass::Is_Attenuation_Enabled -- Get the state of the ATTENUATE flag            *
 *   TexProjectClass::Is_Depth_Gradient_Enabled -- returns whether the depth gradient is enabl *
 *   TexProjectClass::Enable_Depth_Gradient -- enable/disable depth gradient                   *
 *   TexProjectClass::Init_Multiplicative -- Initialize this to be a multiplicative texture pr *
 *   TexProjectClass::Is_Intensity_Zero -- check if we can eliminate this projector            *
 *   TexProjectClass::Init_Additive -- Set up the projector to be additive                     *
 *   TexProjectClass::Set_Perspective_Projection -- Set up a perspective projection            *
 *   TexProjectClass::Set_Ortho_Projection -- Set up an orthographic projection                *
 *   TexProjectClass::Set_Texture -- Set the texture to be projected                           *
 *   TexProjectClass::Get_Texture -- Returns the texture being projected                       *
 *   TexProjectClass::Peek_Texture -- Returns the texture being projected                      *
 *   TexProjectClass::Peek_Material_Pass -- Returns the material pass object                   *
 *   TexProjectClass::Compute_Perspective_Projection -- Set up a perspective projection of an  *
 *   TexProjectClass::Compute_Perspective_Projection -- Set up a perspective projection of an  *
 *   TexProjectClass::Compute_Ortho_Projection -- Automatic Orthographic projection            *
 *   TexProjectClass::Compute_Ortho_Projection -- Automatic Orthographic projection            *
 *   TexProjectClass::Compute_Texture -- Generates texture by rendering an object              *
 *   TexProjectClass::Configure_Camera -- setup a camera to match this projector               *
 *   TexProjectClass::Pre_Render_Update -- Prepare the projector for rendering                 *
 *   TexProjectClass::Update_WS_Bounding_Volume -- Recalculate the world-space bounding box    *
 *   TexProjectClass::Get_Surface -- Returns pointer to the texture surface                    *
 *   TexProjectClass::Set_Render_Target -- Install a render target for this projector to use   *
 *   TexProjectClass::Needs_Render_Target -- returns wheter this projector needs a render targ *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "texproject.h"
#include "vertmaterial.h"
#include "shader.h"
#include "texture.h"
#include "rendobj.h"
#include "rinfo.h"
#include "camera.h"
#include "matpass.h"
#include "bwrender.h"
#include "assetmgr.h"
#include "dx8wrapper.h"


// DEBUG DEBUG
#include "MPU.H"

#define DEBUG_SHADOW_RENDERING					0
//#define DEFAULT_TEXTURE_SIZE						64

const float INTENSITY_RATE_OF_CHANGE			= 1.0f;			// change in intensity per second


/*
**
** Shadow mapping:  from pre-projected view coordinates back to world space
** then to shadow space
**
** (1) Vshadow = PShadow * Mwrld-shadow * Vwrld
**
** (2) Vview = Mwrld-camera * Vwrld
**
** Using (2) to solve for Vwrld in terms of Vview
**                -1
** (3) Mwrld-camera  * Vview = Vwrld
**
** Substituting (3) into (1)
**                                                   -1
** (4) Vshadow = Pshadow * Mwrld-shadow * Mwrld-camera * Vview
**
** ---------------------------------------------------------------------------------
**
** Shadow mapping: from pre-projected view space to world space, to shadow space,
** then projecting.
**
** (1) Vshadow = Mwrld-shadow * Vwrld
**
** (2) Vview = Mwrld-camera * Vwrld
**
** solving (2) for Vwrld:
**                -1
** (3) Mwrld-camera * Vview = Vwrld
**
** substitute into (1)
**                                         -1
** (4) Vshadow = Mwrld-shadow * Mwrld-camera * Vview
**
** project shadow
**                                                         -1
** (5) Vproj-shadow = Pshadow * (Mwrld-shadow * Mwrld-camera ) * Vview
**
** aaah! same thing :-)
**
** ---------------------------------------------------------------------------------
**
** Shadow mapping:  from pre-projected view coordinates back to obj space
** then to shadow space
**
** (1) Vshadow = PShadow * Mwrld-shadow * Mobj-wrld * Vobj
**
** (2) Vview = Mwrld-camera * Mobj-wrld * Vobj
**             -1             -1
** (3) Mobj-wrld * Mwrld-camera * Vview = Vobj
**                                                            -1             -1
** (4) Vshadow = PShadow * Mwrld-shadow * Mobj-wrld * Mobj-wrld * Mwrld-camera * Vview
**                                                   -1
** (5) Vshadow = PShadow * Mwrld-shadow * Mvrld-camera * Vview
**
**
** Ideas:
** - Use Texture Projectors to implement spot lights and stained glass windows
** - Attenuate texture projections with distance from the projector
** - Should be able to handle lots of pre-calculated static texture projectors.  They
**   should cull well and we can pre-generate the textures.
**
** Ideas maybe used in conjunction with texture projections:
** - Light volumes: the problem is when the volume is cliped it looks funny, we can
**   fill in the clipped face of a non-translucent object by rendering the backfaces
** - Use the backface-fill to make the camera slicing through the commando look like
**   its *really* slicing through the commando :-)
** - The back-face-fill trick might be able to use el-cheapo screen mapping!
**
*/



/***********************************************************************************************
 * TexProjectClass::TexProjectClass -- Constructor                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
TexProjectClass::TexProjectClass(void) :
	Flags(DEFAULT_FLAGS),
	DesiredIntensity(1.0f),
	Intensity(1.0f),
	Attenuation(1.0f),
	MaterialPass(NULL),
	Mapper1(NULL),
	RenderTarget(NULL),
	DepthStencilTarget(NULL),
	HFov(90.0f),
	VFov(90.0f),
	XMin(-10.0f),
	XMax(10.0f),
	YMin(-10.0f),
	YMax(10.0f),
	ZNear(1.0f),
	ZFar(1000.0f)
{
	// create a material pass class
	MaterialPass = NEW_REF(MaterialPassClass,());
	MaterialPass->Set_Cull_Volume(&WorldBoundingVolume);

	// create a vertex material
	VertexMaterialClass * vmtl = NEW_REF(VertexMaterialClass,());
	WWASSERT(vmtl != NULL);

	// Plug our parent's mapper into our vertex material
	// the mapper for stage1 will be allocated as needed
	vmtl->Set_Mapper(Mapper);

	MaterialPass->Set_Material(vmtl);
	vmtl->Release_Ref();
	vmtl = NULL;

	// by default init our material pass to be multiplicative (shadow)
	Init_Multiplicative();
}


/***********************************************************************************************
 * TexProjectClass::~TexProjectClass -- Destructor                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
TexProjectClass::~TexProjectClass(void)
{
	REF_PTR_RELEASE(Mapper1);
	REF_PTR_RELEASE(MaterialPass);
	REF_PTR_RELEASE(RenderTarget);
	REF_PTR_RELEASE(DepthStencilTarget);
}


/***********************************************************************************************
 * TexProjectClass::Set_Texture_Size -- Set the size of texture to use                         *
 *                                                                                             *
 *    This function stores the desired texture size in this TexProjectClass.  Note that        *
 *    this size is only used if you have the TexProjectClass generate a texture.               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *    size - dimension of the texture in pixels                                                *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Set_Texture_Size(int size)
{
	WWASSERT(size > 0);
	WWASSERT(size <= 512);
	Flags &= ~SIZE_MASK;
	Flags |= (size << SIZE_SHIFT);
}


/***********************************************************************************************
 * TexProjectClass::Get_Texture_Size -- Returns the stored texture size                        *
 *                                                                                             *
 *    Returns the size stored in the flags variable.  This can be different than the actual    *
 *    texture's size if you manually installed a texture.  This size is only used when         *
 *    automatically generating a texture as is the case when creating shadows.                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
int TexProjectClass::Get_Texture_Size(void)
{
	return (Flags & SIZE_MASK) >> SIZE_SHIFT;
}


/***********************************************************************************************
 * TexProjectClass::Set_Flag -- Turn specified flag on or off                                  *
 *                                                                                             *
 *    See the TexProjectClass header file for valid flags                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Set_Flag(uint32 flag,bool onoff)
{
	if (onoff) {
		Flags |= flag;
	} else {
		Flags &= ~flag;
	}
}


/***********************************************************************************************
 * TexProjectClass::Get_Flag -- Get the current state of specified flag                        *
 *                                                                                             *
 *    See the TexProjectClass header file for valid flags                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
bool TexProjectClass::Get_Flag(uint32 flag) const
{
	return (Flags & flag) == flag;
}


/***********************************************************************************************
 * TexProjectClass::Set_Intensity -- Set the intensity of this projector                       *
 *                                                                                             *
 *    The "intensity" is a value between 0 and 1.  At 0, the projector will have no effect     *
 *    At 1, the projector will be at its highest strength.  The intensity will automatically   *
 *    smoothly interpolate towards the value specified unless you use the immediate option.    *
 *                                                                                             *
 * INPUT:                                                                                      *
 * intensity - desired intensity, 0 <= intensity <= 1                                          *
 * immediate - should the intensity be immediately set to the desired value? (default = false) *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Set_Intensity(float intensity,bool immediate)
{
	WWASSERT(intensity <= 1.0f);
	WWASSERT(intensity >= 0.0f);

	DesiredIntensity = intensity;
	if (immediate) {
		Intensity = DesiredIntensity;
	}
}


/***********************************************************************************************
 * TexProjectClass::Get_Intensity -- returns the current "desired" intensity                   *
 *                                                                                             *
 *    This will return the last value that was sent into this object through Set_Intensity.    *
 *    Note however that the actual intensity used in rendering may not have arrived at         *
 *    that value yet.                                                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
float TexProjectClass::Get_Intensity(void)
{
	return DesiredIntensity;
}


/***********************************************************************************************
 * TexProjectClass::Is_Intensity_Zero -- check if we can eliminate this projector              *
 *                                                                                             *
 *    Only returns true if the current intensity is zero AND the target intensity is zero      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
bool TexProjectClass::Is_Intensity_Zero(void)
{
	return ((Intensity == 0.0f) && (DesiredIntensity == 0.0f));
}


/***********************************************************************************************
 * TexProjectClass::Set_Attenuation -- Set the attenuation factor                              *
 *                                                                                             *
 *    Attenuation scales the intensity.  I use attenuation to make shadows fade away as they   *
 *    get farther away from the light source or from the viewer.                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Set_Attenuation(float attenuation)
{
	WWASSERT(attenuation >= 0.0f);
	WWASSERT(attenuation <= 1.0f);
	Attenuation = attenuation;
}


/***********************************************************************************************
 * TexProjectClass::Get_Attenuation -- Returns the attenuation value                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
float TexProjectClass::Get_Attenuation(void)
{
	return Attenuation;
}


/***********************************************************************************************
 * TexProjectClass::Enable_Attenuation -- Set the state of the ATTENUATE flag                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Enable_Attenuation(bool onoff)
{
	Set_Flag(ATTENUATE,onoff);
}


/***********************************************************************************************
 * TexProjectClass::Is_Attenuation_Enabled -- Get the state of the ATTENUATE flag              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
bool TexProjectClass::Is_Attenuation_Enabled(void)
{
	return Get_Flag(ATTENUATE);
}


/***********************************************************************************************
 * TexProjectClass::Enable_Depth_Gradient -- enable/disable depth gradient                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Enable_Depth_Gradient(bool onoff)
{
	Set_Flag(USE_DEPTH_GRADIENT,onoff);

	// re-setup the shader settings
	if (Get_Flag(ADDITIVE)) {
		Init_Additive();
	} else {
		Init_Multiplicative();
	}
}


/***********************************************************************************************
 * TexProjectClass::Is_Depth_Gradient_Enabled -- returns whether the depth gradient is enabled *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool TexProjectClass::Is_Depth_Gradient_Enabled(bool onoff)
{
	return Get_Flag(USE_DEPTH_GRADIENT);
}

/***********************************************************************************************
 * TexProjectClass::Init_Multiplicative -- Initialize this to be a multiplicative texture proj *
 *                                                                                             *
 *    Set up the internal materials so that the texture is multiplied with whatever it is      *
 *    projected onto.                                                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/4/00     gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Init_Multiplicative(void)
{
	Set_Flag(ADDITIVE,false);

	/*
	** Set up the shader
	*/
	static ShaderClass mult_shader(		SHADE_CNST(	ShaderClass::PASS_LEQUAL,						//depth_compare,
																	ShaderClass::DEPTH_WRITE_DISABLE,			//depth_mask,
																	ShaderClass::COLOR_WRITE_ENABLE,				//color_mask,
																	ShaderClass::SRCBLEND_ZERO,					//src_blend,
																	ShaderClass::DSTBLEND_SRC_COLOR,				//dst_blend,
																	ShaderClass::FOG_DISABLE,						//fog,
																	ShaderClass::GRADIENT_ADD,						//pri_grad,
																	ShaderClass::SECONDARY_GRADIENT_DISABLE,	//sec_grad,
																	ShaderClass::TEXTURING_ENABLE,				//texture,

																	ShaderClass::ALPHATEST_DISABLE,				//alpha_test,
																	ShaderClass::CULL_MODE_ENABLE,				//cull mode
																	0,														//post_det_color,
																	0) );													//post_det_alpha

	if (WW3DAssetManager::Get_Instance()->Get_Activate_Fog_On_Load()) {
		mult_shader.Enable_Fog ("TexProjectClass");
	}

	if (Get_Flag(USE_DEPTH_GRADIENT)) {

		/*
		** enable multi-texturing
		*/
		mult_shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_ADD);

		/*
		** plug the gradient texture into the second stage
		*/
		TextureClass * grad_tex = WW3DAssetManager::Get_Instance()->Get_Texture("MultProjectorGradient.tga");
		if (grad_tex) {
			grad_tex->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
			grad_tex->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
			MaterialPass->Set_Texture(grad_tex,1);
			grad_tex->Release_Ref();
		} else {
			WWDEBUG_SAY(("Could not find texture: MultProjectorGradient.tga!"));
		}

	} else {

		/*
		** disable multi-texturing
		*/
		mult_shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_DISABLE);

		/*
		** remove the texture from the second stage
		*/
		MaterialPass->Set_Texture(NULL,1);
	}

#if (DEBUG_SHADOW_RENDERING)
	// invert the shader so we can see what polygons it is hitting
	mult_shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE);
	mult_shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ONE);
#endif

	MaterialPass->Set_Shader(mult_shader);

	/*
	** Set up the Vertex Material parameters
	*/
	VertexMaterialClass * vmtl = MaterialPass->Peek_Material();
	vmtl->Set_Ambient(0,0,0);
	vmtl->Set_Diffuse(0,0,0);
	vmtl->Set_Specular(0,0,0);
	vmtl->Set_Emissive(0.0f,0.0f,0.0f);
	vmtl->Set_Opacity(1.0f);
	vmtl->Set_Lighting(true); // I need the emissive value to scale the intensity of the shadow

	/*
	** Set up some mapper settings related to depth gradient
	*/
	if (Get_Flag(USE_DEPTH_GRADIENT)) {
		if (Mapper1 == NULL) {
			Mapper1 = NEW_REF(MatrixMapperClass,(1));
		}
		Mapper1->Set_Type(MatrixMapperClass::DEPTH_GRADIENT);
		vmtl->Set_Mapper(Mapper1,1);
	} else {
		vmtl->Set_Mapper(NULL,1);
	}
}


/***********************************************************************************************
 * TexProjectClass::Init_Additive -- Set up the projector to be additive                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Init_Additive(void)
{
	Set_Flag(ADDITIVE,true);

	/*
	** Set up the shader
	*/
	static ShaderClass add_shader(		SHADE_CNST(	ShaderClass::PASS_LEQUAL,						//depth_compare,
																	ShaderClass::DEPTH_WRITE_DISABLE,			//depth_mask,
																	ShaderClass::COLOR_WRITE_ENABLE,				//color_mask,
																	ShaderClass::SRCBLEND_ONE,						//src_blend,
																	ShaderClass::DSTBLEND_ONE,						//dst_blend,
																	ShaderClass::FOG_DISABLE,						//fog,
																	ShaderClass::GRADIENT_MODULATE,				//pri_grad,
																	ShaderClass::SECONDARY_GRADIENT_DISABLE,	//sec_grad,
																	ShaderClass::TEXTURING_ENABLE,				//texture,
																	ShaderClass::ALPHATEST_DISABLE,				//alpha_test,
																	ShaderClass::CULL_MODE_ENABLE,				//cullmode,
																	ShaderClass::DETAILCOLOR_DISABLE,			//post_det_color,
																	ShaderClass::DETAILALPHA_DISABLE) );		//post_det_alpha

	if (WW3DAssetManager::Get_Instance()->Get_Activate_Fog_On_Load()) {
		add_shader.Enable_Fog ("TexProjectClass");
	}

	/*
	** Additive projectors always use the normal gradient so they need multi-texturing
	*/
	add_shader.Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_SCALE);

	/*
	** plug in the gradient texture
	*/
	TextureClass * grad_tex = WW3DAssetManager::Get_Instance()->Get_Texture("AddProjectorGradient.tga");
	if (grad_tex) {
		grad_tex->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		grad_tex->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		MaterialPass->Set_Texture(grad_tex,1);
		grad_tex->Release_Ref();
	} else {
		WWDEBUG_SAY(("Could not find texture: AddProjectorGradient.tga!"));
	}

#if (DEBUG_SHADOW_RENDERING)
	// invert the shader so we can see what polygons it is hitting
	add_shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_SRC_COLOR);
	add_shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ZERO);
#endif

	MaterialPass->Set_Shader(add_shader);

	/*
	** Set up the Vertex Material parameters
	*/
	VertexMaterialClass * vmtl = MaterialPass->Peek_Material();
	vmtl->Set_Ambient(0,0,0);
	vmtl->Set_Diffuse(0,0,0);
	vmtl->Set_Specular(0,0,0);
	vmtl->Set_Emissive(1,1,1);
	vmtl->Set_Opacity(1.0f);
	vmtl->Set_Lighting(true); //need emissive to scale the intensity of the projector

	/*
	** Set up some mapper settings related to depth gradient
	** Additive texture projections always use the normal gradient
	*/
	if (Mapper1 == NULL) {
		Mapper1 = NEW_REF(MatrixMapperClass,(1));
	}
	Mapper1->Set_Type(MatrixMapperClass::NORMAL_GRADIENT);
	vmtl->Set_Mapper(Mapper1,1);
}


/***********************************************************************************************
 * TexProjectClass::Set_Texture -- Set the texture to be projected                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Set_Texture(TextureClass * texture)
{
	if (texture != NULL)
	{
		texture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		texture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		MaterialPass->Set_Texture(texture);
	}
}


/***********************************************************************************************
 * TexProjectClass::Get_Texture -- Returns the texture being projected                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * the pointer is ref-counted!  make sure to keep track of your references                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
TextureClass * TexProjectClass::Get_Texture(void) const
{
	return MaterialPass->Get_Texture();
}


/***********************************************************************************************
 * TexProjectClass::Peek_Texture -- Returns the texture being projected                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
TextureClass * TexProjectClass::Peek_Texture(void) const
{
	return MaterialPass->Peek_Texture();
}


/***********************************************************************************************
 * TexProjectClass::Peek_Material_Pass -- Returns the material pass object                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
MaterialPassClass * TexProjectClass::Peek_Material_Pass(void)
{
	return MaterialPass;
}


/***********************************************************************************************
 * TexProjectClass::Set_Perspective_Projection -- set up a perspective projection              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/27/00    gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Set_Perspective_Projection(float hfov,float vfov,float znear,float zfar)
{
	HFov = hfov;
	VFov = vfov;
	ZNear = znear;
	ZFar = zfar;

	ProjectorClass::Set_Perspective_Projection(hfov,vfov,znear,zfar);
	Set_Flag(PERSPECTIVE,true);
}


/***********************************************************************************************
 * TexProjectClass::Set_Ortho_Projection -- set up an orthographic projection                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/27/00    gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Set_Ortho_Projection(float xmin,float xmax,float ymin,float ymax,float znear,float zfar)
{
	XMin = xmin;
	XMax = xmax;
	YMin = ymin;
	YMax = ymax;
	ZNear = znear;
	ZFar = zfar;

	ProjectorClass::Set_Ortho_Projection(xmin,xmax,ymin,ymax,znear,zfar);
	Set_Flag(PERSPECTIVE,false);
}

/***********************************************************************************************
 * TexProjectClass::Compute_Perspective_Projection -- Set up a perspective projection of an ob *
 *                                                                                             *
 * This function automates the process of generating the perspective parameters needed to      *
 * tightly bound the projection of an object.                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 * model - object which we are created a projection of                                         *
 * lightpos - positional light source                                                          *
 * znear - distance to near clipping plane for the projection (if -1.0, will be generated)     *
 * zfar - distance to far clipping plane for the projection (if -1.0, will be generated)       *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool TexProjectClass::Compute_Perspective_Projection
(
	RenderObjClass *	model,
	const Vector3 &	lightpos,
	float					znear,
	float					zfar
)
{
	if (model == NULL) {
		WWDEBUG_SAY(("Attempting to generate projection for a NULL model"));
		return false;
	}

	AABoxClass box;
	model->Get_Obj_Space_Bounding_Box(box);
	const Matrix3D & tm = model->Get_Transform();

	return Compute_Perspective_Projection(box,tm,lightpos,znear,zfar);
}


/***********************************************************************************************
 * TexProjectClass::Compute_Perspective_Projection -- Set up a perspective projection of an ob *
 *                                                                                             *
 * This function automates the process of generating the perspective parameters needed to      *
 * tightly bound the projection of an object.                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 * obj_box - object space bounding box of the object we are projecting                         *
 * tm - transform of the object we are projecting                                              *
 * lightpos - positional light source                                                          *
 * user_znear - distance to near clipping plane for the projection (if -1.0, will be generated)*
 * user_zfar - distance to far clipping plane for the projection (if -1.0, will be generated)  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool TexProjectClass::Compute_Perspective_Projection
(
	const AABoxClass & obj_box,
	const Matrix3D & tm,
	const Vector3 & lightpos,
	float user_znear,
	float user_zfar
)
{
	/*
	** Compute the center of the box in world-space
	*/
	Vector3 wrld_center;
	Matrix3D::Transform_Vector(tm,obj_box.Center,&wrld_center);

	/*
	** Create a camera transform looking at the object.
	** Set this as our transform later in this routine.
	*/
	Matrix3D texture_tm,texture_tm_inv;
	texture_tm.Look_At(lightpos,wrld_center,0.0f);
	texture_tm.Get_Orthogonal_Inverse(texture_tm_inv);

	/*
	** Calculate the axis-aligned bounding box of the model in the camera's coordinate system.
	*/
	AABoxClass box = obj_box;
	Matrix3D obj_to_world = tm;
	Matrix3D obj_to_texture;
	Matrix3D::Multiply(texture_tm_inv,obj_to_world,&obj_to_texture);
	box.Transform(obj_to_texture);

	/*
	** If the box is behind the viewpoint or the viewpoint is inside the box
	** our FOV will be > 180 degrees. Have to give up
	*/
	if ((box.Center.Z > 0.0f) || (box.Extent.Z > WWMath::Fabs(box.Center.Z))) {
		return false;
	}

	/*
	** Compute the frustum parameters. Remember that our z coordinates are negative but the
	** projection code needs positive z distances.
	*/
	float znear = -box.Center.Z; //-(box.Center.Z + obj_box.Extent.Quick_Length());
	float zfar = -(box.Center.Z - obj_box.Extent.Quick_Length()) * 2.0f;

	if (user_znear != -1.0f) {
		znear = box.Center.Z + user_znear;
	}
	if (user_zfar != -1.0f) {
		zfar = box.Center.Z + user_zfar;
	}

	float tan_hfov2 = WWMath::Fabs(box.Extent.X / (box.Center.Z + box.Extent.Z));
	float tan_vfov2 = WWMath::Fabs(box.Extent.Y / (box.Center.Z + box.Extent.Z));
	float hfov = 2.0f * WWMath::Atan(tan_hfov2);
	float vfov = 2.0f * WWMath::Atan(tan_vfov2);

	/*
	** Plug in the results.
	*/
	Set_Perspective_Projection(hfov,vfov,znear,zfar);
	Set_Transform(texture_tm);
	return true;
}


/***********************************************************************************************
 * TexProjectClass::Compute_Ortho_Projection -- Automatic Orthographic projection              *
 *                                                                                             *
 * Generates the orthographic projection parameters to tightly bound an object                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 * model - object which we are created a projection of                                         *
 * lightdir - directional light source                                                         *
 * znear - distance to near clipping plane for the projection (if -1.0, will be generated)     *
 * zfar - distance to far clipping plane for the projection (if -1.0, will be generated)       *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool TexProjectClass::Compute_Ortho_Projection
(
	RenderObjClass *	model,
	const Vector3 &	lightdir,
	float					znear,
	float					zfar
)
{
	if (model == NULL) {
		WWDEBUG_SAY(("Attempting to generate projection for a NULL model"));
		return false;
	}

	AABoxClass box;
	model->Get_Obj_Space_Bounding_Box(box);
	const Matrix3D & tm = model->Get_Transform();

	return Compute_Ortho_Projection(box,tm,lightdir,znear,zfar);
}


/***********************************************************************************************
 * TexProjectClass::Compute_Ortho_Projection -- Automatic Orthographic projection              *
 *                                                                                             *
 * Generates the orthographic projection parameters to tightly bound an object                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 * obj_box - object space bounding box of the object we are projecting                         *
 * tm - transform of the object we are projecting                                              *
 * lightdir - directional light                                                                *
 * user_znear - distance to near clipping plane for the projection (if -1.0, will be generated)*
 * user_zfar - distance to far clipping plane for the projection (if -1.0, will be generated)  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool TexProjectClass::Compute_Ortho_Projection
(
	const AABoxClass & obj_box,
	const Matrix3D & tm,
	const Vector3 & lightdir,
	float user_znear,
	float user_zfar
)
{
	/*
	** Compute the center of the box in world-space
	*/
	AABoxClass wrldbox = obj_box;
	wrldbox.Transform(tm);

	/*
	** Create a camera transform looking at the object.
	** Set this as our transform later in this routine.
	*/
	Vector3 camera_target = wrldbox.Center;
	Vector3 camera_position = camera_target - 2.0f * wrldbox.Extent.Length() * lightdir;

	Matrix3D texture_tm,texture_tm_inv;
	texture_tm.Look_At(camera_position,camera_target,0.0f);
	texture_tm.Get_Orthogonal_Inverse(texture_tm_inv);

	/*
	** Calculate the axis-aligned bounding box of the model in the camera's coordinate system.
	*/
	AABoxClass box = obj_box;
	Matrix3D obj_to_world = tm;
	Matrix3D obj_to_texture;
	Matrix3D::Multiply(texture_tm_inv,obj_to_world,&obj_to_texture);
	box.Transform(obj_to_texture);

	/*
	** Expand the box to help with bounding box errors
	*/
	box.Extent *= 1.0f;

	/*
	** Compute the frustum parameters.  Note that znear and zfar are supposed to
	** be positive distances for the projection code.
	*/
	float znear = -box.Center.Z; //-(box.Center.Z + obj_box.Extent.Quick_Length());
	float zfar = -(box.Center.Z - obj_box.Extent.Quick_Length()) * 2.0f;

	if (user_znear != -1.0f) {
		znear = -box.Center.Z + user_znear;
	}
	if (user_zfar != -1.0f) {
		zfar = -box.Center.Z + user_zfar;
	}

	/*
	** All done!
	*/
	Set_Ortho_Projection(	box.Center.X - box.Extent.X,
									box.Center.X + box.Extent.X,
									box.Center.Y - box.Extent.Y,
									box.Center.Y + box.Extent.Y,
									znear,
									zfar	);
	Set_Transform(texture_tm);
	return true;
}

/***********************************************************************************************
 * TexProjectClass::Compute_Texture -- Generates texture by rendering an object                *
 *                                                                                             *
 * INPUT:                                                                                      *
 * model - pointer to the render object to generate a shadow texture for                       *
 * context - shadow render context which has been initialized to <TextureSize> resolution      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *   5/16/02    kjm : Added optional custom depth/stencil target										  *
 *=============================================================================================*/
bool TexProjectClass::Compute_Texture
(
	RenderObjClass * model,
	SpecialRenderInfoClass * context
)
{
	if ((model == NULL) || (context == NULL))
	{
		return false;
	}
	/*
	** Render to texture
	*/
	TextureClass * rtarget=NULL;
	ZTextureClass* ztarget=NULL;

	Peek_Render_Target(&rtarget,&ztarget);

	if (rtarget != NULL)
	{
		// set projector for render context KJM
		context->Texture_Projector=this;

		/*
		** Set the render target
		*/
		DX8Wrapper::Set_Render_Target_With_Z (rtarget,ztarget);

		/*
		** Set up the camera
		*/
		Configure_Camera(context->Camera);

		/*
		** Render the object
		*/
		Vector3 color(0.0f,0.0f,0.0f);
		if (Get_Flag(ADDITIVE) == false) {
			color.Set(1.0f,1.0f,1.0f);
		}

		bool zclear=ztarget!=NULL;

		bool snapshot=WW3D::Is_Snapshot_Activated();
		SNAPSHOT_SAY(("TexProjectCLass::Begin_Render()"));
		WW3D::Begin_Render(true,zclear,color);	// false to zclear as we don't have z-buffer
		WW3D::Render(*model,*context);
		SNAPSHOT_SAY(("TexProjectCLass::End_Render()"));
		WW3D::End_Render(false);
		WW3D::Activate_Snapshot(snapshot);	// End_Render() ends the shapsnot, so restore the state

		DX8Wrapper::Set_Render_Target((IDirect3DSurface8 *)NULL);

	}

#if 0

	/*
	** Render the object with the BW Renderer into our color surface
	*/
	BWRenderClass bwr((unsigned char*)shadow_surface->getDataPtr(),tex_size);
	bwr.Fill(0xff);
	context->BWRenderer = &bwr;
	model->Special_Render(*context);
	context->BWRenderer = NULL;
#endif
	return true;
}


/***********************************************************************************************
 * TexProjectClass::Needs_Render_Target -- returns wheter this projector needs a render target *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/17/2001  gth : Created.                                                                 *
 *=============================================================================================*/
bool TexProjectClass::Needs_Render_Target(void)
{
	return Get_Flag(TEXTURE_DIRTY);
}


/***********************************************************************************************
 * TexProjectClass::Set_Render_Target -- Install a render target for this projector to use     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/16/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Set_Render_Target
(
	TextureClass* render_target,
	ZTextureClass* zbuffer
)
{
	REF_PTR_SET(RenderTarget,render_target);
	Set_Texture(RenderTarget);
	REF_PTR_SET(DepthStencilTarget,zbuffer);
}

/***********************************************************************************************
 * TexProjectClass::Peek_Render_Target -- Returns pointer to the render target if we have one  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/5/2001   gth : Created.                                                                 *
 *   5/16/2002  kjm : Added optional custom zbuffer                                            *
 *=============================================================================================*/
TextureClass* TexProjectClass::Peek_Render_Target
(
	TextureClass** rtarget,
	ZTextureClass** ztarget
)
{
	// some uses of this function just want to know if a render target exists
	if (rtarget==NULL) return RenderTarget;

	*rtarget=RenderTarget;

	// don't set if pointer isn't supplied
	if (ztarget!=NULL)
		*ztarget=DepthStencilTarget;

	return RenderTarget;
}

/***********************************************************************************************
 * TexProjectClass::Configure_Camera -- set up a camera to match this projector                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Configure_Camera(CameraClass & camera)
{
	camera.Set_Transform(Transform);
	camera.Set_Clip_Planes(0.01f,ZFar);

	if (Get_Flag(PERSPECTIVE)) {
		camera.Set_Projection_Type(CameraClass::PERSPECTIVE);
		camera.Set_View_Plane(HFov,VFov);

	} else {
		camera.Set_Projection_Type(CameraClass::ORTHO);
		camera.Set_View_Plane(Vector2(XMin,YMin),Vector2(XMax,YMax));
	}

	// Set one-pixel borders to the texture to avoid "flooding" shadows...
	float size=Get_Texture_Size();
	float inv_size=1.0f/size;
	Vector2 vmin(1.0f*inv_size,1.0f*inv_size);
	Vector2 vmax((size-1.0f)*inv_size,(size-1.0f)*inv_size);
	camera.Set_Viewport(vmin,vmax);
}


/***********************************************************************************************
 * TexProjectClass::Pre_Render_Update -- Prepare the projector for rendering                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Pre_Render_Update(const Matrix3D & camera)
{
	/*
	** Mview-texture = PShadow * Mwrld-texture * Mcamera-vrld
	*/
	Matrix3D		world_to_texture;
	Matrix3D		tmp;
	Matrix4x4	view_to_texture;

	Transform.Get_Orthogonal_Inverse(world_to_texture);
	Matrix3D::Multiply(world_to_texture,camera,&tmp);
	Matrix4x4::Multiply(Projection,tmp,&view_to_texture);

	/*
	** update the current intensity by iterating it towards the desired intensity
	*/
	float frame_time = (float)WW3D::Get_Sync_Frame_Time() / 1000.0f;
	float intensity_delta = DesiredIntensity - Intensity;
	float max_intensity_delta = INTENSITY_RATE_OF_CHANGE * frame_time;

	if (intensity_delta > max_intensity_delta) {
		Intensity += max_intensity_delta;
	} else if (intensity_delta < -max_intensity_delta) {
		Intensity -= max_intensity_delta;
	} else {
		Intensity = DesiredIntensity;
	}

	float actual_intensity = Intensity * Attenuation;

	/*
	** install the current intensity
	*/
	VertexMaterialClass * vmat = MaterialPass->Peek_Material();
	if (Get_Flag(ADDITIVE)) {
		vmat->Set_Emissive(actual_intensity,actual_intensity,actual_intensity);
	} else {
		vmat->Set_Emissive(1.0f - actual_intensity,1.0f - actual_intensity,1.0f - actual_intensity);
	}

	/*
	** update the mappers
	*/
	if (Get_Flag(PERSPECTIVE)) {
		Mapper->Set_Type(MatrixMapperClass::PERSPECTIVE_PROJECTION);
	} else {
		Mapper->Set_Type(MatrixMapperClass::ORTHO_PROJECTION);
	}

	if (Get_Texture_Size() == 0) {
//		SurfaceClass::SurfaceDescription surface_desc;
//		MaterialPass->Peek_Texture()->Get_Level_Description(surface_desc);
		Set_Texture_Size(MaterialPass->Peek_Texture()->Get_Width());
		WWASSERT(Get_Texture_Size() != 0);
	}

	Mapper->Set_Texture_Transform(view_to_texture,Get_Texture_Size());
	if (Mapper1) {
		Mapper1->Set_Texture_Transform(view_to_texture,Get_Texture_Size());
	}
}


/***********************************************************************************************
 * TexProjectClass::Update_WS_Bounding_Volume -- Recalculate the world-space bounding box      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
void TexProjectClass::Update_WS_Bounding_Volume(void)
{
	ProjectorClass::Update_WS_Bounding_Volume();

	/*
	** Tell our culling system that we've changed
	*/
	Vector3 extent;
	WorldBoundingVolume.Compute_Axis_Aligned_Extent(&extent);
	Set_Cull_Box(AABoxClass(WorldBoundingVolume.Center,extent));
}

