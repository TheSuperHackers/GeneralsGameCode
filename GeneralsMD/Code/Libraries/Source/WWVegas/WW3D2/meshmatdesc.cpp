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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/meshmatdesc.cpp                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/18/02 8:03p                                               $*
 *                                                                                             *
 *                    $Revision:: 28                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "meshmatdesc.h"
#include "texture.h"
#include "vertmaterial.h"
#include "realcrc.h"
#include	"dx8wrapper.h"
#include "dx8caps.h"
#include "meshmdl.h"


/**************************************************************************************************
**
**
** MatBufferClass Implementation
**
**
**************************************************************************************************/
MatBufferClass::MatBufferClass(const MatBufferClass & that) :
	ShareBufferClass<VertexMaterialClass *>(that)
{
	// add a reference for each pointer that was copied...
	for (int i=0; i<Count; i++) {
		if (Array[i]) {
			Array[i]->Add_Ref();
		}
	}
}

MatBufferClass::~MatBufferClass(void)
{
	for (int i=0; i<Count; i++) {
		REF_PTR_RELEASE(Array[i]);
	}
}

void MatBufferClass::Set_Element(int index,VertexMaterialClass * mat)
{
	REF_PTR_SET(Array[index],mat);
}

VertexMaterialClass * MatBufferClass::Get_Element(int index)
{
	if (Array[index]) {
		Array[index]->Add_Ref();
	}
	return Array[index];
}

VertexMaterialClass * MatBufferClass::Peek_Element(int index)
{
	return Array[index];
}


/**************************************************************************************************
**
**
** TexBufferClass Implementation
**
**
**************************************************************************************************/
TexBufferClass::TexBufferClass(const TexBufferClass & that) :
	ShareBufferClass<TextureClass *>(that)
{
	// add a reference for each pointer that was copied...
	for (int i=0; i<Count; i++) {
		if (Array[i]) {
			Array[i]->Add_Ref();
		}
	}
}

TexBufferClass::~TexBufferClass(void)
{
	for (int i=0;i<Count;i++) {
		REF_PTR_RELEASE(Array[i]);
	}
}

void TexBufferClass::Set_Element(int index,TextureClass * tex)
{
	REF_PTR_SET(Array[index],tex);
}

TextureClass * TexBufferClass::Get_Element(int index)
{
	if (Array[index]) {
		Array[index]->Add_Ref();
	}
	return Array[index];
}

TextureClass * TexBufferClass::Peek_Element(int index)
{
	return Array[index];
}


/**************************************************************************************************
**
**
** UVBufferClass Implementation
**
**
**************************************************************************************************/
UVBufferClass::UVBufferClass(const UVBufferClass & that) :
	ShareBufferClass<Vector2>(that)
{
	CRC = that.CRC;
}

bool UVBufferClass::operator == (const UVBufferClass & that)
{
	// NOTE: this only works if you've properly called Update_CRC after filling the array
	return (CRC == that.CRC);
}

bool UVBufferClass::Is_Equal_To(const UVBufferClass & that)
{
	// NOTE: this only works if you've properly called Update_CRC after filling the array
	return (CRC == that.CRC);
}


void UVBufferClass::Update_CRC(void)
{
	CRC = CRC_Memory((unsigned char *)Get_Array(),Get_Count() * sizeof(Vector2));
}



/**************************************************************************************************
**
**
** MeshMatDescClass Implementation
**
**
**************************************************************************************************/
ShaderClass MeshMatDescClass::NullShader(0);	// Used to mark no shader data

MeshMatDescClass::MeshMatDescClass(void) :
	PassCount(1),
	VertexCount(0),
	PolyCount(0)
{
	for (int array=0;array < MAX_COLOR_ARRAYS; array++) {
		ColorArray[array] = nullptr;
	}

	for (int uvarray=0;uvarray<MAX_UV_ARRAYS;uvarray++) {
		UV[uvarray] = nullptr;
	}

	for (int pass=0; pass < MAX_PASSES; pass++) {
		for (int stage=0; stage < MAX_TEX_STAGES; stage++) {
			UVSource[pass][stage] = -1;
			Texture[pass][stage] = nullptr;
			TextureArray[pass][stage] = nullptr;
		}
		DCGSource[pass] = VertexMaterialClass::MATERIAL;
		DIGSource[pass] = VertexMaterialClass::MATERIAL;

		Shader[pass] = 0; //ShaderClass::_PresetOpaqueSolidShader;
		Material[pass] = nullptr;
		ShaderArray[pass] = nullptr;
		MaterialArray[pass] = nullptr;
	}
}

