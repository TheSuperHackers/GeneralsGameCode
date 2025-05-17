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
 *                     $Archive:: /Commando/Code/wwmath/catmullromspline.cpp                  $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/08/00 8:50p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CatmullRomSpline3DClass::Update_Tangents -- computes the tangents at each key             *
 *   CatmullRomSpline3DClass::Get_Factory -- returns the factory for CatmullRomSpline3D        *
 *   CatmullRomSpline3DClass::Save -- save this curve                                          *
 *   CatmullRomSpline3DClass::Load -- load this curve                                          *
 *   CatmullRomSpline1DClass::Update_Tangents -- Computes the tangents at each key             *
 *   CatmullRomSpline1DClass::Get_Factory -- returns the factory for CatmullRomSpline1D        *
 *   CatmullRomSpline1DClass::Save -- Save this curve                                          *
 *   CatmullRomSpline1DClass::Load -- Load this curve                                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "catmullromspline.h"
#include "persistfactory.h"
#include "wwmathids.h"
#include "wwhack.h" 

/*
** Force-Link this module because the linker can't detect that we actually need it...
*/
DECLARE_FORCE_LINK(catmullromspline);
 
/*
** Save-Load stuff
*/
SimplePersistFactoryClass<CatmullRomSpline3DClass,WWMATH_CHUNKID_CATMULLROMSPLINE3D>	_CatmullRomSpline3DFactory;
SimplePersistFactoryClass<CatmullRomSpline1DClass,WWMATH_CHUNKID_CATMULLROMSPLINE1D>	_CatmullRomSpline1DFactory;

enum 
{
	// ID's used by CatmullRomSpline3D
	CATMULLROM3D_CHUNK_HERMITE3D					= 0x00020727,	

	// ID's used by CatmullRomSpline1D
	CATMULLROM1D_CHUNK_HERMITE1D					= 0x00020729,	
};


/*
** Catmull-Rom 3D spline implementation
*/

/***********************************************************************************************
 * CatmullRomSpline3DClass::Update_Tangents -- computes the tangents at each key               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * I'm not sure about the tangents for the endpoints of the curve.                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void CatmullRomSpline3DClass::Update_Tangents(void)
{
	if (Keys.Count() < 2) {
		for (int i=0; i<Keys.Count(); i++) {
			Tangents[0].InTangent.Set(0,0,0);
			Tangents[0].OutTangent.Set(0,0,0);
		}
	}

	// first and last knot
	int end = Keys.Count() - 1;
	Tangents[0].InTangent.Set(0,0,0);
	Tangents[end].OutTangent.Set(0,0,0);

	if (IsLooping) {

		// This really only works if the start and end points have the same position...
		Tangents[0].OutTangent.X = 0.5f*(Keys[1].Point.X - Keys[end-1].Point.X); 			
		Tangents[0].OutTangent.Y = 0.5f*(Keys[1].Point.Y - Keys[end-1].Point.Y); 			
		Tangents[0].OutTangent.Z = 0.5f*(Keys[1].Point.Z - Keys[end-1].Point.Z); 			
		Tangents[end].InTangent = Tangents[0].OutTangent;
	
	} else {
		
		// TODO: second derivative = 0... what is formula?  I'm making this up...
		Tangents[0].OutTangent.X = 0.25f*(Keys[1].Point.X - Keys[0].Point.X); 			
		Tangents[0].OutTangent.Y = 0.25f*(Keys[1].Point.Y - Keys[0].Point.Y); 			
		Tangents[0].OutTangent.Z = 0.25f*(Keys[1].Point.Z - Keys[0].Point.Z); 			

		Tangents[end].InTangent.X = 0.25f*(Keys[end].Point.X - Keys[end-1].Point.X); 			
		Tangents[end].InTangent.Y = 0.25f*(Keys[end].Point.Y - Keys[end-1].Point.Y); 			
		Tangents[end].InTangent.Z = 0.25f*(Keys[end].Point.Z - Keys[end-1].Point.Z); 			
	
	}

	float total_time = (Keys[1].Time - Keys[0].Time) + (Keys[end].Time - Keys[end-1].Time);
	float in_factor = 2.0f * (Keys[end].Time - Keys[end-1].Time) / total_time;
	float out_factor = 2.0f * (Keys[1].Time - Keys[0].Time) / total_time;
	Tangents[end].InTangent *= in_factor;
	Tangents[0].OutTangent *= out_factor;

	// inner knots
	for (int i=1; i<Keys.Count()-1; i++) {
		Tangents[i].InTangent.X = 0.5f*(Keys[i+1].Point.X - Keys[i-1].Point.X);
		Tangents[i].InTangent.Y = 0.5f*(Keys[i+1].Point.Y - Keys[i-1].Point.Y);
		Tangents[i].InTangent.Z = 0.5f*(Keys[i+1].Point.Z - Keys[i-1].Point.Z);
		Tangents[i].OutTangent = Tangents[i].InTangent;

		float in_factor = 2.0f * (Keys[i].Time - Keys[i-1].Time) / (Keys[i+1].Time - Keys[i-1].Time);
		float out_factor = 2.0f * (Keys[i+1].Time - Keys[i].Time) / (Keys[i+1].Time - Keys[i-1].Time);
		Tangents[i].InTangent *= in_factor;			// compensating for the un-even keys
		Tangents[i].OutTangent *= out_factor;
	}
	TangentsDirty = false;
}


/***********************************************************************************************
 * CatmullRomSpline3DClass::Get_Factory -- returns the factory for CatmullRomSpline3D          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
const PersistFactoryClass & CatmullRomSpline3DClass::Get_Factory(void) const
{
	return _CatmullRomSpline3DFactory;
}


/***********************************************************************************************
 * CatmullRomSpline3DClass::Save -- save this curve                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool CatmullRomSpline3DClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(CATMULLROM3D_CHUNK_HERMITE3D);
	HermiteSpline3DClass::Save(csave);
	csave.End_Chunk();
	return true;
}


/***********************************************************************************************
 * CatmullRomSpline3DClass::Load -- load this curve                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool CatmullRomSpline3DClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case CATMULLROM3D_CHUNK_HERMITE3D:
				HermiteSpline3DClass::Load(cload);
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
** The 1D Catmull-Rom implementation.
*/


