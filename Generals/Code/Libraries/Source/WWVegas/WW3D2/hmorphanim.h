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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/hmorphanim.h                           $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/27/01 7:41p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef HMORPHANIM_H
#define HMORPHANIM_H

#include "always.h"
#include "hanim.h"
#include "simplevec.h"

class TimeCodedMorphKeysClass;
class ChunkLoadClass;
class ChunkSaveClass;
class TextFileClass;

/**********************************************************************************

	HMorphAnimClass

	This is an animation format designed for facial animation.  It basically morphs the
	htree between a set of poses.  These animations are created by exporting an
	HRawAnimClass which contains the poses, using Magpie to create a text file
	describing which pose to use on each frame, and finally using W3dView to combine
	the data into an HMorphAnimClass.

	There can be multiple channels for the morphing.  For example, some of the
	bones can be controlled by the "phoneme" poses (e.g. mouth) while other bones are
	controlled by the "expression" poses (e.g. eyebrows)

**********************************************************************************/

class HMorphAnimClass : public HAnimClass
{

public:

	enum
	{
		OK,
		LOAD_ERROR
	};

	HMorphAnimClass(void);
	~HMorphAnimClass(void);

	void							Free_Morph(void);
	int							Create_New_Morph(const int channels, HAnimClass *anim[]);
	int							Load_W3D(ChunkLoadClass & cload);
	int							Save_W3D(ChunkSaveClass & csave);

	const char *				Get_Name(void) const								{ return Name; }
	const char *				Get_HName(void) const							{ return HierarchyName; }

	int							Get_Num_Frames(void)								{ return FrameCount; }
	float							Get_Frame_Rate()									{ return FrameRate; }
	float							Get_Total_Time()									{ return (float)FrameCount / FrameRate; }

	void							Get_Translation(Vector3& translation, int pividx,float frame) const;
	void							Get_Orientation(Quaternion& orientation, int pividx,float frame) const;
	void							Get_Transform(Matrix3D& transform, int pividx,float frame) const;
	bool							Get_Visibility(int pividx,float frame)		{ return true; }

	void							Insert_Morph_Key (const int channel, uint32 morph_frame, uint32 pose_frame);
	void							Release_Keys (void);

	bool							Is_Node_Motion_Present(int pividx)			{ return true; }
	int							Get_Num_Pivots(void)	const						{ return NumNodes; }

	void							Set_Name(const char * name);
	void							Set_HName(const char * hname);

	bool							Import(const char *hierarchy_name, TextFileClass &text_desc);

protected:

	void							Free(void);
	void							read_channel(ChunkLoadClass & cload,int channel);
	void							write_channel(ChunkSaveClass & csave,int channel);
	void							Resolve_Pivot_Channels(void);

	char							Name[2*W3D_NAME_LEN];
	char							AnimName[W3D_NAME_LEN];
	char							HierarchyName[W3D_NAME_LEN];

	int							FrameCount;								// number of frames in the animation
	float							FrameRate;								// framerate for playback
	int							ChannelCount;							// number of independent morphing channels
	int							NumNodes;

	HAnimClass **					PoseData;		// pointer to pose for each morph channel
	TimeCodedMorphKeysClass *	MorphKeyData;	// morph keys for each channel
	uint32 *							PivotChannel;	// controlling channel for each pivot/bone

};


/*********************************************************************************************
**
** TimeCodedMorphKeysClass
** This class basically stores a vector of morph keys.  An HMorphAnimClass contains
** one of these for each independent morphing channel.  For example, the facial animation
** stuff generates HMorphAnims which contain 2 channels, one which specifies what the
** "phoneme" bones are doing and one which specifies what the "expression" bones are doing.
**
*********************************************************************************************/

class TimeCodedMorphKeysClass
{
public:

	TimeCodedMorphKeysClass(void);
	~TimeCodedMorphKeysClass(void);

	bool					Load_W3D(ChunkLoadClass & cload);
	bool					Save_W3D(ChunkSaveClass & csave);
	void					Get_Morph_Info(float morph_frame,int * pose_frame0,int * pose_frame1,float * fraction);

	void					Add_Key (uint32 morph_frame, uint32 pose_frame);

private:

	struct MorphKeyStruct
	{
		MorphKeyStruct (void)
			:	MorphFrame (0),
				PoseFrame (0)			{}

		MorphKeyStruct (uint32 _morph, uint32 _pose)
			:	MorphFrame (_morph),
				PoseFrame (_pose)		{}

		uint32	MorphFrame;				// morph animation frame index
		uint32	PoseFrame;				// which pose frame to use at this time
	};

	SimpleDynVecClass<MorphKeyStruct>	Keys;	// morph key data
	uint32				CachedIdx;					// last accessed index

	void 					Free(void);

	uint32				get_index(float time);
	uint32				binary_search_index(float time);

	friend class HMorphAnimClass;
};



#endif


