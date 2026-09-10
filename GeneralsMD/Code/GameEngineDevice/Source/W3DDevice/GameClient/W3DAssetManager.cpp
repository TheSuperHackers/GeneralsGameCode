import Assets.Images.Color;
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : W3DAssetManager                                                  *
 *                                                                                             *
 *                     $Archive::                                                             $*
 *                                                                                             *
 *              Original Author:: Hector Yee (Enbassetmgr)
 * 			     Added/Modified:: Mark Wilczynski
 *                                                                                             *
 *                      $Author::                                                             $*
 *                                                                                             *
 *                     $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision::                                                             $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <WWLib/always.h>
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include <WWMath/vector3.h>
#include "W3DDevice/GameClient/W3DMeshRenderObject.h"
#include "W3DDevice/GameClient/W3DHierarchyRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshResource.h"
import Graphics.Materials.MeshMaterial;
import Graphics.Scene.Props.Material;
#include "W3DDevice/GameClient/W3DTextureHandle.h"

import Graphics.Resources.Textures.Edit;
import Assets.Images.PixelEncoding;
import Assets.Identity;
import Graphics.RHI;
import Assets.Math;
#include <WWDebug/wwprofile.h>
#include "WWDebug/wwmemlog.h"
#include "WWLib/ffactory.h"
#include "Common/PerfTimer.h"
#include "Common/GlobalData.h"
#include "Common/GameCommon.h"
#include <cctype>
#include <cstring>
#include <cstdio>
#include <string>
#include <string_view>
import Assets.Cache.Animations;


//---------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------

const float ident_scale(1.0f);
const float scale_epsilon(0.01f);
const Vector3 ident_HSV(0,0,0);
const float H_epsilon(1.0f);
const float S_epsilon(0.01f);
const float V_epsilon(0.01f);

static inline void Lowercase_String(char *value)
{
	if (value == nullptr)
		return;

	for (char *character = value; *character != '\0'; ++character)
		*character = static_cast<char>(std::tolower(static_cast<unsigned char>(*character)));
}

//---------------------------------------------------------------------
// Externs defined somewhere in W3D.
//---------------------------------------------------------------------


//---------------------------------------------------------------------
// W3DPrototype
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//---------------------------------------------------------------------
// W3DAssetManager
//---------------------------------------------------------------------

//---------------------------------------------------------------------
W3DAssetManager::W3DAssetManager()
	: m_catalog(nullptr,
		[](std::string_view filename, bool &allow_reduction) {
			if (filename.size() >= 3 &&
				std::tolower(static_cast<unsigned char>(filename[0])) == 'z' &&
				std::tolower(static_cast<unsigned char>(filename[1])) == 'h' &&
				std::tolower(static_cast<unsigned char>(filename[2])) == 'c')
				allow_reduction = false;
		},
		[](std::string_view filename) {
#if defined(RTS_DEBUG)
			if (TheGlobalData == nullptr || !TheGlobalData->m_preloadReport)
				return;
			std::string report_name(filename);
			for (char &character : report_name)
				character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
			FILE *logfile = std::fopen("PreloadedAssets.txt", "a+");
			if (logfile != nullptr) {
				std::fprintf(logfile, "3D: %s\n", report_name.c_str());
				std::fclose(logfile);
			}
#else
			(void)filename;
#endif
		})
{
}

//---------------------------------------------------------------------
W3DAssetManager::~W3DAssetManager()
{
}

//---------------------------------------------------------------------
/** 'Generals' specific munging to encode team color and scale in model name */
static inline void Munge_Render_Obj_Name(char *newname, size_t newname_size, const char *oldname, float scale,
	const int color, const char *textureName)
{
	char lower_case_name[255];
	strlcpy(lower_case_name, oldname, ARRAY_SIZE(lower_case_name));
	Lowercase_String(lower_case_name);

	if (!textureName)
		textureName = "";

	snprintf(newname, newname_size, "#%d!%g!%s#%s", color, scale, textureName, lower_case_name);
}

//---------------------------------------------------------------------
static inline void Munge_Texture_Name(char *newname, size_t newname_size, const char *oldname, const int color)
{
	char lower_case_name[255];
	strlcpy(lower_case_name, oldname, ARRAY_SIZE(lower_case_name));
	Lowercase_String(lower_case_name);
	snprintf(newname, newname_size, "#%d#%s", color, lower_case_name);
}