/***********************************************************************************************
 * CatmullRomSpline1DClass::Update_Tangents -- Computes the tangents at each key               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
void CatmullRomSpline1DClass::Update_Tangents(void)
{
	if (Keys.Count() < 2) {
		for (int i=0; i<Keys.Count(); i++) {
			Tangents[i].InTangent = 0.0f;
			Tangents[i].OutTangent = 0.0f;
		}
		return;
	}

	// first and last knot
	int end = Keys.Count() - 1;
	Tangents[0].InTangent = 0.0f;
	Tangents[end].OutTangent = 0.0f;

	if (IsLooping) {

		// This really only works if the start and end points have the same position...
		Tangents[0].OutTangent = 0.5f*(Keys[1].Point - Keys[end-1].Point);
		Tangents[end].InTangent = Tangents[0].OutTangent;
	
	} else {
		
		// TODO: second derivative = 0... what is formula?  I'm making this up...
		Tangents[0].OutTangent = 0.25f*(Keys[1].Point - Keys[0].Point);

		Tangents[end].InTangent = 0.25f*(Keys[end].Point - Keys[end-1].Point);
	
	}

	float total_time = (Keys[1].Time - Keys[0].Time) + (Keys[end].Time - Keys[end-1].Time);
	float in_factor = 2.0f * (Keys[end].Time - Keys[end-1].Time) / total_time;
	float out_factor = 2.0f * (Keys[1].Time - Keys[0].Time) / total_time;
	Tangents[end].InTangent *= in_factor;
	Tangents[0].OutTangent *= out_factor;

	// inner knots
	for (int i=1; i<Keys.Count()-1; i++) {
		Tangents[i].InTangent = 0.5f*(Keys[i+1].Point - Keys[i-1].Point);
		Tangents[i].OutTangent = Tangents[i].InTangent;

		float in_factor = 2.0f * (Keys[i].Time - Keys[i-1].Time) / (Keys[i+1].Time - Keys[i-1].Time);
		float out_factor = 2.0f * (Keys[i+1].Time - Keys[i].Time) / (Keys[i+1].Time - Keys[i-1].Time);
		Tangents[i].InTangent *= in_factor;			// compensating for the un-even keys
		Tangents[i].OutTangent *= out_factor;
	}
	TangentsDirty = false;
}


/***********************************************************************************************
 * CatmullRomSpline1DClass::Get_Factory -- returns the factory for CatmullRomSpline1D          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
const PersistFactoryClass & CatmullRomSpline1DClass::Get_Factory(void) const
{
	return _CatmullRomSpline1DFactory;
}


/***********************************************************************************************
 * CatmullRomSpline1DClass::Save -- Save this curve                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool CatmullRomSpline1DClass::Save(ChunkSaveClass &csave)
{
	csave.Begin_Chunk(CATMULLROM1D_CHUNK_HERMITE1D);
	HermiteSpline1DClass::Save(csave);
	csave.End_Chunk();
	return true;
}


/***********************************************************************************************
 * CatmullRomSpline1DClass::Load -- Load this curve                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/7/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool CatmullRomSpline1DClass::Load(ChunkLoadClass &cload)
{
	while (cload.Open_Chunk()) {
		
		switch(cload.Cur_Chunk_ID()) 
		{
			case CATMULLROM1D_CHUNK_HERMITE1D:
				HermiteSpline1DClass::Load(cload);
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d\r\n",__FILE__,__LINE__));
				break;
		}
		cload.Close_Chunk();
	}

	return true;
}