MeshMatDescClass::MeshMatDescClass(const MeshMatDescClass & that) :
	PassCount(1),
	VertexCount(0),
	PolyCount(0)
{
	int pass;
	int stage;
	int array;

	// init everything to nullptr
	for (array=0;array < MAX_COLOR_ARRAYS; array++) {
		ColorArray[array] = nullptr;
	}
	for (array=0;array < MAX_UV_ARRAYS; array++) {
		UV[array] = nullptr;
	}

	for (pass=0; pass < MAX_PASSES; pass++) {
		for (stage=0; stage < MAX_TEX_STAGES; stage++) {
			UVSource[pass][stage] = -1;
			Texture[pass][stage] = nullptr;
			TextureArray[pass][stage] = nullptr;
		}
		DCGSource[pass] = VertexMaterialClass::MATERIAL;
		DIGSource[pass] = VertexMaterialClass::MATERIAL;

		Shader[pass] = 0; //ShaderClass::_PresetOpaqueSolidShader;
		Material[pass] = nullptr;
		ShaderArray[pass] = nullptr;
		MaterialArray[pass] = nullptr;
	}

	*this = that;
}

MeshMatDescClass &
MeshMatDescClass::operator = (const MeshMatDescClass & that)
{
	if (this != &that) {

		PassCount = that.PassCount;
		VertexCount = that.VertexCount;
		PolyCount = that.PolyCount;

		for (int array=0; array<MAX_COLOR_ARRAYS; array++) {
			REF_PTR_SET(ColorArray[array],that.ColorArray[array]);
		}

		for (int uvarray=0; uvarray<MAX_UV_ARRAYS; uvarray++) {
			REF_PTR_SET(UV[uvarray],that.UV[uvarray]);
		}

		for (int pass=0; pass<MAX_PASSES; pass++) {
			for (int stage=0; stage < MAX_TEX_STAGES; stage++) {
				UVSource[pass][stage] = that.UVSource[pass][stage];
				REF_PTR_SET(Texture[pass][stage],that.Texture[pass][stage]);

				// make our own array of texture pointers.
				REF_PTR_RELEASE(TextureArray[pass][stage]);
				if (that.TextureArray[pass][stage]) {
					TextureArray[pass][stage] = NEW_REF(TexBufferClass,(*that.TextureArray[pass][stage]));
				}
			}

			DCGSource[pass] = that.DCGSource[pass];
			DIGSource[pass] = that.DIGSource[pass];

			Shader[pass] = that.Shader[pass];
			REF_PTR_SET(Material[pass],that.Material[pass]);

			// make our own arrays of shaders and vertex material pointers
			// NOTE: We don't just add-ref these arrays, we make our own copies.
			// The only time we add-ref these arrays are when we make alternate material
			// representations within this mesh... Then we re-use the same arrays in different
			// passes...
			REF_PTR_RELEASE(MaterialArray[pass]);
			if (that.MaterialArray[pass]) {
				MaterialArray[pass] = NEW_REF(MatBufferClass,(*that.MaterialArray[pass]));
			}
			REF_PTR_RELEASE(ShaderArray[pass]);
			if (that.ShaderArray[pass]) {
				ShaderArray[pass] = NEW_REF(ShareBufferClass<ShaderClass>,(*that.ShaderArray[pass]));
			}
		}
	}
	return *this;
}

MeshMatDescClass::~MeshMatDescClass(void)
{
	Reset(0,0,0);
}

TextureClass * MeshMatDescClass::Get_Single_Texture(int pass,int stage) const
{
	if (Texture[pass][stage]) {
		Texture[pass][stage]->Add_Ref();
	}
	return Texture[pass][stage];
}

void MeshMatDescClass::Reset(int polycount,int vertcount,int passcount)
{
	PolyCount = polycount;
	VertexCount = vertcount;
	PassCount = passcount;

	for (int array=0; array<MAX_COLOR_ARRAYS; array++) {
		REF_PTR_RELEASE(ColorArray[array]);
	}

	for (int uvarray=0; uvarray<MAX_UV_ARRAYS; uvarray++) {
		REF_PTR_RELEASE(UV[uvarray]);
	}

	for (int pass=0;pass<MAX_PASSES;pass++) {
		for (int stage=0; stage < MAX_TEX_STAGES; stage++) {
			UVSource[pass][stage] = -1;
			REF_PTR_RELEASE(Texture[pass][stage]);
			REF_PTR_RELEASE(TextureArray[pass][stage]);
		}

		DCGSource[pass] = VertexMaterialClass::MATERIAL;
		DIGSource[pass] = VertexMaterialClass::MATERIAL;
		Shader[pass] = 0;
		REF_PTR_RELEASE(ShaderArray[pass]);

		REF_PTR_RELEASE(Material[pass]);
		REF_PTR_RELEASE(MaterialArray[pass]);

	}
}

