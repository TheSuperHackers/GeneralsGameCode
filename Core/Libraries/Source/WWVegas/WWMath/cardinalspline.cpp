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
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/cardinalspline.cpp                    $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/16/01 4:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "cardinalspline.h"
#include "wwdebug.h"
#include "persistfactory.h"
#include "wwmathids.h"
#include "wwhack.h"

/*
** Force-Link this module because the linker can't detect that we actually need it...
*/
DECLARE_FORCE_LINK(cardinalspline);


/*
** Save-Load stuff
*/
SimplePersistFactoryClass<CardinalSpline3DClass,WWMATH_CHUNKID_CARDINALSPLINE3D>	_CardinalSpline3DFactory;
SimplePersistFactoryClass<CardinalSpline1DClass,WWMATH_CHUNKID_CARDINALSPLINE1D>	_CardinalSpline1DFactory;

enum 
{
	// ID's used by CardinalSpline3D
	CARDINAL3D_CHUNK_HERMITE3D					= 0x02070957,	
	CARDINAL3D_CHUNK_TIGHTNESSKEYS,			

	// ID's used by CardinalSpline1D
	CARDINAL1D_CHUNK_HERMITE1D					= 0x02070959,	
	CARDINAL1D_CHUNK_TIGHTNESSKEYS
};


/*
** CardinalSpline3DClass Implementation
*/
int CardinalSpline3DClass::Add_Key(const Vector3 & point,float t)
{
	int index = HermiteSpline3DClass::Add_Key(point,t);
	float tightness = 0.5f;
	Tightness.Insert(index,tightness);
	return index;
}

void CardinalSpline3DClass::Remove_Key(int i)
{
	Tightness.Delete(i);
	HermiteSpline3DClass::Remove_Key(i);
}

void CardinalSpline3DClass::Clear_Keys(void)
{
	Tightness.Clear();
	HermiteSpline3DClass::Clear_Keys();
}

void CardinalSpline3DClass::Set_Tightness(int i,float tightness)
{
	WWASSERT(i >= 0);
	WWASSERT(i < Tightness.Count());
	Tightness[i] = tightness;
	TangentsDirty = true;
}

float CardinalSpline3DClass::Get_Tightness(int i)
{
	return Tightness[i];
}

void CardinalSpline3DClass::Update_Tangents(void)
{
	if (Keys.Count() < 2) {
		for (int i=0; i<Keys.Count(); i++) {
			Tangents[0].InTangent.Set(0,0,0);
			Tangents[0].OutTangent.Set(0,0,0);
		}
		return;
	}

	// First and Last Key: 
	// Only need to compute the OutTangent for key[0] and the InTangent for key[end]
	int end = Keys.Count() - 1;
	Tangents[0].InTangent.Set(0,0,0);
	Tangents[end].OutTangent.Set(0,0,0);

	if (IsLooping) {

		// This really only works if the start and end points have the same position...
		Tangents[0].OutTangent.X = (1.0f - Tightness[0])*(Keys[1].Point.X - Keys[end-1].Point.X); 			
		Tangents[0].OutTangent.Y = (1.0f - Tightness[0])*(Keys[1].Point.Y - Keys[end-1].Point.Y); 			
		Tangents[0].OutTangent.Z = (1.0f - Tightness[0])*(Keys[1].Point.Z - Keys[end-1].Point.Z); 			
		Tangents[end].InTangent = Tangents[0].OutTangent;

	} else {
		
		Tangents[0].OutTangent.X = (1.0f - Tightness[0])*(Keys[1].Point.X - Keys[0].Point.X); 			
		Tangents[0].OutTangent.Y = (1.0f - Tightness[0])*(Keys[1].Point.Y - Keys[0].Point.Y); 			
		Tangents[0].OutTangent.Z = (1.0f - Tightness[0])*(Keys[1].Point.Z - Keys[0].Point.Z); 			

		Tangents[end].InTangent.X = (1.0f - Tightness[0])*(Keys[end].Point.X - Keys[end-1].Point.X); 			
		Tangents[end].InTangent.Y = (1.0f - Tightness[0])*(Keys[end].Point.Y - Keys[end-1].Point.Y); 			
		Tangents[end].InTangent.Z = (1.0f - Tightness[0])*(Keys[end].Point.Z - Keys[end-1].Point.Z); 			

	}

	float total_time = (Keys[1].Time - Keys[0].Time) + (Keys[end].Time - Keys[end-1].Time);
	float in_factor = 2.0f * (Keys[end].Time - Keys[end-1].Time) / total_time;
	float out_factor = 2.0f * (Keys[1].Time - Keys[0].Time) / total_time;
	Tangents[end].InTangent *= in_factor;
	Tangents[0].OutTangent *= out_factor;


	// inner knots
	for (int i=1; i<Keys.Count()-1; i++) {
		Tangents[i].InTangent.X = (1.0f - Tightness[i])*(Keys[i+1].Point.X - Keys[i-1].Point.X);
		Tangents[i].InTangent.Y = (1.0f - Tightness[i])*(Keys[i+1].Point.Y - Keys[i-1].Point.Y);
		Tangents[i].InTangent.Z = (1.0f - Tightness[i])*(Keys[i+1].Point.Z - Keys[i-1].Point.Z);
		Tangents[i].OutTangent = Tangents[i].InTangent;

		float in_factor = 2.0f * (Keys[i].Time - Keys[i-1].Time) / (Keys[i+1].Time - Keys[i-1].Time);
		float out_factor = 2.0f * (Keys[i+1].Time - Keys[i].Time) / (Keys[i+1].Time - Keys[i-1].Time);
		
		Tangents[i].InTangent *= in_factor;			// compensating for the un-even keys
		Tangents[i].OutTangent *= out_factor;
	}
	TangentsDirty = false;
}