//---------------------------------------------------------------------
int W3DAssetManager::replaceAssetTexture(W3DRenderObject *robj, W3DTextureHandle *oldTex, W3DTextureHandle *newTex)
{
	switch (robj->Class_ID())	{
	case W3DRenderObject::CLASSID_MESH:
		return replaceMeshTexture(robj, oldTex, newTex);
		break;
	case W3DRenderObject::CLASSID_HLOD:
		return replaceHLODTexture(robj, oldTex, newTex);
		break;
	}
	return 0;
}

//---------------------------------------------------------------------
Int W3DAssetManager::replaceHLODTexture(W3DRenderObject *robj, W3DTextureHandle *oldTex, W3DTextureHandle *newTex)
{
	int didReplace=0;

	int num_sub = robj->Get_Num_Sub_Objects();
	for(int i = 0; i < num_sub; i++) {
		W3DRenderObject *sub_obj = robj->Get_Sub_Object(i);
		didReplace |= replaceAssetTexture(sub_obj, oldTex, newTex);
		REF_PTR_RELEASE(sub_obj);
	}
	return didReplace;
}

//---------------------------------------------------------------------
Int W3DAssetManager::replaceMeshTexture(W3DRenderObject *robj, W3DTextureHandle *oldTex, W3DTextureHandle *newTex)
{
	int i;
	int didReplace=0;

	W3DMeshRenderObject *mesh=(W3DMeshRenderObject*) robj;
	W3DMeshResource * model = mesh->Get_Model();
	auto material = mesh->Get_Material_Info();

	for (i=0; i<static_cast<int>(material->textures.size()); i++)
	{
		if (material->textures[i].Peek() == oldTex)
		{
			model->Replace_Texture(oldTex,newTex);
			material->textures[i] = RefCountPtr<W3DTextureHandle>::Create_Add_Ref(newTex);
			didReplace=1;
		}
	}

	material.reset();
	REF_PTR_RELEASE(model);
	return didReplace;
}

//---------------------------------------------------------------------
/** Replaces all references to old texture with new texture.  Operation is performed on
	asset prototype so it will affect most instances of this object.  Objects which have
	been customized with house color will not be affected unless they are created after
	this function is called.
*/
int W3DAssetManager::replacePrototypeTexture(W3DRenderObject *robj, const char * oldname, const char * newname)
{
	//search model for old texture
	W3DTextureHandle *oldTex=m_catalog.Get_Texture(oldname);
	W3DTextureHandle *newTex=m_catalog.Get_Texture(newname);

	int retCode=replaceAssetTexture(robj,oldTex,newTex);

	REF_PTR_RELEASE(oldTex);
	REF_PTR_RELEASE(newTex);

	return retCode;
}

//---------------------------------------------------------------------
/** Generals specific version that looks for a texture which has been house-color tinted.
	Don't use this unless you really need it.  Generic texture requests use the catalog.
*/
W3DTextureHandle * W3DAssetManager::Find_Texture(const char * name, const int color)
{
	char newname[512];
	Munge_Texture_Name(newname, ARRAY_SIZE(newname), name, color);

	// see if we have a cached copy
	W3DTextureHandle *newtex = m_catalog.Find_Texture_Borrowed(newname);
	if (newtex) {
		newtex->Add_Ref();
	}
	return newtex;
}

//---------------------------------------------------------------------
W3DTextureHandle * W3DAssetManager::Recolor_Texture(W3DTextureHandle *texture, const int color)
{
	const char *name=texture->Get_Texture_Name();

	W3DTextureHandle *newtex = Find_Texture(name, color);
	if (newtex) {
		return newtex;
	}

	return Recolor_Texture_One_Time(texture, color);
}

//---------------------------------------------------------------------
const Int TEAM_COLOR_PALETTE_SIZE = 16;
const UnsignedShort houseColorScale[TEAM_COLOR_PALETTE_SIZE] =
{
	255,239,223,211,195,174,167,151,135,123,107,91,79,63,47,35
};

//---------------------------------------------------------------------
static void remapPalette16Bit(Assets::ImageDescription *sd, UnsignedShort *palette, unsigned int color)
{
	UnsignedShort pal[TEAM_COLOR_PALETTE_SIZE];
	Vector3 rgb,v_color((float)((color>>16)&0xff)/255.0f/255.0f,(float)((color>>8)&0xff)/255.0f/255.0f,(float)(color&0xff)/255.0f/255.0f);

	//Generate a new color gradient palette based on reference color
	for (Int y=0; y<TEAM_COLOR_PALETTE_SIZE; y++)
	{
		rgb.X=(Real)houseColorScale[y]*v_color.X;
		rgb.Y=(Real)houseColorScale[y]*v_color.Y;
		rgb.Z=(Real)houseColorScale[y]*v_color.Z;
		pal[y]=0xffff;	//preset alpha to known value
		Assets::Replace_Image_RGB({reinterpret_cast<std::byte*>(&pal[y]), sizeof(pal[y])},
            sd->encoding, {rgb.X,rgb.Y,rgb.Z});
	}

	//check if this pixel is part of team color palette
	for (Int p=0; p<TEAM_COLOR_PALETTE_SIZE; p++)
	{	palette[p]=pal[p];	//replace color with house color
	}
}

