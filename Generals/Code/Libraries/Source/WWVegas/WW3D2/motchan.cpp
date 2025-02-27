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

/* $Header: /Commando/Code/ww3d2/motchan.cpp 3     5/05/01 7:10p Jani_p $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/motchan.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 5/05/01 6:28p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MotionChannelClass::MotionChannelClass -- constructor                                     *
 *   MotionChannelClass::~MotionChannelClass -- destructor                                     *
 *   MotionChannelClass::Free -- de-allocates all memory in use                                *
 *   MotionChannelClass::Load -- loads a motion channel from a file                            *
 *   BitChannelClass::BitChannelClass -- Constructor for BitChannelClass                       *
 *   BitChannelClass::~BitChannelClass -- Destructor for BitChannelClass                       *
 *   BitChannelClass::Free -- Free all "external" memory in use                                *
 *   BitChannelClass::Load -- Read a bit channel from a w3d chunk                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "motchan.h"
#include "w3d_file.h"
#include "chunkio.h"
#include "Vector.h"
#include "wwmath.h"
#include "quat.h"

// Static Table, for Adaptive Delta Decompressor
#define FILTER_TABLE_SIZE (256)
#define FILTER_TABLE_GEN_START (16)
#define FILTER_TABLE_GEN_SIZE  (FILTER_TABLE_SIZE - FILTER_TABLE_GEN_START)

static float filtertable[FILTER_TABLE_SIZE] = {
	0.00000001f,
	0.0000001f,
	0.000001f,
	0.00001f,
	0.0001f,
	0.001f,
	0.01f,
	0.1f,
	1.0f,
	10.0f,
	100.0f,
	1000.0f,
	10000.0f,
	100000.0f,
	1000000.0f,
	10000000.0f,

};
static bool table_valid = false;


/***********************************************************************************************
 * MotionChannelClass::MotionChannelClass -- constructor                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
MotionChannelClass::MotionChannelClass(void) :
	PivotIdx(0),
	Type(0),
	VectorLen(0),
	Data(NULL),
	FirstFrame(-1),
	LastFrame(-1)
{
}

/***********************************************************************************************
 * MotionChannelClass::~MotionChannelClass -- destructor                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
MotionChannelClass::~MotionChannelClass(void)
{
	Free();
}

/***********************************************************************************************
 * MotionChannelClass::Free -- de-allocates all memory in use                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void MotionChannelClass::Free(void)
{
	delete[] Data;
	Data = NULL;
}


/***********************************************************************************************
 * MotionChannelClass::Load -- loads a motion channel from a file                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
bool MotionChannelClass::Load_W3D(ChunkLoadClass & cload)
{
	int size = cload.Cur_Chunk_Length();
	unsigned int datasize = (size - sizeof(W3dAnimChannelStruct));
	unsigned int num_floats = (datasize / sizeof(float32)) + 1;

	W3dAnimChannelStruct chan;
	if (cload.Read(&chan,sizeof(W3dAnimChannelStruct)) != sizeof(W3dAnimChannelStruct)) {
		return false;
	}

	FirstFrame = chan.FirstFrame;
	LastFrame  = chan.LastFrame;
	VectorLen  = chan.VectorLen;
	Type 			 = chan.Flags;
	PivotIdx   = chan.Pivot;

	Data = MSGW3DNEWARRAY("MotionChannelClass::Data") float32[num_floats];
	Data[0] = chan.Data[0];

	if (cload.Read(&(Data[1]),datasize) != datasize) {
		Free();
		return false;
	}
	return true;
}



/***********************************************************************************************
 * BitChannelClass::BitChannelClass -- Constructor for BitChannelClass                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
BitChannelClass::BitChannelClass(void) :
	PivotIdx(0),
	Type(0),
	DefaultVal(0),
	FirstFrame(-1),
	LastFrame(-1),
	Bits(NULL)
{
}


/***********************************************************************************************
 * BitChannelClass::~BitChannelClass -- Destructor for BitChannelClass                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
BitChannelClass::~BitChannelClass(void)
{
	Free();
}


/***********************************************************************************************
 * BitChannelClass::Free -- Free all "external" memory in use                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void BitChannelClass::Free(void)
{
	delete[] Bits;
	Bits = NULL;
}


/***********************************************************************************************
 * BitChannelClass::Load -- Read a bit channel from a w3d chunk                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool BitChannelClass::Load_W3D(ChunkLoadClass & cload)
{
	Free();

	int chunk_size = cload.Cur_Chunk_Length();

	W3dBitChannelStruct chan;
	if (cload.Read(&chan,sizeof(W3dBitChannelStruct)) != sizeof(W3dBitChannelStruct)) {
		return false;
	}

	FirstFrame = chan.FirstFrame;
	LastFrame = chan.LastFrame;
	Type = chan.Flags;
	PivotIdx = chan.Pivot;
	DefaultVal = chan.DefaultVal;

	uint32 numbits = LastFrame - FirstFrame + 1;
	uint32 numbytes = (numbits + 7) / 8;
	uint32 bytesleft = numbytes - 1;

	assert((sizeof(W3dBitChannelStruct) + bytesleft) == (unsigned)chunk_size);

	Bits = MSGW3DNEWARRAY("BitChannelClass::Bits") uint8[numbytes];
	assert(Bits);

	Bits[0] = chan.Data[0];

	if (bytesleft > 0) {
		if (cload.Read(&(Bits[1]),bytesleft) != bytesleft) {
			Free();
			return false;
		}
	}

	return true;
}


/***********************************************************************************************
 * TimeCodedMotionChannelClass::TimeCodedMotionChannelClass -- constructor                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
TimeCodedMotionChannelClass::TimeCodedMotionChannelClass(void) :
	PivotIdx(0),
	Type(0),
	VectorLen(0),
	PacketSize(0),
	Data(NULL),
	NumTimeCodes(0),
	LastTimeCodeIdx(0),	// absolute index to last time code
	CachedIdx(0)			// Last Index Used
{
}

/***********************************************************************************************
 * MotionChannelClass::~MotionChannelClass -- destructor                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
TimeCodedMotionChannelClass::~TimeCodedMotionChannelClass(void)
{
	Free();
}

/***********************************************************************************************
 * TimeCodedMotionChannelClass::Free -- de-allocates all memory in use                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void TimeCodedMotionChannelClass::Free(void)
{
	delete[] Data;
	Data = NULL;
}


/***********************************************************************************************
 * TimeCodedMotionChannelClass::Load -- loads a motion channel from a file                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
bool TimeCodedMotionChannelClass::Load_W3D(ChunkLoadClass & cload)
{
	int size = cload.Cur_Chunk_Length();
	unsigned int datasize = size - sizeof(W3dTimeCodedAnimChannelStruct);
	unsigned int numInts  = (datasize / sizeof(uint32)) + 1;

	W3dTimeCodedAnimChannelStruct chan;
	if (cload.Read(&chan,sizeof(W3dTimeCodedAnimChannelStruct)) != sizeof(W3dTimeCodedAnimChannelStruct)) {
		return false;
	}

	NumTimeCodes = chan.NumTimeCodes;
	VectorLen    = chan.VectorLen;
	Type 		    = chan.Flags;
	PivotIdx     = chan.Pivot;
	PacketSize   = VectorLen+1;
	CachedIdx	 = 0;
	LastTimeCodeIdx = (NumTimeCodes-1) * PacketSize;

	Data = MSGW3DNEWARRAY("TimeCodedMotionChannelClass::Data") uint32[numInts];
	Data[0] = chan.Data[0];

	if (cload.Read(&(Data[1]), datasize) != datasize) {
		Free();
		return false;
	}
	return true;
}


/***********************************************************************************************
 * TimeCodedMotionChannelClass::Get_Vector -- returns the vector for the specified frame #              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void	TimeCodedMotionChannelClass::Get_Vector(float32 frame,float * setvec)
{

  uint32	tc0;

  tc0 = frame;

  uint32 pidx = get_index( tc0 );
  uint32 p2idx;

  if (pidx == ((NumTimeCodes - 1) * PacketSize))  {

     float32 *frm = (float32 *) &Data[pidx+1];

		for (int i=0; i < VectorLen; i++)  {

	   	setvec[i] = frm[i];

	  }

     return;

  }
  else {
  	p2idx = pidx + PacketSize;
  }

  uint32 time = Data[p2idx];

   if (time & W3D_TIMECODED_BINARY_MOVEMENT_FLAG) {
		float32 *frm = (float32 *) &Data[pidx+1];
		for (int i=0; i < VectorLen; i++) {
			setvec[i] = frm[i];
		}
		return;
   }

  float32 time1 = (Data[pidx]  & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG);
  float32 time2 = (time & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG);

  float32 ratio = (frame - time1) / (time2 - time1);

  float32 *frame1 = (float32 *) &Data[pidx+1];
  float32 *frame2 = (float32 *) &Data[p2idx+1];

  for (int i=0; i < VectorLen; i++)  {

     setvec[i] = WWMath::Lerp(frame1[i],frame2[i],ratio);

  }

}


Quaternion TimeCodedMotionChannelClass::Get_QuatVector(float32 frame)
{

	assert(VectorLen == 4);

	Quaternion q(1);

	uint32	tc0;

	tc0 = frame;

	uint32 pidx = get_index( tc0 );
	uint32 p2idx;

	if (pidx == ((NumTimeCodes - 1) * PacketSize))  {

		float32 *vec = (float32 *) &Data[pidx+1];

		q.Set(vec[0], vec[1], vec[2], vec[3]);

		return( q );

	}
	else {
  		p2idx = pidx + PacketSize;
	}

	uint32 time = Data[p2idx];

	if (time & W3D_TIMECODED_BINARY_MOVEMENT_FLAG) {
		// its a binary movement!
		float32 *vec = (float32 *) &Data[pidx+1];

		q.Set(vec[0], vec[1], vec[2], vec[3]);

		return( q );
	}

	float32 time1 = (Data[pidx]  & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG);
	float32 time2 = (time & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG);

	float32 ratio = (frame - time1) / (time2 - time1);

	float32 *frame1 = (float32 *) &Data[pidx+1];
	float32 *frame2 = (float32 *) &Data[p2idx+1];

	Quaternion q1(1);
	Quaternion q2(1);

	q1.Set(frame1[0], frame1[1], frame1[2], frame1[3]);
	q2.Set(frame2[0], frame2[1], frame2[2], frame2[3]);

	Fast_Slerp(q, q1, q2, ratio);

	return( q );

}



/***********************************************************************************************
 * TimeCodedMotionChannelClass::binary_search_index / returns packet index 				        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/27/2000 JGA  : Created.                                                                *
 *=============================================================================================*/
