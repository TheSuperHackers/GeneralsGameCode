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
 *                     $Archive:: /VSS_Sync/wwmath/hermitespline.cpp                          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 6/13/01 2:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "hermitespline.h"

/*
** Hermite Spline
*/
const HermiteSpline3DClass& HermiteSpline3DClass::operator=(const HermiteSpline3DClass& that)
{
	//
	//	This is included for completeness only, it basically
	// implements the default bitwise copy operator.
	//
	TangentsDirty = that.TangentsDirty;
	Tangents = that.Tangents;
	Curve3DClass::operator=(that);
	return (*this);
}

void HermiteSpline3DClass::Set_Looping(bool onoff)
{
	if (onoff != IsLooping)
	{
		Curve3DClass::Set_Looping(onoff);
		TangentsDirty = true;
	}
}

void HermiteSpline3DClass::Evaluate(float time, Vector3* set_val)
{
	// if we're outside the range, return the start or end...
	if (time < Keys[0].Time)
	{
		*set_val = Keys[0].Point;
		return;
	}

	if (time > Keys[Keys.Count() - 1].Time)
	{
		*set_val = Keys[Keys.Count() - 1].Point;
		return;
	}

	// if the tangents are marked dirty, give derived classes a chance to recompute them
	if (TangentsDirty)
	{
		Update_Tangents();
	}

	// ok find the segment
	int i0, i1;
	float t;
	Find_Interval(time, &i0, &i1, &t);

	float t2 = t * t;
	float t3 = t2 * t;

	// hermite basis functions:
	float h0 = 2 * t3 - 3 * t2 + 1;
	float h1 = -2 * t3 + 3 * t2;
	float h2 = t3 - 2 * t2 + t;
	float h3 = t3 - t2;

	set_val->X = h0 * Keys[i0].Point.X + h1 * Keys[i1].Point.X +
	             h2 * Tangents[i0].OutTangent.X + h3 * Tangents[i1].InTangent.X;

	set_val->Y = h0 * Keys[i0].Point.Y + h1 * Keys[i1].Point.Y +
	             h2 * Tangents[i0].OutTangent.Y + h3 * Tangents[i1].InTangent.Y;

	set_val->Z = h0 * Keys[i0].Point.Z + h1 * Keys[i1].Point.Z +
	             h2 * Tangents[i0].OutTangent.Z + h3 * Tangents[i1].InTangent.Z;
}

void HermiteSpline3DClass::Evaluate_Derivative(float time, Vector3* set_val)
{
	// if we're outside the range, return the value for the start or end...
	float min_time = Keys[0].Time;
	float max_time = Keys[Keys.Count() - 1].Time;
	time = MAX(time, min_time);
	time = MIN(time, max_time);

	// if the tangents are marked dirty, give derived classes a chance to recompute them
	if (TangentsDirty)
	{
		Update_Tangents();
	}

	// ok find the segment
	int i0, i1;
	float t;
	Find_Interval(time, &i0, &i1, &t);

	float t2 = t * t;

	// derivatives of hermite basis functions:
	float dh0 = 6 * t2 - 6 * t;
	float dh1 = -6 * t2 + 6 * t;
	float dh2 = 3 * t2 - 4 * t + 1;
	float dh3 = 3 * t2 - 2 * t;

	set_val->X = dh0 * Keys[i0].Point.X + dh1 * Keys[i1].Point.X +
	             dh2 * Tangents[i0].OutTangent.X + dh3 * Tangents[i1].InTangent.X;

	set_val->Y = dh0 * Keys[i0].Point.Y + dh1 * Keys[i1].Point.Y +
	             dh2 * Tangents[i0].OutTangent.Y + dh3 * Tangents[i1].InTangent.Y;

	set_val->Z = dh0 * Keys[i0].Point.Z + dh1 * Keys[i1].Point.Z +
	             dh2 * Tangents[i0].OutTangent.Z + dh3 * Tangents[i1].InTangent.Z;
}

void HermiteSpline3DClass::Set_Key(int i, const Vector3& point)
{
	Curve3DClass::Set_Key(i, point);
	TangentsDirty = true;
}