//---------------------------------------------------------------------
static void remapTexture16Bit(Int dx, Int dy, Int pitch, Assets::ImageDescription *sd, UnsignedShort *palette, UnsignedShort *data, unsigned int color)
{
	UnsignedShort pal[TEAM_COLOR_PALETTE_SIZE];
	Vector3 rgb,v_color((float)((color>>16)&0xff)/255.0f/255.0f,(float)((color>>8)&0xff)/255.0f/255.0f,(float)(color&0xff)/255.0f/255.0f);

	//Generate a new color gradient palette based on reference color
	Int y=0;
	for (; y<TEAM_COLOR_PALETTE_SIZE; y++)
	{
		rgb.X=(Real)houseColorScale[y]*v_color.X;
		rgb.Y=(Real)houseColorScale[y]*v_color.Y;
		rgb.Z=(Real)houseColorScale[y]*v_color.Z;
		pal[y]=0xffff;	//preset alpha to known value
		Assets::Replace_Image_RGB({reinterpret_cast<std::byte*>(&pal[y]), sizeof(pal[y])},
            sd->encoding, {rgb.X,rgb.Y,rgb.Z});
	}

	for (y=0; y<dy; y++)
	{	for (Int x=0; x<dx; x++)
		{	//check if this pixel is part of team color palette
			for (Int p=0; p<TEAM_COLOR_PALETTE_SIZE; p++)
			{	if (palette[p]==data[x])
				{	data[x]=pal[p];	//replace color with house color
					break;
				}
			}
		}

		data += pitch;
	}
}

//Use hue shift instead of alpha blend to apply house color to textures
#define DO_HUE_SHIFT

//---------------------------------------------------------------------
//Input texture is assumed to be in ARGB 4444 format.
static void remapAlphaTexture16Bit(Int dx, Int dy, Int pitch, Assets::ImageDescription *sd, UnsignedShort *data, unsigned int color)
{
	UnsignedShort pixel;
	UnsignedShort pixelAlpha;
#ifndef DO_HUE_SHIFT
	float fpixelAlpha,fpixelAlphaInv;
#endif
	Vector3 rgb,v_color((float)((color>>16)&0xff)/255.0f,(float)((color>>8)&0xff)/255.0f,(float)(color&0xff)/255.0f);
	Int x,y;

#ifdef DO_HUE_SHIFT
	Assets::Vector3f hsv;
	const auto hsv_color = Assets::RGB_To_HSV({v_color.X, v_color.Y, v_color.Z});
#endif

	for (y=0; y<dy; y++)
	{
		for (x=0; x<dx; x++)
		{
			pixel=data[x];
			pixelAlpha=15-(pixel>>12);	//get alpha for house color
			if (pixelAlpha)
			{	//some house color needs to show through
				///@todo: optimize this alpha blend to use fixed point math.
#ifdef DO_HUE_SHIFT
				hsv = Assets::RGB_To_HSV({((pixel >> 8) & 15) / 15.0f, ((pixel >> 4) & 15) / 15.0f, (pixel & 15) / 15.0f});
				hsv.x=hsv_color.x;
				hsv.y*=hsv_color.y;
				const auto converted = Assets::HSV_To_RGB(hsv);
                rgb.Set(converted.x, converted.y, converted.z);
#else
				fpixelAlpha=pixelAlpha/15.0f;
				fpixelAlphaInv=1.0f-fpixelAlpha;
				rgb.X=fpixelAlpha * v_color.X + fpixelAlphaInv*(Real)((pixel>>8)&0xf)/15.0f;	//red
				rgb.Y=fpixelAlpha * v_color.Y + fpixelAlphaInv*(Real)((pixel>>4)&0xf)/15.0f; //green
				rgb.Z=fpixelAlpha * v_color.Z + fpixelAlphaInv*(Real)(pixel&0xf)/15.0f; //blue
#endif
				data[x] = REAL_TO_INT(rgb.X*15.0f)<<8 | REAL_TO_INT(rgb.Y*15.0f)<<4 | REAL_TO_INT(rgb.Z*15.0f);
			}
			data[x] |= 0xf000;	//force alpha to opaque.
		}
		data += pitch;
	}
}

