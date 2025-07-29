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
 *                     $Archive:: /Commando/Code/ww3d2/light.cpp                              $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 7/10/01 2:27p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   LightClass::LightClass -- Constructor                                                     *
 *   LightClass::LightClass -- copy constructor                                                *
 *   LightClass::operator == -- assignment operator                                            *
 *   LightClass::~LightClass -- destructor                                                     *
 *   LightClass::Clone -- virtual copy constructor                                             *
 *   LightClass::Get_Attenuation_Range -- returns a dist beyond which the light is attenuated  *
 *   LightClass::Get_Obj_Space_Bounding_Sphere -- returns the object space bounding sphere     *
 *   LightClass::Get_Obj_Space_Bounding_Box -- returns the object space bounding box           *
 *   LightClass::Vertex_Processor_Push -- pushes the light into the GERD                       *
 *   LightClass::Vertex_Processor_Pop -- pops the light from the GERD                          *
 *   LightClass::Set_Transform -- sets transform and marks srLight transform as dirty.         *
 *   LightClass::Set_Position -- sets position and marks srLight transform as dirty.           *
 *   LightClass::Notify_Added -- lights add themselves to the VP list when added               *
 *   LightClass::Notify_Removed -- lights remove themselves from the VP list when removed      *
 *   LightClass::Load_W3D -- Initialize this light from a W3D file                             *
 *   LightClass::Save_W3D -- Save this light's settings into a W3D file                        *
 *   LightClass::Get_Factory -- get the PersistFactory for LightClass                          *
 *   LightClass::Save -- persistant object support                                             *
 *   LightClass::Load -- persistant object support                                             *
 *   LightImpClass::LightImpClass -- constructor                                               *
 *   LightImpClass::Process_Push -- exposes the "push" process for an srLight                  *
 *   LightImpClass::Process_Pop -- exposes the "pop" process for an srLight                    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "light.h"
#include "ww3d.h"
#include "ww3dids.h"
#include "w3d_file.h"
#include "w3d_util.h"
#include "w3derr.h"
#include "chunkio.h"
#include "rinfo.h"
#include "scene.h"
#include "persistfactory.h"
#include "statistics.h"



/*
** PersistFactory for LightClasses - lights have custom save-load support
*/
SimplePersistFactoryClass<LightClass,WW3D_PERSIST_CHUNKID_LIGHT>	_LightFactory;

/*
** Chunk ID's used by LightClass's save-load support (different from the W3D file format...)
*/
enum 
{
	LIGHT_CHUNK_W3DFILE						= 0x02157100,			// (w3d format light)
	LIGHT_CHUNK_VARIABLES,												// other state not stored in the w3d format

	LIGHT_VARIABLE_TRANSFORM				= 0x00,					// transform for the light
};



