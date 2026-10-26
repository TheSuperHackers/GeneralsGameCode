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
#include "WWDebug/wwdebug.h"

/*
** CardinalSpline3DClass Implementation
*/
int CardinalSpline3DClass::Add_Key(const Vector3& point, float t)
{
	int index = HermiteSpline3DClass::Add_Key(point, t);
	float tightness = 0.5f;
	Tightness.Insert(index, tightness);
	return index;
}

void CardinalSpline3DClass::Remove_Key(int i)
{
	Tightness.Delete(i);
	HermiteSpline3DClass::Remove_Key(i);
}

void CardinalSpline3DClass::Clear_Keys()
{
	Tightness.Clear();
	HermiteSpline3DClass::Clear_Keys();
}

void CardinalSpline3DClass::Set_Tightness(int i, float tightness)
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

void CardinalSpline3DClass::Update_Tangents()
{
	if (Keys.Count() < 2)
	{
		for (int i = 0; i < Keys.Count(); i++)
		{
			Tangents[0].InTangent.Set(0, 0, 0);
			Tangents[0].OutTangent.Set(0, 0, 0);
		}
		return;
	}

	// First and Last Key:
	// Only need to compute the OutTangent for key[0] and the InTangent for key[end]
	int end = Keys.Count() - 1;
	Tangents[0].InTangent.Set(0, 0, 0);
	Tangents[end].OutTangent.Set(0, 0, 0);

	if (IsLooping)
	{
		// This really only works if the start and end points have the same position...
		Tangents[0].OutTangent.X = (1.0f - Tightness[0]) * (Keys[1].Point.X - Keys[end - 1].Point.X);
		Tangents[0].OutTangent.Y = (1.0f - Tightness[0]) * (Keys[1].Point.Y - Keys[end - 1].Point.Y);
		Tangents[0].OutTangent.Z = (1.0f - Tightness[0]) * (Keys[1].Point.Z - Keys[end - 1].Point.Z);
		Tangents[end].InTangent = Tangents[0].OutTangent;
	}
	else
	{
		Tangents[0].OutTangent.X = (1.0f - Tightness[0]) * (Keys[1].Point.X - Keys[0].Point.X);
		Tangents[0].OutTangent.Y = (1.0f - Tightness[0]) * (Keys[1].Point.Y - Keys[0].Point.Y);
		Tangents[0].OutTangent.Z = (1.0f - Tightness[0]) * (Keys[1].Point.Z - Keys[0].Point.Z);

		Tangents[end].InTangent.X = (1.0f - Tightness[0]) * (Keys[end].Point.X - Keys[end - 1].Point.X);
		Tangents[end].InTangent.Y = (1.0f - Tightness[0]) * (Keys[end].Point.Y - Keys[end - 1].Point.Y);
		Tangents[end].InTangent.Z = (1.0f - Tightness[0]) * (Keys[end].Point.Z - Keys[end - 1].Point.Z);
	}

	float total_time = (Keys[1].Time - Keys[0].Time) + (Keys[end].Time - Keys[end - 1].Time);
	float in_factor = 2.0f * (Keys[end].Time - Keys[end - 1].Time) / total_time;
	float out_factor = 2.0f * (Keys[1].Time - Keys[0].Time) / total_time;
	Tangents[end].InTangent *= in_factor;
	Tangents[0].OutTangent *= out_factor;

	// inner knots
	for (int i = 1; i < Keys.Count() - 1; i++)
	{
		Tangents[i].InTangent.X = (1.0f - Tightness[i]) * (Keys[i + 1].Point.X - Keys[i - 1].Point.X);
		Tangents[i].InTangent.Y = (1.0f - Tightness[i]) * (Keys[i + 1].Point.Y - Keys[i - 1].Point.Y);
		Tangents[i].InTangent.Z = (1.0f - Tightness[i]) * (Keys[i + 1].Point.Z - Keys[i - 1].Point.Z);
		Tangents[i].OutTangent = Tangents[i].InTangent;

		float in_factor = 2.0f * (Keys[i].Time - Keys[i - 1].Time) / (Keys[i + 1].Time - Keys[i - 1].Time);
		float out_factor = 2.0f * (Keys[i + 1].Time - Keys[i].Time) / (Keys[i + 1].Time - Keys[i - 1].Time);

		Tangents[i].InTangent *= in_factor;    // compensating for the un-even keys
		Tangents[i].OutTangent *= out_factor;
	}
	TangentsDirty = false;
}

/*
** CardinalSpline1DClass Implementation
*/
int CardinalSpline1DClass::Add_Key(float point, float t, unsigned int extra)
{
	int index = HermiteSpline1DClass::Add_Key(point, t, extra);
	float tightness = 0.5f;
	Tightness.Insert(index, tightness);
	return index;
}

void CardinalSpline1DClass::Remove_Key(int i)
{
	Tightness.Delete(i);
	HermiteSpline1DClass::Remove_Key(i);
}

void CardinalSpline1DClass::Clear_Keys()
{
	Tightness.Clear();
	HermiteSpline1DClass::Clear_Keys();
}

void CardinalSpline1DClass::Set_Tightness(int i, float tightness)
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

void CardinalSpline1DClass::Update_Tangents()
{
	if (Keys.Count() < 2)
	{
		for (int i = 0; i < Keys.Count(); i++)
		{
			Tangents[0].InTangent = 0;
			Tangents[0].OutTangent = 0;
		}
	}

	// First and Last Key:
	// Only need to compute the OutTangent for key[0] and the InTangent for key[end]
	int end = Keys.Count() - 1;
	Tangents[0].InTangent = 0;
	Tangents[end].OutTangent = 0;

	if (IsLooping)
	{
		// This really only works if the start and end points have the same position...
		Tangents[0].OutTangent = (1.0f - Tightness[0]) * (Keys[1].Point - Keys[end - 1].Point);
		Tangents[end].InTangent = Tangents[0].OutTangent;
	}
	else
	{
		Tangents[0].OutTangent = (1.0f - Tightness[0]) * (Keys[1].Point - Keys[0].Point);
		Tangents[end].InTangent = (1.0f - Tightness[0]) * (Keys[end].Point - Keys[end - 1].Point);
	}

	float total_time = (Keys[1].Time - Keys[0].Time) + (Keys[end].Time - Keys[end - 1].Time);
	float in_factor = 2.0f * (Keys[end].Time - Keys[end - 1].Time) / total_time;
	float out_factor = 2.0f * (Keys[1].Time - Keys[0].Time) / total_time;
	Tangents[end].InTangent *= in_factor;
	Tangents[0].OutTangent *= out_factor;

	// inner knots
	for (int i = 1; i < Keys.Count() - 1; i++)
	{
		Tangents[i].InTangent = (1.0f - Tightness[i]) * (Keys[i + 1].Point - Keys[i - 1].Point);
		Tangents[i].OutTangent = Tangents[i].InTangent;

		float in_factor = 2.0f * (Keys[i].Time - Keys[i - 1].Time) / (Keys[i + 1].Time - Keys[i - 1].Time);
		float out_factor = 2.0f * (Keys[i + 1].Time - Keys[i].Time) / (Keys[i + 1].Time - Keys[i - 1].Time);

		Tangents[i].InTangent *= in_factor;    // compensating for the un-even keys
		Tangents[i].OutTangent *= out_factor;
	}
	TangentsDirty = false;
}