// New version that uses a binary search, and no cache
uint32 TimeCodedMotionChannelClass::binary_search_index(uint32 timecode)
{
	int leftIdx = 0;
	int rightIdx = NumTimeCodes - 2;
	int dx;
	uint32 time;


	int idx = LastTimeCodeIdx;  //((rightIdx+1) * PacketSize;)

	//int32		LastTimeCodeIdx;	// absolute index to last time code
	//int32		CachedIdx;			// Last Index Used

	// special case last packet
	time = Data[idx] & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG;
	if (timecode >= time) return(idx);

	for (;;) {

		dx = rightIdx - leftIdx;

		dx>>=1;	// divide by 2

		dx += leftIdx;

		idx = dx * PacketSize;

		time = Data[idx] & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG;

		if (timecode < time) {
			rightIdx = dx;
			continue;
		}

		time = Data[idx + PacketSize] & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG;

		if (timecode < time) return(idx);

		if (leftIdx ^ dx) {
			leftIdx = dx;
			continue;
		}

		//
		// if leftIdx == dx prior to assignment, then leftIdx is stuck.
		//

		leftIdx++;

	}

	assert(0);
	return(0);

}


/***********************************************************************************************
 * TimeCodedMotionChannelClass::get_index / returns packet index												       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/27/2000 JGA  : Created.                                                                *
 *=============================================================================================*/