int HermiteSpline3DClass::Add_Key(const Vector3& point, float t)
{
	int index = Curve3DClass::Add_Key(point, t);
	TangentsDirty = true;
	TangentsClass tan;
	tan.InTangent.Set(0, 0, 0);
	tan.OutTangent.Set(0, 0, 0);
	Tangents.Insert(index, tan);
	return index;
}

void HermiteSpline3DClass::Remove_Key(int i)
{
	Tangents.Delete(i);
	Curve3DClass::Remove_Key(i);
	TangentsDirty = true;
}

void HermiteSpline3DClass::Clear_Keys()
{
	Tangents.Clear();
	Curve3DClass::Clear_Keys();
	TangentsDirty = true;
}

void HermiteSpline3DClass::Set_Tangents(int i, const Vector3& in_tan, const Vector3& out_tan)
{
	assert(i >= 0);
	assert(i < Keys.Count());
	Tangents[i].InTangent = in_tan;
	Tangents[i].OutTangent = out_tan;
}

void HermiteSpline3DClass::Get_Tangents(int i, Vector3* set_in, Vector3* set_out)
{
	assert(i >= 0);
	assert(i < Keys.Count());
	*set_in = Tangents[i].InTangent;
	*set_out = Tangents[i].OutTangent;
}

/*
** 1-Dimensional Hermite Spline
*/
void HermiteSpline1DClass::Set_Looping(bool onoff)
{
	if (onoff != IsLooping)
	{
		Curve1DClass::Set_Looping(onoff);
		TangentsDirty = true;
	}
}

void HermiteSpline1DClass::Evaluate(float time, float* set_val)
{
	if (Keys.Count() == 1)
	{
		*set_val = Keys[0].Point;
		return;
	}

	if (!IsLooping)
	{
		// if we're outside the range, return the start or end...
		if (time < Keys[0].Time)
		{
			*set_val = Keys[0].Point;
			return;
		}

		if (time > Keys[Keys.Count() - 1].Time)
		{
			*set_val = Keys[Keys.Count() - 1].Point;
			return;
		}
	}

	// if the tangents are marked dirty, give derived classes a chance to recompute them
	if (TangentsDirty)
	{
		Update_Tangents();
	}

	// ok find the segment
	int i0, i1;
	float t;
	Find_Interval(time, &i0, &i1, &t);

	float t2 = t * t;
	float t3 = t2 * t;

	// hermite basis functions:
	float h0 = 2 * t3 - 3 * t2 + 1;
	float h1 = -2 * t3 + 3 * t2;
	float h2 = t3 - 2 * t2 + t;
	float h3 = t3 - t2;

	*set_val = h0 * Keys[i0].Point + h1 * Keys[i1].Point +
	           h2 * Tangents[i0].OutTangent + h3 * Tangents[i1].InTangent;
}

void HermiteSpline1DClass::Set_Key(int i, float point, unsigned int extra)
{
	Curve1DClass::Set_Key(i, point, extra);
	TangentsDirty = true;
}

int HermiteSpline1DClass::Add_Key(float point, float t, unsigned int extra)
{
	int index = Curve1DClass::Add_Key(point, t, extra);
	TangentsDirty = true;
	TangentsClass tan;
	tan.InTangent = 0.0f;
	tan.OutTangent = 0.0f;
	Tangents.Insert(index, tan);
	return index;
}

void HermiteSpline1DClass::Remove_Key(int i)
{
	Tangents.Delete(i);
	Curve1DClass::Remove_Key(i);
	TangentsDirty = true;
}

void HermiteSpline1DClass::Clear_Keys()
{
	Tangents.Clear();
	Curve1DClass::Clear_Keys();
	TangentsDirty = true;
}

void HermiteSpline1DClass::Set_Tangents(int i, float in_tan, float out_tan)
{
	assert(i >= 0);
	assert(i < Keys.Count());
	Tangents[i].InTangent = in_tan;
	Tangents[i].OutTangent = out_tan;
}

void HermiteSpline1DClass::Get_Tangents(int i, float* set_in, float* set_out)
{
	assert(i >= 0);
	assert(i < Keys.Count());
	*set_in = Tangents[i].InTangent;
	*set_out = Tangents[i].OutTangent;
}