//---------------------------------------------------------------------
static void remapPalette32Bit(Assets::ImageDescription *sd, UnsignedInt *palette, unsigned int color)
{
	UnsignedInt pal[TEAM_COLOR_PALETTE_SIZE];
	Vector3 rgb,v_color((float)((color>>16)&0xff)/255.0f/255.0f,(float)((color>>8)&0xff)/255.0f/255.0f,(float)(color&0xff)/255.0f/255.0f);

	//Generate a new color gradient palette based on reference color
	for (Int y=0; y<TEAM_COLOR_PALETTE_SIZE; y++)
	{
		rgb.X=(Real)houseColorScale[y]*v_color.X;
		rgb.Y=(Real)houseColorScale[y]*v_color.Y;
		rgb.Z=(Real)houseColorScale[y]*v_color.Z;
		pal[y]=0xffffffff;	//preset alpha to known value
		Assets::Replace_Image_RGB({reinterpret_cast<std::byte*>(&pal[y]), sizeof(pal[y])},
            sd->encoding, {rgb.X,rgb.Y,rgb.Z});
	}

	//check if this pixel is part of team color palette
	for (Int p=0; p<TEAM_COLOR_PALETTE_SIZE; p++)
	{	palette[p]=pal[p];	//replace color with house color
	}
}

//---------------------------------------------------------------------
static void remapTexture32Bit(Int dx, Int dy, Int pitch, Assets::ImageDescription *sd, UnsignedInt *palette, UnsignedInt *data, unsigned int color)
{
	UnsignedInt pal[TEAM_COLOR_PALETTE_SIZE];
	Vector3 rgb,v_color((float)((color>>16)&0xff)/255.0f/255.0f,(float)((color>>8)&0xff)/255.0f/255.0f,(float)(color&0xff)/255.0f/255.0f);

	//Generate a new color gradient palette based on reference color
	Int y=0;
	for (; y<TEAM_COLOR_PALETTE_SIZE; y++)
	{
		rgb.X=(Real)houseColorScale[y]*v_color.X;
		rgb.Y=(Real)houseColorScale[y]*v_color.Y;
		rgb.Z=(Real)houseColorScale[y]*v_color.Z;
		pal[y]=0xffffffff;	//preset alpha to known value
		Assets::Replace_Image_RGB({reinterpret_cast<std::byte*>(&pal[y]), sizeof(pal[y])},
            sd->encoding, {rgb.X,rgb.Y,rgb.Z});
	}

	for (y=0; y<dy; y++)
	{	for (Int x=0; x<dx; x++)
		{	//check if this pixel is part of team color palette
			for (Int p=0; p<TEAM_COLOR_PALETTE_SIZE; p++)
			{	if (palette[p]==data[x])
				{	data[x]=pal[p];	//replace color with house color
					break;
				}
			}
		}
		data += pitch;
	}
}

//---------------------------------------------------------------------
static void remapAlphaTexture32Bit(Int dx, Int dy, Int pitch, Assets::ImageDescription *sd, UnsignedInt *data, unsigned int color)
{
	UnsignedInt pixel;
	UnsignedInt pixelAlpha;
#ifndef DO_HUE_SHIFT
	float fpixelAlpha,fpixelAlphaInv;
#endif
	Vector3 rgb,v_color((float)((color>>16)&0xff)/255.0f,(float)((color>>8)&0xff)/255.0f,(float)(color&0xff)/255.0f);
	Int x,y;
#ifdef DO_HUE_SHIFT
	Assets::Vector3f hsv;
	const auto hsv_color = Assets::RGB_To_HSV({v_color.X, v_color.Y, v_color.Z});
#endif

	for (y=0; y<dy; y++)
	{	for (x=0; x<dx; x++)
		{
			pixel=data[x];
			pixelAlpha=255-(pixel>>24);	//get alpha for house color
			if (pixelAlpha)
			{	//some house color needs to show through
#ifdef DO_HUE_SHIFT
				hsv = Assets::RGB_To_HSV({((pixel >> 16) & 255) / 255.0f, ((pixel >> 8) & 255) / 255.0f, (pixel & 255) / 255.0f});
				hsv.x=hsv_color.x;
				hsv.y*=hsv_color.y;
				const auto converted = Assets::HSV_To_RGB(hsv);
                rgb.Set(converted.x, converted.y, converted.z);
#else
				///@todo: optimize this alpha blend to use fixed point math.
				fpixelAlpha=pixelAlpha/255.0f;
				fpixelAlphaInv=1.0f-fpixelAlpha;
				rgb.X=fpixelAlpha * v_color.X + fpixelAlphaInv*(Real)((pixel>>16)&0xff)/255.0f;	//red
				rgb.Y=fpixelAlpha * v_color.Y + fpixelAlphaInv*(Real)((pixel>>8)&0xff)/255.0f; //green
				rgb.Z=fpixelAlpha * v_color.Z + fpixelAlphaInv*(Real)(pixel&0xff)/255.0f; //blue
#endif
				data[x] = REAL_TO_INT(rgb.X*255.0f)<<16 |	REAL_TO_INT(rgb.Y*255.0f)<<8 | REAL_TO_INT(rgb.Z*255.0f);
			}
			data[x] |= 0xff000000;	//force alpha to opaque.
		}
		data += pitch;
	}
}