const PersistFactoryClass & CardinalSpline3DClass::Get_Factory(void) const
{
	return _CardinalSpline3DFactory;
}

bool CardinalSpline3DClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(CARDINAL3D_CHUNK_HERMITE3D);
	HermiteSpline3DClass::Save(csave);
	csave.End_Chunk();
	
	csave.Begin_Chunk(CARDINAL3D_CHUNK_TIGHTNESSKEYS);
	for (int i=0; i<Tightness.Count(); i++) {
		float tightness = Tightness[i];
		csave.Write(&(tightness),sizeof(tightness));
	}
	csave.End_Chunk();
	return true;
}

bool CardinalSpline3DClass::Load(ChunkLoadClass &cload)
{
	int i;
	float tightness;

	// reset the array of tightness keys
	Tightness.Delete_All();

	// read in the chunks
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case CARDINAL3D_CHUNK_HERMITE3D:
				HermiteSpline3DClass::Load(cload);
				break;

			case CARDINAL3D_CHUNK_TIGHTNESSKEYS:
				for (i=0; i<Keys.Count(); i++) {
					cload.Read(&(tightness),sizeof(tightness));
					Tightness.Add(tightness);
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",__FILE__,__LINE__));
				break;
		}
		cload.Close_Chunk();
	}

	return true;
}


/*
** CardinalSpline1DClass Implementation
*/
int CardinalSpline1DClass::Add_Key(float point,float t)
{
	int index = HermiteSpline1DClass::Add_Key(point,t);
	float tightness = 0.5f;
	Tightness.Insert(index,tightness);
	return index;
}

void CardinalSpline1DClass::Remove_Key(int i)
{
	Tightness.Delete(i);
	HermiteSpline1DClass::Remove_Key(i);
}

void CardinalSpline1DClass::Clear_Keys(void)
{
	Tightness.Clear();
	HermiteSpline1DClass::Clear_Keys();
}

void CardinalSpline1DClass::Set_Tightness(int i,float tightness)
{
	WWASSERT(i >= 0);
	WWASSERT(i < Tightness.Count());
	Tightness[i] = tightness;
	TangentsDirty = true;
}

float CardinalSpline1DClass::Get_Tightness(int i)
{
	return Tightness[i];
}

void CardinalSpline1DClass::Update_Tangents(void)
{
	if (Keys.Count() < 2) {
		for (int i=0; i<Keys.Count(); i++) {
			Tangents[0].InTangent = 0;
			Tangents[0].OutTangent = 0;
		}
	}

	// First and Last Key: 
	// Only need to compute the OutTangent for key[0] and the InTangent for key[end]
	int end = Keys.Count() - 1;
	Tangents[0].InTangent = 0;
	Tangents[end].OutTangent = 0;

	if (IsLooping) {

		// This really only works if the start and end points have the same position...
		Tangents[0].OutTangent = (1.0f - Tightness[0])*(Keys[1].Point - Keys[end-1].Point); 			
		Tangents[end].InTangent = Tangents[0].OutTangent;

	} else {
		
		Tangents[0].OutTangent = (1.0f - Tightness[0])*(Keys[1].Point - Keys[0].Point); 			
		Tangents[end].InTangent = (1.0f - Tightness[0])*(Keys[end].Point - Keys[end-1].Point); 			
	}

	float total_time = (Keys[1].Time - Keys[0].Time) + (Keys[end].Time - Keys[end-1].Time);
	float in_factor = 2.0f * (Keys[end].Time - Keys[end-1].Time) / total_time;
	float out_factor = 2.0f * (Keys[1].Time - Keys[0].Time) / total_time;
	Tangents[end].InTangent *= in_factor;
	Tangents[0].OutTangent *= out_factor;


	// inner knots
	for (int i=1; i<Keys.Count()-1; i++) {
		Tangents[i].InTangent = (1.0f - Tightness[i])*(Keys[i+1].Point - Keys[i-1].Point);
		Tangents[i].OutTangent = Tangents[i].InTangent;

		float in_factor = 2.0f * (Keys[i].Time - Keys[i-1].Time) / (Keys[i+1].Time - Keys[i-1].Time);
		float out_factor = 2.0f * (Keys[i+1].Time - Keys[i].Time) / (Keys[i+1].Time - Keys[i-1].Time);
		
		Tangents[i].InTangent *= in_factor;			// compensating for the un-even keys
		Tangents[i].OutTangent *= out_factor;
	}
	TangentsDirty = false;
}

const PersistFactoryClass & CardinalSpline1DClass::Get_Factory(void) const
{
	return _CardinalSpline1DFactory;
}

bool CardinalSpline1DClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(CARDINAL1D_CHUNK_HERMITE1D);
	HermiteSpline1DClass::Save(csave);
	csave.End_Chunk();
	
	csave.Begin_Chunk(CARDINAL1D_CHUNK_TIGHTNESSKEYS);
	for (int i=0; i<Tightness.Count(); i++) {
		float tightness = Tightness[i];
		csave.Write(&(tightness),sizeof(tightness));
	}
	csave.End_Chunk();
	return true;
}

bool CardinalSpline1DClass::Load(ChunkLoadClass &cload)
{
	int i;
	float tightness;

	// reset the array of tangents
	Tightness.Delete_All();

	// read in the chunks
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case CARDINAL1D_CHUNK_HERMITE1D:
				HermiteSpline1DClass::Load(cload);
				break;

			case CARDINAL1D_CHUNK_TIGHTNESSKEYS:
				for (i=0; i<Keys.Count(); i++) {
					cload.Read(&(tightness),sizeof(tightness));
					Tightness.Add(tightness);
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",__FILE__,__LINE__));
				break;
		}
		cload.Close_Chunk();
	}

	return true;
}