void MeshMatDescClass::Init_Alternate(MeshMatDescClass & default_materials,MeshMatDescClass & alternate_materials)
{
	// just copy the counts
	PassCount = default_materials.PassCount;
	VertexCount = default_materials.VertexCount;
	PolyCount = default_materials.PolyCount;

	// Color arrays
	for (int array=0; array<MAX_COLOR_ARRAYS; array++) {
		if (alternate_materials.ColorArray[array] != nullptr) {
			REF_PTR_SET(ColorArray[array],alternate_materials.ColorArray[array]);
		} else {
			REF_PTR_SET(ColorArray[array],default_materials.ColorArray[array]);
		}
	}

	// Copy the uv-arrays from the alternate materials to start.  Needed uv arrays from
	// the default material set will be brought over as encountered below
	for (int i=0; i<alternate_materials.Get_UV_Array_Count(); i++) {
		REF_PTR_SET(UV[i],alternate_materials.UV[i]);
	}

	// add-ref the arrays in default_materials except when the same array is present in alternate_materials
	for (int pass = 0; pass < MAX_PASSES; pass++) {
		for (int stage = 0; stage < MAX_TEX_STAGES; stage++) {

			// UV Coorindate arrays, Each UVSource[pass][stage] which is -1 in the alternate_materials
			// but not -1 in the default_materials causes us to copy over a uv array from the default_materials
			// and set its index into our UVSource array.
			if (alternate_materials.UVSource[pass][stage] == -1) {
				if (default_materials.UVSource[pass][stage] != -1) {

					// Look up the uv array in default_materials that we need to bring over.
					int default_uv_source = default_materials.UVSource[pass][stage];
					UVBufferClass * uvarray = default_materials.UV[default_uv_source];
					int found_index = -1;

					// Check if we already have it.
					for (int i=0; i<Get_UV_Array_Count(); i++) {
						if (uvarray->Get_CRC() == UV[i]->Get_CRC()) {
							found_index = i;
							break;
						}
					}

					// If we already have it, just set the source index.  Otherwise add-ref it
					// into a new slot in our uv array and set that index.
					if (found_index != -1) {
						UVSource[pass][stage] = found_index;
					} else {
						int new_index = Get_UV_Array_Count();
						REF_PTR_SET(UV[new_index],default_materials.UV[default_uv_source]);
						UVSource[pass][stage] = new_index;
					}
				}
			} else {
				UVSource[pass][stage] = alternate_materials.UVSource[pass][stage];
			}

			// Texture pointer(s):  If alternate_materials has either a single texture or an array of textures,
			// then add-ref only the texture data it contains.  Otherwise, add-ref the data in default_materials.
			if ((alternate_materials.Texture[pass][stage] != nullptr) || (alternate_materials.TextureArray[pass][stage])) {
				REF_PTR_SET(Texture[pass][stage] , alternate_materials.Texture[pass][stage]);
				REF_PTR_SET(TextureArray[pass][stage] , alternate_materials.TextureArray[pass][stage]);
			} else {
				REF_PTR_SET(Texture[pass][stage] , default_materials.Texture[pass][stage]);
				REF_PTR_SET(TextureArray[pass][stage] , default_materials.TextureArray[pass][stage]);
			}
		}

		// Vertex color configuration
		if (alternate_materials.DCGSource[pass] == VertexMaterialClass::MATERIAL) {
			DCGSource[pass] = default_materials.DCGSource[pass];
		} else {
			DCGSource[pass] = alternate_materials.DCGSource[pass];
		}

		// Shaders, currently I can't tell if the alternate data has a shader...  Can't override the shader for now.
		Shader[pass] = default_materials.Shader[pass];
		REF_PTR_SET(ShaderArray[pass],default_materials.ShaderArray[pass]);

		// Vertex Materials.  If alternate_materials has either a single or array of materials, then copy them
		if ((alternate_materials.Material[pass] != nullptr) || (alternate_materials.MaterialArray[pass] != nullptr)) {
			REF_PTR_SET(Material[pass],alternate_materials.Material[pass]);
			REF_PTR_SET(MaterialArray[pass],alternate_materials.MaterialArray[pass]);
		} else {
			// Dont share vertex materials! (because the UVSources can be different!)
			if (default_materials.Material[pass]) {
				Material[pass] = NEW_REF(VertexMaterialClass,(*(default_materials.Material[pass])));
			} else {
				if (default_materials.MaterialArray[pass]) {
					WWDEBUG_SAY(("Unimplemented case: mesh has more than one default vertex material but no alternate vertex materials have been defined."));
				}
				Material[pass] = nullptr;
			}
		}
	}
}