//---------------------------------------------------------------------
/** Surface is assumed to come in the following format:
First 16 pixels are a palette composed of 24-Bit RGB values.
Any pixels in remainder of image that use these 24-bit values
will be remapped using a pre-defined formula.
*/
void W3DAssetManager::Remap_Palette(Graphics::TextureEdit *surface, const int color, Bool doPaletteOnly, Bool useAlpha)
{
//	unsigned int x;
	Assets::ImageDescription sd;
	sd=surface->Image().Description();
	int pitch,size;
//	UnsignedInt newPalette[TEAM_COLOR_PALETTE_SIZE];

	size=Assets::Pixel_Size(sd.encoding);
	const auto mapping=surface->Map();
    if (mapping.bytes.empty()) return;
    pitch=static_cast<int>(mapping.row_pitch);
    unsigned char *bits=reinterpret_cast<unsigned char*>(mapping.bytes.data());

	if (doPaletteOnly)
	{	//only recolor the palette which is stored in top row.  Model only references these pixels.
		if (size == 2)
			remapPalette16Bit(&sd, (UnsignedShort *)bits, color);
		else
		if (size == 4)
			remapPalette32Bit(&sd, (UnsignedInt *)bits,color);
	}
	else
	{
		if (useAlpha)
		{
			if (size == 2)
				remapAlphaTexture16Bit(sd.width, sd.height, pitch>>1, &sd, (UnsignedShort *)bits,color);
			else
			if (size == 4)
				remapAlphaTexture32Bit(sd.width, sd.height, pitch>>2, &sd, (UnsignedInt *)bits,color);
		}
		else
		{	//Recolor the image using the palette stored in top row
			if (size == 2)
				remapTexture16Bit(sd.width, sd.height-1, pitch>>1, &sd, (UnsignedShort *)bits, (UnsignedShort *)(bits+pitch), color);
			else
			if (size == 4)
				remapTexture32Bit(sd.width, sd.height-1, pitch>>2, &sd, (UnsignedInt *)bits, (UnsignedInt *)(bits+pitch),color);
		}
	}

	surface->Unmap();
}

