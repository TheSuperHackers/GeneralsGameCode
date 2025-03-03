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
 *                     $Archive:: /VSS_Sync/ww3d2/shader.cpp                                  $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   ShaderClass::Init_From_Material3 -- Initialize a shader from a Material3 structure        *
 *   ShaderClass::Apply -- Apply the renderstates for this shader                              *
 *   ShaderClass::Invert_Backface_Culling -- Globally invert the sense of all backface culling *
 *   ShaderClass::Is_Backface_Culling_Inverted -- Is backface culling inverted?                *
 *   ShaderClass::Get_SS_Category -- Helper function for static sort system                    *
 *   ShaderClass::Guess_Sort_Level -- Guess the static sort level                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "shader.h"
#include "w3d_file.h"
#include "wwdebug.h"
#include "dx8wrapper.h"
#include "dx8caps.h"


bool ShaderClass::ShaderDirty=true;
unsigned long ShaderClass::CurrentShader=0;
unsigned long _PolygonCullMode = D3DCULL_CW;


/*
** Definitions of the preset shaders:
*/

// Texturing, zbuffer, primary gradient, no blending

#define SC_OPAQUE ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_ENABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, \
	DSTBLEND_ZERO, FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetOpaqueShader(SC_OPAQUE);

// Texturing, zbuffer, disabled zbuffer write, primary gradient, additive blending
#define SC_ADDITIVE ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, \
	DSTBLEND_ONE, FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetAdditiveShader(SC_ADDITIVE);

// Texturing, zbuffer, disabled zbuffer write, primary gradient, additive blending, bumpenvmap
#define SC_BUMPENVMAP ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, \
	DSTBLEND_ONE, FOG_DISABLE, GRADIENT_BUMPENVMAP, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_ADD, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetBumpenvmapShader(SC_BUMPENVMAP);

// Texturing, zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_SRC_ALPHA, \
	DSTBLEND_ONE_MINUS_SRC_ALPHA, FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetAlphaShader(SC_ALPHA);

// Texturing, zbuffer, disabled zbuffer write, primary gradient, multiplicative blending
#define SC_MULTIPLICATIVE ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ZERO, \
	DSTBLEND_SRC_COLOR, FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetMultiplicativeShader(SC_MULTIPLICATIVE);

// Texturing, no zbuffer reading/writing, no gradients, no blending, no
// fogging - mostly for opaque 2D objects.
#define SC_OP_2D ( SHADE_CNST(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, \
	DSTBLEND_ZERO, FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetOpaque2DShader(SC_OP_2D);

// Texturing, no zbuffer reading/writing, no gradients, additive blending,
// no fogging - mostly for additive 2D objects.
#define SC_ADD_2D ( SHADE_CNST(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, \
	DSTBLEND_ONE, FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetAdditive2DShader(SC_ADD_2D);

// Texturing, no zbuffer reading/writing, no gradients, alpha blending, no
// fogging - mostly for alpha-blended 2D objects.
#define SC_ALPHA_2D ( SHADE_CNST(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA, FOG_DISABLE, GRADIENT_DISABLE, \
	SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetAlpha2DShader(SC_ALPHA_2D);

// Texturing, default zbuffer reading, no zbuffer writing, no gradients,
// additive blending, no fogging - mostly for use in additive sprite
// objects.
#define SC_ADD_SPRITE ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_ONE, DSTBLEND_ONE, FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, \
	TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetAdditiveSpriteShader(SC_ADD_SPRITE);

// Texturing, default zbuffer reading, no zbuffer writing, no gradients,
// alpha blending, no fogging - mostly for use in alpha-blended sprite
// objects.
#define SC_ALPHA_SPRITE ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA, FOG_DISABLE, GRADIENT_DISABLE, \
	SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetAlphaSpriteShader(SC_ALPHA_SPRITE);