bool MeshMatDescClass::Is_Empty(void)
{
	for (int array=0; array<MAX_COLOR_ARRAYS; array++) {
		if (ColorArray[array] != nullptr) return false;
	}

	for (int uvarray=0; uvarray<MAX_UV_ARRAYS; uvarray++) {
		if (UV[uvarray] != nullptr) return false;
	}

	for (int pass=0; pass<MAX_PASSES; pass++) {
		for (int stage=0; stage<MAX_TEX_STAGES; stage++) {
			if (Texture[pass][stage] != nullptr) return false;
			if (TextureArray[pass][stage] != nullptr) return false;
		}

//		if (UVIndex[pass] != nullptr) return false;
		if (Material[pass] != nullptr) return false;
		if (MaterialArray[pass] != nullptr) return false;

	}

	return true;
}

void MeshMatDescClass::Set_Single_Material(VertexMaterialClass * vmat,int pass)
{
	REF_PTR_SET(Material[pass],vmat);
}

void MeshMatDescClass::Set_Single_Texture(TextureClass * tex,int pass,int stage)
{
	REF_PTR_SET(Texture[pass][stage],tex);
}

void MeshMatDescClass::Set_Single_Shader(ShaderClass shader,int pass)
{
	Shader[pass] = shader;
}

void MeshMatDescClass::Set_Material(int vidx,VertexMaterialClass * vmat,int pass)
{
	MatBufferClass * mats = Get_Material_Array(pass,true);
	mats->Set_Element(vidx,vmat);
}

void MeshMatDescClass::Set_Shader(int pidx,ShaderClass shader,int pass)
{
	ShaderClass * shaders = Get_Shader_Array(pass,true);
	shaders[pidx] = shader;
}

void MeshMatDescClass::Set_Texture(int pidx,TextureClass * tex,int pass,int stage)
{
	TexBufferClass * textures = Get_Texture_Array(pass,stage,true);
	textures->Set_Element(pidx,tex);
}

VertexMaterialClass * MeshMatDescClass::Get_Material(int vidx,int pass) const
{
	if (MaterialArray[pass]) {

		return MaterialArray[pass]->Get_Element(vidx);

	} else if (Material[pass] != nullptr) {

		Material[pass]->Add_Ref();
		return Material[pass];

	}
	return nullptr;
}

ShaderClass	MeshMatDescClass::Get_Shader(int pidx,int pass) const
{
	if (ShaderArray[pass]) {
		return ShaderArray[pass]->Get_Element(pidx);
	}
	return Shader[pass];
}

TextureClass * MeshMatDescClass::Get_Texture(int pidx,int pass,int stage) const
{
	if (TextureArray[pass][stage]) {

		return TextureArray[pass][stage]->Get_Element(pidx);

	} else if (Texture[pass][stage] != nullptr) {

		Texture[pass][stage]->Add_Ref();
		return Texture[pass][stage];

	}
	return nullptr;
}

VertexMaterialClass * MeshMatDescClass::Peek_Material(int vidx,int pass) const
{
	if (MaterialArray[pass]) {
		return MaterialArray[pass]->Peek_Element(vidx);
	}
	return Material[pass];
}

TextureClass * MeshMatDescClass::Peek_Texture(int pidx,int pass,int stage) const
{
	if (TextureArray[pass][stage]) {
		return TextureArray[pass][stage]->Peek_Element(pidx);
	}
	return Texture[pass][stage];
}

TexBufferClass * MeshMatDescClass::Get_Texture_Array(int pass,int stage,bool create)
{
	if (create && TextureArray[pass][stage] == nullptr) {
		TextureArray[pass][stage] = NEW_REF(TexBufferClass,(PolyCount, "MeshMatDescClass::TextureArray"));
	}
	return TextureArray[pass][stage];
}

MatBufferClass * MeshMatDescClass::Get_Material_Array(int pass,bool create)
{
	if (create && MaterialArray[pass] == nullptr) {
		MaterialArray[pass] = NEW_REF(MatBufferClass,(VertexCount, "MeshMatDescClass::MaterialArray"));
	}
	return MaterialArray[pass];
}

