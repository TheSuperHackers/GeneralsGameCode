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
 *                 Project Name : W3D Tools                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/bchannel.cpp                   $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/30/00 5:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "bchannel.h"
#include "w3d_file.h"
#include "logdlg.h"
#include "exportlog.h"


BitChannelClass::BitChannelClass
(
	uint32 id,
	int maxframes,
	uint32 chntype,
	bool default_val
) :
	ID(id),
	ChannelType(chntype),
	MaxFrames(maxframes),
	IsEmpty(true),
	DefaultVal(default_val),
	Data(maxframes),
	Begin(0),
	End(0)
{
	// start "Begin" at the end of the array, whenever we set a value
	// at an index less than "Begin", we push "Begin" back.
	Begin = MaxFrames;
	End = 0;
}

BitChannelClass::~BitChannelClass(void)
{
}

void BitChannelClass::Set_Bit(int frameidx,bool bit)
{
	assert(frameidx >= 0);
	assert(frameidx < MaxFrames);

	Data[frameidx] = bit;

	if (!is_default(bit)) {
		IsEmpty = false;
	}
}

void BitChannelClass::Set_Bits(BooleanVectorClass & bits)
{
	for (int i=0; i<bits.Length(); i++) {
		Set_Bit(i,bits[i]);
	}
}


bool BitChannelClass::Get_Bit(int frameidx)
{
	assert(frameidx >= 0);
	assert(frameidx < MaxFrames);

	return Data[frameidx];
}


bool BitChannelClass::Save(ChunkSaveClass & csave, bool compress)
{
	if (IsEmpty) return true;

  if (compress)  {
  	// Save the Channel Data Compressed
    // TIMECODED
		if (!csave.Begin_Chunk(W3D_CHUNK_COMPRESSED_BIT_CHANNEL)) {
			return false;
		}

    uint32	channelsize  = sizeof(W3dTimeCodedBitChannelStruct);
		uint32  packetsize   = sizeof(uint32);

    channelsize	+= packetsize * MaxFrames;
    channelsize -= sizeof(uint32);

    W3dTimeCodedBitChannelStruct * chn = (W3dTimeCodedBitChannelStruct *)malloc(channelsize);

    if (chn == NULL)  {
    	return false;
    }

    chn->NumTimeCodes = MaxFrames;
    chn->Pivot        = ID;
    chn->Flags        = ChannelType;
    chn->DefaultVal   = DefaultVal;

    // copy data into the channel struct, in timecoded raw format

		for (uint32 fcount=0; fcount < chn->NumTimeCodes; fcount++) {

			if (Get_Bit(fcount)) {
      		chn->Data[fcount] = fcount | W3D_TIMECODED_BIT_MASK;
			}
			else  {
      		chn->Data[fcount] = fcount;
			}
		}

    // Compress the new structure

    BitChannelClass::compress( chn );

	 float originalchannelsize = channelsize;

    // Update Channel Size
    channelsize  = sizeof(W3dTimeCodedBitChannelStruct);
    channelsize	+= packetsize * chn->NumTimeCodes;
    channelsize -= sizeof(uint32);

	 float percent = (((float) channelsize) / originalchannelsize) * 100.0f;

	 ExportLog::printf("%.0f", percent);


    // save

		if (csave.Write(chn,channelsize) != channelsize) {
			return false;
		}

		if (chn != NULL) {
			free(chn);
		}

		if (!csave.End_Chunk()) {
			return false;
		}

  }
  else  {
  	// Stock Raw Save
		if (!csave.Begin_Chunk(W3D_CHUNK_BIT_CHANNEL)) {
			return false;
		}

		compute_range();

		int numbits = End - Begin + 1;
		assert(numbits > 0);
		int numbytes = (numbits + 7) / 8;

		unsigned int channelsize = sizeof(W3dBitChannelStruct);
		channelsize += numbytes - 1; // one byte inside the W3dBitChannelStruct...

		W3dBitChannelStruct * chn = (W3dBitChannelStruct *)malloc(channelsize);

		if (chn == NULL) {
			return false;
		}

		chn->FirstFrame = Begin;
		chn->LastFrame = End;
		chn->Flags = ChannelType;
		chn->Pivot = ID;
		chn->DefaultVal = DefaultVal;

		uint8 * bits = (uint8 *)&(chn->Data[0]);

		for (int fcount=0; fcount < End-Begin+1; fcount++) {
			::Set_Bit(bits,fcount,Get_Bit(Begin + fcount));
		}

		if (csave.Write(chn,channelsize) != channelsize) {
			return false;
		}

		if (chn != NULL) {
			free(chn);
		}

		if (!csave.End_Chunk()) {
			return false;
		}

  }

	return true;
}


bool BitChannelClass::is_default(bool bit)
{
	return (bit == DefaultVal);
}

void BitChannelClass::compute_range(void)
{
	Begin = 0;
	while ((Begin < MaxFrames) && (is_default(Get_Bit(Begin)))) {
		Begin++;
	}

	End = MaxFrames-1;
	while ((End >= 0) && (is_default(Get_Bit(End)))) {
		End--;
	}
}	 // compute_range


//
// find a packet that isn't needed, and return the index
// if all packets are necessary, then return back PACKETS_ALL_USEFUL
// a useless packet is defined, as a packet that can be recreated
//
#define PACKETS_ALL_USEFUL (0xFFFFFFFF)
//
uint32 BitChannelClass::find_useless_packet(W3dTimeCodedBitChannelStruct * c)
{

  assert( c );	// make sure pointer exists
  assert( c->NumTimeCodes );	// make sure some packets exist

  	if (c->NumTimeCodes > 2)  {

      for(uint32 try_idx = 0; try_idx < (c->NumTimeCodes - 1); try_idx++)  {

        if ((c->Data[try_idx]   & W3D_TIMECODED_BIT_MASK) ==
            (c->Data[try_idx+1] & W3D_TIMECODED_BIT_MASK))  {
            	return(try_idx + 1);
            }

      }	// for
    }

	return( PACKETS_ALL_USEFUL );

} // find_useless_packet


//
//  Remove a packet from a W3dTimeCodedBitChannelStruct
//
void BitChannelClass::remove_packet(W3dTimeCodedBitChannelStruct * c, uint32 packet_idx)
{
	assert( c );
  assert( c->NumTimeCodes > 1 );

  uint32 packet_size = 1;
  uint32 packet_len  = packet_size * sizeof(uint32);

  uint32 *src, *dst;

  dst = (uint32 *) &c->Data[ packet_size * packet_idx ];
  src = (uint32 *) &c->Data[ packet_size * (packet_idx + 1) ];

  uint32 copy_length = (c->NumTimeCodes - (packet_idx + 1)) * packet_len;

  if (copy_length)  {

    memcpy(dst, src, copy_length);

  }

  // Decrement Packet Count
  c->NumTimeCodes--;

} // remove_packet

//
//  Take a non-compressed TimeCoded Bit Channel
//  and compress the packets
//
void BitChannelClass::compress(W3dTimeCodedBitChannelStruct * c)
{
	while(1) {

		uint32 idx = find_useless_packet( c );

		if (PACKETS_ALL_USEFUL == idx) break;

		remove_packet( c, idx );

	}

} // compress




// EOF - bchannel.cpp
