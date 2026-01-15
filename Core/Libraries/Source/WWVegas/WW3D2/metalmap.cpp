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
 *                     $Archive:: /Commando/Code/ww3d2/metalmap.cpp                           $*
 *                                                                                             *
 *                  $Org Author:: Hector_y                                  $*
 *                                                                         *
 *                      $Author:: Kenny_m                                  $*
 *                                                                         *
 *                     $Modtime:: 08/05/02 10:44a                          $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 * 08/05/02 KM Texture class redesign
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MMMC::MetalMapManagerClass -- Create metal map manager according to given metal parameters*
 *   MMMC::MetalMapManagerClass -- Create metal map manager from INI                           *
 *   MMMC::~MetalMapManagerClass -- MetalMapManagerClass destructor                            *
 *   MMMC::Get_Metal_Map -- Get the texture for a metal map by id number                       *
 *   MMMC::Metal_Map_Count -- Get the number of metal maps in the manager                      *
 *   MMMC::Update_Lighting -- Update the lighting parameters used for generating the maps      *
 *   MMMC::Update_Textures -- Update  metal map textures (call once/frame before rendering)    *
 *   MMMC::initialize_normal_table -- Utility function to initialize the normal table          *
 *   MMMC::initialize_metal_params -- Utility function (shared CTor code)                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "metalmap.h"
#include "texture.h"
#include "ww3dformat.h"
#include "ww3d.h"
#include <vp.h>
#include <INI.h>
#include <Point.h>
#include <hashtemplate.h>
#include <wwstring.h>
#include <wwmath.h>

/*
** Class static members:
*/
Vector3 * MetalMapManagerClass::_NormalTable = nullptr;

/***********************************************************************************************
 * MMMC::MetalMapManagerClass -- Create metal map manager from INI                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/1999 NH : Created.                                                                  *
 *=============================================================================================*/
MetalMapManagerClass::MetalMapManagerClass(INIClass &ini) :
	MapCount(0),
	Textures(nullptr),
	MetalParameters(nullptr),
	CurrentAmbient(0.0f, 0.0f, 0.0f),
	CurrentMainLightColor(0.0f, 0.0f, 0.0f),
	CurrentMainLightDir(1.0f, 0.0f, 0.0f),
	CurrentCameraDir(1.0f,0.0f,0.0f),
	Use16Bit(false)
{

	// If the static normal table has not been initialized yet, initialize it
	if (!_NormalTable) {
		initialize_normal_table();
	}

	// Determine how many metals are in this file
	char section[255];

	int lp = 0;
	for (; ; lp++) {
		sprintf(section, "Metal%02d", lp);
		if (!ini.Find_Section(section)) {
			break;			// NAK - Mar 8, 2000: changed to a break to fix off by one error in lp
		}
	}

	if (lp > 0) {
		// Create metal params structs and fill from INI:
		MetalParams *metal_params = W3DNEWARRAY MetalParams[lp];
		TPoint3D<float> white_tpoint(255.0f, 255.0f, 255.0f);
		Vector3 white(1.0f, 1.0f, 1.0f);
		Vector3 black(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < lp; i++) {
			sprintf(section, "Metal%02d", i);
			static const float cf =  0.003921568627451f;	// 1 / 255
			TPoint3D<float> color;
			color = ini.Get_Point(section, "AmbientColor", white_tpoint);
			metal_params[i].AmbientColor.Set(color.X * cf, color.Y * cf, color.Z * cf);
			metal_params[i].AmbientColor.Update_Min(white);
			metal_params[i].AmbientColor.Update_Max(black);
			color = ini.Get_Point(section, "DiffuseColor", white_tpoint);
			metal_params[i].DiffuseColor.Set(color.X * cf, color.Y * cf, color.Z * cf);
			metal_params[i].AmbientColor.Update_Min(white);
			metal_params[i].AmbientColor.Update_Max(black);
			color = ini.Get_Point(section, "SpecularColor", white_tpoint);
			metal_params[i].SpecularColor.Set(color.X * cf, color.Y * cf, color.Z * cf);
			metal_params[i].AmbientColor.Update_Min(white);
			metal_params[i].AmbientColor.Update_Max(black);
			float shininess = ini.Get_Float(section, "Shininess", 0.0f);
			metal_params[i].Shininess = WWMath::Clamp(shininess, 0.0f, 127.0f);
		}

		initialize_metal_params(lp, metal_params);
		delete [] metal_params;
	} else {
		assert(0);
	}

	int w,h,bits;
	bool windowed;

	WW3D::Get_Device_Resolution(w,h,bits,windowed);
	Use16Bit=(bits<=16);

	WW3DFormat format=(Use16Bit?WW3D_FORMAT_A4R4G4B4:WW3D_FORMAT_A8R8G8B8);


	for (int i = 0; i < lp; i++) {
		Textures[i]=NEW_REF(TextureClass,(METALMAP_SIZE,METALMAP_SIZE,format,MIP_LEVELS_1));
		Textures[i]->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		Textures[i]->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		StringClass tex_name;
		tex_name.Format("!m%02d.tga", i);
		Textures[i]->Set_Texture_Name(tex_name);
	}
}


