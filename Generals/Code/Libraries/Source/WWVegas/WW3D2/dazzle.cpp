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
 *                     $Archive:: /Commando/Code/ww3d2/dazzle.cpp                             $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                       $Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 32                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dazzle.h"
#include "simplevec.h"
#include "vector2.h"
#include "camera.h"
#include "ww3d.h"
#include "wwstring.h"
#include "wwdebug.h"
#include "assetmgr.h"
#include "Vector3i.h"
#include "quat.h"
#include "INI.h"
#include "Point.h"
#include "rinfo.h"
#include "vertmaterial.h"
#include "chunkio.h"
#include "WWFILE.h"
#include "inisup.h"
#include "persistfactory.h"
#include "ww3dids.h"
#include "dx8wrapper.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "sortingrenderer.h"
#include "texture.h"
#include "scene.h"
#include "wwprofile.h"
#include <limits.h>


// All dazzle types appear under Dazzles_List in the dazzle.ini file.
const char* DAZZLE_LIST_STRING="Dazzles_List";

// After the dot product between the camera direction and the camera space location of the light source,
// we do a power to Halopow for the halo size and power to DazzlePow for the dazzle size. Halo/DazzleArea
// defines the angle where the values are valid, so any angle beyond HaloArea/DazzleArea results the halo or
// dazzle being scaled to zero. The angles are re-scaled from normalized angle scale of (-1.0...1.0)
// only HaloArea/DazzleArea defined part is used.
const char* HALO_INTENSITY_POW_STRING = "HaloIntensityPow";		// 1.0 would be linear fadeout, smaller than that will steepen the curve (smaller hotspot)
const char* HALO_SIZE_POW_STRING =	"HaloSizePow";		// 1.0 would be linear fadeout, smaller than that will steepen the curve (smaller hotspot)
const char* DAZZLE_INTENSITY_POW_STRING = "DazzleIntensityPow";
const char* DAZZLE_SIZE_POW_STRING = "DazzleSizePow";
const char* HALO_AREA_STRING =		"HaloArea";
const char* DAZZLE_AREA_STRING =		"DazzleArea";		// Something like 0.05 is a good starting point for a dazzle...

// Halo and dazzle are scaled by these values
const char* HALO_SCALE_X_STRING =	"HaloScaleX";
const char* HALO_SCALE_Y_STRING =	"HaloScaleY";
const char* DAZZLE_SCALE_X_STRING =	"DazzleScaleX";
const char* DAZZLE_SCALE_Y_STRING =	"DazzleScaleY";

// Dazzle and halo intensity control the base intensities.
const char* HALO_INTENSITY_STRING =	"HaloIntensity";
const char* DAZZLE_INTENSITY_STRING =	"DazzleIntensity";

// Direction area defines the maximum difference between the light direction and the eyespace location,
// so the dazzle can only be seen if the camera is inside the light cone. Value 0.0 means dazzle is not
// directional, it can be seen from any direction. Halo is not affected. Dazzle direction defines the light
// direction vector.
const char* DAZZLE_DIRECTION_AREA_STRING = "DazzleDirectionArea"; // Something like 0.5 might be a good test value
const char* DAZZLE_DIRECTION_STRING	= "DazzleDirection";

// Dazzle and halo start to afde out after FadeoutStart meters and are completely gone by FadeoutEnd meters.
const char* FADEOUT_START_STRING =	"FadeoutStart";
const char* FADEOUT_END_STRING =		"FadeoutEnd";

// To be done: This parameter needs to be redefined.
const char* SIZE_OPTIMIZATION_LIMIT_STRING = "SizeOptimizationLimit";

// The weight of history for the intensities. The history weight is per millisecond, so if you want to have
// any real history, values higher than 0.95 are recommended (don't use value of 1.0 or anything higher!)
const char* HISTORY_WEIGHT_STRING =	"HistoryWeight";

// Textures to be used.
const char* DAZZLE_TEXTURE_STRING = "DazzleTextureName";
const char* HALO_TEXTURE_STRING = "HaloTextureName";

// If false, camera matrix's translation term isn't used. If translation isn't used the dazzle is treated as
// always visible, scene graph visibility is not used.
const char* USE_CAMERA_TRANSLATION = "UseCameraTranslation";

// Halo and dazzle colors, RGB, (1.0, 1.0, 1.0) is white and (0.0, 0.0, 0.0) is black
const char* HALO_COLOR_STRING = "HaloColor";
const char* DAZZLE_COLOR_STRING = "DazzleColor";

// Dazzle test color could be white in many cases but if the level has a lot of white then another color can be defined.
const char* DAZZLE_TEST_COLOR_STRING = "DazzleTestColor";

// Dazzles can blink on and off.  The blink period defines the time for one cycle, the blink "on time" defines
// the amount of time within that period that the dazzle is "on"
const char* BLINK_PERIOD_STRING = "BlinkPeriod";
const char* BLINK_ON_TIME_STRING = "BlinkOnTime";

// Dazzle can use a lensflare by defining a name of lensflare in the ini
const char* DAZZLE_LENSFLARE_STRING = "LensflareName";

// Lensflare definitions
const char* LENSFLARE_LIST_STRING = "Lensflares_List";

// Texture to be used by the lensflare
const char* LENSFLARE_TEXTURE_STRING = "TextureName";

// The number of flare sprites in lensflare. The FlareLocation, FlareSize and FlareColor paremeters are procedural, the
// names are produced based on this parameter. If FlareCount is 3, there exists FlareLocation1, FlareLocation2 and
// FlareLocation3... And so on.
const char* FLARE_COUNT_STRING = "FlareCount";

// 1D-locations of the flares. 0.0 means the location of the center of the screen and 1.0 means the location of the
// lightsource. The values can be smaller than zero and larger than 1.0.
const char* FLARE_LOCATION_STRING = "FlareLocation";

// Normalized flare sizes
const char* FLARE_SIZE_STRING = "FlareSize";

// Colors for each flare sprite
const char* FLARE_COLOR_STRING = "FlareColor";

// Uv coordinates (as there can be only one texture used, but one may want multiple images). The values is a 4-float
// vector: start_u, start_v, end_u, end_v.
const char* FLARE_UV_STRING = "FlareUV";

// Radius of dazzle used for raycast tests
const char * RADIUS_STRING = "Radius";


/*
; DAZZLE.INI example

[Lensflares_List]
0=DEFAULT_LENSFLARE

[Dazzles_List]
0=DEFAULT

;========================================== LENSFLARE DEFINITIONS =================

[DEFAULT_LENSFLARE]
TextureName=dazzle_secondary.jpg
FlareCount=5
FlareLocation1=0.1
FlareLocation2=0.5
FlareLocation3=1.1
FlareLocation4=1.5
FlareLocation5=-0.4
FlareSize1=0.1
FlareSize2=0.05
FlareSize3=0.08
FlareSize4=0.15
FlareSize5=0.10
FlareColor1=1.0,0.5,0.5
FlareColor2=0.5,1.0,0.5
FlareColor3=0.5,0.5,1.0
FlareColor4=0.5,0.5,1.0
FlareColor5=0.5,0.5,1.0


;========================================== DAZZLE DEFINITIONS ====================

[DEFAULT]
HaloIntensity=1.0
HaloIntensityPow=0.95
HaloSizePow=0.95
HaloArea=1.0
HaloScaleX=0.2
HaloScaleY=0.2
DazzleArea=0.05
DazzleDirectionArea=0
DazzleDirection=0,1,1
DazzleSizePow=0.9
DazzleIntensityPow=0.9
DazzleIntensity=50
DazzleScaleX=1.0
DazzleScaleY=1.0
FadeoutStart=30.0
FadeoutEnd=40.0
SizeOptimizationLimit=0.05
HistoryWeight=0.975
UseCameraTranslation=1
DazzleTextureName=dazzle_primary.jpg
HaloTextureName=dazzle_secondary.jpg
DazzleColor=1,1,1
HaloColor=0,0,1
DazzleTestColor=1,1,1
LensflareName=DEFAULT_LENSFLARE
*/

