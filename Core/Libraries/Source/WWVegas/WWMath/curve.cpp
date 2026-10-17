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
 *                     $Archive:: /VSS_Sync/wwmath/curve.cpp                                  $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 6/13/01 2:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "curve.h"
#include "WWDebug/wwdebug.h"

/***********************************************************************************************
**
** Curve3DCLass Implementation
**
***********************************************************************************************/
Curve3DClass::Curve3DClass() :
	IsLooping(false)
{
}

Curve3DClass::Curve3DClass(const Curve3DClass & that)
{
	*this = that;
}

Curve3DClass::~Curve3DClass()
{
}

Curve3DClass & Curve3DClass::operator = (const Curve3DClass & that)
{
	IsLooping = that.IsLooping;
	Keys = that.Keys;
	return *this;
}

bool Curve3DClass::Is_Looping()
{
	return IsLooping;
}

void Curve3DClass::Set_Looping(bool onoff)
{
	IsLooping = onoff;
}

float Curve3DClass::Get_Start_Time()
{
	if (Keys.Count() > 0) {
		return Keys[0].Time;
	} else {
		return 0.0f;
	}
}

float Curve3DClass::Get_End_Time()
{
	if (Keys.Count() > 0) {
		return Keys[Keys.Count() - 1].Time;
	} else {
		return 0.0f;
	}
}

int Curve3DClass::Key_Count()
{
	return Keys.Count();
}

void Curve3DClass::Get_Key(int i,Vector3 * set_point,float * set_t)
{
	assert(i >= 0);
	assert(i < Keys.Count());
	if (set_point != nullptr) {
		*set_point = Keys[i].Point;
	}
	if (set_t != nullptr) {
		*set_t = Keys[i].Time;
	}
}

void Curve3DClass::Set_Key(int i,const Vector3 & point)
{
	assert(i >= 0);
	assert(i < Keys.Count());
	Keys[i].Point = point;
}


int Curve3DClass::Add_Key(const Vector3 & point,float t)
{
	int idx = 0;
	while (idx < Keys.Count() && Keys[idx].Time < t) {
		idx++;
	}

	KeyClass newkey;
	newkey.Point = point;
	newkey.Time = t;

	Keys.Insert(idx,newkey);
	return idx;
}

void Curve3DClass::Remove_Key(int i)
{
	assert(i >= 0);
	assert(i < Keys.Count());
	Keys.Delete(i);
}

void Curve3DClass::Clear_Keys()
{
	Keys.Clear();
}

void Curve3DClass::Find_Interval(float time,int * i0,int * i1,float * t)
{
	WWASSERT(time >= Keys[0].Time);
	WWASSERT(time <= Keys[Keys.Count()-1].Time);

	int i=0;
	while (time > Keys[i+1].Time) {
		i++;
	}
	*i0 = i;
	*i1 = i+1;
	*t = (time - Keys[i].Time) / (Keys[i+1].Time - Keys[i].Time);
}

/***********************************************************************************************
**
** LinearCurve3DClass Implementation
** Linear curve, linearly interpolates the keys
**
***********************************************************************************************/
void LinearCurve3DClass::Evaluate(float time,Vector3 * set_val)
{
	if (time < Keys[0].Time) {
		*set_val = Keys[0].Point;
		return;
	}

	if (time >= Keys[Keys.Count() - 1].Time) {
		*set_val = Keys[Keys.Count() - 1].Point;
		return;
	}

	int i0,i1;
	float t;
	Find_Interval(time,&i0,&i1,&t);

	*set_val = Keys[i0].Point + t * (Keys[i1].Point - Keys[i0].Point);
}

/***********************************************************************************************
**
** Curve1DClass
**
***********************************************************************************************/
Curve1DClass::Curve1DClass() :
	IsLooping(false)
{
}

Curve1DClass::Curve1DClass(const Curve1DClass & that)
{
	*this = that;
}

