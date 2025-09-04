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
 *                     $Archive:: /Commando/Code/ww3d2/hmorphanim.cpp                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/16/02 6:39p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "hmorphanim.h"
#include "w3d_file.h"
#include "chunkio.h"
#include "assetmgr.h"
#include "htree.h"
#include "wwstring.h"
#include "textfile.h"
#include "simplevec.h"



TimeCodedMorphKeysClass::TimeCodedMorphKeysClass(void)
	:	CachedIdx (0)
{
}

TimeCodedMorphKeysClass::~TimeCodedMorphKeysClass(void)
{
	Free();
}

void TimeCodedMorphKeysClass::Free(void)
{
	Keys.Delete_All ();
	CachedIdx = 0;
}

bool TimeCodedMorphKeysClass::Load_W3D(ChunkLoadClass & cload)
{
	Free();
	uint32 key_count = cload.Cur_Chunk_Length() / sizeof(W3dMorphAnimKeyStruct);

	W3dMorphAnimKeyStruct w3dkey;
	for (uint32 i=0; i<key_count; i++) {
		cload.Read(&w3dkey,sizeof(w3dkey));
		Keys.Add (MorphKeyStruct (w3dkey.MorphFrame, w3dkey.PoseFrame));
	}
	CachedIdx = 0;
	return true;
}

bool TimeCodedMorphKeysClass::Save_W3D(ChunkSaveClass & csave)
{
	W3dMorphAnimKeyStruct w3dkey;
	for (int i=0; i<Keys.Count (); i++) {
		w3dkey.MorphFrame = Keys[i].MorphFrame;
		w3dkey.PoseFrame = Keys[i].PoseFrame;
		csave.Write(&w3dkey,sizeof(w3dkey));
	}
	return true;
}

void TimeCodedMorphKeysClass::Add_Key (uint32 morph_frame, uint32 pose_frame)
{
	Keys.Add (MorphKeyStruct (morph_frame, pose_frame));
	return ;
}

void TimeCodedMorphKeysClass::Get_Morph_Info(float morph_frame,int * pose_frame0,int * pose_frame1,float * fraction)
{
	if (morph_frame < 0.0f) {
		*pose_frame0 = *pose_frame1 = Keys[0].PoseFrame;
		*fraction = 0.0f;
		return;
	}

	if (morph_frame >= Keys[Keys.Count ()-1].MorphFrame) {
		*pose_frame0 = *pose_frame1 = Keys[Keys.Count ()-1].PoseFrame;
		*fraction = 0.0f;
		return;
	}

	int key_index = get_index(morph_frame);

	*pose_frame0 = Keys[key_index].PoseFrame;
	*pose_frame1 = Keys[key_index+1].PoseFrame;
	*fraction = (morph_frame - Keys[key_index].MorphFrame) / (Keys[key_index+1].MorphFrame - Keys[key_index].MorphFrame);
}


uint32 TimeCodedMorphKeysClass::get_index(float frame)
{
	assert(CachedIdx <= (uint32)Keys.Count ()-1);

	float	cached_frame = Keys[CachedIdx].MorphFrame;

	// check if the requested time is in the cached interval or the following one
	if (frame >= cached_frame) {

		// special case for end packets
		if (CachedIdx == (uint32)Keys.Count ()-1) return(CachedIdx);

		// check if the requested time is still in the cached interval
		if (frame < Keys[CachedIdx + 1].MorphFrame) return(CachedIdx);

		// do one time look-ahead before reverting to a search
		CachedIdx++;

		// again, special case the end interval
		if (CachedIdx == (uint32)Keys.Count ()-1) return(CachedIdx);

		// check if requested time is in this interval
		if (frame < Keys[CachedIdx + 1].MorphFrame) return(CachedIdx);
	}

	// nope, fall back to a binary search for the requested interval
	CachedIdx = binary_search_index(frame);

	return(CachedIdx);
}