//---------------------------------------------------------------------
W3DTextureHandle * W3DAssetManager::Recolor_Texture_One_Time(W3DTextureHandle *texture, const int color)
{
	const char *name=texture->Get_Texture_Name();

	// if texture is procedural return nullptr
	if (name && name[0]=='!') return nullptr;

	// make sure texture is loaded
	if (!texture->Is_Initialized())
		Graphics::Get_Resource_Load_Queue().Request(texture->Loading_Source(), Graphics::ResourceLoadPriority::Immediate);

	Assets::ImageDescription desc;
	Graphics::TextureEdit *newsurf, *oldsurf;
	texture->Get_Level_Description(desc);

	Int psize;
	psize=Assets::Pixel_Size(desc.encoding);
	DEBUG_ASSERTCRASH( psize == 2 || psize == 4, ("Can't Recolor Texture %s", name) );

	oldsurf=texture->Get_Surface_Level();

	newsurf=Graphics::TextureEdit::Create(desc.width,desc.height,desc.encoding);
	if (!oldsurf || !newsurf || !newsurf->Copy_From(*oldsurf,
        {0,0,int(desc.width),int(desc.height)}, {0,0,int(desc.width),int(desc.height)})) {
        delete oldsurf; delete newsurf; return nullptr;
    }

	if (*(name+3) == 'D' || *(name+3) == 'd')
		Remap_Palette(newsurf,color, true, false );	//texture only contains a palette stored in top row.
	else
	if (*(name+3) == 'A' || *(name+3) == 'a')
		Remap_Palette(newsurf,color, false, true );	//texture only contains a palette stored in top row.

	W3DTextureHandle * newtex=NEW_REF(W3DTextureHandle,(newsurf,(MipCountType)texture->Get_Mip_Level_Count()));
	newtex->Get_Sampling().magnification = texture->Get_Sampling().magnification;
	newtex->Get_Sampling().minification = texture->Get_Sampling().minification;
	newtex->Get_Sampling().mipmap = texture->Get_Sampling().mipmap;
	newtex->Get_Sampling().address[0] = texture->Get_Sampling().address[0];
	newtex->Get_Sampling().address[1] = texture->Get_Sampling().address[1];

	char newname[512];
	Munge_Texture_Name(newname, ARRAY_SIZE(newname), name, color);
	newtex->Set_Texture_Name(newname);

	RefCountPtr<W3DTextureHandle> owner =
		RefCountPtr<W3DTextureHandle>::Create_No_Add_Ref(newtex);

	delete oldsurf; oldsurf = nullptr;
	delete newsurf; newsurf = nullptr;

	return m_catalog.Adopt_Texture(owner);
}

//---------------------------------------------------------------------
/** Generals specific code to generate customized render objects for each team color
	Scale==1.0, color==0x00000000, and oldTexture==nullptr are defaults that do nothing.
*/
W3DRenderObject * W3DAssetManager::Create_Render_Obj(
	const char * name,
	float scale,
	const int color,
	const char *oldTexture,
	const char *newTexture
)
{
	Bool reallyscale = (WWMath::Fabs(scale - ident_scale) > scale_epsilon);
	Bool reallycolor = (color & 0xFFFFFF) != 0;	//black is not a valid color and assumes no custom coloring.
	Bool reallytexture = (oldTexture != nullptr && newTexture != nullptr);

	// base case, no scale or color
	if (!reallyscale && !reallycolor && !reallytexture)
	{
		return m_catalog.Create_Render_Obj(name);
	}

	char newname[512];
	Munge_Render_Obj_Name(newname, ARRAY_SIZE(newname), name, scale, color, newTexture);

	// see if we got a cached version
	W3DRenderObject *rendobj = nullptr;

	const bool load_on_demand = m_catalog.Get_Load_On_Demand();
	m_catalog.Set_Load_On_Demand(false); // munged names are never source files.
	rendobj = m_catalog.Create_Render_Obj(newname);
	if (rendobj)
	{	//store the color that we used to create asset so we can read it back out
		//when we need to save this render object to a file.  Used during saving
		//of fog of war ghost objects.
		rendobj->Set_ObjectColor(color);
		m_catalog.Set_Load_On_Demand(load_on_demand);
		return rendobj;
	}
	m_catalog.Set_Load_On_Demand(load_on_demand);

	// create a new one based on existing prototype

	WWPROFILE( "W3DAssetManager::Create_Render_Obj" );
	WWMEMLOG(MEM_GEOMETRY);

	// Try to find a prototype
	Graphics::ModelFactory<W3DRenderObject> * proto = m_catalog.Find_Prototype(name);

	if (load_on_demand && proto == nullptr)
	{
		// If we didn't find one, try to load on demand
		std::string filename;
		const char *mesh_name = strchr (name, '.');
		if (mesh_name != nullptr)
		{
			filename.assign(name, static_cast<size_t>(mesh_name - name));
			filename += ".w3d";
		} else {
			filename = std::string(name) + ".w3d";
		}

		// If we can't find it, try the parent directory
		if ( m_catalog.Load_3D_Assets( filename.c_str() ) == false )
		{
			StringClass	new_filename = StringClass("..\\") + filename.c_str();
			m_catalog.Load_3D_Assets(new_filename);
		}

		proto = m_catalog.Find_Prototype(name);		// try again
	}

	if (proto == nullptr)
	{
		static int warning_count = 0;
		if (++warning_count <= 20)
		{
			WWDEBUG_SAY(("WARNING: Failed to create Render Object: %s",name));
		}
		return nullptr;		// Failed to find a prototype
	}

	rendobj = proto->Instantiate();

	if (!rendobj)
	{
		return nullptr;
	}

	if (reallyscale)
		rendobj->Scale(scale);	//this also makes it unique

	Make_Unique(rendobj,reallyscale,reallycolor);

	if (reallytexture)
	{
		W3DTextureHandle *oldTex = m_catalog.Get_Texture(oldTexture);
		W3DTextureHandle *newTex = m_catalog.Get_Texture(newTexture);
		replaceAssetTexture(rendobj,oldTex,newTex);
		REF_PTR_RELEASE(newTex);
		REF_PTR_RELEASE(oldTex);
	}

	if (reallycolor)
		Recolor_Asset(rendobj,color);

	const std::shared_ptr<W3DRenderObject> source(rendobj,[](W3DRenderObject* object) { object->Release_Ref(); });
    auto* w3dproto = new Graphics::ModelFactory<W3DRenderObject>(newname,rendobj->Class_ID(),[source] {
        return static_cast<W3DRenderObject*>(SET_REF_OWNER(source->Clone()));
    });
	(void)m_catalog.Add_Prototype(
		std::unique_ptr<Graphics::ModelFactory<W3DRenderObject>>(w3dproto));

	rendobj = w3dproto->Instantiate();
	rendobj->Set_ObjectColor(color);

	return rendobj;
}