// No texturing, default zbuffer reading/writing, primary gradient, no
// blending, no fogging - mostly for use in solid-colored opaque objects.
#define SC_OPAQUE_SOLID ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_ENABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_ONE, DSTBLEND_ZERO, FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, \
	TEXTURING_DISABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetOpaqueSolidShader(SC_OPAQUE_SOLID);

// No texturing, default zbuffer reading, no zbuffer writing, primary
// gradient, additive blending, no fogging - mostly for use in solid-colored
// additive objects.
#define SC_ADDITIVE_SOLID ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_ONE, DSTBLEND_ONE, FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, \
	TEXTURING_DISABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetAdditiveSolidShader(SC_ADDITIVE_SOLID);

// No texturing, default zbuffer reading, no zbuffer writing, primary
// gradient, alpha blending, no fogging - mostly for use in solid-colored
// alpha objects.
#define SC_ALPHA_SOLID ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA, FOG_DISABLE, GRADIENT_MODULATE, \
	SECONDARY_GRADIENT_DISABLE, TEXTURING_DISABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetAlphaSolidShader(SC_ALPHA_SOLID);

// Texturing, no zbuffer reading/writing, no gradients, no blending, alpha
// testing, no fogging - mostly for "pure" alpha-tested 2D objects.
#define SC_ATEST_2D ( SHADE_CNST(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_ONE, DSTBLEND_ZERO, FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, \
	TEXTURING_ENABLE, ALPHATEST_ENABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetATest2DShader(SC_ATEST_2D);

// Texturing, default zbuffer reading and writing, no gradients, no
// blending, alpha testing, no fogging - mostly for "pure" alpha-tested
// sprite objects.
#define SC_ATEST_SPRITE ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_ENABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_ONE, DSTBLEND_ZERO, FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, \
	TEXTURING_ENABLE, ALPHATEST_ENABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetATestSpriteShader(SC_ATEST_SPRITE);

// Texturing, no zbuffer reading/writing, no gradients, alpha blending AND
// alpha testing, no fogging - mostly for alpha-tested and blended 2D
// objects.
#define SC_ATESTBLEND_2D ( SHADE_CNST(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA, FOG_DISABLE, GRADIENT_DISABLE, \
	SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_ENABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetATestBlend2DShader(SC_ATESTBLEND_2D);

// Texturing, default zbuffer reading and writing, no gradients, alpha
// blending AND alpha testing, no fogging - mostly for use in alpha-tested
// and blended sprite objects.
#define SC_ATESTBLEND_SPRITE ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_ENABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA, FOG_DISABLE, GRADIENT_DISABLE, \
	SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_ENABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetATestBlendSpriteShader(SC_ATESTBLEND_SPRITE);

// Texturing, no zbuffer reading/writing, no gradients, screen blending,
// no fogging - mostly for screen-blended 2D objects.
#define SC_SCREEN_2D ( SHADE_CNST(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, \
	DSTBLEND_ONE_MINUS_SRC_COLOR, FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, \
	TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetScreen2DShader(SC_SCREEN_2D);

// Texturing, default zbuffer reading, no zbuffer writing, no gradients,
// screen blending, no fogging - mostly for use in screen-blended sprite
// objects.
#define SC_SCREEN_SPRITE ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_ONE, DSTBLEND_ONE_MINUS_SRC_COLOR, FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, \
	TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetScreenSpriteShader(SC_SCREEN_SPRITE);

// Texturing, no zbuffer reading/writing, no gradients, multiplicative
// blending, no fogging - mostly for multiplicatively blended 2D objects.
#define SC_MUL_2D ( SHADE_CNST(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ZERO, \
	DSTBLEND_SRC_COLOR, FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetMultiplicative2DShader(SC_MUL_2D);

// Texturing, default zbuffer reading, no zbuffer writing, no gradients,
// multiplicative blending, no fogging - mostly for use in multiplicatively
// blended sprite objects.
#define SC_MUL_SPRITE ( SHADE_CNST(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_ZERO, DSTBLEND_SRC_COLOR, FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, \
	TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, \
	DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetMultiplicativeSpriteShader(SC_MUL_SPRITE);