uint32 TimeCodedMorphKeysClass::binary_search_index(float req_frame)
{
	// special case first and last packet
	if (req_frame < Keys[0].MorphFrame) return 0;
	if (req_frame >= Keys[Keys.Count ()-1].MorphFrame) return Keys.Count ()-1;

	int leftIdx = 0;
	int rightIdx = Keys.Count ();
	int idx,dx;

	// binary search for the desired interval
	for (;;) {

		// if we've zeroed in on the interval, return the left index
		dx = rightIdx - leftIdx;
		if (dx == 1) {
			WWASSERT(req_frame >= Keys[leftIdx].MorphFrame);
			WWASSERT(req_frame < Keys[rightIdx].MorphFrame);
			return leftIdx;
		}

		// otherwise, split our interval in half and descend into one of them
		dx>>=1;
		idx = leftIdx + dx;

		if (req_frame < Keys[idx].MorphFrame) {
			rightIdx = idx;
		} else {
			leftIdx = idx;
		}
	}

	assert(0);
	return(0);
}


/*********************************************************************************************
**
** HMorphAnimClass Implementation
**
*********************************************************************************************/

HMorphAnimClass::HMorphAnimClass(void) :
	FrameCount(0),
	FrameRate(0.0f),
	ChannelCount(0),
	NumNodes(0),
	PoseData(NULL),
	MorphKeyData(NULL),
	PivotChannel(NULL)
{
	memset(Name,0,sizeof(Name));
	memset(AnimName,0,sizeof(AnimName));
	memset(HierarchyName,0,sizeof(HierarchyName));
}

HMorphAnimClass::~HMorphAnimClass(void)
{
	Free();
}

void HMorphAnimClass::Free(void)
{
	if (PoseData != NULL) {
		for (int i=0; i<ChannelCount; i++) {
			REF_PTR_RELEASE(PoseData[i]);
		}
		delete[] PoseData;
		PoseData = NULL;
	}

	if (MorphKeyData != NULL) {
		delete[] MorphKeyData;
		MorphKeyData = NULL;
	}

	if (PivotChannel != NULL) {
		delete[] PivotChannel;
		PivotChannel = NULL;
	}
}


static int Build_List_From_String
(
	const char *	buffer,
	const char *	delimiter,
	StringClass **	string_list
)
{
	int count = 0;

	WWASSERT (buffer != NULL);
	WWASSERT (delimiter != NULL);
	WWASSERT (string_list != NULL);
	if ((buffer != NULL) &&
		 (delimiter != NULL) &&
		 (string_list != NULL))
	{
		int delim_len = ::strlen (delimiter);

		//
		// Determine how many entries there will be in the list
		//
		const char *entry = buffer;
		for (;
			  (entry != NULL) && (entry[1] != 0);
			  entry = ::strstr (entry, delimiter))
		{

			//
			// Move past the current delimiter (if necessary)
			//
			if ((::strnicmp (entry, delimiter, delim_len) == 0) && (count > 0)) {
				entry += delim_len;
			}

			// Increment the count of entries
			count ++;
		}

		if (count > 0) {

			//
			// Allocate enough StringClass objects to hold all the strings in the list
			//
			(*string_list) = W3DNEWARRAY StringClass[count];

			//
			// Parse the string and pull out its entries.
			//
			count = 0;
			for (entry = buffer;
				  (entry != NULL) && (entry[1] != 0);
				  entry = ::strstr (entry, delimiter))
			{

				//
				// Move past the current delimiter (if necessary)
				//
				if ((::strnicmp (entry, delimiter, delim_len) == 0) && (count > 0)) {
					entry += delim_len;
				}

				//
				// Copy this entry into its own string
				//
				StringClass entry_string = entry;
				char *delim_start = ::strstr (entry_string.Peek_Buffer(), delimiter);
				if (delim_start != NULL) {
					delim_start[0] = 0;
				}

				//
				// Add this entry to our list
				//
				if ((entry_string.Get_Length () > 0) || (count == 0)) {
					(*string_list)[count++] = entry_string;
				}
			}

		} else if (delim_len > 0) {
			count = 1;
			(*string_list) = W3DNEWARRAY StringClass[count];
			(*string_list)[0] = buffer;
		}

	}

	//
	// Return the number of entries in our list
	//
	return count;
}


bool Is_Number (const char *str)
{
	bool retval = true;

	while (retval && str[0] != NULL){
		retval = ((str[0] >= '0' && str[0] <= '9') || str[0] == '-' || str[0] == '.');
		str ++;
	}

	return retval;
}