// Global instance of a dazzle loader
DazzleLoaderClass		_DazzleLoader;

static SimpleVecClass<DazzleRenderObjClass*> temp_ptrs;

static DazzleTypeClass** types;
static unsigned type_count;

// Current dazzle layer - must be set before rendering
static DazzleLayerClass * current_dazzle_layer = nullptr;

static LensflareTypeClass** lensflares;
static unsigned lensflare_count;

static ShaderClass default_dazzle_shader;
static ShaderClass default_halo_shader;
static ShaderClass vis_shader;
static ShaderClass debug_shader;

// static instance of a default dazzle visibility handler
static DazzleVisibilityClass		_DefaultVisibilityHandler;
static const DazzleVisibilityClass *	_VisibilityHandler = &_DefaultVisibilityHandler;


static void Init_Shaders()
{
	default_dazzle_shader.Set_Cull_Mode( ShaderClass::CULL_MODE_DISABLE );
	default_dazzle_shader.Set_Depth_Mask( ShaderClass::DEPTH_WRITE_DISABLE );
	default_dazzle_shader.Set_Depth_Compare( ShaderClass::PASS_ALWAYS );
	default_dazzle_shader.Set_Dst_Blend_Func( ShaderClass::DSTBLEND_ONE );
	default_dazzle_shader.Set_Src_Blend_Func( ShaderClass::SRCBLEND_ONE );
	default_dazzle_shader.Set_Fog_Func( ShaderClass::FOG_DISABLE );
	default_dazzle_shader.Set_Primary_Gradient( ShaderClass::GRADIENT_MODULATE );
	default_dazzle_shader.Set_Texturing( ShaderClass::TEXTURING_ENABLE );

	default_halo_shader.Set_Cull_Mode( ShaderClass::CULL_MODE_DISABLE );
	default_halo_shader.Set_Depth_Mask( ShaderClass::DEPTH_WRITE_DISABLE );
	default_halo_shader.Set_Depth_Compare( ShaderClass::PASS_LEQUAL );
	default_halo_shader.Set_Dst_Blend_Func( ShaderClass::DSTBLEND_ONE );
	default_halo_shader.Set_Src_Blend_Func( ShaderClass::SRCBLEND_ONE );
	default_halo_shader.Set_Fog_Func( ShaderClass::FOG_DISABLE );
	default_halo_shader.Set_Primary_Gradient( ShaderClass::GRADIENT_MODULATE );
	default_halo_shader.Set_Texturing( ShaderClass::TEXTURING_ENABLE );

	vis_shader.Set_Cull_Mode( ShaderClass::CULL_MODE_DISABLE );
	vis_shader.Set_Depth_Mask( ShaderClass::DEPTH_WRITE_DISABLE );
	vis_shader.Set_Dst_Blend_Func( ShaderClass::DSTBLEND_ZERO );
	vis_shader.Set_Src_Blend_Func( ShaderClass::SRCBLEND_ONE );
	vis_shader.Set_Fog_Func( ShaderClass::FOG_DISABLE );
	vis_shader.Set_Primary_Gradient( ShaderClass::GRADIENT_MODULATE );
	vis_shader.Set_Texturing( ShaderClass::TEXTURING_DISABLE );

	debug_shader.Set_Cull_Mode( ShaderClass::CULL_MODE_DISABLE );
	debug_shader.Set_Depth_Mask( ShaderClass::DEPTH_WRITE_DISABLE );
	debug_shader.Set_Dst_Blend_Func( ShaderClass::DSTBLEND_SRC_ALPHA );
	debug_shader.Set_Src_Blend_Func( ShaderClass::SRCBLEND_ONE );
	debug_shader.Set_Fog_Func( ShaderClass::FOG_DISABLE );
	debug_shader.Set_Primary_Gradient( ShaderClass::GRADIENT_MODULATE );
	debug_shader.Set_Texturing( ShaderClass::TEXTURING_DISABLE );

}

/*
** Derived INI to support Vector4
*/
class DazzleINIClass : public INIClass {
public:
	DazzleINIClass(FileClass & file) : INIClass(file) {}
	const Vector2	Get_Vector2(char const *section, char const *entry, const Vector2 & defvalue = Vector2(0,0) );
	const Vector3	Get_Vector3(char const *section, char const *entry, const Vector3 & defvalue = Vector3(0,0,0) );
	const Vector4	Get_Vector4(char const *section, char const *entry, const Vector4 & defvalue = Vector4(0,0,0,0) ) const;
};

const Vector2 DazzleINIClass::Get_Vector2(char const *section, char const *entry, const Vector2 & defvalue)
{
	if (section != nullptr && entry != nullptr) {
		INIEntry * entryptr = Find_Entry(section, entry);
		if (entryptr && entryptr->Value != nullptr) {
			Vector2	ret;
			if ( sscanf( entryptr->Value, "%f,%f", &ret[0], &ret[1] ) == 2 ) {
				return ret;
			}
		}
	}
	return defvalue;
}

const Vector3 DazzleINIClass::Get_Vector3(char const *section, char const * entry, const Vector3 & defvalue )
{
	if (section != nullptr && entry != nullptr) {
		INIEntry * entryptr = Find_Entry(section, entry);
		if (entryptr && entryptr->Value != nullptr) {
			Vector3	ret;
			if ( sscanf( entryptr->Value, "%f,%f,%f", &ret[0], &ret[1], &ret[2] ) == 3 ) {
				return ret;
			}
		}
	}
	return defvalue;
}

const Vector4 DazzleINIClass::Get_Vector4(char const *section, char const *entry, const Vector4 & defvalue) const
{
	if (section != nullptr && entry != nullptr) {
		INIEntry * entryptr = Find_Entry(section, entry);
		if (entryptr && entryptr->Value != nullptr) {
			Vector4	ret;
			if ( sscanf( entryptr->Value, "%f,%f,%f,%f", &ret[0], &ret[1], &ret[2], &ret[3] ) == 4 ) {
				return ret;
			}
		}
	}
	return defvalue;
}

// ----------------------------------------------------------------------------

LensflareTypeClass::LensflareTypeClass(const LensflareInitClass& is)
	:
	lic(is),
	texture(nullptr)
{
}

LensflareTypeClass::~LensflareTypeClass()
{
	REF_PTR_RELEASE(texture);
}

TextureClass* LensflareTypeClass::Get_Texture()
{
	if (!texture) {
		texture = WW3DAssetManager::Get_Instance()->Get_Texture(lic.texture_name);
		SET_REF_OWNER(texture);
	}
	return texture;
}