uint32 TimeCodedMotionChannelClass::get_index(uint32 timecode)
{
	assert(CachedIdx <= LastTimeCodeIdx);

	uint32	time;

	time = Data[CachedIdx] & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG;

	if (timecode >= time) {
		// possibly in the current packet

		// special case for end packets
		if (CachedIdx == LastTimeCodeIdx) return(CachedIdx);
		time = Data[CachedIdx + PacketSize]	& ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG;
		if (timecode < time) return(CachedIdx);

		// Do one time look-ahead before reverting to a search
		CachedIdx+=PacketSize;
		if (CachedIdx == LastTimeCodeIdx) return(CachedIdx);
		time = Data[CachedIdx + PacketSize]	& ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG;
		if (timecode < time) return(CachedIdx);
	}

	CachedIdx = binary_search_index( timecode );

	return(CachedIdx);

}

/***********************************************************************************************
 * TimeCodedMotionChannelClass::set_identity -- returns an "identity" vector (not really...hmm...)      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void TimeCodedMotionChannelClass::set_identity(float * setvec)
{
	if (Type == ANIM_CHANNEL_Q) {

		setvec[0] = 0.0f;
		setvec[1] = 0.0f;
		setvec[2] = 0.0f;
		setvec[3] = 1.0f;

	} else {

		setvec[0] = 0.0f;

	}
}


/***********************************************************************************************
 * TimeCodedBitChannelClass::TimeCodedBitChannelClass -- Constructor for BitChannelClass                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
TimeCodedBitChannelClass::TimeCodedBitChannelClass(void) :
	PivotIdx(0),
	Type(0),
	DefaultVal(0),
	Bits(NULL),
	CachedIdx(0)
{
}


/***********************************************************************************************
 * TimeCodedBitChannelClass::~TimeCodedBitChannelClass -- Destructor for BitChannelClass                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
TimeCodedBitChannelClass::~TimeCodedBitChannelClass(void)
{
	Free();
}


/***********************************************************************************************
 * TimeCodedBitChannelClass::Free -- Free all "external" memory in use                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void TimeCodedBitChannelClass::Free(void)
{
	delete[] Bits;
	Bits = NULL;
}


/***********************************************************************************************
 * TimeCodedBitChannelClass::Load -- Read a bit channel from a w3d chunk                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool TimeCodedBitChannelClass::Load_W3D(ChunkLoadClass & cload)
{
	Free();

	int chunk_size = cload.Cur_Chunk_Length();

	W3dTimeCodedBitChannelStruct chan;
	if (cload.Read(&chan,sizeof(W3dTimeCodedBitChannelStruct)) != sizeof(W3dTimeCodedBitChannelStruct)) {
		return false;
	}

	NumTimeCodes = chan.NumTimeCodes;
	Type 			 = chan.Flags;
	PivotIdx 	 = chan.Pivot;
	DefaultVal = chan.DefaultVal;
	CachedIdx = 0;

	uint32 bytesleft = (NumTimeCodes - 1) * sizeof(uint32);

	assert((sizeof(W3dTimeCodedBitChannelStruct) + bytesleft) == (unsigned)chunk_size);

	Bits = MSGW3DNEWARRAY("TimeCodedBitChannelClass::Bits") uint32[NumTimeCodes];
	assert(Bits);

	Bits[0] = chan.Data[0];

	if (bytesleft > 0) {
		if (cload.Read(&(Bits[1]),bytesleft) != bytesleft) {
			Free();
			return false;
		}
	}

	return true;
}


/***********************************************************************************************
 * TimeCodedBitChannelClass::Get_Bit -- Lookup a bit in the bit channel                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
int TimeCodedBitChannelClass::Get_Bit(int frame)
{
	assert(frame >= 0);
	assert(CachedIdx < NumTimeCodes);

	int time;
	int idx=0;

	time = Bits[CachedIdx] & ~W3D_TIMECODED_BIT_MASK;


	if (frame >= time) {

		// start from here
		idx = CachedIdx+1;

	}

	for (;idx < (int) NumTimeCodes ; idx++)  {

		time = Bits[idx] &~W3D_TIMECODED_BIT_MASK;

		if (frame < time) break;

	}

	idx--;

	if (idx < 0) idx = 0;

	CachedIdx = idx;

	return (((Bits[idx] & W3D_TIMECODED_BIT_MASK) == W3D_TIMECODED_BIT_MASK));

}


// Begin Adaptive Delta


/***********************************************************************************************
 * AdaptiveDeltaMotionChannelClass::AdaptiveDeltaMotionChannelClass -- constructor                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/18/2000 JGA : Created.                                                                 *
 *=============================================================================================*/
