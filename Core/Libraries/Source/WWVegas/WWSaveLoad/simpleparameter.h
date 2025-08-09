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
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwsaveload/simpleparameter.h                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/16/01 11:12a                                              $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif




#ifndef __SIMPLE_PARAMETER_H
#define __SIMPLE_PARAMETER_H

#include "always.h"
#include "parameter.h"
#include "vector2.h"
#include "vector3.h"
#include "matrix3d.h"
#include "rect.h"
#include <float.h>


//////////////////////////////////////////////////////////////////////////////////
//
//	SimpleParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
template <class T, ParameterClass::Type type>
class SimpleParameterClass : public ParameterClass
{
public:

	///////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////////
	SimpleParameterClass (void *data, const char *name);

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	bool				operator== (const ParameterClass &src);

	///////////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////////
	const T &		Get_Value (void) const;
	void				Set_Value (const T &new_value);

	// From Parameter class
	ParameterClass::Type	Get_Type (void) const;
	void						Copy_Value (const ParameterClass &src);

private:

	///////////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////////
	T *			m_Data;
	T				m_Min;
	T				m_Max;
};

//////////////////////////////////////////////////////////////////////////////////
//	SimpleParameterClass
//////////////////////////////////////////////////////////////////////////////////
template <class T, ParameterClass::Type type> inline
SimpleParameterClass<T, type>::SimpleParameterClass (void *data, const char *name)
{
	Set_Name (name);
	m_Data = (T *)data;
	return ;
}

//////////////////////////////////////////////////////////////////////////////////
//	Get_Value
//////////////////////////////////////////////////////////////////////////////////
template <class T, ParameterClass::Type type> inline bool
SimpleParameterClass<T, type>::operator== (const ParameterClass &src)
{
	bool retval = false;
	if (src.Get_Type () == Get_Type ()) {
		retval = ((*m_Data) == *(((const SimpleParameterClass &)src).m_Data));
	}

	return retval;
}

//////////////////////////////////////////////////////////////////////////////////
//	Get_Value
//////////////////////////////////////////////////////////////////////////////////
template <class T, ParameterClass::Type type> inline const T &
SimpleParameterClass<T, type>::Get_Value (void) const
{
	return (*m_Data);
}

//////////////////////////////////////////////////////////////////////////////////
//	Set_Value
//////////////////////////////////////////////////////////////////////////////////
template <class T, ParameterClass::Type type> inline  void
SimpleParameterClass<T, type>::Set_Value (const T &new_value)
{
	(*m_Data) = new_value;
	Set_Modified ();
	return ;
}

//////////////////////////////////////////////////////////////////////////////////
//	Get_Type
//////////////////////////////////////////////////////////////////////////////////
template <class T, ParameterClass::Type type> inline ParameterClass::Type
SimpleParameterClass<T, type>::Get_Type (void) const
{
	return type;
}

//////////////////////////////////////////////////////////////////////////////////
//	Copy_Value
//////////////////////////////////////////////////////////////////////////////////
template <class T, ParameterClass::Type type> inline void
SimpleParameterClass<T, type>::Copy_Value (const ParameterClass &src)
{
	if (Get_Type () == src.Get_Type ()) {
		(*m_Data) = ((SimpleParameterClass<T, type> &)src).Get_Value ();
	}

	ParameterClass::Copy_Value (src);
	return ;
}

//////////////////////////////////////////////////////////////////////////////////
//
//	Simple parameter types
//
//////////////////////////////////////////////////////////////////////////////////
typedef SimpleParameterClass<bool,		ParameterClass::TYPE_BOOL>			BoolParameterClass;
typedef SimpleParameterClass<Vector2,	ParameterClass::TYPE_VECTOR2>		Vector2ParameterClass;
typedef SimpleParameterClass<Vector3,	ParameterClass::TYPE_VECTOR3>		Vector3ParameterClass;
typedef SimpleParameterClass<Matrix3D, ParameterClass::TYPE_MATRIX3D>	Matrix3DParameterClass;
typedef SimpleParameterClass<RectClass,ParameterClass::TYPE_RECT>			RectParameterClass;
typedef SimpleParameterClass<Vector3,	ParameterClass::TYPE_COLOR	>		ColorParameterClass;
typedef SimpleParameterClass<int,		ParameterClass::TYPE_STRINGSDB_ID>	StringsDBEntryParameterClass;


//////////////////////////////////////////////////////////////////////////////////
//
//	RangedParameterClass
//
//		Extends simple paramter types so they can have minimum/maximum values.
//
//////////////////////////////////////////////////////////////////////////////////
template <class T, ParameterClass::Type type>
class RangedParameterClass : public SimpleParameterClass<T, type>
{
public:
	///////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////////
	RangedParameterClass (void *data, const char *name)
		:	SimpleParameterClass<T, type> (data, name) { }

	///////////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////////
	void				Set_Range (const T &min, const T &max)	{ m_Min = min; m_Max = max; }
	const T &		Get_Min (void) const							{ return m_Min; }
	const T &		Get_Max (void) const							{ return m_Max; }

private:

	///////////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////////
	T				m_Min;
	T				m_Max;
};


//////////////////////////////////////////////////////////////////////////////////
//	IntParameterClass
//////////////////////////////////////////////////////////////////////////////////
class IntParameterClass : public RangedParameterClass<int, ParameterClass::TYPE_INT>
{
public:
	IntParameterClass (void *data, const char *name)
		:	RangedParameterClass<int, ParameterClass::TYPE_INT> (data, name)
			{ Set_Range (-1000000000L, 1000000000L); }
};

//////////////////////////////////////////////////////////////////////////////////
//	FloatParameterClass
//////////////////////////////////////////////////////////////////////////////////
class FloatParameterClass  : public RangedParameterClass<float, ParameterClass::TYPE_FLOAT>
{
public:
	FloatParameterClass  (void *data, const char *name)
		:	RangedParameterClass<float, ParameterClass::TYPE_FLOAT> (data, name)
			{ Set_Range (-100000.0F, 100000.0F); }
};

//////////////////////////////////////////////////////////////////////////////////
//	AngleParameterClass
//////////////////////////////////////////////////////////////////////////////////
class AngleParameterClass   : public RangedParameterClass<float, ParameterClass::TYPE_ANGLE>
{
public:
	AngleParameterClass   (void *data, const char *name)
		:	RangedParameterClass<float, ParameterClass::TYPE_ANGLE> (data, name)
			{ Set_Range (0.0F,  6.283185307F); }
};

#endif //__SIMPLE_PARAMETER_H

