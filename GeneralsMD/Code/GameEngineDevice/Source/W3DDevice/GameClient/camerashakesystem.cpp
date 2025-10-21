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
 *                     $Archive:: /Commando/Code/wwphys/camerashakesystem.cpp                 $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/12/01 10:25a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <stdlib.h>
#include <string.h>
#include <assetmgr.h>
#include <texture.h>
#include <tri.h>
#include <colmath.h>
#include <coltest.h>
#include <rinfo.h>
#include <camera.h>
#include <d3dx8core.h>
#include "Common/GlobalData.h"
#include "Common/PerfTimer.h"

#include "GameClient/TerrainVisual.h"
#include "GameClient/View.h"
#include "GameClient/Water.h"

#include "GameLogic/AIPathfind.h"
#include "GameLogic/TerrainLogic.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DBibBuffer.h"
#include "W3DDevice/GameClient/W3DTreeBuffer.h"
#include "W3DDevice/GameClient/W3DRoadBuffer.h"
#include "W3DDevice/GameClient/W3DBridgeBuffer.h"
#include "W3DDevice/GameClient/W3DWaypointBuffer.h"
#include "W3DDevice/GameClient/W3DCustomEdging.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/light.h"
#include "WW3D2/scene.h"
#include "W3DDevice/GameClient/W3DPoly.h"
#include "W3DDevice/GameClient/W3DCustomScene.h"

#include "W3DDevice/GameClient/camerashakesystem.h"
#include "WW3D2/camera.h"

//#include "W3DDevice/GameClient/camera.h"
//#include "W3DDevice/GameClient/wwmemlog.h"

/*
** (gth) According to my "research" the artists say that there are several factors that
** go into a good camera shake.
** - The motion should be sinusoidal.
** - Camera rotation is more effective than camera motion (good, I won't use any translation)
** - The camera should pitch up and down a lot more than it yaws left and right.
*/

DEFINE_AUTO_POOL(CameraShakeSystemClass::CameraShakerClass,256);

const float MIN_OMEGA			= DEG_TO_RADF(12.5f*360.0f);
const float MAX_OMEGA			= DEG_TO_RADF(15.0f*360.0f);
const float END_OMEGA			= DEG_TO_RADF(360.0f);
const float MIN_PHI				= DEG_TO_RADF(0.0f);
const float MAX_PHI				= DEG_TO_RADF(360.0f);
const Vector3 AXIS_ROTATION	= Vector3(DEG_TO_RADF(7.5f),DEG_TO_RADF(15.0f),DEG_TO_RADF(5.0f));


/************************************************************************************************
**
** CameraShakeSystemClass::CameraShakerClass Implementation
**
************************************************************************************************/
CameraShakeSystemClass::CameraShakerClass::CameraShakerClass
(
	const Vector3 & position,
	float radius,
	float duration,
	float intensity
) :
	Position(position),
	Radius(radius),
	Duration(duration),
	Intensity(intensity),
	ElapsedTime(0.0f)
{
	/*
	** Initialize random sinusoid values
	*/
	Omega.X = WWMath::Random_Float(MIN_OMEGA,MAX_OMEGA);
	Omega.Y = WWMath::Random_Float(MIN_OMEGA,MAX_OMEGA);
	Omega.Z = WWMath::Random_Float(MIN_OMEGA,MAX_OMEGA);
	Phi.X = WWMath::Random_Float(MIN_PHI,MAX_PHI);
	Phi.Y = WWMath::Random_Float(MIN_PHI,MAX_PHI);
	Phi.Z = WWMath::Random_Float(MIN_PHI,MAX_PHI);
}

CameraShakeSystemClass::CameraShakerClass::~CameraShakerClass(void)
{
}


