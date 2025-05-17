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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/vehiclecurve.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/12/01 10:02a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "vehiclecurve.h"
#include "vector3.h"
#include "matrix3d.h"
#include "persistfactory.h"
#include "wwmathids.h"
#include "wwmemlog.h"


//////////////////////////////////////////////////////////////////////
// Save-Load stuff
//////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<VehicleCurveClass,WWMATH_CHUNKID_VEHICLECURVE>	_VehicleCurveFactory;

////////////////////////////////////////////////////////////////
//	Save/Load constants
////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_PARENT					= 0x11071217,
	CHUNKID_ARC_INFO,
	CHUNKID_VARIABLES
};

enum
{
	VARID_IS_DIRTY					= 1,
	VARID_RADIUS,
};


//////////////////////////////////////////////////////////////////////
//	Local prototypes
//////////////////////////////////////////////////////////////////////
bool	Find_Tangent (const Vector3 &center, float radius, const Vector3 &point, bool clockwise, float *result);
float	Get_Angle_Delta (float angle1, float angle2, bool clockwise);
void	Find_Turn_Arc (const Matrix3D &transform, float radius, const Vector3 &prev_pt, const Vector3 &curr_pt, const Vector3 &next_pt, Vector3 *arc_center, bool *is_right_turn);
void	Find_Tangents (float radius, const Vector3 &prev_pt, const Vector3 &curr_pt, const Vector3 &next_pt, const Vector3 &arc_center, bool is_right_turn, float *point_angle, float *angle_in_delta, float *angle_out_delta);