AdaptiveDeltaMotionChannelClass::AdaptiveDeltaMotionChannelClass(void) :
	PivotIdx(0),
	Type(0),
	VectorLen(0),
	Data(NULL),
	NumFrames(0),
	CacheData(NULL),
	Scale(0.0f)
{

	if (false == table_valid) {
		// Create Filter Table, used in delta compression

		for (int i=0; i<FILTER_TABLE_GEN_SIZE; i++)
		{
			float ratio = i;

			//ratio = ((ratio + 1.0f) / 128.0f);
			ratio/=((float) FILTER_TABLE_GEN_SIZE);

			filtertable[i + FILTER_TABLE_GEN_START] = 1.0f - WWMath::Sin( DEG_TO_RAD(90.0f * ratio));
		}

		table_valid = true;

	}

}

/***********************************************************************************************
 * MotionChannelClass::~MotionChannelClass -- destructor                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/18/1000 JGA : Created.                                                                 *
 *=============================================================================================*/
AdaptiveDeltaMotionChannelClass::~AdaptiveDeltaMotionChannelClass(void)
{
	Free();
}

/***********************************************************************************************
 * AdaptiveDeltaMotionChannelClass::Free -- de-allocates all memory in use                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/18/2000 JGA  : Created.                                                                 *
 *=============================================================================================*/