/***********************************************************************************************
 * ShaderClass::Init_From_Material3 -- Initialize a shader from a Material3 structure          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/24/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void ShaderClass::Init_From_Material3(const W3dMaterial3Struct & mat3)
{
	if ( mat3.Attributes & W3DMATERIAL_USE_ALPHA ) {
		Set_Depth_Mask( DEPTH_WRITE_DISABLE );
		Set_Dst_Blend_Func( DSTBLEND_ONE_MINUS_SRC_ALPHA );
		Set_Src_Blend_Func( SRCBLEND_SRC_ALPHA );
	}
}


/***********************************************************************************************
 * ShaderClass::Enable_Fog -- Turn on fog for this shader.                                     *
 *                                                                                             *
 * Enable most appropriate fog mode (FOG_ENABLE, FOG_SCALE_FRAGMENT or FOG_WHITE) for given	  *
 *	source and destination blends.																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:																												  *
 *		05/02/00    IML : Created.																					  *	
 *=============================================================================================*/

void ShaderClass::Enable_Fog (const char *source)
{
	// Enable the appropriate fog mode based on shader's source and destination blends.
	switch (Get_Src_Blend_Func()) {

		case ShaderClass::SRCBLEND_ZERO:
			if (Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_SRC_COLOR) {
				Set_Fog_Func (ShaderClass::FOG_WHITE);
			} else {
				Report_Unable_To_Fog (source);
			}	
			break;

		case ShaderClass::SRCBLEND_ONE:
			switch (Get_Dst_Blend_Func()) {
				
				case ShaderClass::DSTBLEND_ZERO:							// Opaque.
					Set_Fog_Func (ShaderClass::FOG_ENABLE);
					break;

				case ShaderClass::DSTBLEND_ONE:							// Additive.
				case ShaderClass::DSTBLEND_ONE_MINUS_SRC_COLOR:		// Screen.
					Set_Fog_Func (ShaderClass::FOG_SCALE_FRAGMENT);
					break;

				default:
					Report_Unable_To_Fog (source);
					break;
			}
			break;

		case ShaderClass::SRCBLEND_SRC_ALPHA:
			if (Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA) {
				Set_Fog_Func (ShaderClass::FOG_ENABLE);
			} else {
				Report_Unable_To_Fog (source);
			}	
			break;

		case ShaderClass::SRCBLEND_ONE_MINUS_SRC_ALPHA:
			if (Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_SRC_ALPHA) {
				Set_Fog_Func (ShaderClass::FOG_ENABLE);
			} else {
				Report_Unable_To_Fog (source);
			}	
			break;
	}
}


/***********************************************************************************************
 * ShaderClass::Report_Unable_To_Fog --																		  *	
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:																												  *
 *		10/04/00    IML : Created.																					  *	
 *=============================================================================================*/
void ShaderClass::Report_Unable_To_Fog (const char *source)
{
	#ifdef WWDEBUG
	static unsigned _reportcount = 0;

	const char		*unabletofogtext		= "WARNING: Unable to fog shader in %s with given blending mode.\r\n";
	const char		*unabletofogmoretext = "WARNING: Unable to fog additional shaders (further warnings will be suppressed).\r\n";
	const unsigned  maxreportcount		= 10;

	// Limit the no. of warning messages to some practical maximum. Suppress all subsequent warnings.
	if (_reportcount < maxreportcount) {
		WWDEBUG_SAY ((unabletofogtext, source));
		_reportcount++;
	} else {
		if (_reportcount == maxreportcount) {
			WWDEBUG_SAY ((unabletofogmoretext));
			_reportcount++;
		}
	}
	#endif
}

class Blend
{
public:

	Blend(D3DBLEND f, bool ab)
	{
		func = f;
		useAlpha = ab;
	}

