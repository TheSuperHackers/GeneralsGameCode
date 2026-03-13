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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/hrawanim.h                             $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/27/01 7:50p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "hanim.h"

class MotionChannelClass;
class BitChannelClass;

struct NodeMotionStruct
{
	NodeMotionStruct();
	~NodeMotionStruct();

	MotionChannelClass *		X;
	MotionChannelClass *		Y;
	MotionChannelClass *		Z;
	MotionChannelClass *		XR;
	MotionChannelClass *		YR;
	MotionChannelClass *		ZR;
	MotionChannelClass *		Q;

	BitChannelClass *			Vis;
};

/**********************************************************************************

	HRawAnimClass

	Stores motion data to be applied to a HierarchyTree.  Each frame
	of the motion contains deltas from the HierarchyTree's base position
	to the desired position.

**********************************************************************************/

class HRawAnimClass : public HAnimClass
{

public:

	enum
	{
		OK,
		LOAD_ERROR
	};

	HRawAnimClass();
	virtual ~HRawAnimClass() override;

	int							Load_W3D(ChunkLoadClass & cload);

	virtual const char *				Get_Name() const override { return Name; }
	virtual const char *				Get_HName() const override { return HierarchyName; }
	virtual int							Get_Num_Frames() override { return NumFrames; }
	virtual float							Get_Frame_Rate() override { return FrameRate; }
	virtual float							Get_Total_Time() override { return (float)NumFrames / FrameRate; }

	virtual void							Get_Translation(Vector3& translation, int pividx,float frame) const override;
	virtual void							Get_Orientation(Quaternion& orientation, int pividx,float frame) const override;
	virtual void							Get_Transform(Matrix3D& transform, int pividx,float frame) const override;
	virtual bool							Get_Visibility(int pividx,float frame) override;

	virtual bool							Is_Node_Motion_Present(int pividx) override;
	virtual int							Get_Num_Pivots() const override { return NumNodes; }

	// Methods that test the presence of a certain motion channel.
	virtual bool							Has_X_Translation (int pividx) override;
	virtual bool							Has_Y_Translation (int pividx) override;
	virtual bool							Has_Z_Translation (int pividx) override;
	virtual bool							Has_Rotation (int pividx) override;
	virtual bool							Has_Visibility (int pividx) override;
	NodeMotionStruct				*Get_Node_Motion_Array() {return NodeMotion;}
	virtual int					Class_ID()	const override { return CLASSID_HRAWANIM; }

private:

	char							Name[2*W3D_NAME_LEN];
	char							HierarchyName[W3D_NAME_LEN];

	int							NumFrames;
	int							NumNodes;
	float							FrameRate;

	NodeMotionStruct *		NodeMotion;

	void Free();
	bool read_channel(ChunkLoadClass & cload,MotionChannelClass * * newchan,bool pre30);
	void add_channel(MotionChannelClass * newchan);

	bool read_bit_channel(ChunkLoadClass & cload,BitChannelClass * * newchan,bool pre30);
	void add_bit_channel(BitChannelClass * newchan);

};