ShaderClass * MeshMatDescClass::Get_Shader_Array(int pass,bool create)
{
	if (create && ShaderArray[pass] == nullptr) {
		ShaderArray[pass] = NEW_REF(ShareBufferClass<ShaderClass>,(PolyCount, "MeshMatDescClass::ShaderArray"));
		ShaderArray[pass]->Clear();
	}
	if (ShaderArray[pass]) {
		return ShaderArray[pass]->Get_Array();
	}
	return nullptr;
}

void MeshMatDescClass::Make_UV_Array_Unique(int pass,int stage)
{
	int uvindex = UVSource[pass][stage];
	if (UV[uvindex]->Num_Refs() > 1) {
		UVBufferClass * unique_uv = NEW_REF(UVBufferClass,(*UV[uvindex]));
		UV[uvindex]->Release_Ref();
		UV[uvindex] = unique_uv;
	}
}

void MeshMatDescClass::Make_Color_Array_Unique(int array)
{
	if ((ColorArray[array] != nullptr) && (ColorArray[array]->Num_Refs() > 1)) {
		ShareBufferClass<unsigned> * unique_color_array = NEW_REF(ShareBufferClass<unsigned>,(*ColorArray[array]));
		ColorArray[array]->Release_Ref();
		ColorArray[array] = unique_color_array;
	}
}

void MeshMatDescClass::Install_UV_Array(int pass,int stage,Vector2 * uvs,int count)
{
	/*
	** Compute the crc of this uv array
	*/
	unsigned int crc = CRC_Memory((unsigned char *)uvs,count * sizeof(Vector2));

	/*
	** See if there is an existing uv-array that matches the one just loaded
	*/
	bool found = false;

	for (int i=0; i<Get_UV_Array_Count(); i++) {
		if (UV[i]->Get_CRC() == crc) {
			found = true;
			Set_UV_Source(pass,stage,i);
			break;
		}
	}

	/*
	** If there was no existing uv array, install this one
	*/
	if (found == false) {

		/*
		** Find the first empty UV-array slot
		*/
		int new_index = 0;
		while ((UV[new_index] != nullptr) && (new_index < MAX_UV_ARRAYS)) {
			new_index++;
		}

		if (new_index < MAX_UV_ARRAYS) {

			WWASSERT(UV[new_index] == nullptr);
			UV[new_index] = NEW_REF(UVBufferClass,(count, "MeshMatDescClass::UV"));
			memcpy(UV[new_index]->Get_Array(),uvs,count * sizeof(Vector2));
			UV[new_index]->Update_CRC();  // update the crc for future comparision
			Set_UV_Source(pass,stage,new_index);
		}
	}
}