void LensflareTypeClass::Generate_Vertex_Buffers(
	VertexFormatXYZNDUV2* vertex,
	int& vertex_count,
	float screen_x_scale,
	float screen_y_scale,
	float dazzle_intensity,
	const Vector4& transformed_location)
{
// Lensflares are placed on a line 2D that goes through the lightsource and the center of the screen
//	float scale=sqrt(transformed_location[0]*transformed_location[0]+transformed_location[1]*transformed_location[1]);

	float z=transformed_location[2];

	float distance_multiplier=sqrt(transformed_location[0]*transformed_location[0]+transformed_location[1]*transformed_location[1])+1.0f;

	for (int a=0;a<lic.flare_count;++a) {
		float x=lic.flare_locations[a]*transformed_location[0];
		float y=lic.flare_locations[a]*transformed_location[1];
		float size=lic.flare_sizes[a]*distance_multiplier;
		float ix=size*screen_x_scale;
		float iy=size*screen_y_scale;

		Vector3 col=lic.flare_colors[a]*dazzle_intensity;
		if (col[0]>1.0f) col[0]=1.0f;
		if (col[1]>1.0f) col[1]=1.0f;
		if (col[2]>1.0f) col[2]=1.0f;
		unsigned color=DX8Wrapper::Convert_Color(col,1.0f);

		vertex->x=x+ix;
		vertex->y=y-iy;
		vertex->z=z;
		vertex->u1=lic.flare_uv[a][0];
		vertex->v1=lic.flare_uv[a][1];
		vertex->diffuse=color;
		vertex++;

		vertex->x=x+ix;
		vertex->y=y+iy;
		vertex->z=z;
		vertex->u1=lic.flare_uv[a][2];
		vertex->v1=lic.flare_uv[a][1];
		vertex->diffuse=color;
		vertex++;

		vertex->x=x-ix;
		vertex->y=y+iy;
		vertex->z=z;
		vertex->u1=lic.flare_uv[a][2];
		vertex->v1=lic.flare_uv[a][3];
		vertex->diffuse=color;
		vertex++;

		vertex->x=x-ix;
		vertex->y=y-iy;
		vertex->z=z;
		vertex->u1=lic.flare_uv[a][0];
		vertex->v1=lic.flare_uv[a][3];
		vertex->diffuse=color;
		vertex++;

		vertex_count+=4;
	}
}

// ----------------------------------------------------------------------------

DazzleTypeClass::DazzleTypeClass(const DazzleInitClass& is)
	:
	ic(is),
	dazzle_shader(default_dazzle_shader),
	halo_shader(default_halo_shader),
	primary_texture(nullptr),
	secondary_texture(nullptr),
	lensflare_id(DazzleRenderObjClass::Get_Lensflare_ID(is.lensflare_name)),
	radius(is.radius)
{
	fadeout_end_sqr=ic.fadeout_end*ic.fadeout_end;
	fadeout_start_sqr=ic.fadeout_start*ic.fadeout_start;
	dazzle_test_color_integer=
		0xff000000|
		(unsigned(255.0f*ic.dazzle_test_color[2])<<16)|
		(unsigned(255.0f*ic.dazzle_test_color[1])<<8)|
		(unsigned(255.0f*ic.dazzle_test_color[0]));
	dazzle_test_mask_integer=dazzle_test_color_integer&0xf8f8f8f8;
}

DazzleTypeClass::~DazzleTypeClass()
{
	REF_PTR_RELEASE(primary_texture);
	REF_PTR_RELEASE(secondary_texture);
}

// ----------------------------------------------------------------------------

void DazzleTypeClass::Set_Dazzle_Shader(const ShaderClass& s)	// Set shader for the dazzle type
{
	dazzle_shader=s;
}

// ----------------------------------------------------------------------------

void DazzleTypeClass::Set_Halo_Shader(const ShaderClass& s)	// Set shader for the dazzle type
{
	halo_shader=s;
}

// ----------------------------------------------------------------------------

TextureClass* DazzleTypeClass::Get_Dazzle_Texture()
{
	if (!primary_texture) {
		primary_texture = WW3DAssetManager::Get_Instance()->Get_Texture(ic.primary_texture_name);
		SET_REF_OWNER(primary_texture);
	}
	return primary_texture;
}

// ----------------------------------------------------------------------------

TextureClass* DazzleTypeClass::Get_Halo_Texture()
{
	if (!secondary_texture) {
		secondary_texture = WW3DAssetManager::Get_Instance()->Get_Texture(ic.secondary_texture_name);
		SET_REF_OWNER(secondary_texture);
	}
	return secondary_texture;
}

// ----------------------------------------------------------------------------

void DazzleTypeClass::Calculate_Intensities(
	float& dazzle_intensity,
	float& dazzle_size,
	float& halo_intensity,
	const Vector3& camera_dir,
	const Vector3& dazzle_dir,
	float distance) const
{
	if (ic.use_camera_translation && distance>(fadeout_end_sqr)) {
		dazzle_intensity=0.0f;
		return;
	}

	dazzle_intensity-=1.0f-ic.dazzle_area;
	dazzle_intensity/=ic.dazzle_area;
	WWMath::Clamp(dazzle_intensity);

	if (ic.dazzle_direction_area>0.0f) {
		float angle=-Vector3::Dot_Product(camera_dir,dazzle_dir);
		angle-=1.0f-ic.dazzle_direction_area;
		angle/=ic.dazzle_direction_area;
		WWMath::Clamp(angle);
		dazzle_intensity*=angle;
	}

	if (dazzle_intensity>0.0f) {
		dazzle_size=pow(dazzle_intensity,ic.dazzle_size_pow);
		dazzle_intensity=pow(dazzle_intensity,ic.dazzle_intensity_pow);
	}
	else {
		dazzle_intensity=0.0f;
	}

	dazzle_intensity*=ic.dazzle_intensity;
	halo_intensity*=ic.halo_intensity;

	// Flare fadeout distance - fade it out...
	if (ic.use_camera_translation) {
		if (distance>(fadeout_start_sqr)) {
			distance=sqrt(distance);
			distance-=ic.fadeout_start;
			distance/=ic.fadeout_end-ic.fadeout_start;
			dazzle_intensity*=1.0f-distance;	// Scale down intensity
			halo_intensity*=1.0f-distance;	// Scale down intensity
		}
	}

	return;
}

// ----------------------------------------------------------------------------

