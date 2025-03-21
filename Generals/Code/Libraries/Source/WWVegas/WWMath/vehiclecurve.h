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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/wwmath/vehiclecurve.h              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/13/01 2:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __VEHICLE_CURVE_H
#define __VEHICLE_CURVE_H

#include "curve.h"
#include "Vector.H"


////////////////////////////////////////////////////////////////////////////////////////////
//
//	VehicleCurveClass
//
//	A vehicle curve represents the path a vehicle would take through a series of points.
// Each point on the curve passes through a turn-arc of the vehicle.  The size of this
// arc is determined by the turn radius which is used to initialize the curve.
//
////////////////////////////////////////////////////////////////////////////////////////////
class VehicleCurveClass : public Curve3DClass
{
public:

	///////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////////////
	VehicleCurveClass (void)
		:	m_IsDirty (true),
			m_Radius (0),
			m_LastTime (0),
			m_Sharpness (0),
			m_SharpnessPos (0, 0, 0),
			Curve3DClass () { }

	VehicleCurveClass (float radius)
		:	m_IsDirty (true),
			m_Radius (radius),
			m_LastTime (0),
			m_Sharpness (0),
			m_SharpnessPos (0, 0, 0),
			Curve3DClass () { }

	virtual ~VehicleCurveClass () {}


	///////////////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////////////

	//
	//	Initialization
	//	
	void			Initialize_Arc (float radius);
	
	//
	//	From Curve3DClass
	//	
	void			Evaluate (float time, Vector3 *set_val);
	void			Set_Key (int i,const Vector3 & point);
	int			Add_Key (const Vector3 & point,float t);
	void			Remove_Key (int i);
	void			Clear_Keys (void);

	//
	//	Vehicle curve specific
	//
	float			Get_Current_Sharpness (Vector3 *position) const	{ *position = m_SharpnessPos; return m_Sharpness; }
	float			Get_Last_Eval_Time (void) const						{ return m_LastTime; }

	//
	// Save-load support
	//
	virtual const PersistFactoryClass &	Get_Factory(void) const;
	virtual bool								Save(ChunkSaveClass &csave);
	virtual bool								Load(ChunkLoadClass &cload);

protected:
	
	///////////////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////////////
	void			Update_Arc_List (void);
	void			Load_Variables (ChunkLoadClass &cload);


	///////////////////////////////////////////////////////////////////////////
	//	Protected data types
	///////////////////////////////////////////////////////////////////////////
	typedef struct _ArcInfoStruct
	{
		Vector3	center;
		Vector3	point_in;
		Vector3	point_out;
		float		point_angle;
		float		radius;
		float		angle_in_delta;
		float		angle_out_delta;

		_ArcInfoStruct (void)
			:	center (0, 0, 0),
				point_in (0, 0, 0),
				point_out (0, 0, 0),
				point_angle (0),
				radius (0),
				angle_in_delta (0),
				angle_out_delta (0)	{ }

		bool operator== (const _ArcInfoStruct &src)	{ return false; }
		bool operator!= (const _ArcInfoStruct &src)	{ return true; }

	} ArcInfoStruct;

	typedef DynamicVectorClass<ArcInfoStruct>	ARC_LIST;

	///////////////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////////////
	bool			m_IsDirty;
	float			m_Radius;
	ARC_LIST		m_ArcList;	

	float			m_LastTime;
	float			m_Sharpness;
	Vector3		m_SharpnessPos;
};

///////////////////////////////////////////////////////////////////////////
//	Set_Key
///////////////////////////////////////////////////////////////////////////
inline void
VehicleCurveClass::Set_Key (int i,const Vector3 & point)
{
	m_IsDirty = true;
	Curve3DClass::Set_Key (i, point);
	return ;
}

///////////////////////////////////////////////////////////////////////////
//	Add_Key
///////////////////////////////////////////////////////////////////////////
inline int
VehicleCurveClass::Add_Key (const Vector3 & point,float t)
{
	m_IsDirty = true;
	return Curve3DClass::Add_Key (point, t);
}

///////////////////////////////////////////////////////////////////////////
//	Remove_Key
///////////////////////////////////////////////////////////////////////////
inline void
VehicleCurveClass::Remove_Key (int i)
{
	m_IsDirty = true;
	Curve3DClass::Remove_Key (i);
	return ;
}	

///////////////////////////////////////////////////////////////////////////
//	Clear_Keys
///////////////////////////////////////////////////////////////////////////
inline void
VehicleCurveClass::Clear_Keys (void)
{
	m_IsDirty = true;
	Curve3DClass::Clear_Keys ();
	return ;
}	


#endif //__VEHICLE_CURVE_H

