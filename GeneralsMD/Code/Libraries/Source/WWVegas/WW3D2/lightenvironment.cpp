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
#include "colorspace.h"

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
	float atten = 1.0f;
	double atten_start,atten_end;
	light.Get_Far_Attenuation_Range(atten_start,atten_end);

	if (light.Get_Flag(LightClass::FAR_ATTENUATION)) {

		if (WWMath::Fabs(atten_end - atten_start) < WWMATH_EPSILON) {

			/*
			** Start and end are equal, attenuation is a "step" function
			*/
			if (dist > atten_start) {
				atten = 0.0f;
			}

		} else {

			/*
			** Compute the attenuation
			*/
			atten = 1.0f - (dist - atten_start) / (atten_end - atten_start);
			atten = WWMath::Clamp(atten,0.0f,1.0f);
		}
	}


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
	Ambient *= light.Get_Intensity();  //(gth) CNC3 obey the intensity parameter
	Diffuse *= light.Get_Intensity();

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

	// Guard against a direction that is invalid
	if(Direction.Length2() == 0.0f) {
		Direction.X = 1.0f;
	}
}



/************************************************************************************************
**
** LightEnvironmentClass Implementation
**
************************************************************************************************/

LightEnvironmentClass::LightEnvironmentClass(void) :
	LightCount(0),
	ObjectCenter(0,0,0),
	OutputAmbient(0,0,0),
	FillLight(),
	FillIntensity(0.0f)
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
	// Jani: Don't accept lights that are almost black
	Vector3 diff;
	light.Get_Diffuse(&diff);
	if (diff[0]<0.05f && diff[1]<0.05f && diff[2]<0.05f) {
		return;
	}

	/*
	** Compute the equivalent directional + ambient light
	*/

	InputLightStruct new_light;
	new_light.Init(light, ObjectCenter);

	// If we have the fill light set, we also want to the diffuse light to be modified by the intensity of the light source
	if(FillIntensity) new_light.Diffuse *= light.Get_Intensity();

	/*
	** Add in the ambient component
	*/
	OutputAmbient += new_light.Ambient;

	/*
	** If not rejected, add the directional component to the active lights
	*/
	if (new_light.DiffuseRejected == false || new_light.m_point) {

		// Insert the light into the sorted list of InputLights if it's contribution is greater than the any of the current number of lights
		for (int light_index=0; light_index < LightCount; light_index++) {
			if (new_light.Contribution() > InputLights[light_index].Contribution()) {

				// Move back the lights in the InputLights Array to make space for the new light.
				// The last light might be discarded if it moves off the array as it is the weakest light in the list.
				for (int i = LightCount; i > light_index; --i) {
					if (i < MAX_LIGHTS) {
						InputLights[i] = InputLights[i - 1];
					}
				}

				// Add the new light into the InputLights List where it belongs
				InputLights[light_index] = new_light;

				// Increment the light count if we have not reach the maximum lights limit yet
				LightCount = min(LightCount + 1, (int)MAX_LIGHTS);

				// Since we have inserted a new light, we are done for this function
				return;
			}
		}

		// If the light was not inserted but there are still spots empty in the InputLights list, insert the lights at the end of the list
		if (LightCount < MAX_LIGHTS) {
			InputLights[LightCount] = new_light;
			++LightCount;
		}
	}
}

void LightEnvironmentClass::Pre_Render_Update(const Matrix3D & camera_tm)
{
	/*
	** Calculate and set up the fill light for the object
	*/
	Calculate_Fill_Light();

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

/************************************************************************************************
**
** LightEnvironmentClass::Add_Fill_Light Implementation
** The fill light is inserted in the InputLights list as ont of the lights, and if the
** list is already full, it preempts the last and weakest light in that list.
**
************************************************************************************************/
void LightEnvironmentClass::Add_Fill_Light(void)
{
	// Don't add black (or almost black) lights!
	if (FillLight.Diffuse[0]<0.05f && FillLight.Diffuse[1]<0.05f && FillLight.Diffuse[2]<0.05f) {
		OutputAmbient += FillLight.Ambient;
		return;
	}

	// Get the 1st empty light slot or the very last slot regardless of whether the last slot is empty or not
	int slot = 0;
	if (LightCount == MAX_LIGHTS) {
		slot = MAX_LIGHTS - 1;
	} else {
		slot = LightCount;
		++LightCount;
	}

	/*
	** Add in the ambient component
	*/
	OutputAmbient += FillLight.Ambient;

	/*
	** Insert the fill light into the calculated slot of the InputLights
	*/
	InputLights[slot] = FillLight;
}

/************************************************************************************************
**
** LightEnvironmentClass::Calculate_Fill_Light Implementation
** The fill light takes up to the top 3 lights in the InputList and averages them into 1 light source.
** The averaged light source is then flipped in direction and location as well as in HUE of the color.
** This final light is used to support the top 3 lights by providing a calculated fill to augment the lights.
**
************************************************************************************************/
void LightEnvironmentClass::Calculate_Fill_Light(void)
{
	// Early exit if we have no lights at all or if the fill light intensity is zero
	if (LightCount == 0 || FillIntensity == 0.0f) return;

	// Initialize the averaged light to the primary light source (light with the most contribution)
	float primary_contribution = InputLights[0].Contribution();
	InputLightStruct average_light = InputLights[0];

	// Loop through the remaining lights on the list (up to 2) and add their contributions to the averaged light
	int num_lights = min(LightCount, MAX_LIGHTS - 1);
	for (int i = 1; i < num_lights; ++i) {

		// The ratio is the percentage of the remaining light's contribution compared to the primary light source
		float ratio = InputLights[i].Contribution() / primary_contribution;

		average_light.Direction += (InputLights[i].Direction * ratio);
		average_light.Ambient	+= (InputLights[i].Ambient * ratio);
		average_light.Diffuse	+= (InputLights[i].Diffuse * ratio);
	}

	// Normalize the averaged light direction
	average_light.Direction.Normalize();

	// Now we have the averaged light, we should derive the fill light
	// Convert from RGB to HSV to get the reserve hue and with the value modified by the fill intensity
	Vector3 temp;
	RGB_To_HSV(temp, average_light.Diffuse);
	temp.X += 180.0f;				// Get the opposite hue from the averaged light
	if(temp.X > 360.0f) {
		temp.X -= 360.0f;
	}
	temp.Z *= FillIntensity;	// fraction of the intensity
	HSV_To_RGB(FillLight.Diffuse, temp);

	// Zero out the fill ambient
	FillLight.Ambient.Set(0.0f, 0.0f, 0.0f);

	// now we set the fill light direction to be opposite the average light
	FillLight.Direction = average_light.Direction * (-1.0f);
	FillLight.DiffuseRejected = false;

	// Add the fill light into the InputLights list
	Add_Fill_Light();
}