void MeshMatDescClass::Post_Load_Process(bool lighting_enabled,MeshModelClass * parent)
{
	/*
	** Configure all vertex materials to source the uv coordinates and colors from the correct arrays
	** Pre-multiply the vertex color arrays.
	*/
	bool set_lighting_to_false=true;
	int pass=0;
	for (; pass<PassCount; pass++) {

		/*
		** If this pass doesn't have a vertex material, create one
		*/
		if ((Material[pass] == nullptr) && (MaterialArray[pass] == nullptr)) {
			Material[pass] = NEW_REF(VertexMaterialClass,());
		}

		/*
		** Configure the materials to source the uv coordinates and colors
		*/
		if (Material[pass] != nullptr) {

			Configure_Material(Material[pass],pass,lighting_enabled);

		} else {
			VertexMaterialClass * prev_mtl = nullptr;
			VertexMaterialClass * mtl = Peek_Material(pass,0);

			for (int vidx=0; vidx<VertexCount; vidx++) {

				mtl = Peek_Material(vidx,pass);
				if ((mtl != prev_mtl) && (mtl != nullptr)) {
					Configure_Material(mtl,pass,lighting_enabled);
					prev_mtl = mtl;
				}
			}
		}

		// Analyze material array types and apply hacks for supporting SR-lighting pipeline if possible.

		if (!ColorArray[0] && !ColorArray[1]) continue;	// If no color arrays, we don't have a problem

		Vector3 single_diffuse(0.0f,0.0f,0.0f);
		Vector3 single_ambient(0.0f,0.0f,0.0f);
		Vector3 single_emissive(0.0f,0.0f,0.0f);
		float single_opacity=1.0f;
		bool single_diffuse_used=true;
		bool single_ambient_used=true;
		bool single_emissive_used=true;
		bool single_opacity_used=true;
		bool diffuse_used=false;
		bool ambient_used=false;
		bool emissive_used=false;
		bool opacity_used=false;

		Vector3 mtl_diffuse;
		Vector3 mtl_ambient;
		Vector3 mtl_emissive;
		float mtl_opacity = 1.0f;

		VertexMaterialClass * prev_mtl = nullptr;
		VertexMaterialClass * mtl = Peek_Material(0, pass);
		if (mtl) {
			mtl->Get_Diffuse(&single_diffuse);
			single_opacity = mtl->Get_Opacity();
			mtl->Get_Ambient(&single_ambient);
			mtl->Get_Emissive(&single_emissive);

			if (single_diffuse.X || single_diffuse.Y || single_diffuse.Z) diffuse_used=true;
			if (single_ambient.X || single_ambient.Y || single_ambient.Z) ambient_used=true;
			if (single_emissive.X || single_emissive.Y || single_emissive.Z) emissive_used=true;
			if (single_opacity!=1.0f) opacity_used=true;
		}

		for (int vidx=0; vidx<VertexCount; vidx++) {
			mtl = Peek_Material(vidx,pass);
			if (mtl != prev_mtl) {
				prev_mtl = mtl;
				mtl->Get_Diffuse(&mtl_diffuse);
				mtl_opacity = mtl->Get_Opacity();
				mtl->Get_Ambient(&mtl_ambient);
				mtl->Get_Emissive(&mtl_emissive);
			}

			if (mtl_diffuse.X!=single_diffuse.X || mtl_diffuse.Y!=single_diffuse.Y || mtl_diffuse.Z!=single_diffuse.Z) {
				single_diffuse_used=false;
			}
			if (mtl_ambient.X!=single_ambient.X || mtl_ambient.Y!=single_ambient.Y || mtl_ambient.Z!=single_ambient.Z) {
				single_ambient_used=false;
			}
			if (mtl_emissive.X!=single_emissive.X || mtl_emissive.Y!=single_emissive.Y || mtl_emissive.Z!=single_emissive.Z) {
				single_emissive_used=false;
			}
			if (mtl_opacity!=single_opacity) {
				single_opacity_used=false;
			}

			if (mtl_diffuse.X || mtl_diffuse.Y || mtl_diffuse.Z) diffuse_used=true;
			if (mtl_ambient.X || mtl_ambient.Y || mtl_ambient.Z) ambient_used=true;
			if (mtl_emissive.X || mtl_emissive.Y || mtl_emissive.Z) emissive_used=true;
			if (mtl_opacity!=1.0f) opacity_used=true;

		}

		// If both DCG and DIG arrays are submitted, multiply them together to DCG channel
		if ((DCGSource[pass] != VertexMaterialClass::MATERIAL) && (ColorArray[0] != nullptr) &&
			 (DIGSource[pass] != VertexMaterialClass::MATERIAL) && (ColorArray[1] != nullptr)) {
			unsigned * diffuse_array = ColorArray[0]->Get_Array();
			unsigned * emissive_array = ColorArray[1]->Get_Array();

			for (int vidx=0; vidx<VertexCount; vidx++) {
				Vector4 diffuse=DX8Wrapper::Convert_Color(diffuse_array[vidx]);
				Vector4 emissive=DX8Wrapper::Convert_Color(emissive_array[vidx]);
				diffuse.X *= emissive.X;
				diffuse.Y *= emissive.Y;
				diffuse.Z *= emissive.Z;
				diffuse_array[vidx]=DX8Wrapper::Convert_Color(diffuse);
			}
		}
		DIGSource[pass]=VertexMaterialClass::MATERIAL;	// DIG channel no more

		if ((DCGSource[pass] != VertexMaterialClass::MATERIAL) && (ColorArray[0] != nullptr)) {
			unsigned * diffuse_array = ColorArray[0]->Get_Array();
			Vector3 mtl_diffuse;
			float mtl_opacity = 1.0f;

			VertexMaterialClass * prev_mtl = nullptr;
			VertexMaterialClass * mtl = Peek_Material(0,pass);

			for (int vidx=0; vidx<VertexCount; vidx++) {

				mtl = Peek_Material(vidx,pass);
				if (mtl != prev_mtl) {
					prev_mtl = mtl;
					mtl->Get_Diffuse(&mtl_diffuse);
					mtl_opacity = mtl->Get_Opacity();
				}

				// If only diffuse is used apply diffuse to color channel and set diffuse source to color 1
				if (diffuse_used && !ambient_used && !emissive_used) {
					Vector4 diffuse=DX8Wrapper::Convert_Color(diffuse_array[vidx]);
					diffuse.X *= mtl_diffuse.X;
					diffuse.Y *= mtl_diffuse.Y;
					diffuse.Z *= mtl_diffuse.Z;
					diffuse.W *= mtl_opacity;
					diffuse_array[vidx]=DX8Wrapper::Convert_Color(diffuse);

					mtl->Set_Ambient_Color_Source(VertexMaterialClass::MATERIAL);
					mtl->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
					mtl->Set_Emissive_Color_Source(VertexMaterialClass::MATERIAL);
				}

				// If diffuse and ambient are used, apply diffuse to color channel and set diffuse
				// and ambient sources to color 1. (this is not completely correct if diffuse and
				// ambient are different but is probably the most reasonable thing to do. Why set
				// diffuse and ambient differently anyway?)
				if (diffuse_used && ambient_used && !emissive_used) {
					Vector4 diffuse=DX8Wrapper::Convert_Color(diffuse_array[vidx]);
					diffuse.X *= mtl_diffuse.X;
					diffuse.Y *= mtl_diffuse.Y;
					diffuse.Z *= mtl_diffuse.Z;
					diffuse.W *= mtl_opacity;
					diffuse_array[vidx]=DX8Wrapper::Convert_Color(diffuse);

					mtl->Set_Ambient_Color_Source(VertexMaterialClass::COLOR1);
					mtl->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
					mtl->Set_Emissive_Color_Source(VertexMaterialClass::MATERIAL);
				}

				// If only ambient is used apply ambient to color channel and set ambient source to color 1
				if (!diffuse_used && ambient_used && !emissive_used) {
					Vector4 diffuse=DX8Wrapper::Convert_Color(diffuse_array[vidx]);
					diffuse.X *= mtl_ambient.X;
					diffuse.Y *= mtl_ambient.Y;
					diffuse.Z *= mtl_ambient.Z;
					diffuse.W *= mtl_opacity;
					diffuse_array[vidx]=DX8Wrapper::Convert_Color(diffuse);

					mtl->Set_Ambient_Color_Source(VertexMaterialClass::COLOR1);
					mtl->Set_Diffuse_Color_Source(VertexMaterialClass::MATERIAL);
					mtl->Set_Emissive_Color_Source(VertexMaterialClass::MATERIAL);
				}

				// If only emissive is used apply emissive to color channel, set diffuse source to color 1, and turn off lighting
				if (!diffuse_used && !ambient_used && emissive_used) {
					Vector4 diffuse=DX8Wrapper::Convert_Color(diffuse_array[vidx]);
					diffuse.X *= mtl_emissive.X;
					diffuse.Y *= mtl_emissive.Y;
					diffuse.Z *= mtl_emissive.Z;
					diffuse.W *= mtl_opacity;
					diffuse_array[vidx]=DX8Wrapper::Convert_Color(diffuse);

					mtl->Set_Ambient_Color_Source(VertexMaterialClass::MATERIAL);
					mtl->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
					mtl->Set_Emissive_Color_Source(VertexMaterialClass::MATERIAL);
//					mtl->Set_Lighting(false);
				}
				else {
					if (PassCount!=1) {
						set_lighting_to_false=false;		// Lighting can only be set to false if ALL passes and ALL materials are requesting it
					}
				}
			}
		}
	}


	/*
	** HACK: Kill BUMPENV passes on hardware that doesn't support BUMPENV
	** HACK: Set lighting to false on all passes if all passes are of type NO DIFFUSE, NO AMBIENT, YES EMISSIVE
	*/
	for (pass=0; pass<PassCount; pass++) {
		bool kill_pass = false;

		/*
		// HY: Earth and beyond uses a different fallback from Renegade with regards to bump environment maps
		// we keep the pass but change it to an unbumped environment
		if ( (Shader[pass].Get_Primary_Gradient() == ShaderClass::GRADIENT_BUMPENVMAP) &&
			  (!DX8Wrapper::Is_Initted() || DX8Wrapper::Get_Current_Caps()->Support_Bump_Envmap() == false) )
		{
			kill_pass = true;
		}

		if ( (Shader[pass].Get_Primary_Gradient() == ShaderClass::GRADIENT_BUMPENVMAPLUMINANCE) &&
			  (!DX8Wrapper::Is_Initted() || DX8Wrapper::Get_Current_Caps()->Support_Bump_Envmap_Luminance() == false) )
		{
			kill_pass = true;
		}
		*/

		if (kill_pass) {
			if (Material[pass] != nullptr) {
				Material[pass]->Set_Ambient(0,0,0);
				Material[pass]->Set_Diffuse(0,0,0);
				Material[pass]->Set_Emissive(0,0,0);
				Material[pass]->Set_Specular(0,0,0);
			}

			Shader[pass].Set_Texturing(ShaderClass::TEXTURING_DISABLE);
			Shader[pass].Set_Post_Detail_Color_Func(ShaderClass::DETAILCOLOR_DISABLE);
			Shader[pass].Set_Post_Detail_Alpha_Func(ShaderClass::DETAILALPHA_DISABLE);
		}
		// Set lighting to false if requested in all passes...
		else if (set_lighting_to_false) {
			Vector3 single_diffuse(0.0f,0.0f,0.0f);
			Vector3 single_ambient(0.0f,0.0f,0.0f);
			Vector3 single_emissive(0.0f,0.0f,0.0f);
			bool diffuse_used=false;
			bool ambient_used=false;
			bool emissive_used=false;

			Vector3 mtl_diffuse;
			Vector3 mtl_ambient;
			Vector3 mtl_emissive;

			VertexMaterialClass * prev_mtl = nullptr;
			VertexMaterialClass * mtl = Peek_Material(0, pass);
			if (mtl) {
				mtl->Get_Diffuse(&single_diffuse);
				mtl->Get_Ambient(&single_ambient);
				mtl->Get_Emissive(&single_emissive);

				if (single_diffuse.X || single_diffuse.Y || single_diffuse.Z) diffuse_used=true;
				if (single_ambient.X || single_ambient.Y || single_ambient.Z) ambient_used=true;
				if (single_emissive.X || single_emissive.Y || single_emissive.Z) emissive_used=true;
			}

			for (int vidx=0; vidx<VertexCount; vidx++) {
				mtl = Peek_Material(vidx,pass);
				if (mtl != prev_mtl) {
					prev_mtl = mtl;
					mtl->Get_Diffuse(&mtl_diffuse);
					mtl->Get_Ambient(&mtl_ambient);
					mtl->Get_Emissive(&mtl_emissive);
				}

				if (mtl_diffuse.X || mtl_diffuse.Y || mtl_diffuse.Z) diffuse_used=true;
				if (mtl_ambient.X || mtl_ambient.Y || mtl_ambient.Z) ambient_used=true;
				if (mtl_emissive.X || mtl_emissive.Y || mtl_emissive.Z) emissive_used=true;
			}

			if ((DCGSource[pass] != VertexMaterialClass::MATERIAL) && (ColorArray[0] != nullptr)) {
				VertexMaterialClass * prev_mtl = nullptr;
				VertexMaterialClass * mtl = Peek_Material(0,pass);
				for (int vidx=0; vidx<VertexCount; vidx++) {
					mtl = Peek_Material(vidx,pass);
					if (mtl != prev_mtl) {
						prev_mtl = mtl;
						// If only emissive is used apply emissive to color channel, set diffuse source to color 1, and turn off lighting
						if (!diffuse_used && !ambient_used && emissive_used) {
							mtl->Set_Lighting(false);
						}
					}
				}
			}
		}
	}
}