void CameraShakeSystemClass::CameraShakerClass::Compute_Rotations(const Vector3 & camera_position, Vector3 * set_angles)
{
	WWASSERT(set_angles != NULL);

	/*
	** We want several different sinusiods, each with a different phase shift and
	** frequency.  The frequency is a function of time as well, stretching the
	** sine wave out.  These waves are modulated based on the distance from the
	** center of the "shake", the intensity of the shake, and based on the axis
	** being affected.  The vertical axis should have about 3x the amplitude of
	** the horizontal axis.
	*/

	float len2 = (camera_position - Position).Length2();


	if (len2 > Radius*Radius) {
		return;
	}


	/*
	** f(t) = intensity(t,pos) * sin( omega(t) * t + phi );
	** intensity(t,pos) = intensity * (radius/distance) * timeremaing/totaltime
	** omega(t) = start_omega + (end_omega - start_omega) * t
	** phi = random(0..start_omega)
	*/
	float intensity = Intensity * (1.0f - WWMath::Sqrt(len2) / Radius) * (1.0f - ElapsedTime / Duration);
	for (int i=0; i<3; i++) {
		float omega = Omega[i] + (END_OMEGA - Omega[i]) * ElapsedTime;
		(*set_angles)[i] += AXIS_ROTATION[i] * intensity * WWMath::Sin(omega * ElapsedTime + Phi[i]);

		//WST 11/14/2002. Add in additional random fudge.  There seems to be a too mathematical pattern of shake with the above
		Vector3 secondary_angles;
		float minor_intensity = intensity * 0.5f;
		secondary_angles.X = WWMath::Random_Float(-minor_intensity,minor_intensity);
		secondary_angles.Y = WWMath::Random_Float(-minor_intensity,minor_intensity);
		secondary_angles.Z = WWMath::Random_Float(-minor_intensity,minor_intensity);
		(*set_angles) += secondary_angles;
	}
}


/************************************************************************************************
**
** CameraShakeSystemClass Implementation
**
************************************************************************************************/
CameraShakeSystemClass::CameraShakeSystemClass(void)
{
}

CameraShakeSystemClass::~CameraShakeSystemClass(void)
{
	/*
	** delete all of the objects out of the list
	*/
	while (!CameraShakerList.Is_Empty()) {
		CameraShakerClass * obj = CameraShakerList.Remove_Head();
		CameraShakerList.Remove(obj);
		delete obj;
	}
}

void CameraShakeSystemClass::Add_Camera_Shake
(
	const Vector3 & position,
	float radius,
	float duration,
	float power
)
{
	//WWMEMLOG(MEM_PHYSICSDATA);
	/*
	** Allocate a new camera shaker object.  Note that these are mem-pooled so the allocation
	** is very cheap.
	*/

	//Power is in degrees of amplitude.
	power = power * PI/180.0f;

	CameraShakerClass * shaker = new CameraShakerClass(position,radius,duration,power);
	CameraShakerList.Add(shaker);
}

bool CameraShakeSystemClass::IsCameraShaking(void)
{
	/*
	** Loop through to find if there is any active camera shakers
	*/
	MultiListIterator<CameraShakerClass> iterator(&CameraShakerList);
	for (iterator.First(); !iterator.Is_Done(); iterator.Next()) {
		CameraShakerClass * obj = iterator.Peek_Obj();
		if (obj){
			return (true);
		}
	}
	return(false);
}


void CameraShakeSystemClass::Timestep(float dt)
{
	/*
	** Allow each camera shaker to timestep.  Any that expire are added to a temporary
	** list for deletion.
	*/
	MultiListClass<CameraShakerClass> deletelist;
	MultiListIterator<CameraShakerClass> iterator(&CameraShakerList);
	for (iterator.First(); !iterator.Is_Done(); iterator.Next()) {
		CameraShakerClass * obj = iterator.Peek_Obj();
		obj->Timestep(dt);
		if (obj->Is_Expired()) {
			deletelist.Add(obj);
		}
	}

	/*
	** Remove and delete all the ones that expired
	*/
	while (!deletelist.Is_Empty()) {
		CameraShakerClass * obj = deletelist.Remove_Head();
		CameraShakerList.Remove(obj);
		delete obj;
	}
}

void CameraShakeSystemClass::Update_Camera_Shaker(Vector3 camera_position, Vector3 *shaker_angle)
{
	MultiListIterator<CameraShakerClass> iterator(&CameraShakerList);

	Vector3 angles(0,0,0);
	Matrix3D camera_transform;

	//camera_transform = camera.Get_Transform();
	//camera_transform.Get_Translation(&camera_position);

	/*
	** Accumulate the effects of any active camera shakers
	*/

	for (iterator.First(); !iterator.Is_Done(); iterator.Next()) {
		iterator.Peek_Obj()->Compute_Rotations(camera_position,&angles);
	}

	/*
	** Clamp the result
	*/
	for (int i=0; i<3; i++) {
		WWMath::Clamp(angles[i],-AXIS_ROTATION[i],AXIS_ROTATION[i]);
	}

	*shaker_angle = angles;

	/*
	** Apply to the camera
	*/
	/*
	camera_transform.Rotate_X(angles.X);
	camera_transform.Rotate_Y(angles.Y);
	camera_transform.Rotate_Z(angles.Z);
	camera.Set_Transform(camera_transform);
	*/
}

// The Instance of the system
CameraShakeSystemClass CameraShakerSystem; //WST 11/12/2002 This is the new Camera Shaker system upgrade

