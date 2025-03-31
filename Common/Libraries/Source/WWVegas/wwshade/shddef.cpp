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
 *                     $Archive:: wwshade/shddef.cpp                           $*
 *                                                                                             *
 *                  $Org Author:: Jani_p
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 6/07/02 3:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                          $*
 *                                                                                             *
 *   6/07/02	 KJM : Added validation enum
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "shddef.h"
#include "chunkio.h"
#include "w3d_file.h"


ShdDefClass::ShdDefClass(uint32 classid) :
	ClassID(classid),
	Name("UnNamed"),
	SurfaceType(0)
{

	ENUM_PARAM (
		ShdDefClass, SurfaceType, 
		(	
			"Light Metal",						SURFACE_TYPE_LIGHT_METAL,
			"Heavy Metal",						SURFACE_TYPE_HEAVY_METAL,
			"Water",								SURFACE_TYPE_WATER,
			"Sand",								SURFACE_TYPE_SAND,
			"Dirt",								SURFACE_TYPE_DIRT,
			"Mud",								SURFACE_TYPE_MUD,
			"Grass",								SURFACE_TYPE_GRASS,
			"Wood",								SURFACE_TYPE_WOOD,
			"Concrete",							SURFACE_TYPE_CONCRETE,
			"Flesh",								SURFACE_TYPE_FLESH,
			"Rock",								SURFACE_TYPE_ROCK,
			"Snow",								SURFACE_TYPE_SNOW,
			"Ice",								SURFACE_TYPE_ICE,
			"Default",							SURFACE_TYPE_DEFAULT,
			"Glass",								SURFACE_TYPE_GLASS,
			"Cloth",								SURFACE_TYPE_CLOTH,
			"Tiberium Field",					SURFACE_TYPE_TIBERIUM_FIELD,
			"Foliage Permeable",				SURFACE_TYPE_FOLIAGE_PERMEABLE,
			"Glass Permeable",				SURFACE_TYPE_GLASS_PERMEABLE,
			"Ice Permeable",					SURFACE_TYPE_ICE_PERMEABLE,
			"Cloth Permeable",				SURFACE_TYPE_CLOTH_PERMEABLE,
			"Electrical",						SURFACE_TYPE_ELECTRICAL,
			"Electrical Permeable",			SURFACE_TYPE_ELECTRICAL_PERMEABLE,
			"Flammable",						SURFACE_TYPE_FLAMMABLE,
			"Flammable Permeable",			SURFACE_TYPE_FLAMMABLE_PERMEABLE	,
			"Steam",								SURFACE_TYPE_STEAM,
			"Steam Permeable",				SURFACE_TYPE_STEAM_PERMEABLE,
			"Water Permeable",				SURFACE_TYPE_WATER_PERMEABLE,
			"Tiberium Water",					SURFACE_TYPE_TIBERIUM_WATER,
			"Tiberium Water Permeable",	SURFACE_TYPE_TIBERIUM_WATER_PERMEABLE,
			"Underwater Dirt",				SURFACE_TYPE_UNDERWATER_DIRT,
			"Underwater Tiberium Dirt",	SURFACE_TYPE_UNDERWATER_TIBERIUM_DIRT,
			NULL
		)
	);
}

ShdDefClass::ShdDefClass(const ShdDefClass & that):
	ClassID(that.ClassID),
	Name(that.Name),
	SurfaceType(that.SurfaceType)
{
}

ShdDefClass::~ShdDefClass(void)
{
}

const char * ShdDefClass::Get_Name (void) const
{
	return Name;
}

void ShdDefClass::Set_Name (const char *new_name)
{
	Name = new_name;
}

void ShdDefClass::Reset(void)
{
	SurfaceType = 0;
}




/*******************************************************************************
**
** Save-Load methods for ShdDefClass
**
*******************************************************************************/
enum 
{
	CHUNKID_VARIABLES =			0x16490430,

	VARID_NAME =					0x00,
	VARID_SURFACETYPE,
};



//**********************************************************************************************
//! Serialize this ShdDef into a chunk saver
/*!
	@param csave - ChunkSave object to write into
*/
bool ShdDefClass::Save (ChunkSaveClass &csave)
{
	bool retval=true;

	csave.Begin_Chunk(CHUNKID_VARIABLES);	

		retval=Save_Variables(csave);

	csave.End_Chunk ();

	return retval;
}



//**********************************************************************************************
//! Load this ShdDef from a chunk loader
/*!
	@param cload - ChunkLoad object to read from
*/
bool ShdDefClass::Load (ChunkLoadClass &cload)
{
	bool retval = true;

	cload.Open_Chunk();

		retval=cload.Cur_Chunk_ID()==CHUNKID_VARIABLES;
		Load_Variables(cload);

	cload.Close_Chunk();

	return retval;
}


//**********************************************************************************************
//! Save the variables for this object
/*!
	Each variable is stored in its own "micro-chunk" for flexibility.

	@param csave - chunk saver to write to.
*/
bool ShdDefClass::Save_Variables (ChunkSaveClass &csave)
{
	bool retval = true;
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_NAME, Name);
	WRITE_MICRO_CHUNK (csave, VARID_SURFACETYPE, SurfaceType);
	return retval;
}


//**********************************************************************************************
//! Load the variables for this object from a chunk
/*!
	@param cload - chunk loader to read from
*/
bool ShdDefClass::Load_Variables (ChunkLoadClass &cload)
{
	bool retval = true;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) 
	{
		switch (cload.Cur_Micro_Chunk_ID ()) 
		{
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_NAME, Name);
			READ_MICRO_CHUNK (cload, VARID_SURFACETYPE, SurfaceType);
		}

		cload.Close_Micro_Chunk();
	}

	return retval;
}