	D3DBLEND	func;
	bool		useAlpha;
};

const Blend srcBlendLUT[ShaderClass::SRCBLEND_MAX] = 
{
	Blend(D3DBLEND_ZERO, false),
	Blend(D3DBLEND_ONE, false),
	Blend(D3DBLEND_SRCALPHA, true),
 	Blend(D3DBLEND_DESTCOLOR, true)
};

const Blend dstBlendLUT[ShaderClass::DSTBLEND_MAX] = 
{
	Blend(D3DBLEND_ZERO, false),
	Blend(D3DBLEND_ONE, false),
 	Blend(D3DBLEND_SRCCOLOR, false),
 	Blend(D3DBLEND_INVSRCCOLOR, false),
 	Blend(D3DBLEND_SRCALPHA, true),
 	Blend(D3DBLEND_INVSRCALPHA, true)
};


/***********************************************************************************************
 * ShaderClass::Apply -- Apply the renderstates for this shader                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/24/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void ShaderClass::Apply()
{
	unsigned long diff;

	unsigned int TextureOpCaps=DX8Caps::Get_Default_Caps().TextureOpCaps;

	if (ShaderDirty)
	{
		diff=0xffffffff;
	}
	else
	{
		diff=CurrentShader^ShaderBits;
	}

	if(!diff) return;


	CurrentShader=ShaderBits;
	ShaderDirty=false;
	// COLOR MASK

	if(diff & (ShaderClass::MASK_COLORMASK | ShaderClass::MASK_SRCBLEND | ShaderClass::MASK_DSTBLEND | ShaderClass::MASK_ALPHATEST))
	{
		ULONG planeMask = 0xffffff;

		if(Get_Color_Mask() != ShaderClass::COLOR_WRITE_ENABLE)
			planeMask = 0;

		D3DBLEND	sf;
		D3DBLEND	df;
		bool	blendAlpha = false;

		if(!planeMask)
		{
			sf = D3DBLEND_ZERO;
			df = D3DBLEND_ONE;
		}
		else
		{
			sf = srcBlendLUT[ int(Get_Src_Blend_Func()) ].func;
			df = dstBlendLUT[ int(Get_Dst_Blend_Func()) ].func;
			blendAlpha |= srcBlendLUT[ int(Get_Src_Blend_Func()) ].useAlpha;
			blendAlpha |= dstBlendLUT[ int(Get_Dst_Blend_Func()) ].useAlpha;
		}

		BOOL blendOn = FALSE;

		if(sf != D3DBLEND_ONE || df != D3DBLEND_ZERO)
		{
			DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND,sf);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND,df);
			blendOn = TRUE;
		}
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE,blendOn);

		BOOL alphaTest = FALSE;

		if(Get_Alpha_Test() == ShaderClass::ALPHATEST_ENABLE)
		{
			unsigned char alphareference = 0x60;	// Alpha reference value that produces best results with mip-mapped textures.
			
			if(sf == D3DBLEND_INVSRCALPHA)
			{
				DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF,0xff - alphareference);
				DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAFUNC,D3DCMP_LESSEQUAL);
			}
			else
			{
				DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF,alphareference);
				DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAFUNC,D3DCMP_GREATEREQUAL);
			}
			blendAlpha = true;
			alphaTest = TRUE;
		}
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE,alphaTest);

		diff &= ~(ShaderClass::MASK_COLORMASK | ShaderClass::MASK_SRCBLEND | ShaderClass::MASK_DSTBLEND | ShaderClass::MASK_ALPHATEST);
		if(!diff)
			return;
	}

	if(diff & (ShaderClass::MASK_FOG))
	{
		// Whenever fog is enabled or disabled, the entire shader is invalidated. This is why we
		// can defer the "fog enabled" check inside the "fog settings changed" check.
		if (DX8Wrapper::Get_Fog_Enable()) {

			BOOL fm = FALSE;
			D3DCOLOR fogColor = DX8Wrapper::Get_Fog_Color();
			
			switch(Get_Fog_Func())
			{
			case ShaderClass::FOG_ENABLE:
				fm = TRUE;
				break;
			case ShaderClass::FOG_SCALE_FRAGMENT:
				fogColor = 0;	
				fm = TRUE;
				break;
			case ShaderClass::FOG_WHITE:
				fogColor = 0xffffff;
				fm = TRUE;
				break;
			case ShaderClass::FOG_DISABLE:
				fm = FALSE;
				break;
			}

			DX8Wrapper::Set_DX8_Render_State(D3DRS_FOGENABLE,fm);

			if(fm)
			{
				DX8Wrapper::Set_DX8_Render_State(D3DRS_FOGCOLOR,fogColor);
			}

		} else {
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FOGENABLE,FALSE);
		}
		
		diff &= ~(ShaderClass::MASK_FOG);
		if(!diff)
			return;
	}

	if(diff & (ShaderClass::MASK_PRIGRADIENT|ShaderClass::MASK_TEXTURING))
	{
		D3DTEXTUREOP	cOp = D3DTOP_SELECTARG1;
		D3DTEXTUREOP	aOp = D3DTOP_SELECTARG1;
		DWORD	cArg1 = D3DTA_DIFFUSE, cArg2 = D3DTA_DIFFUSE;
		DWORD	aArg1 = D3DTA_DIFFUSE, aArg2 = D3DTA_DIFFUSE;

		if(Get_Texturing() == ShaderClass::TEXTURING_ENABLE)
		{
			switch(Get_Primary_Gradient())
			{
			case ShaderClass::GRADIENT_DISABLE:
				//Decal
				cOp = D3DTOP_SELECTARG1;
				cArg1 = D3DTA_TEXTURE;
				cArg2 = D3DTA_CURRENT;
				aOp = D3DTOP_SELECTARG1;
				aArg1 = D3DTA_TEXTURE;
				aArg2 = D3DTA_CURRENT;
				break;
			default:
			case ShaderClass::GRADIENT_MODULATE:
				//Modulate Alpha
				cOp = D3DTOP_MODULATE;
				cArg1 = D3DTA_TEXTURE;
				cArg2 = D3DTA_DIFFUSE;
				aOp = D3DTOP_MODULATE;
				aArg1 = D3DTA_TEXTURE;
				aArg2 = D3DTA_DIFFUSE;
				break;
			case ShaderClass::GRADIENT_ADD:
				if(!(TextureOpCaps & D3DTEXOPCAPS_ADD))	
					cOp = D3DTOP_MODULATE;
				else
					cOp = D3DTOP_ADD;
				cArg1 = D3DTA_TEXTURE;
				cArg2 = D3DTA_DIFFUSE;
				aOp = D3DTOP_MODULATE;
				aArg1 = D3DTA_TEXTURE;
				aArg2 = D3DTA_DIFFUSE;
				break;
			// Bump map is a hack currently as we only have two stages in use!
			case ShaderClass::GRADIENT_BUMPENVMAP:
				if(TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP)
				{
					cOp=D3DTOP_BUMPENVMAP;
					cArg1=D3DTA_TEXTURE;
					cArg2=D3DTA_DIFFUSE;
					aOp = D3DTOP_DISABLE;
					aArg1 = D3DTA_TEXTURE;
					aArg2 = D3DTA_CURRENT;
				}
				break;

			// Bump map is a hack currently as we only have two stages in use!
			case ShaderClass::GRADIENT_BUMPENVMAPLUMINANCE:
				if(TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE)
				{
					cOp=D3DTOP_BUMPENVMAPLUMINANCE;
					cArg1=D3DTA_TEXTURE;
					cArg2=D3DTA_DIFFUSE;
					aOp = D3DTOP_DISABLE;
					aArg1 = D3DTA_TEXTURE;
					aArg2 = D3DTA_CURRENT;
				}
				break;

			// Bump map is a hack currently as we only have two stages in use!
			case ShaderClass::GRADIENT_DOTPRODUCT3:
				if(TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3)
				{
					cOp=D3DTOP_DOTPRODUCT3;
					cArg1=D3DTA_TEXTURE;
					cArg2=D3DTA_DIFFUSE;
					aOp = D3DTOP_DISABLE;
					aArg1 = D3DTA_TEXTURE;
					aArg2 = D3DTA_CURRENT;
				}
				break;
			}
		}
		else
		{
			switch(Get_Primary_Gradient())
			{
			case ShaderClass::GRADIENT_DISABLE:
				cOp = D3DTOP_DISABLE;
				cArg1 = D3DTA_TEXTURE;
				cArg2 = D3DTA_CURRENT;
				aOp = D3DTOP_DISABLE;
				aArg1 = D3DTA_TEXTURE;
				aArg2 = D3DTA_CURRENT;
				break;
			default:
			case ShaderClass::GRADIENT_MODULATE:
				cOp = D3DTOP_SELECTARG2;
				cArg1 = D3DTA_TEXTURE;
				cArg2 = D3DTA_DIFFUSE;
				aOp = D3DTOP_SELECTARG2;
				aArg1 = D3DTA_TEXTURE;
				aArg2 = D3DTA_DIFFUSE;
				break;
			case ShaderClass::GRADIENT_ADD:
				cOp = D3DTOP_SELECTARG2;
				cArg1 = D3DTA_TEXTURE;
				cArg2 = D3DTA_DIFFUSE;
				aOp = D3DTOP_SELECTARG2;
				aArg1 = D3DTA_TEXTURE;
				aArg2 = D3DTA_DIFFUSE;
				break;
			}
		}

		if (WW3D::Is_Coloring_Enabled())
		{
			cArg2=aArg2=D3DTA_TFACTOR;
			cOp=aOp=D3DTOP_SELECTARG2;
		}

		DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLOROP,cOp);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLORARG1,cArg1);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_COLORARG2,cArg2);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAOP,aOp);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAARG1,aArg1);
		DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_ALPHAARG2,aArg2);
		diff &= ~(ShaderClass::MASK_PRIGRADIENT);
	}

	if(diff & (ShaderClass::MASK_POSTDETAILCOLORFUNC|ShaderClass::MASK_TEXTURING))
	{
		D3DTEXTUREOP	cOp	= D3DTOP_DISABLE;
		DWORD			cArg1 = D3DTA_TEXTURE;
		DWORD			cArg2 = D3DTA_CURRENT;

		if(Get_Texturing()== ShaderClass::TEXTURING_ENABLE)
		{
			switch(Get_Post_Detail_Color_Func())
			{
			default:
			case ShaderClass::DETAILCOLOR_DISABLE:
				break;

			case ShaderClass::DETAILCOLOR_DETAIL:
				if(TextureOpCaps & D3DTEXOPCAPS_MODULATE)
				{
					cOp = D3DTOP_SELECTARG1;
					cArg1 = D3DTA_TEXTURE;
					cArg2 = D3DTA_CURRENT;
				}
				break;

			case ShaderClass::DETAILCOLOR_SCALE:
				if(TextureOpCaps & D3DTEXOPCAPS_MODULATE)
				{
					cOp = D3DTOP_MODULATE;
					cArg1 = D3DTA_TEXTURE;
					cArg2 = D3DTA_CURRENT;
				}
				break;

			case ShaderClass::DETAILCOLOR_INVSCALE:
				if(TextureOpCaps & D3DTEXOPCAPS_ADDSMOOTH)
				{
					cOp = D3DTOP_ADDSMOOTH;
					cArg1 = D3DTA_TEXTURE;
					cArg2 = D3DTA_CURRENT;
				}
				break;

			case ShaderClass::DETAILCOLOR_ADD:
				if(TextureOpCaps & D3DTEXOPCAPS_ADD)
				{
					cOp = D3DTOP_ADD;
					cArg1 = D3DTA_TEXTURE;
					cArg2 = D3DTA_CURRENT;
				}
				break;

			case ShaderClass::DETAILCOLOR_SUB:
				if(TextureOpCaps & D3DTEXOPCAPS_SUBTRACT)
				{
					cOp = D3DTOP_SUBTRACT;
					cArg1 = D3DTA_TEXTURE;
					cArg2 = D3DTA_CURRENT;
				}
				break;

			case ShaderClass::DETAILCOLOR_SUBR:
				if(TextureOpCaps & D3DTEXOPCAPS_SUBTRACT)
				{
					cOp = D3DTOP_SUBTRACT;
					cArg1 = D3DTA_CURRENT;
					cArg2 = D3DTA_TEXTURE;
				}
				break;

			case ShaderClass::DETAILCOLOR_BLEND:
				if(TextureOpCaps & D3DTEXOPCAPS_BLENDTEXTUREALPHA)
				{
					cOp = D3DTOP_BLENDTEXTUREALPHA;
					cArg1 = D3DTA_TEXTURE;
					cArg2 = D3DTA_CURRENT;
				}
				break;

			case ShaderClass::DETAILCOLOR_DETAILBLEND:
				if(TextureOpCaps & D3DTEXOPCAPS_BLENDCURRENTALPHA)
				{
					cOp = D3DTOP_BLENDCURRENTALPHA;
					cArg1 = D3DTA_TEXTURE;
					cArg2 = D3DTA_CURRENT;
				}
				break;
			}
		}
		DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_COLOROP,cOp);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_COLORARG1,cArg1);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_COLORARG2,cArg2);
	}
	diff &= ~(ShaderClass::MASK_POSTDETAILCOLORFUNC);

	if(diff & (ShaderClass::MASK_POSTDETAILALPHAFUNC|ShaderClass::MASK_TEXTURING))
	{
		D3DTEXTUREOP	aOp	= D3DTOP_DISABLE;
		DWORD			aArg1 = D3DTA_TEXTURE;
		DWORD			aArg2 = D3DTA_CURRENT;

		if(Get_Texturing() == ShaderClass::TEXTURING_ENABLE)
		{
			switch(Get_Post_Detail_Alpha_Func())
			{
			default:
			case ShaderClass::DETAILALPHA_DISABLE:
				break;

			case ShaderClass::DETAILALPHA_DETAIL:
				if(TextureOpCaps & D3DTEXOPCAPS_MODULATE)
				{
					aOp = D3DTOP_SELECTARG1;
					aArg1 = D3DTA_TEXTURE;
					aArg2 = D3DTA_CURRENT;
				}
				break;

			case ShaderClass::DETAILALPHA_SCALE:
				if(TextureOpCaps & D3DTEXOPCAPS_MODULATE)
				{
					aOp = D3DTOP_MODULATE;
					aArg1 = D3DTA_TEXTURE;
					aArg2 = D3DTA_CURRENT;
				}
				break;

			case ShaderClass::DETAILALPHA_INVSCALE:
				if(TextureOpCaps & D3DTEXOPCAPS_ADDSMOOTH)
				{
					aOp = D3DTOP_ADDSMOOTH;
					aArg1 = D3DTA_TEXTURE;
					aArg2 = D3DTA_CURRENT;
				}
				break;
			}
		}
		DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_ALPHAOP,aOp);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_ALPHAARG1,aArg1);
		DX8Wrapper::Set_DX8_Texture_Stage_State(1,D3DTSS_ALPHAARG2,aArg2);
	}
	diff &= ~(ShaderClass::MASK_POSTDETAILALPHAFUNC);
	diff &= ~(ShaderClass::MASK_TEXTURING);

	if(!diff)
		return;

	DX8Wrapper::Set_DX8_Render_State(D3DRS_SPECULARENABLE,BOOL(Get_Secondary_Gradient()));

	// DEPTH COMPARE FUNCTION
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC,D3DCMPFUNC(int(Get_Depth_Compare())+1));

	// DEPTH MASK
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE,BOOL(Get_Depth_Mask()));

	// DITHERING
//	DX8Wrapper::Set_DX8_Render_State(D3DRS_DITHERENABLE,BOOL(Get_Dither_Mask()));

	// CULLMODE
	DX8Wrapper::Set_DX8_Render_State(D3DRS_CULLMODE,Get_Cull_Mode() ? _PolygonCullMode : D3DCULL_NONE);

	// NPATCHES
	if (diff&ShaderClass::MASK_NPATCHENABLE) {
		float level=1.0f;
		if (Get_NPatch_Enable()) level=float(WW3D::Get_NPatches_Level());
		DX8Wrapper::Set_DX8_Render_State(D3DRS_PATCHSEGMENTS,*((DWORD*)&level));
	}

	// Enable/disable alpha test
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE,BOOL(Get_Alpha_Test()));	

	// Enable/disable stencil test
	// Not supported yet
}


/***********************************************************************************************
 * ShaderClass::Invert_Backface_Culling -- Globally invert the sense of all backface culling   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/24/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void ShaderClass::Invert_Backface_Culling(bool onoff)
{
	if (onoff == true) {
		_PolygonCullMode = D3DCULL_CCW;
	} else {
		_PolygonCullMode = D3DCULL_CW;
	}
	Invalidate();
}


/***********************************************************************************************
 * ShaderClass::Get_SS_Category -- Helper function for static sort system                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
ShaderClass::StaticSortCategoryType ShaderClass::Get_SS_Category(void) const
{
	// category: Opaque
	if ( (ALPHATEST_DISABLE==Get_Alpha_Test()) && (DSTBLEND_ZERO==Get_Dst_Blend_Func()) )
		return SSCAT_OPAQUE;
	// category: Alpha Test
	if (ALPHATEST_ENABLE==Get_Alpha_Test())	{
		if (DSTBLEND_ZERO==Get_Dst_Blend_Func()) return SSCAT_ALPHA_TEST;
		if ( (SRCBLEND_SRC_ALPHA==Get_Src_Blend_Func()) &&
			  (DSTBLEND_ONE_MINUS_SRC_ALPHA==Get_Dst_Blend_Func()) )
			  return SSCAT_ALPHA_TEST;
	}
	// category: Additive
	if ( (SRCBLEND_ONE==Get_Src_Blend_Func()) && (DSTBLEND_ONE==Get_Dst_Blend_Func()) )
		return SSCAT_ADDITIVE;
	// category: Everything else
	return SSCAT_OTHER;
}


/***********************************************************************************************
 * ShaderClass::Guess_Sort_Level -- Guess the static sort level                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/27/2001  hy : Created.                                                                  *
 *=============================================================================================*/
int ShaderClass::Guess_Sort_Level(void) const
{
	int sort_level;
	StaticSortCategoryType scat=Get_SS_Category();

	switch (scat)
	{
	case ShaderClass::SSCAT_OPAQUE:
	case ShaderClass::SSCAT_ALPHA_TEST:
		sort_level=SORT_LEVEL_NONE;
		break;
	case ShaderClass::SSCAT_ADDITIVE:
		sort_level=SORT_LEVEL_BIN3;
		break;
	default:
		sort_level=SORT_LEVEL_BIN1;
		break;
	}
	return sort_level;
}

/***********************************************************************************************
 * ShaderClass::Is_Backface_Culling_Inverted -- Is backface culling inverted?                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/24/2001  gth : Created.                                                                 *
 *=============================================================================================*/
bool ShaderClass::Is_Backface_Culling_Inverted(void)
{
	return (_PolygonCullMode == D3DCULL_CCW);
}