void MeshMatDescClass::Configure_Material(VertexMaterialClass * mtl,int pass,bool lighting_enabled)
{
	mtl->Set_Diffuse_Color_Source(DCGSource[pass]);
	mtl->Set_Emissive_Color_Source(DIGSource[pass]);

	mtl->Set_Lighting(lighting_enabled);

	for (int stage=0; stage<MAX_TEX_STAGES; stage++) {
		int src = UVSource[pass][stage];
		if (src == -1) {
			src = 0;
		}
		mtl->Set_UV_Source(stage,src);
	}
}

bool MeshMatDescClass::Do_Mappers_Need_Normals(void)
{
	if (DX8Wrapper::Is_Initted() && DX8Wrapper::Get_Current_Caps()->Support_NPatches() && WW3D::Get_NPatches_Level()>1) return true;

	for (int pass=0; pass<PassCount; pass++) {
		/*
		** Check the materials on this pass to see if any have mappers which require normals
		*/
		if (Material[pass] != nullptr) {

			if (Material[pass]->Do_Mappers_Need_Normals()) return true;

		} else {
			VertexMaterialClass * prev_mtl = nullptr;
			VertexMaterialClass * mtl = Peek_Material(pass,0);

			for (int vidx=0; vidx<VertexCount; vidx++) {

				mtl = Peek_Material(vidx,pass);
				if ((mtl != prev_mtl) && (mtl != nullptr)) {

					if (mtl->Do_Mappers_Need_Normals()) return true;
					prev_mtl = mtl;
				}
			}
		}
	}

	return false;
}