void AdaptiveDeltaMotionChannelClass::Free(void)
{
	delete[] Data;
	Data = NULL;

	delete CacheData;
	CacheData = NULL;
}


/***********************************************************************************************
 * AdaptiveDeltaMotionChannelClass::Load -- loads a motion channel from a file                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/18/2000 JGA : Created.                                                                 *
 *=============================================================================================*/
bool AdaptiveDeltaMotionChannelClass::Load_W3D(ChunkLoadClass & cload)
{
	int size = cload.Cur_Chunk_Length();
	unsigned int datasize = size - sizeof(W3dAdaptiveDeltaAnimChannelStruct);
	unsigned int numInts  = (datasize / sizeof(uint32)) + 1;

	W3dAdaptiveDeltaAnimChannelStruct chan;
	if (cload.Read(&chan,sizeof(W3dAdaptiveDeltaAnimChannelStruct)) != sizeof(W3dAdaptiveDeltaAnimChannelStruct)) {
		return false;
	}

	VectorLen   = chan.VectorLen;
	Type 		   = chan.Flags;
	PivotIdx    = chan.Pivot;
	NumFrames	= chan.NumFrames;
	Scale			= chan.Scale;
	CacheFrame	= 0x7FFFFFFF;	// a big number, so we know its not valid
	CacheData   = MSGW3DNEWARRAY("AdaptiveDeltaMotionChannelClass::CacheData") float[VectorLen * 2]; // cacheframe & cachedframe+1 by VectorLen

	Data = MSGW3DNEWARRAY("AdaptiveDeltaMotionChannelClass::Data") uint32[numInts];
	Data[0] = chan.Data[0];

	if (cload.Read(&(Data[1]), datasize) != datasize) {
		Free();
		return false;
	}
	return true;

}