/***********************************************************************************************
 * MMMC::~MetalMapManagerClass -- MetalMapManagerClass destructor                              *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/1999 NH : Created.                                                                  *
 *=============================================================================================*/
MetalMapManagerClass::~MetalMapManagerClass(void)
{
	if (Textures) {
		for (int i = 0; i < MapCount; i++) {
			REF_PTR_RELEASE(Textures[i]);
		}
		delete [] Textures;
		Textures = nullptr;
	}

	delete [] MetalParameters;
	MetalParameters = nullptr;
}


/***********************************************************************************************
 * MMMC::Get_Metal_Map -- Get the texture for a metal map by id number                         *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/1999 NH : Created.                                                                  *
 *=============================================================================================*/
TextureClass * MetalMapManagerClass::Get_Metal_Map(int id)
{
	if (id < 0 || id >= MapCount) return nullptr;
	Textures[id]->Add_Ref();
	return Textures[id];
}


/***********************************************************************************************
 * MMMC::Metal_Map_Count -- Get the number of metal maps in the manager                        *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/1999 NH : Created.                                                                  *
 *=============================================================================================*/
int MetalMapManagerClass::Metal_Map_Count(void)
{
	return MapCount;
}


/***********************************************************************************************
 * MMMC::Update_Lighting -- Update the lighting parameters used for generating the maps        *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/1999 NH : Created.                                                                  *
 *=============================================================================================*/
void MetalMapManagerClass::Update_Lighting(const Vector3& ambient, const Vector3& main_light_color,
	const Vector3& main_light_dir, const Vector3& camera_dir)
{
	CurrentAmbient = ambient;
	CurrentMainLightColor = main_light_color;
	CurrentMainLightDir = main_light_dir;
	CurrentCameraDir= camera_dir;
}

/***********************************************************************************************
 * MMMC::Update_Textures -- Update  metal map textures (call once/frame before rendering)      *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/1999 NH : Created.                                                                  *
 *=============================================================================================*/
void MetalMapManagerClass::Update_Textures(void)
{
	// Currently the lighting is done using a simple Phong (actually Blinn) model.
	Vector3 &l = CurrentMainLightDir;
	Vector3 &v = CurrentCameraDir;

	// Calculate halfway vector
	Vector3 h = l+v;
	h.Normalize();

	// NOTE: when our lighting equation gets more complicated we might want to do some testing to
	// detect zero components...

	// Calculate quantities which are the same for all metal maps
	float n_dot_l[METALMAP_SIZE_2];
	float n_dot_h[METALMAP_SIZE_2];

	VectorProcessorClass::DotProduct(n_dot_l,l,_NormalTable,METALMAP_SIZE_2);
	VectorProcessorClass::ClampMin(n_dot_l, n_dot_l, 0.0f, METALMAP_SIZE_2);
	VectorProcessorClass::DotProduct(n_dot_h,h,_NormalTable, METALMAP_SIZE_2);
	VectorProcessorClass::ClampMin(n_dot_h, n_dot_h, 0.0f, METALMAP_SIZE_2);

	// Loop over each metal map and update it
	for (int i = 0; i < MapCount; i++) {
		MetalParams &cur_params = MetalParameters[i];

		// If shinyness > 1, apply it to specular value array
		float *specular = nullptr;
		float temp_specular[METALMAP_SIZE_2];
		float shinyness = cur_params.Shininess;
		if (shinyness > 1.0f) {
			VectorProcessorClass::Power(temp_specular, n_dot_h, shinyness, METALMAP_SIZE_2);
			specular = &(temp_specular[0]);
		} else {
			specular = &(n_dot_h[0]);
		}

		// Generate metal map row by row
		Vector3 specular_color(cur_params.SpecularColor.X * CurrentMainLightColor.X,
			cur_params.SpecularColor.Y * CurrentMainLightColor.Y,
			cur_params.SpecularColor.Z * CurrentMainLightColor.Z);
		Vector3 diffuse_color(cur_params.DiffuseColor.X * CurrentMainLightColor.X,
			cur_params.DiffuseColor.Y * CurrentMainLightColor.Y,
			cur_params.DiffuseColor.Z * CurrentMainLightColor.Z);
		Vector3 ambient_color(cur_params.AmbientColor.X * CurrentAmbient.X,
			cur_params.AmbientColor.Y * CurrentAmbient.Y,
			cur_params.AmbientColor.Z * CurrentAmbient.Z);
		Vector3 white(1.0f, 1.0f, 1.0f);

		SurfaceClass * metal_map_surface = Textures[i]->Get_Surface_Level(0);
		int pitch;
		unsigned char *map=(unsigned char *) metal_map_surface->Lock(&pitch);
		int idx=0;
		for (int y = 0; y < METALMAP_SIZE; y++) {
			for (int x = 0; x < METALMAP_SIZE; x++) {
				Vector3 result = ambient_color + (diffuse_color * n_dot_l[idx]) + (specular_color * specular[idx]);
				result.Update_Min(white);	// Clamp to white

				unsigned char b,g,r,a;
				b= (unsigned char)WWMath::Floor(result.Z * 255.99f);	// B
				g= (unsigned char)WWMath::Floor(result.Y * 255.99f);	// G
				r= (unsigned char)WWMath::Floor(result.X * 255.99f);	// R
				a= 0xFF;													// A

				if (Use16Bit) {
					unsigned short tmp;
					tmp=(a&0xf0)<<8;
					tmp|=(r&0xf0)<<4;
					tmp|=(g&0xf0);
					tmp|=(b&0xf0)>>4;
					*(unsigned short*)&map[2*x]=tmp;
				} else {
					map[4*x]=b;
					map[4*x+1]=g;
					map[4*x+2]=r;
					map[4*x+3]=a;
				}
				idx++;
			}
			map+=pitch;
		}
		metal_map_surface->Unlock();
		REF_PTR_RELEASE(metal_map_surface);
	}
}