void DazzleRenderObjClass::Init_From_INI(const INIClass* ini)
{
	if (!ini) return;
	if (ini->Section_Count()==0) return;

	Init_Shaders();

	unsigned count=ini->Entry_Count( LENSFLARE_LIST_STRING );
	unsigned entry;
	for ( entry = 0; entry < count; entry++ )	{
		char	section_name[80];
		ini->Get_String(
			LENSFLARE_LIST_STRING,
			ini->Get_Entry( LENSFLARE_LIST_STRING, entry),
			"",
			section_name,
			sizeof( section_name ));

		LensflareInitClass lic;

		ini->Get_String(lic.texture_name,section_name,LENSFLARE_TEXTURE_STRING);
		lic.flare_count=ini->Get_Int(section_name,FLARE_COUNT_STRING,0);
		if (lic.flare_count) {
			lic.flare_locations=W3DNEWARRAY float[lic.flare_count];	// Flare location is 1D
			lic.flare_sizes=W3DNEWARRAY float[lic.flare_count];
			lic.flare_colors=W3DNEWARRAY Vector3[lic.flare_count];
			lic.flare_uv=W3DNEWARRAY Vector4[lic.flare_count];
		}
		else {
			lic.flare_locations=nullptr;
			lic.flare_sizes=nullptr;
			lic.flare_colors=nullptr;
		}

		for (int flare=0;flare<lic.flare_count;++flare) {
			// Flare location
			StringClass tmp_name(true);
			tmp_name.Format("%s%d",FLARE_LOCATION_STRING,flare+1);
			lic.flare_locations[flare]=ini->Get_Float(section_name,tmp_name,0.0f);
			// Flare size
			tmp_name.Format("%s%d",FLARE_SIZE_STRING,flare+1);
			lic.flare_sizes[flare]=ini->Get_Float(section_name,tmp_name,1.0f);
			// Flare color
			tmp_name.Format("%s%d",FLARE_COLOR_STRING,flare+1);
			TPoint3D<float> tp=ini->Get_Point(section_name,tmp_name,TPoint3D<float>(1.0f,1.0f,1.0f));
			lic.flare_colors[flare]=reinterpret_cast<const Vector3&>(tp);
			// Flare uv
			const DazzleINIClass* dini=reinterpret_cast<const DazzleINIClass*>(ini);
			tmp_name.Format("%s%d",FLARE_UV_STRING,flare+1);
			lic.flare_uv[flare]=dini->Get_Vector4(section_name,tmp_name,Vector4(0.0f,0.0f,1.0f,1.0f));
		}
		lic.type=entry;

		Init_Lensflare(lic);
		lensflares[entry]->name=section_name;
	}

	count=ini->Entry_Count( DAZZLE_LIST_STRING );
	for ( entry = 0; entry < count; entry++ )	{
		char	section_name[80];
		ini->Get_String(
			DAZZLE_LIST_STRING,
			ini->Get_Entry( DAZZLE_LIST_STRING, entry),
			"",
			section_name,
			sizeof( section_name ));

		DazzleInitClass dic;

		ini->Get_String(dic.primary_texture_name,section_name,DAZZLE_TEXTURE_STRING);
		ini->Get_String(dic.secondary_texture_name,section_name,HALO_TEXTURE_STRING);
		dic.halo_intensity=ini->Get_Float(section_name,HALO_INTENSITY_STRING,0.95f);
		dic.halo_intensity_pow=ini->Get_Float(section_name,HALO_INTENSITY_POW_STRING,0.95f);
		dic.halo_size_pow=ini->Get_Float(section_name,HALO_SIZE_POW_STRING,0.95f);
		dic.halo_area=ini->Get_Float(section_name,HALO_AREA_STRING,0.2f);
		dic.halo_scale_x=ini->Get_Float(section_name,HALO_SCALE_X_STRING,2.0f);
		dic.halo_scale_y=ini->Get_Float(section_name,HALO_SCALE_Y_STRING,2.0f);
		dic.dazzle_area=ini->Get_Float(section_name,DAZZLE_AREA_STRING,0.05f);
		dic.dazzle_direction_area=ini->Get_Float(section_name,DAZZLE_DIRECTION_AREA_STRING,0.50f);
		dic.dazzle_intensity=ini->Get_Float(section_name,DAZZLE_INTENSITY_STRING,0.90f);
		dic.dazzle_intensity_pow=ini->Get_Float(section_name,DAZZLE_INTENSITY_POW_STRING,0.90f);
		dic.dazzle_size_pow=ini->Get_Float(section_name,DAZZLE_SIZE_POW_STRING,0.90f);
		dic.dazzle_scale_x=ini->Get_Float(section_name,DAZZLE_SCALE_X_STRING,100.0f);
		dic.dazzle_scale_y=ini->Get_Float(section_name,DAZZLE_SCALE_Y_STRING,25.0f);
		dic.fadeout_start=ini->Get_Float(section_name,FADEOUT_START_STRING,25.0f);
		dic.fadeout_end=ini->Get_Float(section_name,FADEOUT_END_STRING,50.0f);
		dic.size_optimization_limit=ini->Get_Float(section_name,SIZE_OPTIMIZATION_LIMIT_STRING,0.05f);
		dic.history_weight=ini->Get_Float(section_name,HISTORY_WEIGHT_STRING,0.5f);
		dic.use_camera_translation=!!ini->Get_Int(section_name,USE_CAMERA_TRANSLATION,1);
		ini->Get_String(dic.lensflare_name,section_name,DAZZLE_LENSFLARE_STRING);
		dic.type=entry;

		TPoint3D<float> tp=ini->Get_Point(section_name,DAZZLE_DIRECTION_STRING,TPoint3D<float>(0.0f,0.0f,0.0f));
		tp.Normalize();
		dic.dazzle_direction=reinterpret_cast<const Vector3&>(tp);

		tp=ini->Get_Point(section_name,DAZZLE_TEST_COLOR_STRING,TPoint3D<float>(1.0f,1.0f,1.0f));
		dic.dazzle_test_color=reinterpret_cast<const Vector3&>(tp);

		tp=ini->Get_Point(section_name,DAZZLE_COLOR_STRING,TPoint3D<float>(1.0f,1.0f,1.0f));
		dic.dazzle_color=reinterpret_cast<const Vector3&>(tp);

		tp=ini->Get_Point(section_name,HALO_COLOR_STRING,TPoint3D<float>(1.0f,1.0f,1.0f));
		dic.halo_color=reinterpret_cast<const Vector3&>(tp);

		dic.radius=ini->Get_Float(section_name,RADIUS_STRING,1.0f);

		dic.blink_period=ini->Get_Float(section_name,BLINK_PERIOD_STRING,0.0f);
		dic.blink_on_time=ini->Get_Float(section_name,BLINK_ON_TIME_STRING,0.0f);

		Init_Type(dic);
		types[entry]->name=section_name;
	}
}

// ----------------------------------------------------------------------------

void DazzleRenderObjClass::Init_Type(const DazzleInitClass& i)
{
	Init_Shaders();

	if (i.type>=type_count) {
		unsigned new_count=i.type+1;
		DazzleTypeClass** new_types=W3DNEWARRAY DazzleTypeClass*[new_count];
		unsigned a=0;
		unsigned copy_count = min(type_count, new_count);
		for (;a<copy_count;++a) {
			new_types[a]=types[a];
		}
		for (;a<new_count;++a) {
			new_types[a]=nullptr;
		}
		delete[] types;
		types=new_types;
		type_count=new_count;
	}
	delete types[i.type];
	types[i.type]=W3DNEW DazzleTypeClass(i);
}

// ----------------------------------------------------------------------------

void DazzleRenderObjClass::Init_Lensflare(const LensflareInitClass& i)
{
	Init_Shaders();

	if (i.type>=lensflare_count) {
		unsigned new_count=i.type+1;
		LensflareTypeClass** new_lensflares=W3DNEWARRAY LensflareTypeClass*[new_count];
		unsigned a=0;
		unsigned copy_count = min(lensflare_count, new_count);
		for (;a<copy_count;++a) {
			new_lensflares[a]=lensflares[a];
		}
		for (;a<new_count;++a) {
			new_lensflares[a]=nullptr;
		}
		delete[] lensflares;
		lensflares=new_lensflares;
		lensflare_count=new_count;
	}
	delete lensflares[i.type];
	lensflares[i.type]=W3DNEW LensflareTypeClass(i);
}

// ----------------------------------------------------------------------------

void DazzleRenderObjClass::Deinit()
{
	// Deinit dazzle types
	if (types) {
		for (unsigned a=0;a<type_count;++a) {
			delete types[a];
		}
		delete[] types;
	}
	types=nullptr;
	type_count=0;

	// Deinit lensflare types
	if (lensflares) {
		for (unsigned a=0;a<lensflare_count;++a) {
			delete lensflares[a];
		}
		delete[] lensflares;
	}
	lensflares=nullptr;
	lensflare_count=0;

}