/***********************************************************************************************
 * AdaptiveDeltaMotionChannelClass::decompress																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/23/2000 JGA  : Created.                                                                *
 *=============================================================================================*/
#define PACKET_SIZE (9)
void AdaptiveDeltaMotionChannelClass::decompress(uint32 frame_idx, float *outdata)
{
	// Start Over from the beginning
	float *base	= (float *) &Data[0];	// pointer to our true know beginning values

	bool done = false;

	for(int vi=0; vi<VectorLen; vi++) {
		// Decompress all the vector indices, since they will probably all be needed
		unsigned char *pPacket = (unsigned char *) Data;	// pointer to current packet
		pPacket+= (sizeof(float) * VectorLen);					// skip non-compressed header information
		pPacket+= PACKET_SIZE * vi;								// skip to the appropriate packet start

		float last_value = base[vi];

		for (uint32 frame=1; frame<=frame_idx;) {
			// Frame Loop
			uint32 filter_index = *pPacket;	// packet header

			pPacket++;								// skip to nybble compressed data

			float filter = filtertable[filter_index] * Scale;	// decompression filter

			// data is grouped in sets of 16 nybbles
			for (int fi=0; fi < 16; fi++) {

				int pi = fi>>1;	// create packet index

				int factor;			// factor, contains extracted nybble -8 to +7

				if (fi & 1) {
					factor = pPacket[pi];
					factor>>=4;
				}
				else {
					factor = pPacket[pi];
				}

				// Sign Extend
				factor &=0xF;
				if (factor & 0x8) {
					factor |= 0xFFFFFFF0;
				}

				// Convert to Floating Point

				float ffactor = factor;

				float delta = ffactor * filter;

				last_value+=delta;

				if (frame == frame_idx) {
            	done = true;
               break;
				}
				frame++;

			}

			if (done) break;	// we're at the desired frame

			pPacket+= ((PACKET_SIZE * VectorLen) - 1);	// skip to next packet

		}

      outdata[vi] = last_value;

	}

}

void AdaptiveDeltaMotionChannelClass::decompress(uint32 src_idx, float *srcdata, uint32 frame_idx, float *outdata)
{
	// Contine decompressing from src_idx, up to frame_idx

   assert(src_idx < frame_idx);
   src_idx++;

	float *base	= (float *) &Data[0];	// pointer to our true know beginning values
   base += VectorLen;						// skip header information

	bool done = false;

	for(int vi=0; vi<VectorLen; vi++) {
		// Decompress all the vector indices, since they will probably all be needed
		unsigned char *pPacket = (unsigned char *) base;	// pointer to current packet
		pPacket+= PACKET_SIZE * vi;								// skip to the appropriate packet start
		pPacket+= (PACKET_SIZE * VectorLen) * ((src_idx-1)>>4); // skip out to current packet

      // initial filter index
      int fi = (src_idx-1) & 0xF;

		float last_value = srcdata[vi];

		for (uint32 frame=src_idx; frame<=frame_idx;) {
			// Frame Loop
			uint32 filter_index = *pPacket;	// packet header

			pPacket++;								// skip to nybble compressed data

			float filter = filtertable[filter_index] * Scale;	// decompression filter

			// data is grouped in sets of 16 nybbles
			for (fi; fi < 16; fi++) {

				int pi = fi>>1;	// create packet index

				int factor;			// factor, contains extracted nybble -8 to +7

				if (fi & 1) {
					factor = pPacket[pi];
					factor>>=4;
				}
				else {
					factor = pPacket[pi];
				}

				// Sign Extend
				factor &=0xF;
				if (factor & 0x8) {
					factor |= 0xFFFFFFF0;
				}

				// Convert to Floating Point

				float ffactor = factor;

				float delta = ffactor * filter;

				last_value+=delta;

				if (frame == frame_idx) {
            	done = true;
               break;
				}
				frame++;

			}
         fi = 0;

			if (done) break;	// we're at the desired frame

			pPacket+= ((PACKET_SIZE * VectorLen) - 1);	// skip to next packet

		}

      outdata[vi] = last_value;

	}

}