/***********************************************************************************************
 * MMMC::initialize_normal_table -- Utility function to initialize the normal table            *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/1999 NH : Created.                                                                  *
 *=============================================================================================*/
void MetalMapManagerClass::initialize_normal_table(void)
{
	// NOTE: changing the actual static _NormalTable member must be the last thing this function
	// does to avoid synchronization errors.

	static Vector3 _normal_table[METALMAP_SIZE_2];

	// Calculate vectors (area outside sphere should be filled with a radial fill of the vectors at
	// the sphere's edge to avoid aliasing artifacts)
	float step = 2.0f / (float)METALMAP_SIZE;
	int idx = 0;
	for (int y = 0; y < METALMAP_SIZE; y++) {
		for (int x = 0; x < METALMAP_SIZE; x++) {
			Vector3 &normal = _normal_table[idx];
			// Set vector to point to surface of unit sphere
			normal.Set((step * (float)x) - 1.0f, (step * (float)y) - 1.0f, 0.0f);
			float z2 = 1 - ((normal.X * normal.X) + (normal.Y * normal.Y));
			z2 = MAX(z2, 0.0f);	// If outside the sphere, treat as if on its edge
			normal.Z = sqrt(z2);
			normal.Normalize();	// Needed for "outside sphere" case and for safety's sake

			idx++;
		}
	}

	_NormalTable = &(_normal_table[0]);

}

/***********************************************************************************************
 * MMMC::initialize_metal_params -- Utility function (shared CTor code)                        *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/1999 NH : Created.                                                                  *
 *=============================================================================================*/
void MetalMapManagerClass::initialize_metal_params(int map_count, MetalParams *metal_params)
{
	MapCount = map_count;
	if (MapCount > 0) {
		Textures = W3DNEWARRAY TextureClass *[MapCount];
		MetalParameters = W3DNEWARRAY MetalParams[MapCount];

		for (int i = 0; i < MapCount; i++) {

			// Copy metal parameters (assert if invalid)
			MetalParameters[i] = metal_params[i];
			assert(MetalParameters[i].AmbientColor.X >= 0.0f && MetalParameters[i].AmbientColor.X <= 1.0f);
			assert(MetalParameters[i].AmbientColor.Y >= 0.0f && MetalParameters[i].AmbientColor.Y <= 1.0f);
			assert(MetalParameters[i].AmbientColor.Z >= 0.0f && MetalParameters[i].AmbientColor.Z <= 1.0f);
			assert(MetalParameters[i].DiffuseColor.X >= 0.0f && MetalParameters[i].DiffuseColor.X <= 1.0f);
			assert(MetalParameters[i].DiffuseColor.Y >= 0.0f && MetalParameters[i].DiffuseColor.Y <= 1.0f);
			assert(MetalParameters[i].DiffuseColor.Z >= 0.0f && MetalParameters[i].DiffuseColor.Z <= 1.0f);
			assert(MetalParameters[i].SpecularColor.X >= 0.0f && MetalParameters[i].SpecularColor.X <= 1.0f);
			assert(MetalParameters[i].SpecularColor.Y >= 0.0f && MetalParameters[i].SpecularColor.Y <= 1.0f);
			assert(MetalParameters[i].SpecularColor.Z >= 0.0f && MetalParameters[i].SpecularColor.Z <= 1.0f);
			assert(MetalParameters[i].Shininess > 0.0f);
		}
	}
}