/***********************************************************************************************
 * LightClass::LightClass -- Constructor                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 * type - type of light to create.                                                             *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
LightClass::LightClass(LightType type) :
	Type(type),	
	Flags(0),
	CastShadows(false),
	Intensity(1.0f),
	Ambient(1,1,1),
	Diffuse(1,1,1),
	Specular(1,1,1),
	NearAttenStart(0.0f),
	NearAttenEnd(0.0f),
	FarAttenStart(50.0f),
	FarAttenEnd(100.0f),
	SpotAngle(DEG_TO_RADF(45.0f)),
	SpotAngleCos(0.707f),
	SpotExponent(1.0f),
	SpotDirection(0,0,1)//,
	//Donut (false)

{
	if (type == DIRECTIONAL) {
		Set_Force_Visible(true);	// The light has no position so culling cant work.
	}
}


/***********************************************************************************************
 * LightClass::LightClass -- copy constructor                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
LightClass::LightClass(const LightClass & src) :
	Type(src.Type),	
	Flags(src.Flags),
	CastShadows(src.CastShadows),
	Intensity(src.Intensity),
	Ambient(src.Ambient),
	Diffuse(src.Diffuse),
	Specular(src.Specular),
	NearAttenStart(src.NearAttenStart),
	NearAttenEnd(src.NearAttenEnd),
	FarAttenStart(src.FarAttenStart),
	FarAttenEnd(src.FarAttenEnd),
	SpotAngle(src.SpotAngle),
	SpotAngleCos(src.SpotAngleCos),
	SpotExponent(src.SpotExponent),
	SpotDirection(src.SpotDirection)
{
}


/***********************************************************************************************
 * LightClass::operator == -- assignment operator                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
LightClass & LightClass::operator = (const LightClass & that)
{
	if (this != &that) {
		RenderObjClass::operator = (that);

		Type = that.Type;	
		Flags = that.Flags;
		CastShadows = that.CastShadows;
		Intensity = that.Intensity;
		Ambient = that.Ambient;
		Diffuse = that.Diffuse;
		Specular = that.Specular;
		NearAttenStart = that.NearAttenStart;
		NearAttenEnd = that.NearAttenEnd;
		FarAttenStart = that.FarAttenStart;
		FarAttenEnd = that.FarAttenEnd;
		SpotAngle = that.SpotAngle;
		SpotAngleCos = that.SpotAngleCos;
		SpotExponent = that.SpotExponent;
		SpotDirection = that.SpotDirection;
	}
	return * this;
}


/***********************************************************************************************
 * LightClass::~LightClass -- destructor                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
LightClass::~LightClass(void)
{
}


/***********************************************************************************************
 * LightClass::Clone -- virtual copy constructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * LightClass::Clone(void) const
{
	return W3DNEW LightClass(*this);
}


/***********************************************************************************************
 * LightClass::Notify_Added -- lights add themselves to the VP list when added                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/26/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void LightClass::Notify_Added(SceneClass * scene)
{
	RenderObjClass::Notify_Added(scene);
	scene->Register(this,SceneClass::LIGHT);
}


/***********************************************************************************************
 * LightClass::Notify_Removed -- lights remove themselves from the VP list when removed        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/26/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void LightClass::Notify_Removed(SceneClass * scene)
{
	scene->Unregister(this,SceneClass::LIGHT);
	RenderObjClass::Notify_Removed(scene);
}



/***********************************************************************************************
 * LightClass::Get_Obj_Space_Bounding_Sphere -- returns the object space bounding sphere       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void LightClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	sphere.Center.Set(0,0,0);
	sphere.Radius = Get_Attenuation_Range();
}


/***********************************************************************************************
 * LightClass::Get_Obj_Space_Bounding_Box -- returns the object space bounding box             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void LightClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	float r = Get_Attenuation_Range();
	box.Center.Set(0,0,0);
	box.Extent.Set(r,r,r);
}


/***********************************************************************************************
 * LightClass::Load_W3D -- Initialize this light from a W3D file                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/23/99    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType LightClass::Load_W3D(ChunkLoadClass & cload)
{
	W3dLightStruct lightinfo;

	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == W3D_CHUNK_LIGHT_INFO);
	cload.Read(&lightinfo,sizeof(lightinfo));
	cload.Close_Chunk();

	switch(lightinfo.Attributes & W3D_LIGHT_ATTRIBUTE_TYPE_MASK)
	{
	case W3D_LIGHT_ATTRIBUTE_POINT:
		Type = POINT;
		break;
	case W3D_LIGHT_ATTRIBUTE_DIRECTIONAL:
		Type = DIRECTIONAL;
		break;
	case W3D_LIGHT_ATTRIBUTE_SPOT:
		Type = SPOT;
		break;
	}
	
	Enable_Shadows((lightinfo.Attributes & W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS) == W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS);
	Set_Intensity(lightinfo.Intensity);

	Vector3 color;
	W3dUtilityClass::Convert_Color(lightinfo.Ambient,&color);
	Set_Ambient(color);
	W3dUtilityClass::Convert_Color(lightinfo.Diffuse,&color);
	Set_Diffuse(color);
	W3dUtilityClass::Convert_Color(lightinfo.Specular,&color);
	Set_Specular(color);

	W3dSpotLightStruct				spotinfo;
	W3dLightAttenuationStruct		atteninfo;
	Vector3								vec;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) 
		{
		case W3D_CHUNK_SPOT_LIGHT_INFO:
			cload.Read(&spotinfo,sizeof(spotinfo));
			Set_Spot_Angle(spotinfo.SpotAngle);
			Set_Spot_Exponent(spotinfo.SpotExponent);
			W3dUtilityClass::Convert_Vector(spotinfo.SpotDirection,&vec);
			Set_Spot_Direction(vec);
			break;
		
		case W3D_CHUNK_NEAR_ATTENUATION:
			cload.Read(&atteninfo,sizeof(atteninfo));
			Set_Flag(NEAR_ATTENUATION,true);
			Set_Near_Attenuation_Range(atteninfo.Start,atteninfo.End);
			break;

		case W3D_CHUNK_FAR_ATTENUATION:
			cload.Read(&atteninfo,sizeof(atteninfo));
			Set_Flag(FAR_ATTENUATION,true);
			Set_Far_Attenuation_Range(atteninfo.Start,atteninfo.End);
			break;
		}
		cload.Close_Chunk();
	}

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * LightClass::Save_W3D -- Save this light's settings into a W3D file                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/23/99    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType LightClass::Save_W3D(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(W3D_CHUNK_LIGHT);

	csave.Begin_Chunk(W3D_CHUNK_LIGHT_INFO);

	W3dLightStruct lightinfo;
	memset(&lightinfo,0,sizeof(lightinfo));
	
	switch (Type) 
	{
	case POINT:
		lightinfo.Attributes |= W3D_LIGHT_ATTRIBUTE_POINT;
		break;
	case DIRECTIONAL:
		lightinfo.Attributes |= W3D_LIGHT_ATTRIBUTE_DIRECTIONAL;
		break;
	case SPOT:
		lightinfo.Attributes |= W3D_LIGHT_ATTRIBUTE_SPOT;
		break;
	}

	if (Are_Shadows_Enabled()) {
		lightinfo.Attributes |= W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS;
	}

	Vector3 color;
	Get_Ambient(&color);
	W3dUtilityClass::Convert_Color(color,(&lightinfo.Ambient));
	Get_Diffuse(&color);
	W3dUtilityClass::Convert_Color(color,(&lightinfo.Diffuse));
	Get_Specular(&color);
	W3dUtilityClass::Convert_Color(color,(&lightinfo.Specular));
	
	lightinfo.Intensity = Get_Intensity();
	
	csave.Write(&lightinfo,sizeof(lightinfo));
	csave.End_Chunk();

	if (Type == SPOT) {
		csave.Begin_Chunk(W3D_CHUNK_SPOT_LIGHT_INFO);

		W3dSpotLightStruct spotinfo;
		memset(&spotinfo,0,sizeof(spotinfo));
		spotinfo.SpotAngle = SpotAngle;
		spotinfo.SpotExponent = SpotExponent;
		spotinfo.SpotDirection.X = SpotDirection.X;
		spotinfo.SpotDirection.Y = SpotDirection.Y;
		spotinfo.SpotDirection.Z = SpotDirection.Z;
		csave.Write(&spotinfo,sizeof(spotinfo));
		
		csave.End_Chunk();
	}

	if (Get_Flag(NEAR_ATTENUATION)) {
		csave.Begin_Chunk(W3D_CHUNK_NEAR_ATTENUATION);
		
		double start,end;
		Get_Near_Attenuation_Range(start,end);

		W3dLightAttenuationStruct atten;
		memset(&atten,0,sizeof(atten));
		atten.Start = start;
		atten.End = end;
		csave.Write(&atten,sizeof(atten));

		csave.End_Chunk();
	}

	if (Get_Flag(FAR_ATTENUATION)) {
		csave.Begin_Chunk(W3D_CHUNK_FAR_ATTENUATION);
		
		double start,end;
		Get_Far_Attenuation_Range(start,end);

		W3dLightAttenuationStruct atten;
		memset(&atten,0,sizeof(atten));
		atten.Start = start;
		atten.End = end;
		csave.Write(&atten,sizeof(atten));

		csave.End_Chunk();
	}

	csave.End_Chunk();
	
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * LightClass::Get_Factory -- get the PersistFactory for LightClass                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/23/99    GTH : Created.                                                                 *
 *=============================================================================================*/