/***********************************************************************************************
 * AdaptiveDeltaMotionChannelClass::getframe returns decompressed data for frame/vectorindex   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/18/2000 JGA  : Created.                                                                 *
 *=============================================================================================*/
float AdaptiveDeltaMotionChannelClass::getframe(uint32 frame_idx, uint32 vector_idx)
{
	// Make sure frame_idx is valid

	if (frame_idx >= NumFrames) frame_idx = NumFrames - 1;

	// Check to see if the data is already in cache?

	if (CacheFrame == frame_idx) {
		return(CacheData[vector_idx]);
	}

	if ((CacheFrame+1) == frame_idx) {
		return(CacheData[vector_idx + VectorLen]);
	}

	if (frame_idx < CacheFrame)  {
		// Requested Frame isn't cached, so cache it, and frame_idx+1, and return the decompressed data
      // from frame_idx

      decompress(frame_idx, &CacheData[0]);

      if (frame_idx != (NumFrames - 1))  {
      	decompress(frame_idx, &CacheData[0], frame_idx+1, &CacheData[VectorLen]);
      }

      CacheFrame = frame_idx;

      return(CacheData[vector_idx]);
	}

   // Copy last known Cached data down

   if (frame_idx == (CacheFrame + 2))  {

      // Sliding window
   	memcpy(&CacheData[0], &CacheData[VectorLen], VectorLen * sizeof(float));

      CacheFrame++;

      decompress(CacheFrame, &CacheData[0], frame_idx, &CacheData[VectorLen]);

    	return(CacheData[VectorLen + vector_idx]);
   }

   // Else just use last known frame to decompress forwards

   assert(VectorLen <= 4);

   float temp[4];

   memcpy(&temp[0], &CacheData[VectorLen], VectorLen * sizeof(float));

   decompress(CacheFrame, &temp[0], frame_idx, &CacheData[0]);
   CacheFrame = frame_idx;

   if (frame_idx != (NumFrames - 1))  {
   	decompress(CacheFrame, &CacheData[0], frame_idx+1, &CacheData[VectorLen]);
   }

   return(CacheData[vector_idx]);

}

/***********************************************************************************************
 * AdaptiveDeltaMotionChannelClass::Get_Vector -- returns the vector for the specified frame # *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/18/2000 JGA  : Created.                                                                 *
 *=============================================================================================*/
void	AdaptiveDeltaMotionChannelClass::Get_Vector(float32 frame,float * setvec)
{

	uint32 frame1 = frame;

	float ratio = frame - frame1;

	float value1 = getframe(frame1);
	float value2 = getframe(frame1 + 1);

   *setvec = WWMath::Lerp(value1,value2,ratio);


}


//
//  Special Case Quats, so we can use Slerp
//
Quaternion AdaptiveDeltaMotionChannelClass::Get_QuatVector(float32 frame)
{

	uint32 frame1 = frame;
	uint32 frame2 = frame1+1;
	float ratio = frame - frame1;

	Quaternion q1(1);
	q1.Set( getframe(frame1, 0),
			  getframe(frame1, 1),
			  getframe(frame1, 2),
			  getframe(frame1, 3) );

	Quaternion q2(1);

	q2.Set( getframe(frame2, 0),
			  getframe(frame2, 1),
			  getframe(frame2, 2),
			  getframe(frame2, 3) );


	Quaternion q(1);

	Fast_Slerp(q, q1, q2, ratio);

	return( q );

}


// EOF - motchan.cpp