void DazzleRenderObjClass::Install_Dazzle_Visibility_Handler(const DazzleVisibilityClass * visibility_handler)
{
	if (visibility_handler == nullptr) {
		_VisibilityHandler = &_DefaultVisibilityHandler;
	} else {
		_VisibilityHandler = visibility_handler;
	}
}


// ----------------------------------------------------------------------------
//
// The parameter t defines the dazzle type. t has to be a valid type id in order
// for the dazzle to work.
//
// ----------------------------------------------------------------------------

DazzleRenderObjClass::DazzleRenderObjClass(unsigned t)
	:
	succ(nullptr),
	type(t),
	current_dazzle_intensity(0.0f),
	current_dazzle_size(0.0f),
	dazzle_color(1.0f,1.0f,1.0f),
	halo_color(1.0f,1.0f,1.0f),
	lensflare_intensity(1.0f),
	on_list(false),
	visibility(0.0f),
	radius(types[t]->radius)
{
	creation_time = WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------
//
// Creating a dazzle with its type name is a bit slower (due to scanning) but
// works equally to the creation with id.
//
// ----------------------------------------------------------------------------

DazzleRenderObjClass::DazzleRenderObjClass(const char * type_name)
	:
	succ(nullptr),
	type(Get_Type_ID(type_name)),
	current_dazzle_intensity(0.0f),
	current_dazzle_size(0.0f),
	dazzle_color(1.0f,1.0f,1.0f),
	halo_color(1.0f,1.0f,1.0f),
	lensflare_intensity(1.0f),
	on_list(false),
	visibility(0.0f),
	radius(types[Get_Type_ID(type_name)]->radius)
{
	creation_time = WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------
//
// Copy constructor creates an exact copy of the object, except for the visibility
// information, which is initialised with default values.
//
// ----------------------------------------------------------------------------

DazzleRenderObjClass::DazzleRenderObjClass(const DazzleRenderObjClass & src)
	:
	succ(nullptr),
	type(src.type),
	current_dazzle_intensity(src.current_dazzle_intensity),
	current_dazzle_size(src.current_dazzle_size),
	current_dir(src.current_dir),
	dazzle_color(src.dazzle_color),
	halo_color(src.halo_color),
	lensflare_intensity(src.lensflare_intensity),
	on_list(false),
	visibility(src.visibility),
	radius(src.radius)
{
	creation_time = WW3D::Get_Sync_Time();
}

DazzleRenderObjClass& DazzleRenderObjClass::operator = (const DazzleRenderObjClass & src)
{
	type=src.type;
	current_dir=src.current_dir;
	current_dazzle_intensity=src.current_dazzle_intensity;
	current_dazzle_size=src.current_dazzle_size;
	dazzle_color=src.dazzle_color;
	halo_color=src.halo_color;
	lensflare_intensity=src.lensflare_intensity;
	visibility=src.visibility;
	radius=src.radius;
	creation_time = WW3D::Get_Sync_Time();
	return *this;
}

void DazzleRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	sphere.Center.Set(0,0,0);
	sphere.Radius = radius;
}

void DazzleRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box.Center.Set(0,0,0);
	box.Extent.Set(radius,radius,radius);
}

void DazzleRenderObjClass::Set_Layer(DazzleLayerClass *layer)
{
	// Never insert a dazzle into a list if it already is on one - this caould create infinite
	// loops in the dazzle list.
	if (on_list) return;

	// This function sets the dazzle to be visible in the given layer.
	if (!layer) {
		WWASSERT(0);
		return;
	}

	if (type>=type_count) return;	// If the type doesn't exist the visibility can't be changed

	// Insert at the head of the layer's visible list
	succ = layer->visible_lists[type];
	layer->visible_lists[type] = this;
	on_list = true;
}

void DazzleRenderObjClass::Set_Current_Dazzle_Layer(DazzleLayerClass *layer)
{
	current_dazzle_layer = layer;
}

/////////////////////////////////////////////////////////////////////////////
// Render Object Interface
/////////////////////////////////////////////////////////////////////////////

RenderObjClass* DazzleRenderObjClass::Clone(void) const
{
	return NEW_REF(DazzleRenderObjClass, (*this));
}

// ----------------------------------------------------------------------------
//
// DazzleRenderObjClass's Render() function doesn't actually render the dazzle
// immediately but just sets the dazzle visible. This is due to the way the
// dazzle system works (the dazzles need to be rendered after everything else).
// Having the Render() function flag the visibility offers us the visibility
// functionality of the scene graph.
//
// ----------------------------------------------------------------------------
void DazzleRenderObjClass::Render(RenderInfoClass & rinfo)
{
	WWPROFILE("Dazzle::Render");
	if (Is_Not_Hidden_At_All()) {

		// First check if the dazzle is blinking and is "off"
		bool is_on = true;
		DazzleInitClass & ic = types[type]->ic;

		if (ic.blink_period > 0.0f) {
			float elapsed_time = ((float)(WW3D::Get_Sync_Time() - creation_time)) / 1000.0f;
			float wrapped_time = fmodf(elapsed_time,ic.blink_period);
			if (wrapped_time > ic.blink_on_time) {
				is_on = false;
			}
		}

		// Next, check visibility
		visibility = 1.0f;

		if (is_on == false) {
			visibility = 0.0f;
		} else {
//			Vector3 position;
//			Get_Transform().Get_Translation(&position);
//			visibility = _VisibilityHandler->Compute_Dazzle_Visibility(rinfo,this,position);

			Matrix4x4 view_transform,projection_transform;
			DX8Wrapper::Get_Transform(D3DTS_VIEW,view_transform);
			DX8Wrapper::Get_Transform(D3DTS_PROJECTION,projection_transform);
			Vector3 camera_loc(rinfo.Camera.Get_Position());
			Vector3 camera_dir(-view_transform[2][0],-view_transform[2][1],-view_transform[2][2]);

			Vector3 loc=Get_Position();
			transformed_loc=view_transform*loc;
			transformed_loc=projection_transform*transformed_loc;
			transformed_loc[0]/=transformed_loc[3];
			transformed_loc[1]/=transformed_loc[3];
			transformed_loc[2]/=transformed_loc[3];
			transformed_loc[3]=1.0f;
			current_vloc=Vector3(transformed_loc[0],transformed_loc[1],transformed_loc[2]);

			float dazzle_intensity;
			Vector3 dir;
			dir=camera_loc-loc;
			current_distance=dir.Length2();
			dir.Normalize();
			dazzle_intensity=-Vector3::Dot_Product(dir,camera_dir);
//			dazzle_intensity*=visibility;

			float dazzle_size;
			current_halo_intensity=1.0f;
			const DazzleTypeClass* params=types[type];
			params->Calculate_Intensities(dazzle_intensity,dazzle_size,current_halo_intensity,camera_dir,current_dir,current_distance);

			float time_ms=WW3D::Get_Logic_Frame_Time_Milliseconds();
			float weight=pow(params->ic.history_weight,time_ms);

			if (dazzle_intensity>0.0f) {
				visibility = _VisibilityHandler->Compute_Dazzle_Visibility(rinfo,this,loc);
//visibility=1.0f;
				dazzle_intensity*=visibility;
			}
			else {
				visibility=0.0f;
			}
			if (visibility == 0.0f) {

				float i=dazzle_intensity*(1.0f-weight)+current_dazzle_intensity*weight;
				current_dazzle_intensity=i;
				if (current_dazzle_intensity<0.05f) current_dazzle_intensity=0.0f;
				dazzle_intensity=i;

				float s=dazzle_size*(1.0f-weight)+current_dazzle_size*weight;
				current_dazzle_size=s;
				dazzle_size=s;

			} else {
				current_dazzle_intensity = dazzle_intensity;
				current_dazzle_size = dazzle_size;
			}

		}

		// If this dazzle is visible or it is currently fading, submit it for rendering
		if (visibility > 0.0f || current_dazzle_intensity>0.0f) {
			WWASSERT(types[type]);
			Set_Layer(current_dazzle_layer);
		}
	}
	else {
		visibility=0.0f;
	}
}

