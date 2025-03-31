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
 *                     $Archive:: /Commando/Code/ww3d2/shdbumpspec.cpp                           $*
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

#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "assetmgr.h"

#include "shdbumpspec.h"
#include "shd6bumpspec.h"
#include "shd7bumpspec.h"
#include "shd8bumpspec.h"

#include "editable.h"
#include "shdclassids.h"
#include "shddeffactory.h"
#include "shdinterface.h"

#include "persistfactory.h"
#include "wwhack.h"

DECLARE_FORCE_LINK(BumpSpecShader);
REGISTER_SHDDEF(ShdBumpSpecDefClass,SHDDEF_CLASSID_BUMPSPEC,"Bump Specular");

// static member
ShdVersion ShdBumpSpecDefClass::Version;

// Save-Load methods for ShdDefClass
enum 
{
	CHUNKID_VARIABLES =			0x16490450,

	VARID_TEXTURE_NAME =					0x00,
	VARID_BUMP_MAP_NAME,

	VARID_AMBIENT_COLOR,
	VARID_DIFFUSE_COLOR,
	VARID_SPECULAR_COLOR,
	VARID_DIFFUSE_BUMPINESS,
	VARID_SPECULAR_BUMPINESS
};

ShdBumpSpecDefClass::ShdBumpSpecDefClass() 
:	ShdDefClass(SHDDEF_CLASSID_BUMPSPEC),
	Ambient(1,1,1),
	Diffuse(1,1,1),
	Specular(1,1,1),
	Diffuse_Bumpiness(1,0),
	Specular_Bumpiness(1,0)
{
	NAMED_TEXTURE_FILENAME_PARAM(ShdBumpSpecDefClass, TextureName, "Base and Alpha Gloss Map", "Targa Files", ".tga");
	NAMED_TEXTURE_FILENAME_PARAM(ShdBumpSpecDefClass, BumpMapName, "Bump Map", "Targa Files", ".tga");

	NAMED_EDITABLE_PARAM(ShdBumpSpecDefClass, ParameterClass::TYPE_COLOR, Ambient, "Ambient");
	NAMED_EDITABLE_PARAM(ShdBumpSpecDefClass, ParameterClass::TYPE_COLOR, Diffuse, "Diffuse");
	NAMED_EDITABLE_PARAM(ShdBumpSpecDefClass, ParameterClass::TYPE_COLOR, Specular, "Specular");
	
	NAMED_EDITABLE_PARAM(ShdBumpSpecDefClass, ParameterClass::TYPE_FLOAT, Diffuse_Bumpiness.X, "Diffuse Bump Scale");
	NAMED_EDITABLE_PARAM(ShdBumpSpecDefClass, ParameterClass::TYPE_FLOAT, Diffuse_Bumpiness.Y, "Diffuse Bump Bias");
	NAMED_EDITABLE_PARAM(ShdBumpSpecDefClass, ParameterClass::TYPE_FLOAT, Specular_Bumpiness.X, "Specular Bump Scale");
	NAMED_EDITABLE_PARAM(ShdBumpSpecDefClass, ParameterClass::TYPE_FLOAT, Specular_Bumpiness.Y, "Specular Bump Bias");
}

ShdBumpSpecDefClass::ShdBumpSpecDefClass(const ShdBumpSpecDefClass& that)
:	ShdDefClass(that),
	Ambient(that.Ambient),
	Diffuse(that.Diffuse),
	Specular(that.Specular),
	Diffuse_Bumpiness(that.Diffuse_Bumpiness),
	Specular_Bumpiness(that.Specular_Bumpiness)
{
	TextureName=that.TextureName;
	BumpMapName=that.BumpMapName;
}

ShdBumpSpecDefClass::~ShdBumpSpecDefClass()
{
}

bool ShdBumpSpecDefClass::Is_Valid_Config(StringClass &message)
{
	return true;
}