bool HMorphAnimClass::Import(const char *hierarchy_name, TextFileClass &text_desc)
{
	bool retval = false;
	Free ();
	FrameCount = 0;
	FrameRate = 30.0F;

	//
	// Copy the hierarchy name into a class variable
	//
	::strlcpy (HierarchyName, hierarchy_name, W3D_NAME_LEN);

	//
	// Attempt to load the new base pose
	//
	HTreeClass * base_pose = WW3DAssetManager::Get_Instance()->Get_HTree(HierarchyName);
	WWASSERT (base_pose != NULL);
	NumNodes = base_pose->Num_Pivots();

	//
	// Read the header from the file
	//
	StringClass header;
	bool success = text_desc.Read_Line (header);
	if (success) {

		//
		// Get the list of comma-delimited strings from the header
		//
		StringClass *column_list = NULL;
		int column_count = Build_List_From_String (header, ",", &column_list);

		//
		// The first column header should be 'Frame#', all other headers
		// should be channel animation names.
		//
		ChannelCount = column_count - 1;

		WWASSERT (ChannelCount > 0);
		if (ChannelCount > 0) {

			//
			// Allocate and initialize each animation channel
			//
			PoseData			= W3DNEWARRAY HAnimClass *[ChannelCount];
			MorphKeyData	= W3DNEWARRAY TimeCodedMorphKeysClass[ChannelCount];
			for (int index = 0; index < ChannelCount; index ++) {
				StringClass channel_anim_name;
				channel_anim_name.Format ("%s.%s", HierarchyName, (const char *)column_list[index + 1]);
				PoseData[index] = WW3DAssetManager::Get_Instance()->Get_HAnim (channel_anim_name);
			}

			//
			// Now read the animation data for each frame
			//
			StringClass frame_desc;
			while (text_desc.Read_Line (frame_desc)) {

				//
				// Get the frame descriptions from this line
				//
				StringClass *channel_list = NULL;
				int list_count = Build_List_From_String (frame_desc, ",", &channel_list);

				WWASSERT (list_count > 0);
				if (list_count > 0) {

					//
					// The first column contains the morph frame number
					//
					int frame = ::atoi (channel_list[0]);

					//
					// Now read the animation frame number for each channel
					//
					for (int index = 1; index < list_count; index ++) {
						StringClass &channel_frame = channel_list[index];

						//
						// If this channel contains a valid number, then record
						// its animation frame
						//
						if (::Is_Number (channel_frame)) {
							MorphKeyData[index - 1].Add_Key (frame, ::atoi (channel_frame));
						}
					}

					FrameCount = frame + 1;
				}

				//
				// Cleanup
				//
				if (channel_list != NULL) {
					delete [] channel_list;
					channel_list = NULL;
				}
			}

			//
			// Allocate the pivot channel list
			//
			PivotChannel = W3DNEWARRAY uint32[NumNodes];
			Resolve_Pivot_Channels ();
		}

		//
		// Cleanup
		//
		if (column_list != NULL) {
			delete [] column_list;
			column_list = NULL;
		}
	}

	return retval;
}

void HMorphAnimClass::Resolve_Pivot_Channels(void)
{
	WWASSERT (PivotChannel != NULL);

	//
	//	Loop over all the pivots in the HTree
	//
	for (int pivot = 0; pivot < NumNodes; pivot ++) {
		PivotChannel[pivot] = 0;

		//
		// Find out which animation channel affects this pivot
		//
		for (int channel = 0; channel < ChannelCount; channel ++) {
			if (PoseData[channel]->Is_Node_Motion_Present (pivot)) {
				PivotChannel[pivot] = channel;
			}
		}
	}

	return ;
}

void HMorphAnimClass::Set_Name(const char * name)
{
	//
	// Copy the full name
	//
	::strcpy (Name, name);

	//
	// Try to find the separator (a period)
	//
	StringClass full_name	= name;
	char *separator			= ::strchr (full_name.Peek_Buffer(), '.');
	if (separator != NULL) {

		//
		// Null out the separator and copy the two names
		// into our two buffers
		//
		separator[0] = 0;
		::strcpy (AnimName, separator + 1);
		::strcpy (HierarchyName, full_name);
	}

	return ;
}