//////////////////////////////////////////////////////////////////////
//
//	Find_Tangent
//
//////////////////////////////////////////////////////////////////////
bool
Find_Tangent
(
	const Vector3 &	center,
	float					radius,
	const Vector3 &	point,
	bool					clockwise,
	float	*				result
)
{
	bool retval = false;

	//
	//	Calculate the distance from the point to the center of the circle
	//
	float delta_x = point.X - center.X;
	float delta_y = point.Y - center.Y;
	float dist = ::sqrt (delta_x * delta_x + delta_y * delta_y);
	if (dist >= radius) {

		//
		//	Determine the offset angle (from the line between the point and center)
		// where the 2 tangent points lie.
		//
		float angle_offset	= WWMath::Acos (radius / dist);
		float base_angle		= WWMath::Atan2 (delta_x, -delta_y);
		base_angle = WWMath::Wrap (base_angle, 0, DEG_TO_RADF (360));

		//
		//	Determine which tangent angle we would come across first, depending
		// on our orientation
		//
		float angle = 0;
		if (clockwise) {
			angle = base_angle - angle_offset;
		} else {
			angle = base_angle + angle_offset;
		}
		angle = WWMath::Wrap (angle, 0, DEG_TO_RADF (360));
		(*result) = angle;

		retval = true;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Angle_Delta
//
//	Angle deltas need to be wrapped around 360 degrees differently
// depending on the orientation (clockwise/counterclockwise).  This
// function takes orientation into consideration when determining
// the delta.
//
//////////////////////////////////////////////////////////////////////
float
Get_Angle_Delta
(
	float	angle1,
	float angle2,
	bool	clockwise
)
{
	float result = angle1 - angle2;

	if (clockwise) {
		if (angle1 < angle2) {
			result = angle1 - (angle2 - DEG_TO_RADF (360));
		}
	} else {
		if (angle1 > angle2) {
			result = (angle1 - DEG_TO_RADF (360)) - angle2;
		}
	}

	return result;
}


//////////////////////////////////////////////////////////////////////
//
//	Find_Turn_Arc
//
//////////////////////////////////////////////////////////////////////
void
Find_Turn_Arc
(
	const Matrix3D &	transform,
	float					radius,
	const Vector3 &	prev_pt,
	const Vector3 &	curr_pt,
	const Vector3 &	next_pt,
	Vector3 *			arc_center,
	bool *				is_right_turn
)
{
	//
	//	The center of the turn arc can lie anywhere on the circle centered
	// at the current point and 'radius' meters in radius.
	//
	//	We will assume the optimal center of the turn arc will lie at
	// the point halfway between the angles formed by the (prev-curr) and
	// (next-curr) vectors.
	//
	float angle1 = ::WWMath::Atan2 ((prev_pt.Y - curr_pt.Y), prev_pt.X - curr_pt.X);
	angle1 = WWMath::Wrap (angle1, 0, DEG_TO_RADF (360));

	float angle2 = ::WWMath::Atan2 ((next_pt.Y - curr_pt.Y), next_pt.X - curr_pt.X);
	angle2 = WWMath::Wrap (angle2, 0, DEG_TO_RADF (360));

	float avg_angle = (angle1 + angle2) * 0.5F;

	//
	//	Find the shortest delta between the two angles (either clockwise or
	// counterclockwise).
	//
	float delta1 = WWMath::Fabs (::Get_Angle_Delta (angle1, angle2, true));
	float delta2 = WWMath::Fabs (::Get_Angle_Delta (angle1, angle2, false));
	if (delta1 < delta2) {
		avg_angle = angle1 - (delta1 * 0.5F);
	} else {
		avg_angle = angle1 + (delta2 * 0.5F);
	}

	//
	//	Find the point on the circle at this angle
	//
	arc_center->X = curr_pt.X + (radius * ::WWMath::Cos (avg_angle));
	arc_center->Y = curr_pt.Y + (radius * ::WWMath::Sin (avg_angle));
	arc_center->Z = curr_pt.Z;	

	//
	//	Will we be making a right turn or a left turn?
	//
	Vector3 rel_center;
	Matrix3D::Inverse_Transform_Vector (transform, *arc_center, &rel_center);
	(*is_right_turn) = (rel_center.Y > 0);
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Find_Tangents
//
//////////////////////////////////////////////////////////////////////
void
Find_Tangents
(
	float					radius,
	const Vector3 &	prev_pt,
	const Vector3 &	curr_pt,
	const Vector3 &	next_pt,
	const Vector3 &	arc_center,
	bool					is_right_turn,
	float *				point_angle,
	float *				angle_in_delta,
	float *				angle_out_delta
)
{

	//
	//	Find the 'in' and 'out' tangent angles
	//
	float angle_in	= 0;
	float angle_out = 0;
	bool valid_in = ::Find_Tangent (arc_center, radius, prev_pt, is_right_turn, &angle_in);
	bool valid_out = ::Find_Tangent (arc_center, radius, next_pt, !is_right_turn, &angle_out);

	//
	//	Find the angle where the current position lies on the turn arc
	//
	(*point_angle) = ::WWMath::Atan2 (curr_pt.X - arc_center.X, -(curr_pt.Y - arc_center.Y));	
	(*point_angle) = WWMath::Wrap ((*point_angle), 0, DEG_TO_RADF (360));

	//
	//	If the tangent-in is valid, find its delta from the 'point angle.
	//
	if (valid_in) {
		(*angle_in_delta) = ::Get_Angle_Delta (angle_in, (*point_angle), is_right_turn);
	} else {
		(*angle_in_delta) = 0;
	}

	//
	//	If the tangent-out is valid, find its delta from the 'point angle.
	//
	if (valid_out) {
		(*angle_out_delta) = ::Get_Angle_Delta (angle_out, (*point_angle), !is_right_turn);
	} else {
		(*angle_out_delta) = 0;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Update_Arc_List
//
//////////////////////////////////////////////////////////////////////
void
VehicleCurveClass::Update_Arc_List (void)
{
	WWMEMLOG(MEM_PATHFIND);
	m_ArcList.Delete_All ();
	
	//
	//	Bail out if there is nothing to do
	//
	int count = Key_Count ();
	if (count == 0) {
		return ;
	}

	//
	//	Add a record for the starting point of the arc...
	//
	ArcInfoStruct arc_start;
	arc_start.point_in			= Keys[0].Point;
	arc_start.point_out			= Keys[0].Point;
	arc_start.center				= Keys[0].Point;
	arc_start.point_angle		= 0;
	arc_start.radius				= 0;
	arc_start.angle_in_delta	= 0;
	arc_start.angle_out_delta	= 0;
	m_ArcList.Add (arc_start);
	
	//
	//	Loop over each 'interior' point and generate arc information
	// for each.
	//
	for (int index = 1; index < count - 1; index ++) {

		//
		//	Get information about the previous, next, and current points.
		//
		Vector3 prev_pt;
		Vector3 next_pt;
		Vector3 curr_pt;
		float time = 0;
		Get_Key (index-1, &prev_pt, &time);
		Get_Key (index,	&curr_pt, &time);
		Get_Key (index+1, &next_pt, &time);

		//
		//	Determine the last known point on the path
		//
		Vector3 last_path_pt = m_ArcList[index-1].point_out;

		//
		//	Create a transformation matrix to simulate the vehicle's position and
		// orientation at the last point...
		//
		Vector3 x_vector (curr_pt - last_path_pt);
		Vector3 z_vector (0, 0, 1);
		x_vector.Normalize ();
#ifdef ALLOW_TEMPORARIES
		Vector3 y_vector = Vector3::Cross_Product (x_vector, z_vector);
#else
		Vector3 y_vector;
		Vector3::Cross_Product (x_vector, z_vector, &y_vector);
#endif
		Matrix3D tm (x_vector, y_vector, z_vector, last_path_pt);

		//
		//	Find where the turn arc should be centered and whether we should
		// make a right-turn or a left turn...
		//
		bool is_right_turn = false;
		Vector3 arc_center (0, 0, 0);
		::Find_Turn_Arc (	tm,
								m_Radius,
								last_path_pt,
								curr_pt,
								next_pt,
								&arc_center,
								&is_right_turn);

		//
		//	Determine where the vehicle should enter and exit the turn
		//
		float angle_in_delta		= 0;
		float angle_out_delta	= 0;
		float point_angle			= 0;
		::Find_Tangents (	m_Radius,
								last_path_pt,
								curr_pt,
								next_pt,
								arc_center,
								is_right_turn,
								&point_angle,
								&angle_in_delta,
								&angle_out_delta);

		//
		//	Determine at what points these angles intersect the arc
		//
		Vector3 point_in (0, 0, 0);
		point_in.X = arc_center.X + (m_Radius * ::WWMath::Sin (point_angle + angle_in_delta));
		point_in.Y = arc_center.Y + (m_Radius * -::WWMath::Cos (point_angle + angle_in_delta));

		Vector3 point_out (0, 0, 0);
		point_out.X = arc_center.X + (m_Radius * ::WWMath::Sin (point_angle + angle_out_delta));
		point_out.Y = arc_center.Y + (m_Radius * -::WWMath::Cos (point_angle + angle_out_delta));

		//
		//	Sanity check to ensure the vehicle doesn't try to go the long way around the
		// turn arc...
		//
		if (	angle_in_delta > DEG_TO_RADF (200) || angle_out_delta > DEG_TO_RADF (200) ||
				angle_in_delta < -DEG_TO_RADF (200) || angle_out_delta < -DEG_TO_RADF (200) )
		{
			//
			//	Record information about this arc
			//
			ArcInfoStruct arc_info;	
			arc_info.center				= curr_pt;
			arc_info.point_angle			= 0;
			arc_info.point_in				= curr_pt;
			arc_info.point_out			= curr_pt;
			arc_info.radius				= 0;
			arc_info.angle_in_delta		= 0;
			arc_info.angle_out_delta	= 0;
			m_ArcList.Add (arc_info);

		} else {

			//
			//	Record information about this arc
			//
			ArcInfoStruct arc_info;	
			arc_info.center				= arc_center;
			arc_info.point_angle			= point_angle;
			arc_info.point_in				= point_in;
			arc_info.point_out			= point_out;
			arc_info.radius				= m_Radius;
			arc_info.angle_in_delta		= angle_in_delta;
			arc_info.angle_out_delta	= angle_out_delta;
			m_ArcList.Add (arc_info);
		}
	}

	//
	//	Add a record for the starting point of the arc...
	//
	if (count > 1) {
		ArcInfoStruct arc_end;
		arc_end.point_in				= Keys[count-1].Point;
		arc_end.point_out				= Keys[count-1].Point;
		arc_end.center					= Keys[count-1].Point;
		arc_end.point_angle			= 0;
		arc_end.radius					= 0;
		arc_end.angle_in_delta		= 0;
		arc_end.angle_out_delta		= 0;
		m_ArcList.Add (arc_end);
	}

	m_IsDirty = false;
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Evaluate
//
//////////////////////////////////////////////////////////////////////
void
VehicleCurveClass::Evaluate (float time, Vector3 *set_val)
{
	int count = Keys.Count ();
	m_Sharpness = 0;

	if (time < Keys[0].Time) {
		*set_val		= Keys[0].Point;
		m_LastTime	= Keys[0].Time;
		return;
	}

	if (time >= Keys[count - 1].Time) {
		*set_val		= Keys[count - 1].Point;
		m_LastTime	= Keys[count - 1].Time;
		return;
	}

	//
	//	Update the arc information if any of the keys have changed...
	//
	if (m_IsDirty) {
		Update_Arc_List ();
	}

	//
	//	Determine which segment we are on
	//
	int index0 = 0;
	int index1 = 0;
	float seg_time = 0;
	Find_Interval (time, &index0, &index1, &seg_time);

	ArcInfoStruct &arc_info0 = m_ArcList[index0];
	ArcInfoStruct &arc_info1 = m_ArcList[index1];

	//
	//	Determine the lengths of each segment of this curve.
	// The segments are:
	//		- Exit curve from prev point
	//		- Straight line from exit of last curve to enter of this curve
	//		- Enter curve for the current point
	//
	float arc_length0		= arc_info0.radius * WWMath::Fabs (arc_info0.angle_out_delta);
	float arc_length1		= arc_info1.radius * WWMath::Fabs (arc_info1.angle_in_delta);
	float other_length	= ((arc_info1.point_in - arc_info0.point_out).Length ()) / 2;
	float total_length	= arc_length0 + arc_length1 + other_length;

	//
	//	Determine at what times we should switch between parts of the segment
	//
	float time1 = arc_length0 / total_length;
	float time2 = (arc_length0 + other_length) / total_length;

	//
	//	Determine which part of the segment we are on
	//
	if (seg_time < time1) {
		
		//
		//	We are on the initial curve of the segment, so calculate where
		// on the curve we are...
		//
		//float percent = seg_time / time1;
		//float angle = arc_info0.point_angle + (arc_info0.angle_out_delta) * percent;
		float angle = arc_info0.point_angle + arc_info0.angle_out_delta;

		set_val->X = arc_info0.center.X + (arc_info0.radius * ::WWMath::Sin (angle));
		set_val->Y = arc_info0.center.Y + (arc_info0.radius * -::WWMath::Cos (angle));

		m_Sharpness = WWMath::Clamp (WWMath::Fabs (arc_info0.angle_out_delta) / DEG_TO_RADF (15), 0, 1.0F);
		m_SharpnessPos.X = set_val->X;
		m_SharpnessPos.Y = set_val->Y;
		m_SharpnessPos.Z = Keys[index0].Point.Z + (Keys[index1].Point.Z - Keys[index0].Point.Z) * seg_time;

		m_LastTime = Keys[index0].Time + (Keys[index1].Time - Keys[index0].Time) * time1;

	} else if (seg_time < time2) {

		//
		//	We are on the line between the two curves, so calculate where on
		// the line we are
		//			
		float percent = (seg_time - time1) / (time2 - time1);

		if (percent == 0) {
			set_val->X = arc_info0.point_out.X;
			set_val->Y = arc_info0.point_out.Y;
		} else {
			set_val->X = arc_info1.point_in.X;
			set_val->Y = arc_info1.point_in.Y;			
		}

		//set_val->X = arc_info0.point_out.X + (arc_info1.point_in.X - arc_info0.point_out.X) * percent;
		//set_val->Y = arc_info0.point_out.Y + (arc_info1.point_in.Y - arc_info0.point_out.Y) * percent;

		m_Sharpness = WWMath::Clamp (WWMath::Fabs (arc_info1.angle_out_delta) / DEG_TO_RADF (15), 0, 1.0F);
		m_SharpnessPos = arc_info1.point_in;
		
		m_LastTime = Keys[index0].Time + (Keys[index1].Time - Keys[index0].Time) * time2;
		
	} else {

		//
		//	We are on the ending curve of the segment, so calculate where
		// on the curve we are...
		//
		/*float percent = 1.0F - ((seg_time - time2) / (1.0F - time2));
		float angle = arc_info1.point_angle + (arc_info1.angle_in_delta * percent);
		
		set_val->X = arc_info1.center.X + (arc_info1.radius * ::WWMath::Sin (angle));
		set_val->Y = arc_info1.center.Y + (arc_info1.radius * -::WWMath::Cos (angle));			*/

		float angle = arc_info1.point_angle + (arc_info1.angle_out_delta);

		set_val->X = arc_info1.center.X + (arc_info1.radius * ::WWMath::Sin (angle));
		set_val->Y = arc_info1.center.Y + (arc_info1.radius * -::WWMath::Cos (angle));		

		m_Sharpness = WWMath::Clamp (WWMath::Fabs (arc_info1.angle_out_delta) / DEG_TO_RADF (15), 0, 1.0F);
		m_SharpnessPos.X = set_val->X;
		m_SharpnessPos.Y = set_val->Y;
		m_SharpnessPos.Z = Keys[index0].Point.Z + (Keys[index1].Point.Z - Keys[index0].Point.Z) * seg_time;

		m_LastTime = Keys[index1].Time;
	}

	//
	//	Our Z value is just a linear interpolation
	//
	set_val->Z = Keys[index0].Point.Z + (Keys[index1].Point.Z - Keys[index0].Point.Z) * seg_time;	
	return ;
}

const PersistFactoryClass & VehicleCurveClass::Get_Factory(void) const
{
	return _VehicleCurveFactory;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
VehicleCurveClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_PARENT);
		Curve3DClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);

		//
		//	Save each variable to its own microchunk
		//
		WRITE_MICRO_CHUNK (csave, VARID_IS_DIRTY,		m_IsDirty);
		WRITE_MICRO_CHUNK (csave, VARID_RADIUS,		m_Radius);
		
	csave.End_Chunk ();

	//
	//	Save each arc info struct to its own chunk
	//
	for (int index = 0; index < m_ArcList.Count (); index ++) {
		ArcInfoStruct &arc_info = m_ArcList[index];

		csave.Begin_Chunk (CHUNKID_ARC_INFO);
			csave.Write (&arc_info, sizeof (arc_info));
		csave.End_Chunk ();
	}
	
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
VehicleCurveClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_PARENT:
				Curve3DClass::Load (cload);
				break;

			case CHUNKID_ARC_INFO:
			{
				ArcInfoStruct arc_info;
				cload.Read (&arc_info, sizeof (arc_info));
				m_ArcList.Add (arc_info);
			}
			break;

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return true;
}


///////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
///////////////////////////////////////////////////////////////////////
void
VehicleCurveClass::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_IS_DIRTY,	m_IsDirty);
			READ_MICRO_CHUNK (cload, VARID_RADIUS,		m_Radius);
		}

		cload.Close_Micro_Chunk ();
	}

	return ;
}