const PersistFactoryClass & LightClass::Get_Factory (void) const
{
	return _LightFactory;	
}


/***********************************************************************************************
 * LightClass::Save -- persistant object support                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/23/99    GTH : Created.                                                                 *
 *=============================================================================================*/
bool LightClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk(LIGHT_CHUNK_W3DFILE);
	Save_W3D(csave);
	csave.End_Chunk();

	Matrix3D tm = Get_Transform();
	csave.Begin_Chunk(LIGHT_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,LIGHT_VARIABLE_TRANSFORM,tm);
	csave.End_Chunk();

	return true;
}


/***********************************************************************************************
 * LightClass::Load -- persistant object support                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/23/99    GTH : Created.                                                                 *
 *=============================================================================================*/
bool LightClass::Load (ChunkLoadClass &cload)
{
	Matrix3D tm(1);
	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {

			case LIGHT_CHUNK_W3DFILE:
				// The W3D code is non-symmetrical, the Save function writes a W3D_LIGHT chunk
				// but the load function expects the external user to have opened it.  So open it here...
				cload.Open_Chunk();
				Load_W3D(cload);
				cload.Close_Chunk();
				break;

			case LIGHT_CHUNK_VARIABLES:
				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,LIGHT_VARIABLE_TRANSFORM,tm);
					}
					cload.Close_Micro_Chunk();	
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d",__FILE__,__LINE__));
				break;
		}
		cload.Close_Chunk();
	}
	Set_Transform(tm);
	return true;
}