void DazzleRenderObjClass::Render_Dazzle(CameraClass* camera)
{
	Matrix4x4 old_view_transform;
	Matrix4x4 old_world_transform;
	Matrix4x4 old_projection_transform;
	Matrix4x4 view_transform;
	Matrix4x4 world_transform;
	Matrix4x4 projection_transform;
	DX8Wrapper::Get_Transform(D3DTS_VIEW,view_transform);
	DX8Wrapper::Get_Transform(D3DTS_WORLD,world_transform);
	DX8Wrapper::Get_Transform(D3DTS_PROJECTION,projection_transform);
	old_view_transform=view_transform;
	old_world_transform=world_transform;
	old_projection_transform=projection_transform;
	Vector3 camera_loc(camera->Get_Position());
	Vector3 camera_dir(-view_transform[2][0],-view_transform[2][1],-view_transform[2][2]);

	int display_width,display_height,display_bits;
	bool windowed;
	WW3D::Get_Device_Resolution(display_width,display_height,display_bits,windowed);
	float w=float(display_width);
	float h=float(display_height);
	float screen_x_scale=1.0f;
	float screen_y_scale=1.0f;
	if (w>h) {
		screen_y_scale=w/h;
	}
	else {
		screen_x_scale=h/w;
	}

	float halo_scale_x=types[type]->ic.halo_scale_x;
	float halo_scale_y=types[type]->ic.halo_scale_y;
	float dazzle_scale_x=types[type]->ic.dazzle_scale_x;
	float dazzle_scale_y=types[type]->ic.dazzle_scale_y;

	// Allocate some arrays for the dazzle rendering
	int vertex_count=4;

	const DazzleTypeClass* params=types[type];

	int halo_vertex_count=0;
	int dazzle_vertex_count=0;
	int lensflare_vertex_count=0;

	Vector3 dl;

	int lens_max_verts=0;
	LensflareTypeClass* lensflare = DazzleRenderObjClass::Get_Lensflare_Class(types[type]->lensflare_id);
	if (lensflare) {
		lens_max_verts=4*lensflare->lic.flare_count;
	}

	DynamicVBAccessClass vb_access(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,vertex_count*2+lens_max_verts);
	{
		DynamicVBAccessClass::WriteLockClass lock(&vb_access);
		VertexFormatXYZNDUV2* verts=lock.Get_Formatted_Vertex_Array();

		float halo_size=1.0f;


		Vector3 dazzle_dxt(screen_x_scale * dazzle_scale_x,0.0f,0.0f);
		Vector3 halo_dxt(camera->Compute_Projected_Sphere_Radius(current_distance,halo_scale_x),0.0f,0.0f);

		Vector3 dazzle_dyt(0.0f,screen_y_scale * dazzle_scale_y,0.0f);
		Vector3 halo_dyt(0.0f,camera->Compute_Projected_Sphere_Radius(current_distance,halo_scale_y),0.0f);

		if (current_dazzle_intensity>0.0f) {
			VertexFormatXYZNDUV2* vertex=verts;
			dazzle_vertex_count+=4;

			Vector3 col(
				dazzle_color[0]*params->ic.dazzle_color[0],
				dazzle_color[1]*params->ic.dazzle_color[1],
				dazzle_color[2]*params->ic.dazzle_color[2]);
			col*=current_dazzle_intensity;

			if (col[0]>1.0f) col[0]=1.0f;
			if (col[1]>1.0f) col[1]=1.0f;
			if (col[2]>1.0f) col[2]=1.0f;

			unsigned color=DX8Wrapper::Convert_Color(col,1.0f);

			dl=current_vloc+(dazzle_dxt-dazzle_dyt)*current_dazzle_size;
			reinterpret_cast<Vector3&>(vertex->x)=dl;
			vertex->u1=0.0f;
			vertex->v1=0.0f;
			vertex->diffuse=color;
			vertex++;

			dl=current_vloc+(dazzle_dxt+dazzle_dyt)*current_dazzle_size;
			reinterpret_cast<Vector3&>(vertex->x)=dl;
			vertex->u1=1.0f;
			vertex->v1=0.0f;
			vertex->diffuse=color;
			vertex++;

			dl=current_vloc-(dazzle_dxt-dazzle_dyt)*current_dazzle_size;
			reinterpret_cast<Vector3&>(vertex->x)=dl;
			vertex->u1=1.0f;
			vertex->v1=1.0f;
			vertex->diffuse=color;
			vertex++;

			dl=current_vloc-(dazzle_dxt+dazzle_dyt)*current_dazzle_size;
			reinterpret_cast<Vector3&>(vertex->x)=dl;
			vertex->u1=0.0f;
			vertex->v1=1.0f;
			vertex->diffuse=color;
		}

		if (current_halo_intensity) {
			VertexFormatXYZNDUV2* vertex=verts+dazzle_vertex_count;
			halo_vertex_count+=4;

			Vector3 col(
				halo_color[0]*params->ic.halo_color[0],
				halo_color[1]*params->ic.halo_color[1],
				halo_color[2]*params->ic.halo_color[2]);
			col*=current_halo_intensity;
			if (col[0]>1.0f) col[0]=1.0f;
			if (col[1]>1.0f) col[1]=1.0f;
			if (col[2]>1.0f) col[2]=1.0f;

			unsigned color=DX8Wrapper::Convert_Color(col,1.0f);

			dl=current_vloc+(halo_dxt - halo_dyt) * halo_size;
			reinterpret_cast<Vector3&>(vertex->x)=dl;
			vertex->u1=0.0f;
			vertex->v1=0.0f;
			vertex->diffuse=color;
			vertex++;

			dl=current_vloc+(halo_dxt + halo_dyt) * halo_size;
			reinterpret_cast<Vector3&>(vertex->x)=dl;
			vertex->u1=1.0f;
			vertex->v1=0.0f;
			vertex->diffuse=color;
			vertex++;

			dl=current_vloc-(halo_dxt - halo_dyt) * halo_size;
			reinterpret_cast<Vector3&>(vertex->x)=dl;
			vertex->u1=1.0f;
			vertex->v1=1.0f;
			vertex->diffuse=color;
			vertex++;

			dl=current_vloc-(halo_dxt + halo_dyt) * halo_size;
			reinterpret_cast<Vector3&>(vertex->x)=dl;
			vertex->u1=0.0f;
			vertex->v1=1.0f;
			vertex->diffuse=color;
		}

		if (lensflare && current_dazzle_intensity>0.0f) {
			VertexFormatXYZNDUV2* vertex=verts+halo_vertex_count+dazzle_vertex_count;

			lensflare->Generate_Vertex_Buffers(
				vertex,
				lensflare_vertex_count,
				screen_x_scale,
				screen_y_scale,
				current_dazzle_intensity * lensflare_intensity,
				transformed_loc);
			vertex_count+=lensflare_vertex_count;
		}
	}

	int dazzle_poly_count=dazzle_vertex_count>>1;
	int halo_poly_count=halo_vertex_count>>1;
	int lensflare_poly_count=lensflare_vertex_count>>1;
	int poly_count=halo_poly_count>dazzle_poly_count ? halo_poly_count : dazzle_poly_count;
	if (lensflare_poly_count>poly_count) poly_count=lensflare_poly_count;
	if (!poly_count) {
		return;
	}

	DX8Wrapper::Set_Vertex_Buffer(vb_access);

	DynamicIBAccessClass ib_access(BUFFER_TYPE_DYNAMIC_DX8,poly_count*3);
	{
		DynamicIBAccessClass::WriteLockClass lock(&ib_access);
		unsigned short* inds=lock.Get_Index_Array();

		// Proceed two polygons at a time
		for (int a=0;a<poly_count/2;a++) {
			*inds++=short(4*a);
			*inds++=short(4*a+1);
			*inds++=short(4*a+2);
			*inds++=short(4*a);
			*inds++=short(4*a+2);
			*inds++=short(4*a+3);
		}
	}

	DX8Wrapper::Set_World_Identity();
	DX8Wrapper::Set_View_Identity();
	DX8Wrapper::Set_Transform(D3DTS_PROJECTION,Matrix4x4(true));

	if (halo_poly_count) {
		DX8Wrapper::Set_Index_Buffer(ib_access,dazzle_vertex_count);
		DX8Wrapper::Set_Shader(default_halo_shader);
		DX8Wrapper::Set_Texture(0,types[type]->Get_Halo_Texture());
		SphereClass sphere(Get_Position(),0.1f);

		DX8Wrapper::Draw_Triangles(0,halo_poly_count,0,vertex_count);
	}

	if (dazzle_poly_count) {
		DX8Wrapper::Set_Index_Buffer(ib_access,0);
		DX8Wrapper::Set_Shader(default_dazzle_shader);
		DX8Wrapper::Set_Texture(0,types[type]->Get_Dazzle_Texture());
		SphereClass sphere(Vector3(0.0f,0.0f,0.0f),0.0f);
		DX8Wrapper::Draw_Triangles(0,dazzle_poly_count,0,vertex_count);
	}

	if (lensflare_poly_count) {
		DX8Wrapper::Set_Index_Buffer(ib_access,dazzle_vertex_count+halo_vertex_count);
		DX8Wrapper::Set_Shader(default_dazzle_shader);
		DX8Wrapper::Set_Texture(0,lensflare->Get_Texture());
		SphereClass sphere(Vector3(0.0f,0.0f,0.0f),0.0f);
		DX8Wrapper::Draw_Triangles(0,lensflare_poly_count,0,vertex_count);
	}

	DX8Wrapper::Set_Transform(D3DTS_PROJECTION,old_projection_transform);
	DX8Wrapper::Set_Transform(D3DTS_VIEW,old_view_transform);
	DX8Wrapper::Set_Transform(D3DTS_WORLD,old_world_transform);
}

