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
 *                     $Archive:: /VSS_Sync/wwmath/curve.h                                    $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 6/13/01 2:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "Vector.h"
#include "vector3.h"
#include "persist.h"

class ChunkLoadClass;
class ChunkSaveClass;


class Curve3DClass : public PersistClass
{
public:

	Curve3DClass(void);
	Curve3DClass(const Curve3DClass & that);
	virtual ~Curve3DClass(void);
	Curve3DClass & operator = (const Curve3DClass & that);

	virtual void		Evaluate(float time,Vector3 * set_val) = 0;
	virtual bool		Is_Looping(void);
	virtual void		Set_Looping(bool onoff);
	virtual int			Key_Count(void);
	virtual void		Get_Key(int i,Vector3 * set_point,float * set_t);
	virtual void		Set_Key(int i,const Vector3 & point);
	virtual int			Add_Key(const Vector3 & point,float t);
	virtual void		Remove_Key(int i);
	virtual void		Clear_Keys(void);
	float					Get_Start_Time(void);
	float					Get_End_Time(void);

	// persistant object support
	virtual bool		Save (ChunkSaveClass &csave);
	virtual bool		Load (ChunkLoadClass &cload);

protected:

	void					Find_Interval(float time,int * i0,int * i1,float * t);

	class KeyClass
	{
	public:
		Vector3			Point;
		float				Time;
		bool operator == (const KeyClass & that) { return ((Point == that.Point) && (Time == that.Time)); }
		bool operator != (const KeyClass & that) { return !KeyClass::operator==(that); }
	};

	bool									IsLooping;
	DynamicVectorClass<KeyClass>	Keys;
};


class LinearCurve3DClass : public Curve3DClass
{
public:
	virtual void		Evaluate(float time,Vector3 * set_val);

	// persistant object support
	virtual const PersistFactoryClass &	Get_Factory(void) const;
	virtual bool								Save(ChunkSaveClass &csave);
	virtual bool								Load(ChunkLoadClass &cload);
};


/*
** 1-Dimensional curve classes.
*/
class Curve1DClass : public PersistClass
{
public:

	Curve1DClass(void);
	Curve1DClass(const Curve1DClass & that);
	virtual ~Curve1DClass(void);
	Curve1DClass & operator = (const Curve1DClass & that);

	virtual void		Evaluate(float time,float * set_val) = 0;
	virtual bool		Is_Looping(void);
	virtual void		Set_Looping(bool onoff);
	virtual int			Key_Count(void);
	virtual void		Get_Key(int i,float * set_point,float * set_t,unsigned int * extra=nullptr);
	virtual void		Set_Key(int i,float point,unsigned int extra=0);
	virtual int			Add_Key(float point,float t,unsigned int extra=0);
	virtual void		Remove_Key(int i);
	virtual void		Clear_Keys(void);
	float					Get_Start_Time(void);
	float					Get_End_Time(void);

	// persistant object support
	virtual bool		Save (ChunkSaveClass &csave);
	virtual bool		Load (ChunkLoadClass &cload);

protected:

	void				Find_Interval(float time,int * i0,int * i1,float * t);

	class KeyClass
	{
	public:
		float			Point;
		float			Time;
		unsigned int	Extra;
		bool operator == (const KeyClass & that) { return ((Point == that.Point) && (Time == that.Time) && (Extra == that.Extra)); }
		bool operator != (const KeyClass & that) { return !KeyClass::operator==(that); }
	};

	bool							IsLooping;
	DynamicVectorClass<KeyClass>	Keys;
};


class LinearCurve1DClass : public Curve1DClass
{
public:
	virtual void		Evaluate(float time,float * set_val);

	// persistant object support
	virtual const PersistFactoryClass &	Get_Factory(void) const;
	virtual bool								Save(ChunkSaveClass &csave);
	virtual bool								Load(ChunkLoadClass &cload);
};