//---------------------------------------------------------------------
/** Generals specific code to generate customized render objects for each team color
*/
int W3DAssetManager::Recolor_Asset(W3DRenderObject *robj, const int color)
{
	if (TheGlobalData->m_headless)
		return 0;

	switch (robj->Class_ID())	{
	case W3DRenderObject::CLASSID_MESH:
		return Recolor_Mesh(robj,color);
		break;
	case W3DRenderObject::CLASSID_HLOD:
		return Recolor_HLOD(robj,color);
		break;
	}
	return 0;
}

//---------------------------------------------------------------------
/** Generals specific code to generate customized render objects for each team color
*/
int W3DAssetManager::Recolor_Mesh(W3DRenderObject *robj, const int color)
{
	if (TheGlobalData->m_headless)
		return 0;

	int i;
	int didRecolor=0;
	const char *meshName;

	W3DMeshRenderObject *mesh=(W3DMeshRenderObject*) robj;
	W3DMeshResource * model = mesh->Get_Model();
	auto material = mesh->Get_Material_Info();

	// recolor vertex material (assuming mesh is housecolor)
	if ( (( (meshName=strchr(mesh->Get_Name(),'.') ) != nullptr && *(meshName++)) || ( (meshName=mesh->Get_Name()) != nullptr)) &&
		Assets::Asset_Name_Prefix_Equals_No_Case(meshName,"HOUSECOLOR", 10))
	{	for (i=0; i<static_cast<int>(material->materials.size()); i++)
			Recolor_Vertex_Material(material->materials[i].get(),color);
		didRecolor=1;
	}

	// recolor textures
	W3DTextureHandle *newtex,*oldtex;
	for (i=0; i<static_cast<int>(material->textures.size()); i++)
	{
		oldtex=material->textures[i].Peek();
		if (Assets::Asset_Name_Prefix_Equals_No_Case(oldtex->Get_Texture_Name(),"ZHC", 3))
		{	//This texture needs to be adjusted for housecolor
			newtex=Recolor_Texture(oldtex,color);
			if (newtex)
			{
				model->Replace_Texture(oldtex,newtex);
				material->textures[i] = RefCountPtr<W3DTextureHandle>::Create_Add_Ref(newtex);
				REF_PTR_RELEASE(newtex);
				didRecolor=1;
			}
		}
	}

	material.reset();
	REF_PTR_RELEASE(model);
	return didRecolor;
}

//---------------------------------------------------------------------
/** Generals specific code to generate customized render objects for each team color
*/

int W3DAssetManager::Recolor_HLOD(W3DRenderObject *robj, const int color)
{
	if (TheGlobalData->m_headless)
		return 0;

	int didRecolor=0;

	int num_sub = robj->Get_Num_Sub_Objects();
	for(int i = 0; i < num_sub; i++) {
		W3DRenderObject *sub_obj = robj->Get_Sub_Object(i);
		didRecolor |= Recolor_Asset(sub_obj,color);
		REF_PTR_RELEASE(sub_obj);
	}
	return didRecolor;
}