void HMorphAnimClass::Free_Morph(void)
{
	Free();
}

int HMorphAnimClass::Create_New_Morph(const int channels, HAnimClass *anim[])
{
	// clean out the previous instance of this class
	Free();

	// set the number of channels
	ChannelCount = channels;

	// read in the animation header
	if (anim == NULL) {
		return LOAD_ERROR;
	}

	// set up info
	//	FrameCount = anim[0]->Get_Num_Frames();
	FrameCount = 0;
	FrameRate = anim[0]->Get_Frame_Rate();
	NumNodes = anim[0]->Get_Num_Pivots();

	// Set up the anim data for all the channels
	PoseData = W3DNEWARRAY HAnimClass * [ChannelCount];
	for(int i=0;i<ChannelCount;i++)
		PoseData[i] = anim[i];

	// Create a timecodekey array for each channel and initialize the pivot channels
	MorphKeyData = W3DNEWARRAY TimeCodedMorphKeysClass[ChannelCount];
	PivotChannel = W3DNEWARRAY uint32[NumNodes];

	// Resolve the pivots so that they correspond to the proper morphing channels
	memset(PivotChannel,0,NumNodes * sizeof(uint32));
	Resolve_Pivot_Channels();

	// Signal successful process
	return OK;
}

int HMorphAnimClass::Load_W3D(ChunkLoadClass & cload)
{
	Free();

	// read in the animation header
	W3dMorphAnimHeaderStruct header;
	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == W3D_CHUNK_MORPHANIM_HEADER);
	cload.Read(&header,sizeof(header));
	cload.Close_Chunk();

	strlcpy(AnimName,header.Name,sizeof(AnimName));
	strlcpy(HierarchyName,header.HierarchyName,sizeof(HierarchyName));
	strcpy(Name,HierarchyName);
	strcat(Name,".");
	strcat(Name,AnimName);

	HTreeClass * base_pose = WW3DAssetManager::Get_Instance()->Get_HTree(HierarchyName);
	if (base_pose == NULL) {
		return LOAD_ERROR;
	}
	NumNodes = base_pose->Num_Pivots();

	FrameCount = header.FrameCount;
	FrameRate = header.FrameRate;
	ChannelCount = header.ChannelCount;

	PoseData = W3DNEWARRAY HAnimClass * [ChannelCount];
	MorphKeyData = W3DNEWARRAY TimeCodedMorphKeysClass[ChannelCount];
	PivotChannel = W3DNEWARRAY uint32[NumNodes];
	memset(PivotChannel,0,NumNodes * sizeof(uint32));

	// read in the rest of the chunks
	int cur_channel = 0;
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID())
		{
		case W3D_CHUNK_MORPHANIM_CHANNEL:
			read_channel(cload,cur_channel++);
			break;

		case W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA:
			cload.Read(PivotChannel,cload.Cur_Chunk_Length());
			break;
		};
		cload.Close_Chunk();
	}
	return OK;

}

void HMorphAnimClass::read_channel(ChunkLoadClass & cload,int channel)
{
	WWASSERT(channel >= 0);
	WWASSERT(channel < ChannelCount);

	StringClass anim_name;

	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == W3D_CHUNK_MORPHANIM_POSENAME);
	cload.Read(anim_name.Get_Buffer(cload.Cur_Chunk_Length()),cload.Cur_Chunk_Length());
	cload.Close_Chunk();

	//StringClass channel_anim_name;
	//channel_anim_name.Format ("%s.%s", HierarchyName, anim_name);
	PoseData[channel] = WW3DAssetManager::Get_Instance()->Get_HAnim(anim_name);
	WWASSERT(PoseData[channel] != NULL);

	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == W3D_CHUNK_MORPHANIM_KEYDATA);
	MorphKeyData[channel].Load_W3D(cload);
	cload.Close_Chunk();
}


