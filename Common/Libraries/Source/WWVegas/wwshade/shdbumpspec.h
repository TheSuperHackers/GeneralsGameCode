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
 *                     $Archive:: /Commando/Code/ww3d2/shdbumpspec.h                           $*
 *                                                                                             *
 *                       $Author:: Kenny_m
 *																																	
 *								$Modtime:: 5/27/02 2:21p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SHDBUMPSPEC_H
#define SHDBUMPSPEC_H

#include "shddef.h"
#include "saveload.h"

class ShdBumpSpecDefClass : public ShdDefClass
{
public:

	/////////////////////////////////////////////////////////////////////
	//	Editable interface requirements
	/////////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE(ShdBumpSpecDefClass, ShdDefClass);

	ShdBumpSpecDefClass();
	ShdBumpSpecDefClass(const ShdBumpSpecDefClass& that);
	virtual ~ShdBumpSpecDefClass();
	
	virtual ShdDefClass*	Clone() const	{ return new ShdBumpSpecDefClass(*this); }

	// Shader Creation (should create a shader compatible with the current hardware/API)
	virtual void						Init();
	virtual void						Shutdown();
	virtual ShdInterfaceClass*		Create() const;

	// Validation methods
	virtual bool						Is_Valid_Config(StringClass &message);

	// Requirements
	virtual bool						Requires_Normals() const { return true; }
	virtual bool						Requires_Tangent_Space_Vectors() const { return true; }
	virtual bool						Requires_Sorting() const { return false; }
	virtual int							Static_Sort_Index() const { return 0; }

	// From PersistClass
	virtual bool						Save(ChunkSaveClass &csave);
	virtual bool						Load(ChunkLoadClass &cload);

	void									Set_Texture_Name(const StringClass& name) { TextureName=name; }
	const StringClass&				Get_Texture_Name() const { return TextureName; }

	void									Set_Bump_Map_Name(const StringClass& name) { BumpMapName=name; }
	const StringClass&				Get_Bump_Map_Name() const { return BumpMapName; }

	void									Set_Ambient(const Vector3& ambient) { Ambient=ambient; }
	const Vector3&						Get_Ambient() const { return Ambient; }

	void									Set_Diffuse(const Vector3& diffuse) { Diffuse=diffuse; }
	const Vector3&						Get_Diffuse() const { return Diffuse; }

	void									Set_Specular(const Vector3& specular) { Specular=specular; }
	const Vector3&						Get_Specular() const { return Specular; }

	void									Set_Diffuse_Bumpiness(const Vector2& diffuse_bumpiness) { Diffuse_Bumpiness=diffuse_bumpiness; }
	const Vector2&						Get_Diffuse_Bumpiness() const { return Diffuse_Bumpiness; }

	void									Set_Specular_Bumpiness(const Vector2& specular_bumpiness) { Specular_Bumpiness=specular_bumpiness; }
	const Vector2&						Get_Specular_Bumpiness() const { return Specular_Bumpiness; }

private:

	static ShdVersion					Version;

	bool									Save_Variables(ChunkSaveClass &csave);
	bool									Load_Variables(ChunkLoadClass &cload);

	StringClass							TextureName;
	StringClass							BumpMapName;

	Vector3								Ambient;
	Vector3								Diffuse;
	Vector3								Specular;

	Vector2								Diffuse_Bumpiness;
	Vector2								Specular_Bumpiness;
};


#endif //SHDBUMPSPEC_H