//---------------------------------------------------------------------
/** Generals specific code to generate customized render objects for each team color
*/
void W3DAssetManager::Recolor_Vertex_Material(Graphics::MeshMaterial *vmat, const int color)
{
	Vector3 rgb,rgb2;

	rgb.X = (Real)((color >> 16) & 0xff )/255.0f;
	rgb.Y = (Real)((color >> 8) & 0xff )/255.0f;
	rgb.Z = (Real)(color & 0xff)/255.0f;

	//We ignore the existing ambinent/diffuse and assume they were 1.0.  We can change
	//to scaling them if required.

//	vmat->Get_Ambient(&rgb2);
	rgb2.X = rgb.X;	//scale colors
	rgb2.Y = rgb.Y;	//scale colors
	rgb2.Z = rgb.Z;	//scale colors
	vmat->parameters.ambient = {rgb2.X,rgb2.Y,rgb2.Z};

//	vmat->Get_Diffuse(&rgb2);
	rgb2.X = rgb.X;	//scale colors
	rgb2.Y = rgb.Y;	//scale colors
	rgb2.Z = rgb.Z;	//scale colors
	vmat->parameters.diffuse = {rgb2.X,rgb2.Y,rgb2.Z};
}

//---------------------------------------------------------------------
// Uniqing
//---------------------------------------------------------------------

//---------------------------------------------------------------------
/** Generals specific code to generate customized render objects for each team color
*/
void W3DAssetManager::Make_HLOD_Unique(W3DRenderObject *robj, Bool geometry, Bool colors)
{
	int num_sub = robj->Get_Num_Sub_Objects();
	for(int i = 0; i < num_sub; i++) {
		W3DRenderObject *sub_obj = robj->Get_Sub_Object(i);
		Make_Unique(sub_obj, geometry, colors);
		REF_PTR_RELEASE(sub_obj);
	}
}

//---------------------------------------------------------------------
/** Generals specific code to generate customized render objects for each team color
*/
void W3DAssetManager::Make_Unique(W3DRenderObject *robj, Bool geometry, Bool colors)
{
	switch (robj->Class_ID())	{
	case W3DRenderObject::CLASSID_MESH:
		Make_Mesh_Unique(robj,geometry,colors);
		break;
	case W3DRenderObject::CLASSID_HLOD:
		Make_HLOD_Unique(robj,geometry,colors);
		break;
	}
}

//---------------------------------------------------------------------
/** Determine what method is used to apply house color to this mesh (if any) */
static Bool getMeshColorMethods(W3DMeshRenderObject *mesh, Bool &vertexColor, Bool &textureColor)
{
	vertexColor = false;
	textureColor = false;

	//Check if mesh is using custom texture containing house color
	auto material = mesh->Get_Material_Info();
	if (material)
	{	for (int j=0; j<static_cast<int>(material->textures.size()); j++)
			if (Assets::Asset_Name_Prefix_Equals_No_Case(material->textures[j].Peek()->Get_Texture_Name(),"ZHC",3))
			{	textureColor = true;
				break;
			}
		material.reset();
	}

	//Check if mesh is using a custom mesh which contains house color in material.
	//Meshes which are part of another model have names in the form "name.name" while
	//isolated meshes are just "name".  We check for both starting with "HOUSECOLOR".
	const char *meshName;
	if ( ( (meshName=strchr(mesh->Get_Name(),'.') ) != nullptr && *(meshName++)) || ( (meshName=mesh->Get_Name()) != nullptr) )
	{	//Check if this object has housecolors on mesh
		if ( Assets::Asset_Name_Prefix_Equals_No_Case(meshName,"HOUSECOLOR", 10))
			vertexColor = true;
	}

	return (vertexColor || textureColor);
}

//---------------------------------------------------------------------
/** Generals specific code to generate customized render objects for each team color
*/
void W3DAssetManager::Make_Mesh_Unique(W3DRenderObject *robj, Bool geometry, Bool colors)
{
	int i;
	W3DMeshRenderObject *mesh=(W3DMeshRenderObject*) robj;
	Bool isVertexColor, isTextureColor;

	//figure out what type of coloring this mesh requires (if any)
	if ((colors && getMeshColorMethods(mesh,isVertexColor,isTextureColor)) || geometry)
	{	//mesh has some house color applied so make those components unique to mesh.

		//Create unique data for this mesh
		if (!geometry)	//scaling geometry automatically makes it unique so not needed here.
			mesh->Make_Unique();

		W3DMeshResource * model = mesh->Get_Model();

		if (colors && isVertexColor)
		{
			auto material = mesh->Get_Material_Info();
			for (i=0; i<static_cast<int>(material->materials.size()); i++)
				material->materials[i].get()->Make_Unique();
			material.reset();
		}

		REF_PTR_RELEASE(model);
	}
}

//---------------------------------------------------------------------