Curve1DClass::~Curve1DClass()
{
}

Curve1DClass & Curve1DClass::operator = (const Curve1DClass & that)
{
	IsLooping = that.IsLooping;
	Keys = that.Keys;
	return *this;
}

bool Curve1DClass::Is_Looping()
{
	return IsLooping;
}

void Curve1DClass::Set_Looping(bool onoff)
{
	IsLooping = onoff;
}

float Curve1DClass::Get_Start_Time()
{
	if (Keys.Count() > 0) {
		return Keys[0].Time;
	} else {
		return 0.0f;
	}
}

float Curve1DClass::Get_End_Time()
{
	if (Keys.Count() > 0) {
		return Keys[Keys.Count() - 1].Time;
	} else {
		return 0.0f;
	}
}

int Curve1DClass::Key_Count()
{
	return Keys.Count();
}

void Curve1DClass::Get_Key(int i,float * set_point,float * set_t,unsigned int * extra)
{
	assert(i >= 0);
	assert(i < Keys.Count());
	if (set_point != nullptr) {
		*set_point = Keys[i].Point;
	}
	if (set_t != nullptr) {
		*set_t = Keys[i].Time;
	}
	if (extra != nullptr) {
		*extra = Keys[i].Extra;
	}
}

void Curve1DClass::Set_Key(int i,float point,unsigned int extra)
{
	assert(i >= 0);
	assert(i < Keys.Count());
	Keys[i].Point = point;
	Keys[i].Extra = extra;
}


int Curve1DClass::Add_Key(float point,float t,unsigned int extra)
{
	int idx = 0;
	while (idx < Keys.Count() && Keys[idx].Time < t) {
		idx++;
	}

	KeyClass newkey;
	newkey.Point = point;
	newkey.Time = t;
	newkey.Extra = extra;

	Keys.Insert(idx,newkey);
	return idx;
}

void Curve1DClass::Remove_Key(int i)
{
	assert(i >= 0);
	assert(i < Keys.Count());
	Keys.Delete(i);
}

void Curve1DClass::Clear_Keys()
{
	Keys.Clear();
}

void Curve1DClass::Find_Interval(float time,int * i0,int * i1,float * t)
{
	if (IsLooping) {
		if (time < Keys[0].Time) {
			*i0 = Keys.Count() - 1;
			*i1 = 0;
			float interval = 1.0f - Keys[*i0].Time + Keys[*i1].Time;
			*t = (1.0f - Keys[*i0].Time + time) / interval;
			return;
		}
		else if (time > Keys[Keys.Count() - 1].Time) {
			*i0 = Keys.Count() - 1;
			*i1 = 0;
			float interval = 1.0f - Keys[*i0].Time + Keys[*i1].Time;
			*t = (time - Keys[*i0].Time) / interval;
			return;
		}
	}
	else {
		WWASSERT(time >= Keys[0].Time);
		WWASSERT(time <= Keys[Keys.Count()-1].Time);
	}

	int i=0;
	while (time > Keys[i+1].Time) {
		i++;
	}
	*i0 = i;
	*i1 = i+1;
	*t = (time - Keys[i].Time) / (Keys[i+1].Time - Keys[i].Time);
}

/***********************************************************************************************
**
** LinearCurve1DClass, linearly interpolates the keys
**
***********************************************************************************************/
void LinearCurve1DClass::Evaluate(float time,float * set_val)
{
	if (!IsLooping) {
		if (time < Keys[0].Time) {
			*set_val = Keys[0].Point;
			return;
		}

		if (time >= Keys[Keys.Count() - 1].Time) {
			*set_val = Keys[Keys.Count() - 1].Point;
			return;
		}
	}

	int i0,i1;
	float t;
	Find_Interval(time,&i0,&i1,&t);

	*set_val = Keys[i0].Point + t * (Keys[i1].Point - Keys[i0].Point);
}


