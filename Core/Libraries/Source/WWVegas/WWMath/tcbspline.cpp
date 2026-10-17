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
 *                     $Archive:: /VSS_Sync/wwmath/tcbspline.cpp                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 6/13/01 2:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "tcbspline.h"
#include "WWDebug/wwdebug.h"


/*
** TCBSpline3DClass Implemenation
*/

int TCBSpline3DClass::Add_Key(const Vector3 & point,float t)
{
	int index;
	index = HermiteSpline3DClass::Add_Key(point,t);

	TCBClass params;
	params.Tension = 0.0f;
	params.Continuity = 0.0f;
	params.Bias = 0.0f;
	Params.Insert(index,params);
	return index;
}

void TCBSpline3DClass::Remove_Key(int i)
{
	HermiteSpline3DClass::Remove_Key(i);
	Params.Delete(i);
}

void TCBSpline3DClass::Clear_Keys()
{
	HermiteSpline3DClass::Clear_Keys();
	Params.Clear();
}

void TCBSpline3DClass::Set_TCB_Params(int i,float tension,float continuity,float bias)
{
	WWASSERT(i >= 0);
	WWASSERT(i < Params.Count());
	Params[i].Tension = tension;
	Params[i].Continuity = continuity;
	Params[i].Bias = bias;
	TangentsDirty = true;
}

void TCBSpline3DClass::Get_TCB_Params(int i,float *tension,float *continuity,float *bias)
{
	if (tension) *tension = Params[i].Tension;
	if (continuity) *continuity = Params[i].Continuity;
	if (bias) *bias = Params[i].Bias;
}

void TCBSpline3DClass::Update_Tangents()
{
	if (Keys.Count() < 2) {
		for (int i=0; i<Keys.Count(); i++) {
			Tangents[0].InTangent.Set(0,0,0);
			Tangents[0].OutTangent.Set(0,0,0);
		}
	}

	// First and Last Key:
	// Only need to compute the OutTangent for key[0] and the InTangent for key[end]
	int end = Keys.Count() - 1;
	Tangents[0].InTangent.Set(0,0,0);
	Tangents[end].OutTangent.Set(0,0,0);

	if (IsLooping) {

		// This really only works if the start and end points have the same position...
		// Also just using the TCB params from p0 for both
		float k0 = 0.5f * ((1-Params[0].Tension) * (1-Params[0].Continuity) * (1-Params[0].Bias));
		float k1 = 0.5f * ((1-Params[0].Tension) * (1+Params[0].Continuity) * (1+Params[0].Bias));
		float k2 = 0.5f * ((1-Params[0].Tension) * (1+Params[0].Continuity) * (1-Params[0].Bias));
		float k3 = 0.5f * ((1-Params[0].Tension) * (1-Params[0].Continuity) * (1+Params[0].Bias));

		Vector3 dp_in;
		Vector3 dp_out;
		Vector3::Subtract(Keys[0].Point,Keys[end-1].Point,&dp_in);
		Vector3::Subtract(Keys[1].Point,Keys[0].Point,&dp_out);

		Vector3::Add(k0*dp_in, k1*dp_out, &Tangents[end].InTangent);
		Vector3::Add(k2*dp_out, k3*dp_in, &Tangents[0].OutTangent);

	} else {

		float k2 = 0.25f * ((1-Params[0].Tension) * (1+Params[0].Continuity) * (1-Params[0].Bias));
		float k3 = 0.25f * ((1-Params[0].Tension) * (1-Params[0].Continuity) * (1+Params[0].Bias));

		Vector3 dp_in;
		Vector3 dp_out;
		Vector3::Subtract(Keys[1].Point,Keys[0].Point,&dp_out);
		dp_in = dp_out;
		Vector3::Add(k2*dp_out, k3*dp_in, &Tangents[0].OutTangent);

		float k0 = 0.25f * ((1-Params[0].Tension) * (1-Params[0].Continuity) * (1-Params[0].Bias));
		float k1 = 0.25f * ((1-Params[0].Tension) * (1+Params[0].Continuity) * (1+Params[0].Bias));
		Vector3::Subtract(Keys[end].Point,Keys[end-1].Point,&dp_in);
		dp_out = dp_in;
		Vector3::Add(k0*dp_out, k1*dp_in, &Tangents[end].InTangent);
	}

	float total_time = (Keys[1].Time - Keys[0].Time) + (Keys[end].Time - Keys[end-1].Time);
	float in_factor = 2.0f * (Keys[end].Time - Keys[end-1].Time) / total_time;
	float out_factor = 2.0f * (Keys[1].Time - Keys[0].Time) / total_time;
	Tangents[end].InTangent *= in_factor;
	Tangents[0].OutTangent *= out_factor;


	// Now compute the tangents of all of the normal keys...
	for (int pi=1;pi<Keys.Count() - 1; pi++) {

		float k0 = 0.5f * ((1-Params[pi].Tension) * (1-Params[pi].Continuity) * (1-Params[pi].Bias));
		float k1 = 0.5f * ((1-Params[pi].Tension) * (1+Params[pi].Continuity) * (1+Params[pi].Bias));
		float k2 = 0.5f * ((1-Params[pi].Tension) * (1+Params[pi].Continuity) * (1-Params[pi].Bias));
		float k3 = 0.5f * ((1-Params[pi].Tension) * (1-Params[pi].Continuity) * (1+Params[pi].Bias));

		Vector3 dp_in;
		Vector3 dp_out;
		Vector3::Subtract(Keys[pi].Point,Keys[pi-1].Point,&dp_in);
		Vector3::Subtract(Keys[pi+1].Point,Keys[pi].Point,&dp_out);

		Vector3::Add(k0*dp_out, k1*dp_in, &Tangents[pi].InTangent);
		Vector3::Add(k2*dp_out, k3*dp_in, &Tangents[pi].OutTangent);

		float total_time = (Keys[pi+1].Time - Keys[pi-1].Time);
		float in_factor = 2.0f * (Keys[pi].Time - Keys[pi-1].Time) / total_time;
		float out_factor = 2.0f * (Keys[pi+1].Time - Keys[pi].Time) / total_time;

		Tangents[pi].InTangent *= in_factor;		// compensating for un-even keys
		Tangents[pi].OutTangent *= out_factor;

	}
	TangentsDirty = false;
}
