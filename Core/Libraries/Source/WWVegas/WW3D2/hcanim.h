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

/* $Header: /Commando/Code/ww3d2/hcanim.h 2     6/29/01 6:41p Jani_p $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D Library                                      * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/ww3d2/hcanim.h                               $* 
 *                                                                                             * 
 *                       Author:: Greg_h                                                       * 
 *                                                                                             * 
 *                     $Modtime:: 6/27/01 7:35p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 2                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef HCANIM_H
#define HCANIM_H

#include "always.h"
#include "quat.h"
#include "refcount.h"
#include "w3d_file.h"
#include "SLIST.H"
#include "Vector.H"
#include "hanim.h"

struct NodeCompressedMotionStruct;
class TimeCodedMotionChannelClass;
class TimeCodedBitChannelClass;
class AdaptiveDeltaMotionChannelClass;
class HTreeClass;
class ChunkLoadClass;
class ChunkSaveClass;


/**********************************************************************************

	Hierarchy Compressed Animation Class

	Stores motion data to be applied to a HierarchyTree.  Each frame
	of the motion contains deltas from the HierarchyTree's base position
	to the desired position.

**********************************************************************************/

class HCompressedAnimClass : public HAnimClass
{

public:
	
	enum
	{
		OK,
		LOAD_ERROR
	};
	
	HCompressedAnimClass(void);
	~HCompressedAnimClass(void);

	int							Load_W3D(ChunkLoadClass & cload);

	const char *				Get_Name(void) const { return Name; }
	const char *				Get_HName(void) const { return HierarchyName; }
	int							Get_Num_Frames(void) { return NumFrames; }
	float							Get_Frame_Rate() { return FrameRate; }
	float							Get_Total_Time() { return (float)NumFrames / FrameRate; }
	int							Get_Flavor() { return Flavor; }

//	Vector3						Get_Translation(int pividx,float frame);
//	Quaternion					Get_Orientation(int pividx,float frame);
	void							Get_Translation(Vector3& translation, int pividx,float frame) const;
	void							Get_Orientation(Quaternion& orientation, int pividx,float frame) const;
	void							Get_Transform(Matrix3D& transform, int pividx,float frame) const;
	bool							Get_Visibility(int pividx,float frame);

	bool							Is_Node_Motion_Present(int pividx);
	int							Get_Num_Pivots(void)	const	{ return NumNodes; }

	// Methods that test the presence of a certain motion channel.
	bool							Has_X_Translation (int pividx);
	bool							Has_Y_Translation (int pividx);
	bool							Has_Z_Translation (int pividx);
	bool							Has_Rotation (int pividx);
	bool							Has_Visibility (int pividx);

private:

	char							Name[2*W3D_NAME_LEN];
	char							HierarchyName[W3D_NAME_LEN];
	
	int							NumFrames;
	int							NumNodes;
	int							Flavor;
	float							FrameRate;

	NodeCompressedMotionStruct *		NodeMotion;

	void Free(void);	
	bool read_channel(ChunkLoadClass & cload,TimeCodedMotionChannelClass * * newchan);
	bool read_channel(ChunkLoadClass & cload,AdaptiveDeltaMotionChannelClass * * newchan);
	void add_channel(TimeCodedMotionChannelClass * newchan);
	void add_channel(AdaptiveDeltaMotionChannelClass * newchan);


	bool read_bit_channel(ChunkLoadClass & cload,TimeCodedBitChannelClass * * newchan);
	void add_bit_channel(TimeCodedBitChannelClass * newchan);

};


#endif // hcanim.h
 
