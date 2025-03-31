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
 *                 Project Name : WWSHADE                                                         *
 *                                                                                             *
 *                     $Archive:: wwshade/shdglossmask.cpp                           $*
 *                                                                                             *
 *                  $Org Author:: Kenny_m
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 8/01/02 11:39a                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SHDGLOSSMASK_H
#define SHDGLOSSMASK_H

#ifndef SHDINTERFACE_H
#include "shdinterface.h"
#endif

#include "shddef.h"
#include "saveload.h"
#include "shader.h"

class ShdGlossMaskDefClass : public ShdDefClass
{
public:

	/////////////////////////////////////////////////////////////////////
	//	Editable interface requirements
	/////////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE(ShdGlossMaskDefClass, ShdDefClass);

	ShdGlossMaskDefClass();
	ShdGlossMaskDefClass(const ShdGlossMaskDefClass& that);
	virtual ~ShdGlossMaskDefClass();
	
	virtual ShdDefClass*	Clone() const	{ return new ShdGlossMaskDefClass(*this); }

	// Shader Creation (should create a shader compatible with the current hardware/API)
	virtual void						Init();
	virtual void						Shutdown();
	virtual ShdInterfaceClass*		Create() const;

	// Validation methods
	virtual bool						Is_Valid_Config(StringClass &message);

	// Requirements
	virtual bool						Requires_Normals() const { return true; }
	virtual bool						Requires_Tangent_Space_Vectors() const { return false; }
	virtual bool						Requires_Sorting() const { return false; }
	virtual int							Static_Sort_Index() const { return 0; }

	// From PersistClass
	virtual bool						Save(ChunkSaveClass &csave);
	virtual bool						Load(ChunkLoadClass &cload);

	void									Set_Texture_Name(const StringClass& name) { TextureName=name; }
	const StringClass&				Get_Texture_Name() const { return TextureName; }

	void									Set_Ambient(const Vector3& ambient) { Ambient=ambient; }
	const Vector3&						Get_Ambient() const { return Ambient; }

	void									Set_Diffuse(const Vector3& diffuse) { Diffuse=diffuse; }
	const Vector3&						Get_Diffuse() const { return Diffuse; }

	void									Set_Specular(const Vector3& specular) { Specular=specular; }
	const Vector3&						Get_Specular() const { return Specular; }

private:

	bool									Save_Variables(ChunkSaveClass &csave);
	bool									Load_Variables(ChunkLoadClass &cload);

	StringClass							TextureName;

	Vector3								Ambient;
	Vector3								Diffuse;
	Vector3								Specular;
};



class Shd6GlossMaskClass : public ShdInterfaceClass
{
public:
	Shd6GlossMaskClass(const ShdDefClass* def);
	virtual ~Shd6GlossMaskClass();
	
	static void Init();
	static void Shutdown();

	virtual int							Get_Pass_Count() { return 1; }

	virtual int							Get_Texture_Count() const { return 1; }
	virtual TextureClass*			Peek_Texture(int idx) const { return Texture; }

	virtual void						Apply_Shared(int cur_pass, RenderInfoClass& rinfo);
	virtual void						Apply_Instance(int cur_pass, RenderInfoClass& rinfo);

	virtual unsigned					Get_Vertex_Stream_Count() const;
	virtual unsigned					Get_Vertex_Size(unsigned stream) const;
	virtual bool						Use_HW_Vertex_Processing() const { return true; }

	virtual void						Copy_Vertex_Stream
	(
		unsigned stream, 
		void* dest_buffer, 
		const VertexStreamStruct& vss, 
		unsigned vertex_count
	);

protected:

	static Matrix4x4					View_Projection_Matrix;

	D3DMATERIAL8*			Material;

	TextureClass*			Texture;

	Vector4					Ambient;
	Vector4					Diffuse;
	Vector4					Specular;
};


#endif //SHDGLOSSMASK_H