// ----------------------------------------------------------------------------

void DazzleRenderObjClass::Set_Transform(const Matrix3D &m)
{
	RenderObjClass::Set_Transform(m);
	if (type<type_count) {
		Matrix3D::Rotate_Vector(m,types[type]->ic.dazzle_direction,&current_dir);
	}
}

// ----------------------------------------------------------------------------
//
// Return the type id of a dazzle class with given name. If such type is not
// found, return UINT_MAX.
//
// ----------------------------------------------------------------------------

unsigned DazzleRenderObjClass::Get_Type_ID(const char* name)
{
	for (unsigned a=0;a<type_count;++a) {
		if (types[a] && types[a]->name==name) return a;
	}
	return UINT_MAX;
}


// ----------------------------------------------------------------------------
//
// Return the type name of a dazzle class with given id. If the id is out
// of range, return "DEFAULT" which by convention is always defined and is
// id 0.
//
// ----------------------------------------------------------------------------

const char * DazzleRenderObjClass::Get_Type_Name(unsigned id)
{
	if ((id < type_count) && (id >= 0)) {
		return types[id]->name;
	} else {
		return "DEFAULT";
	}
}

// ----------------------------------------------------------------------------
//
// Return pointer to DazzleTypeClass object with given id. If the id is out
// of range (usually UINT_MAX, in can the id was obtained with invalid name
// string) return null.
//
// ----------------------------------------------------------------------------

DazzleTypeClass* DazzleRenderObjClass::Get_Type_Class(unsigned id) // Return dazzle type class pointer, or null if not found
{
	if (id>=type_count) return nullptr;
	return types[id];
}

// ----------------------------------------------------------------------------
//
// Return the type id of a lensflare type class with given name. If such type is
// not found, return UINT_MAX.
//
// ----------------------------------------------------------------------------

unsigned DazzleRenderObjClass::Get_Lensflare_ID(const char* name)
{
	for (unsigned a=0;a<lensflare_count;++a) {
		if (lensflares[a] && lensflares[a]->name==name) return a;
	}
	return UINT_MAX;
}

// ----------------------------------------------------------------------------
//
// Return pointer to LensflareTypeClass object with given id. If the id is out
// of range (usually UINT_MAX, in can the id was obtained with invalid name
// string) return null.
//
// ----------------------------------------------------------------------------

LensflareTypeClass* DazzleRenderObjClass::Get_Lensflare_Class(unsigned id) // Return lensflare type class pointer, or null if not found
{
	if (id>=lensflare_count) return nullptr;
	return lensflares[id];
}

// ----------------------------------------------------------------------------
//
// Should static dazzles require vis information, here's a function that renders
// a quad at the location of the dazzle
//
// ----------------------------------------------------------------------------

void DazzleRenderObjClass::vis_render_dazzle(SpecialRenderInfoClass & rinfo)
{
}

void DazzleRenderObjClass::Special_Render(SpecialRenderInfoClass & rinfo)
{
	if (rinfo.RenderType == SpecialRenderInfoClass::RENDER_VIS) {
		vis_render_dazzle(rinfo);
	}
}



/****************************************************************************************


	DazzleRenderObjClass - Persistent object support.

	Dazzles are going to save their type and their transform and simply
	re-create another dazzle of the same type when loaded.


****************************************************************************************/

class DazzlePersistFactoryClass : public PersistFactoryClass
{
	virtual uint32				Chunk_ID(void) const;
	virtual PersistClass *	Load(ChunkLoadClass & cload) const;
	virtual void				Save(ChunkSaveClass & csave,PersistClass * obj)	const;

	enum
	{
		DAZZLEFACTORY_CHUNKID_VARIABLES		= 1212000336,
		DAZZLEFACTORY_VARIABLE_OBJPOINTER	= 0x00,
		OBSOLETE_DAZZLEFACTORY_VARIABLE_TYPE,
		DAZZLEFACTORY_VARIABLE_TRANSFORM,
		DAZZLEFACTORY_VARIABLE_TYPENAME,
	};
};

static DazzlePersistFactoryClass _DazzleFactory;

uint32 DazzlePersistFactoryClass::Chunk_ID(void) const
{
	return WW3D_PERSIST_CHUNKID_DAZZLE;
}

