/*
**	Command & Conquer Generals(tm)
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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/lightenvironment.cpp                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/01/01 5:40p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "lightenvironment.h"
#include "matrix3d.h"
#include "camera.h"
#include "light.h"


/*
** Constants
*/
const float DIFFUSE_TO_AMBIENT_FRACTION = 1.0f;


/*
** Static variables
*/
// TheSuperHackers @fix xezon 13/03/2025 Set integer type as per the original.
// TODO: Investigate if it should be a float.
static int _LightingLODCutoff			= 0.5f;	
static int _LightingLODCutoff2			= 0.5f * 0.5f;


/************************************************************************************************
**
** LightEnvironmentClass::InputLightStruct Implementation
**
************************************************************************************************/

void LightEnvironmentClass::InputLightStruct::Init
(
	const LightClass & light,
	const Vector3 & object_center
)
{
	m_point = false; 
	switch(light.Get_Type()) 
	{
	case LightClass::POINT:
	case LightClass::SPOT:
		Init_From_Point_Or_Spot_Light(light,object_center);
		break;
	case LightClass::DIRECTIONAL:
		Init_From_Directional_Light(light,object_center);
		break;
	};
}

void LightEnvironmentClass::InputLightStruct::Init_From_Point_Or_Spot_Light
(
	const LightClass & light,
	const Vector3 & object_center
)
{
	/*
	** Compute the direction vector and the distance to the light
	*/
	Direction = light.Get_Position() - object_center;
	float dist = Direction.Length();
	if (dist > 0.0f) {
		Direction /= dist;
	}

	/*
	** Compute the attenuation factor
	*/
	double atten_start,atten_end;
	light.Get_Far_Attenuation_Range(atten_start,atten_end);
	
	float atten = 1.0f - (dist - atten_start) / (atten_end - atten_start);
	atten = WWMath::Clamp(atten,0.0f,1.0f);



	if (light.Get_Type() == LightClass::SPOT) {
		
		Vector3 spot_dir;
		light.Get_Spot_Direction(spot_dir);
		Matrix3D::Rotate_Vector(light.Get_Transform(),spot_dir,&spot_dir);

		float spot_angle_cos = light.Get_Spot_Angle_Cos();
		atten *= (Vector3::Dot_Product(-spot_dir,Direction) - spot_angle_cos) / (1.0f - spot_angle_cos);
		atten = WWMath::Clamp(atten,0.0f,1.0f);
	}

	/*
	** Compute the ambient and diffuse values.  Rejecting the diffuse
	** component and folding it into the ambient component if it is below
	** the LOD cutoff
	*/
	light.Get_Ambient(&Ambient);
	light.Get_Diffuse(&Diffuse);

	m_point = (light.Get_Type() == LightClass::POINT); 
	m_center = light.Get_Position();
	m_innerRadius = atten_start;
	m_outerRadius = atten_end;
	m_ambient = Ambient;
	m_diffuse = Diffuse;

	if (Diffuse.Length2() > _LightingLODCutoff2) {

		DiffuseRejected = false;
		Ambient *= atten;
		Diffuse *= atten;
		
	} else {

		DiffuseRejected = true;
		Ambient *= atten;
		Ambient += atten * DIFFUSE_TO_AMBIENT_FRACTION * Diffuse;
		Diffuse.Set(0,0,0);

	}
}


void LightEnvironmentClass::InputLightStruct::Init_From_Directional_Light
(
	const LightClass & light,
	const Vector3 & object_center
)
{
	Direction = -light.Get_Transform().Get_Z_Vector();

	DiffuseRejected = false;
	light.Get_Ambient(&Ambient);
	light.Get_Diffuse(&Diffuse);
}


float LightEnvironmentClass::InputLightStruct::Contribution(void)
{
	return Diffuse.Length2();
}
	

/************************************************************************************************
**
** LightEnvironmentClass::OutputLightStruct Implementation
**
************************************************************************************************/

void LightEnvironmentClass::OutputLightStruct::Init
(
	const InputLightStruct & input,
	const Matrix3D & camera_tm
)
{
	Diffuse = input.Diffuse;
	Matrix3D::Inverse_Rotate_Vector(camera_tm,input.Direction,&Direction);
}	



/************************************************************************************************
**
** LightEnvironmentClass Implementation
**
************************************************************************************************/

LightEnvironmentClass::LightEnvironmentClass(void) :
	LightCount(0),
	ObjectCenter(0,0,0),
	OutputAmbient(0,0,0)
{
}


LightEnvironmentClass::~LightEnvironmentClass(void)
{
}


void LightEnvironmentClass::Reset(const Vector3 & object_center,const Vector3 & ambient)
{
	LightCount = 0;
	ObjectCenter = object_center;
	OutputAmbient = ambient;
}


void LightEnvironmentClass::Add_Light(const LightClass & light)
{
	/*
	** Compute the equivalent directional + ambient light
	*/
	InputLightStruct new_light;
	new_light.Init(light,ObjectCenter);

	/*
	** Add in the ambient component
	*/
	OutputAmbient += new_light.Ambient;

	/*
	** If not rejected, add the directional component to the active lights
	*/
	if (new_light.DiffuseRejected == false || new_light.m_point) {
		if (LightCount < MAX_LIGHTS) {
			InputLights[LightCount] = new_light;
			LightCount++;
		} else {
			for (int light_index=0; light_index<LightCount; light_index++) {
				if (new_light.Contribution() > InputLights[light_index].Contribution()) {
					InputLights[light_index] = new_light;
					light_index = MAX_LIGHTS;
				}
			}
		}
	}
}

void LightEnvironmentClass::Pre_Render_Update(const Matrix3D & camera_tm)
{
	/*
	** Transform each light into camera space
	** and add up the ambient effect of each light
	*/
	for (int light_index=0; light_index<LightCount; light_index++) {
		OutputLights[light_index].Init(InputLights[light_index],camera_tm);
	}
	// Clamp ambient.
	OutputAmbient.X = WWMath::Clamp(OutputAmbient.X,0.0f,1.0f);
	OutputAmbient.Y = WWMath::Clamp(OutputAmbient.Y,0.0f,1.0f);
	OutputAmbient.Z = WWMath::Clamp(OutputAmbient.Z,0.0f,1.0f);
}

void LightEnvironmentClass::Set_Lighting_LOD_Cutoff(float inten)
{
	_LightingLODCutoff = inten;
	_LightingLODCutoff2 = _LightingLODCutoff * _LightingLODCutoff;
}

float LightEnvironmentClass::Get_Lighting_LOD_Cutoff(void)
{
	return _LightingLODCutoff;
}