int HMorphAnimClass::Save_W3D(ChunkSaveClass & csave)
{
	// W3D objects write their own wrapper chunks
	csave.Begin_Chunk(W3D_CHUNK_MORPH_ANIMATION);

	// init the header data
	W3dMorphAnimHeaderStruct header;
	strlcpy(header.Name,AnimName,sizeof(header.Name));
	strlcpy(header.HierarchyName,HierarchyName,sizeof(header.HierarchyName));

	header.FrameCount = FrameCount;
	header.FrameRate = FrameRate;
	header.ChannelCount = ChannelCount;

	// write out the animation header
	csave.Begin_Chunk(W3D_CHUNK_MORPHANIM_HEADER);
	csave.Write(&header,sizeof(header));
	csave.End_Chunk();

	// write out the morph channels
	for (int ci=0; ci<ChannelCount; ci++) {
		csave.Begin_Chunk(W3D_CHUNK_MORPHANIM_CHANNEL);
		write_channel(csave,ci);
		csave.End_Chunk();
	}

	// write out the pivot attachments
	csave.Begin_Chunk(W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA);
	csave.Write(PivotChannel,NumNodes * sizeof(uint32));
	csave.End_Chunk();

	csave.End_Chunk();
	return OK;
}

void HMorphAnimClass::write_channel(ChunkSaveClass & csave,int channel)
{
	WWASSERT(PoseData[channel] != NULL);

	const char * pose_name = PoseData[channel]->Get_Name();
	csave.Begin_Chunk(W3D_CHUNK_MORPHANIM_POSENAME);
	csave.Write(pose_name,strlen(pose_name) + 1);
	csave.End_Chunk();

	csave.Begin_Chunk(W3D_CHUNK_MORPHANIM_KEYDATA);
	MorphKeyData[channel].Save_W3D(csave);
	csave.End_Chunk();
}


void HMorphAnimClass::Get_Translation(Vector3& trans,int pividx,float frame) const
{
	int channel = PivotChannel[pividx];
	int pose_frame0,pose_frame1;
	float fraction;
	MorphKeyData[channel].Get_Morph_Info(frame,&pose_frame0,&pose_frame1,&fraction);

	Vector3 t0;
	PoseData[channel]->Get_Translation(t0,pividx,pose_frame0);
	Vector3 t1;
	PoseData[channel]->Get_Translation(t1,pividx,pose_frame1);
	Vector3::Lerp(t0,t1,fraction,&trans);
}

void HMorphAnimClass::Get_Orientation(Quaternion& q, int pividx,float frame) const
{
	int channel = PivotChannel[pividx];
	int pose_frame0,pose_frame1;
	float fraction;
	MorphKeyData[channel].Get_Morph_Info(frame,&pose_frame0,&pose_frame1,&fraction);

	Quaternion q0;
	PoseData[channel]->Get_Orientation(q0,pividx,pose_frame0);
	Quaternion q1;
	PoseData[channel]->Get_Orientation(q1,pividx,pose_frame1);
	::Fast_Slerp(q,q0,q1,fraction);
}

void HMorphAnimClass::Get_Transform(Matrix3D& mtx,int pividx,float frame) const
{
	int channel = PivotChannel[pividx];
	int pose_frame0,pose_frame1;
	float fraction;
	MorphKeyData[channel].Get_Morph_Info(frame,&pose_frame0,&pose_frame1,&fraction);

	Quaternion q0;
	PoseData[channel]->Get_Orientation(q0,pividx,pose_frame0);
	Quaternion q1;
	PoseData[channel]->Get_Orientation(q1,pividx,pose_frame1);
	Quaternion q;
	::Fast_Slerp(q,q0,q1,fraction);
	::Build_Matrix3D(q,mtx);
	Vector3 t0;
	PoseData[channel]->Get_Translation(t0,pividx,pose_frame0);
	Vector3 t1;
	PoseData[channel]->Get_Translation(t1,pividx,pose_frame1);
	Vector3 trans;
	Vector3::Lerp(t0,t1,fraction,&trans);
	mtx.Set_Translation(trans);
}


void HMorphAnimClass::Insert_Morph_Key(const int channel, uint32 morph_frame, uint32 pose_frame)
{
	assert(channel<ChannelCount);
	MorphKeyData[channel].Add_Key(morph_frame,pose_frame);

	// update the framecount to reflect the newly added key
	FrameCount = WWMath::Max(morph_frame,FrameCount);
}

void HMorphAnimClass::Release_Keys(void)
{
	for(int i=0;i<ChannelCount;i++)
		MorphKeyData[i].Free();

	// update the framecount as 0
	FrameCount = 0;
}