bool ShdBumpSpecDefClass::Save(ChunkSaveClass &csave)
{
	ShdDefClass::Save(csave);

	csave.Begin_Chunk(CHUNKID_VARIABLES);	

		bool retval = true;
	
		// only save the file name
		char fname[_MAX_PATH];

		_splitpath(TextureName,NULL,NULL,fname,NULL);
		strcat(fname,".tga");
		TextureName=fname;

		WRITE_MICRO_CHUNK_WWSTRING(csave, VARID_TEXTURE_NAME, TextureName);

		_splitpath(BumpMapName,NULL,NULL,fname,NULL);
		strcat(fname,".tga");
		BumpMapName=fname;

		WRITE_MICRO_CHUNK_WWSTRING(csave, VARID_BUMP_MAP_NAME, BumpMapName);

		WRITE_MICRO_CHUNK(csave, VARID_AMBIENT_COLOR, Ambient);
		WRITE_MICRO_CHUNK(csave, VARID_DIFFUSE_COLOR, Diffuse);
		WRITE_MICRO_CHUNK(csave, VARID_SPECULAR_COLOR, Specular);

		WRITE_MICRO_CHUNK(csave, VARID_DIFFUSE_BUMPINESS, Diffuse_Bumpiness);
		WRITE_MICRO_CHUNK(csave, VARID_SPECULAR_BUMPINESS, Specular_Bumpiness);

	csave.End_Chunk();

	return retval;
}

bool ShdBumpSpecDefClass::Load(ChunkLoadClass &cload)
{
	ShdDefClass::Load(cload);

	//	Loop through all the microchunks that define the variables
	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID())
		{
		case CHUNKID_VARIABLES:
			while (cload.Open_Micro_Chunk()) 
			{
				switch (cload.Cur_Micro_Chunk_ID()) 
				{
				READ_MICRO_CHUNK_WWSTRING(cload, VARID_TEXTURE_NAME, TextureName);
				READ_MICRO_CHUNK_WWSTRING(cload, VARID_BUMP_MAP_NAME, BumpMapName);

				READ_MICRO_CHUNK(cload, VARID_AMBIENT_COLOR, Ambient);
				READ_MICRO_CHUNK(cload, VARID_DIFFUSE_COLOR, Diffuse);
				READ_MICRO_CHUNK(cload, VARID_SPECULAR_COLOR, Specular);

				READ_MICRO_CHUNK(cload, VARID_DIFFUSE_BUMPINESS, Diffuse_Bumpiness);
				READ_MICRO_CHUNK(cload, VARID_SPECULAR_BUMPINESS, Specular_Bumpiness);
				}

				cload.Close_Micro_Chunk();
			}
			break;
		
		default:
			break;
		}

		cload.Close_Chunk();
	}

	return true;
}


void ShdBumpSpecDefClass::Init()
{
	// select shader version
	if ((DX8Wrapper::Get_Current_Caps()->Get_Pixel_Shader_Major_Version())==1 &&
		 (DX8Wrapper::Get_Current_Caps()->Get_Pixel_Shader_Minor_Version())>=1)
	{
		Version.Set(SHDVER_8);
	}
	else if (DX8Wrapper::Get_Current_Caps()->Support_Dot3())
	{
		Version.Set(SHDVER_7);
	}
	else
	{
		Version.Set(SHDVER_6);
	}

	switch (Version.Get())
	{
	case SHDVER_8		: Shd8BumpSpecClass::Init(); break;
	case SHDVER_7		: Shd7BumpSpecClass::Init(); break;
	case SHDVER_6		: Shd6BumpSpecClass::Init(); break;
	}
}

void ShdBumpSpecDefClass::Shutdown()
{
	switch (Version.Get())
	{
	case SHDVER_8		: Shd8BumpSpecClass::Shutdown(); break;
	case SHDVER_7		: Shd7BumpSpecClass::Shutdown(); break;
	case SHDVER_6		: Shd6BumpSpecClass::Shutdown(); break;
	}
}

ShdInterfaceClass* ShdBumpSpecDefClass::Create() const
{
	switch (Version.Get())
	{
	case SHDVER_8		: return new Shd8BumpSpecClass(this); break;
	case SHDVER_7		: return new Shd7BumpSpecClass(this); break;
	case SHDVER_6		: return new Shd6BumpSpecClass(this); break;
	}
	return NULL;
}

