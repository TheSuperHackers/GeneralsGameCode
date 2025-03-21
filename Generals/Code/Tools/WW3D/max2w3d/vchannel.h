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

/* $Header: /Commando/Code/Tools/max2w3d/vchannel.h 8     10/30/00 6:56p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - W3D export                                  * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/vchannel.h                     $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/30/00 5:25p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 8                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef VCHANNEL_H
#define VCHANNEL_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#ifndef BITTYPE_H
#include "BITTYPE.H"
#endif

#ifndef CHUNKIO_H
#include "chunkio.h"
#endif

#ifndef W3D_FILE_H
#include "w3d_file.h"
#endif


class BitChannelClass;

/*

	This class is a container for an array of vectors.  It will keep
	track of whether the entire array of vectors is zero, and if not
	where the non-zero vectors begin and end.

	VectorChannelClass is used in exporting motion.  Motion data
	is broken into separate channels for X, Y, Z, and orientation.
	Then if any of the channels are empty, they don't have to be stored.
	The X,Y,Z channels all contain one-dimensional vectors and the
	orientation channel contains four-dimensional vectors.

*/
// for compression
#define DEFAULT_LOSSY_ERROR_TOLERANCE (0.0001)
#define PACKETS_ALL_USEFUL (0xFFFFFFFF)

class VectorChannelClass
{
public:

	VectorChannelClass(uint32 id,int maxframes,uint32 flags,int vectorlength,float32 * identvec);
	~VectorChannelClass(void);

	void		Set_Vector(int framenumber,float32 * vector);
	float *	Get_Vector(int frameidx);
	bool		Is_Empty(void) { return IsEmpty; }
	void		SetSaveOptions(bool compress, int flavor, float Terr, float Rerr, bool reduce, int reduce_percent);
	bool		Save(ChunkSaveClass & csave, BitChannelClass *binmov);
	void		ClearInvisibleData(BitChannelClass *vis);

private:

	uint32		ID;
	uint32		Flags;
	int	  		MaxFrames;
	int	  		VectorLen;
	bool			IsEmpty;

	float32 *	IdentVect;
	float32 *	Data;
	int	  		Begin;
	int	  		End;

	// Save Options

	bool			ReduceAnimation;
	int			ReduceAnimationPercent;
	bool			CompressAnimation;
	int			CompressAnimationFlavor;
	float			CompressAnimationTranslationError;
	float			CompressAnimationRotationError;

	// Write a single value
	void set_value(int framenum,int vindex,float32 val);

	// Read a single value
	float32 get_value(int framenum,int vindex);

	// Test a vector against the "identity" vector
	bool is_identity(float32 * vec);

	// This function finds the start and end of the "non-identity" data
	void compute_range(void);

	// compress functions
	void		compress(W3dTimeCodedAnimChannelStruct * c);
   float		compress(int filter_index, float scale, float value1, float *indata, unsigned char *pPacket, float *outdata);
   float		test_compress(int filter_index, float scale, float value1, float *indata, float *outdata);
	uint32	find_useless_packet(W3dTimeCodedAnimChannelStruct * c, double tolerance);
	uint32	find_useless_packetQ(W3dTimeCodedAnimChannelStruct * c, double tolerance);
	uint32	find_least_useful_packet(W3dTimeCodedAnimChannelStruct *c);
	uint32	find_least_useful_packetQ(W3dTimeCodedAnimChannelStruct *c);
	void		remove_packet(W3dTimeCodedAnimChannelStruct * c, uint32 packet_idx);
	bool		SaveTimeCoded(ChunkSaveClass & csave, BitChannelClass *binmov);
	bool		SaveAdaptiveDelta(ChunkSaveClass & csave, BitChannelClass *binmov);
  
};

#endif /*VCHANNEL_H*/