PersistClass *	DazzlePersistFactoryClass::Load(ChunkLoadClass & cload) const
{
	DazzleRenderObjClass * old_obj = nullptr;
	Matrix3D tm(1);
	char dazzle_type[256];
	dazzle_type[0] = 0;

	/*
	** Load the dazzle parameters
	*/
	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {

			case DAZZLEFACTORY_CHUNKID_VARIABLES:

				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,DAZZLEFACTORY_VARIABLE_OBJPOINTER,old_obj);
						READ_MICRO_CHUNK(cload,DAZZLEFACTORY_VARIABLE_TRANSFORM,tm);
						READ_MICRO_CHUNK_STRING(cload,DAZZLEFACTORY_VARIABLE_TYPENAME,dazzle_type,sizeof(dazzle_type));
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d",__FILE__,__LINE__));
				break;
		};
		cload.Close_Chunk();
	}

	/*
	** Create a new dazzle object
	*/
	int type_index = 0;
	if (strlen(dazzle_type) > 0) {
		type_index = DazzleRenderObjClass::Get_Type_ID(dazzle_type);
	}
	RenderObjClass * new_obj = NEW_REF(DazzleRenderObjClass,(dazzle_type));

	/*
	** If we failed to create it, replace it with a nullptr
	*/
	if (new_obj == nullptr) {
		static int count = 0;
		if ( count++ < 10 ) {
			WWDEBUG_SAY(("DazzlePersistFactory failed to create dazzle of type: %s!!",dazzle_type));
			WWDEBUG_SAY(("Replacing it with a null render object!"));
		}
		new_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj("NULL");
	}

	WWASSERT(new_obj != nullptr);
	if (new_obj) {
		new_obj->Set_Transform(tm);
	}

	/*
	** Register the old pointer for re-mapping to the new pointer
	*/
	SaveLoadSystemClass::Register_Pointer(old_obj,new_obj);
	return new_obj;
}

void DazzlePersistFactoryClass::Save(ChunkSaveClass & csave,PersistClass * obj)	const
{
	DazzleRenderObjClass * robj = (DazzleRenderObjClass *)obj;
	unsigned int dazzle_type = robj->Get_Dazzle_Type();
	const char * dazzle_type_name = DazzleRenderObjClass::Get_Type_Name(dazzle_type);
	Matrix3D tm = robj->Get_Transform();

	csave.Begin_Chunk(DAZZLEFACTORY_CHUNKID_VARIABLES);
	WRITE_MICRO_CHUNK(csave,DAZZLEFACTORY_VARIABLE_OBJPOINTER,robj);
	WRITE_MICRO_CHUNK(csave,DAZZLEFACTORY_VARIABLE_TRANSFORM,tm);
	WRITE_MICRO_CHUNK_STRING(csave,DAZZLEFACTORY_VARIABLE_TYPENAME,dazzle_type_name);

	csave.End_Chunk();
}


/*
** DazzleRenderObj save-load.
*/
const PersistFactoryClass & DazzleRenderObjClass::Get_Factory (void) const
{
	return _DazzleFactory;
}



/**********************************************************************************************
**
** DazzleLayerClass Implementation
**
**********************************************************************************************/
DazzleLayerClass::DazzleLayerClass(void) :
	visible_lists(nullptr)
{
	// Generate an array with one visible list for each type.
	// NOTE - this means that this constructor must be called AFTER all types
	// are initialized
	WWASSERT(type_count);

	visible_lists = W3DNEWARRAY DazzleRenderObjClass *[type_count];
	for (unsigned int i = 0; i < type_count; i++) {
		visible_lists[i] = nullptr;
	}
}

DazzleLayerClass::~DazzleLayerClass(void)
{
	// NOTE - this destructor must be called BEFORE DeInit().
	WWASSERT(type_count);

	for (unsigned int i = 0; i < type_count; i++) {
		Clear_Visible_List(i);
	}

	delete [] visible_lists;
}

void DazzleLayerClass::Render(CameraClass* camera)
{
	if (!camera) return;

	camera->Apply();

	DX8Wrapper::Set_Material(nullptr);

	for (unsigned type=0;type<type_count;++type) {
		if (!types[type]) continue;
		int count = Get_Visible_Item_Count(type);

		if (!count) continue;

		DazzleRenderObjClass* n = visible_lists[type];
		while (n) {
			n->Render_Dazzle(camera);
			n=n->Succ();
		}

		// Must clear the visible list at the end of each render.
		Clear_Visible_List(type);
	}
}

// ----------------------------------------------------------------------------

int DazzleLayerClass::Get_Visible_Item_Count(unsigned int type) const
{
	if (type >= type_count) {
		WWASSERT(0);
		return 0;
	}

	int count=0;

	DazzleRenderObjClass* n = visible_lists[type];
	while (n) {
		count++;
		n=n->Succ();
	}
	return count;
}

// ----------------------------------------------------------------------------

void DazzleLayerClass::Clear_Visible_List(unsigned int type)
{
	if (type >= type_count) {
		WWASSERT(0);
		return;
	}


	DazzleRenderObjClass* n = visible_lists[type];
	while (n) {
		n->on_list = false;
		n=n->Succ();
	}

	visible_lists[type] = nullptr;
}

/**********************************************************************************************
**
** DazzleVisibilityClass Implementation
** The default dazzle visibility handler asks the scene to determine whether the dazzle
** is occluded.
**
**********************************************************************************************/

float DazzleVisibilityClass::Compute_Dazzle_Visibility
(
	RenderInfoClass & rinfo,
	DazzleRenderObjClass * dazzle,
	const Vector3 & point
) const
{
	/*
	** Look up the scene this dazzle is in
	*/
	SceneClass * scene = dazzle->Get_Scene();
	RenderObjClass * container = dazzle->Get_Container();
	while ((scene == nullptr) && (container != nullptr)) {
		scene = container->Get_Scene();
		container = container->Get_Container();
	}

	/*
	** If we found the scene (we SHOULD!) then ask it to compute the visibility
	*/
	if (scene != nullptr) {
		float value = scene->Compute_Point_Visibility(rinfo,point);
		scene->Release_Ref();
		return value;
	} else {
		return 1.0f;
	}
}


/**********************************************************************************************
**
** DazzlePrototypeClass Implementation
**
**********************************************************************************************/

RenderObjClass * DazzlePrototypeClass::Create(void)
{
	return NEW_REF(DazzleRenderObjClass,(DazzleType));
}


WW3DErrorType DazzlePrototypeClass::Load_W3D(ChunkLoadClass & cload)
{
	StringClass dazzle_type;

	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID())
		{
			READ_WWSTRING_CHUNK(cload,W3D_CHUNK_DAZZLE_NAME,Name);
			READ_WWSTRING_CHUNK(cload,W3D_CHUNK_DAZZLE_TYPENAME,dazzle_type);
			default:
				break;
		}
		cload.Close_Chunk();
	}

	DazzleType = DazzleRenderObjClass::Get_Type_ID(dazzle_type);
	if (DazzleType == UINT_MAX) {
		DazzleType = 0;
	}

	return WW3D_ERROR_OK;
}


/**********************************************************************************************
**
** DazzleLoaderClass Implementation
**
**********************************************************************************************/

PrototypeClass * DazzleLoaderClass::Load_W3D(ChunkLoadClass & cload)
{
	DazzlePrototypeClass * new_proto = W3DNEW DazzlePrototypeClass;
	new_proto->Load_W3D(cload);
	return new_proto;
}
