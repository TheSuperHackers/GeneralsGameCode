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

/* $Header: /Commando/Code/Tools/max2w3d/vchannel.cpp 10    10/30/00 6:56p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - W3D export                                  * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/vchannel.cpp                   $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/30/00 5:26p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 10                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "vchannel.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "w3d_file.h"
#include "w3dquat.h"
#include "bchannel.h"
#include "exportlog.h"


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


VectorChannelClass::VectorChannelClass
(
	uint32 id,
	int maxframes,
	uint32 flags,
	int vectorlength,
	float32 * identvect
) :
	ID(id),
	Flags(flags),
	MaxFrames(maxframes),
	VectorLen(vectorlength),
	IsEmpty(true),
	IdentVect(NULL),
	Data(NULL),
	Begin(0),
	End(0),
	ReduceAnimation(false),
	ReduceAnimationPercent(0),
	CompressAnimation(false),
	CompressAnimationFlavor(0),
	CompressAnimationTranslationError(0.0f),
	CompressAnimationRotationError(0.0f)
{

	assert(VectorLen > 0);
	IdentVect = new float32[VectorLen];
	Data = new float32[MaxFrames * VectorLen];
	assert(Data);
	assert(IdentVect);

	memcpy(IdentVect,identvect,VectorLen * sizeof(float32));
	memset(Data,0,MaxFrames * VectorLen * sizeof(float32));

	// start "Begin" at the end of the array, whenever we set a value
	// at an index less than "Begin", we push "Begin" back.
	Begin = MaxFrames;
	End = 0;	
	
	if (false == table_valid) {
		// Create Filter Table, used in delta compression

		for (int i=0; i<FILTER_TABLE_GEN_SIZE; i++)
		{
			float ratio = i;

			//ratio = ((ratio + 1.0f) / 128.0f); 
			ratio/=((float) FILTER_TABLE_GEN_SIZE);

			filtertable[i + FILTER_TABLE_GEN_START] = 1.0f - sin( DEG_TO_RAD(90.0f * ratio));
		}

		table_valid = true;

	}

  																											
}

VectorChannelClass::~VectorChannelClass(void)
{
	if (Data) {
		delete[] Data;
	}
	if (IdentVect) {
		delete[] IdentVect;
	}							 
}

void VectorChannelClass::Set_Vector(int frameidx,float32 * vector)
{
	assert(frameidx >= 0);
	assert(frameidx < MaxFrames);

	for (int vi=0; vi<VectorLen; vi++) {
		set_value(frameidx,vi,vector[vi]);
	}

	if (!is_identity(vector)) {
		IsEmpty = false;
	}
}

float * VectorChannelClass::Get_Vector(int frameidx)
{
	assert(frameidx >= 0);
	assert(frameidx < MaxFrames);

	return &(Data[frameidx * VectorLen]);
}
	

bool VectorChannelClass::SaveTimeCoded(ChunkSaveClass & csave, BitChannelClass *binmov)
{
	uint32 channelsize  = sizeof(W3dTimeCodedAnimChannelStruct);
	uint32 packetsize   = (VectorLen * sizeof(float32)) + sizeof(uint32);
	channelsize			 += packetsize * MaxFrames;
	channelsize			 -= sizeof(uint32);

	W3dTimeCodedAnimChannelStruct * chn = (W3dTimeCodedAnimChannelStruct *)malloc(channelsize);
	
	if (chn == NULL) {
		return false;
	}
  
	chn->NumTimeCodes = MaxFrames;
	chn->VectorLen = VectorLen;
	chn->Pivot = ID;
	chn->Flags = Flags;
	
	// Fetch Channel Data into new format
	// tc [data] tc [data] tc [data] .. ...
	uint32 fidx = 0;

	for (int fcount=0; fcount < MaxFrames; fcount++, fidx += (VectorLen+1) ) {
		
		uint32	* pivec;
		float32	* pfvec;

		pivec = 	&chn->Data[ fidx ];
		pfvec = 	(float32 *) (pivec + 1);
		
		*pivec = fcount;
								
		if (binmov) {
			
			bool binary_move = binmov->Get_Bit( fcount );

			// check for false binary movement
			if (fcount != Begin) {
				if (binary_move)  {
   				*pivec |= W3D_TIMECODED_BINARY_MOVEMENT_FLAG;
				}																		  
			}
			else {
				//if (log) log->printf("\nFALSE Binary\n");
			}
		} 
   
		
		// Copy Vector
		for (int vidx=0; vidx < VectorLen; vidx++) {

			pfvec[vidx] = get_value(fcount,vidx);
		}
	}

	// Compress the new structure

	VectorChannelClass::compress( chn );

	// update the size

	float original_channelsize = channelsize;

	channelsize  = sizeof(W3dTimeCodedAnimChannelStruct);
	channelsize += packetsize * chn->NumTimeCodes;
	channelsize -= sizeof(uint32);

	float percent = (((float)channelsize) / original_channelsize) * 100.0f;
	// save

	ExportLog::printf("%.0f", percent);

	if (csave.Write(chn,channelsize) != channelsize) {
		return false;
	}
	
	if (chn != NULL) {
		free(chn);
	}

	if (!csave.End_Chunk()) {
		return false;
	}
	
	return true;

} // SaveTimeCoded


/*
struct W3dAdaptiveDeltaAnimChannelStruct
{
	uint32		NumFrames;			// number of frames of animation
	uint16		Pivot;				// pivot effected by this channel
	uint8			VectorLen;			// num Channels
	uint8			Flags;				// channel type
	float			Scale;				// Filter Table Scale

	uint32		Data[1];				// OpCode Data Stream

};
*/

struct 
{
	unsigned	char	filter : 7;
	unsigned char	flag   : 1;
	unsigned char	d0		 : 4;
	unsigned char  d1     : 4;
	unsigned char	d2		 : 4;
	unsigned char  d3     : 4;
	unsigned char	d4		 : 4;
	unsigned char  d5     : 4;
	unsigned char	d6		 : 4;
	unsigned char  d7     : 4;
	unsigned char	d8		 : 4;
	unsigned char  d9     : 4;
	unsigned char	d10	 : 4;
	unsigned char  d11    : 4;
	unsigned char	d12	 : 4;
	unsigned char  d13    : 4;
	unsigned char	d14	 : 4;
	unsigned char  d15    : 4;

} AdaptiveDeltaPacketStruct;
							 	  
//
//	test compress Adaptive Delta packet
//
// inputs:  filter - forced filter to use
//				scale (filter table scale)
//				value1 (continue compression from this initial value)
//				float *indata  // 16 values to compress
//				float *outdata	// 16 decompressed values
//
// output: float error;		// aggregate error for packet
//                          
float VectorChannelClass::test_compress(int filter_index, float scale, float value1, float *indata, float *outdata)
{		
	
   float error = 0.0f;						
                     
   // compute filter
   
   float filter = filtertable[filter_index] * scale;
   			  
   assert(filter_index < FILTER_TABLE_SIZE);
                          
   float last_value = value1;	
      
   for(int data_idx=0; data_idx < 16; data_idx++)  {
   		
      // NOTE: DETERMINE best factor via Brute Force
      // This helps under/over-flow problems
      
      // brute
      int   best_factor = 100;
      float	least_error = 999999999.9f;
      for (float try_factor = -8.0f; try_factor < 8.0f; try_factor+=1.0f)  {
      	float temp = (try_factor * filter) + last_value; // decompress using this filter
         temp-=indata[data_idx];									 // delta decompressed value, vs original value
         temp=fabs(temp);
         if (temp < least_error)  {
         	least_error = temp;
            best_factor = try_factor;
         }
      }
      assert(best_factor <= 7);
      assert(best_factor >=-8);
      																	 
      float dfactor = best_factor;
                                                          
      outdata[data_idx] = (dfactor * filter) + last_value;
      // END BRUTE FORCE IT
      
		float delta = outdata[data_idx] - indata[data_idx];

		//if (delta > error) error = delta;
      error+= (delta * delta); 

      last_value = outdata[data_idx];
   }		
   
   return( sqrt(error) );
	
} // test_compress	
          
//
//	compress Adaptive Delta packet
//
// inputs:  filter - forced filter to use
//				scale (filter table scale)
//				value1 (continue compression from this initial value)
//				float *indata  // 16 values to compress
//				unsigned char *pPacket
//
// output: float error;		// aggregate error for packet
//                          
float VectorChannelClass::compress(int filter_index, float scale, float value1, float *indata, unsigned char *pPacket, float *outdata)
{			
   float error = 0.0f;						
                     
   // compute filter
   
   float filter = filtertable[filter_index] * scale;
   			  
   assert(filter_index < FILTER_TABLE_SIZE);
                          
   *pPacket = filter_index;
   pPacket++;
   
   float last_value = value1;	
      
   for(int data_idx=0; data_idx < 16; data_idx++)  {
   		
      // NOTE: PLAN TO ADD A LOOP IN HERE, to DETERMINE best factor via Brute Force
      // This could help under/over-flow problems
      {
      	// brute
         int   best_factor = 100;
         float	least_error = 999999999.9f;
       	for (float try_factor = -8.0f; try_factor < 8.0f; try_factor+=1.0f)  {
       		float temp = (try_factor * filter) + last_value; // decompress using this filter
            temp-=indata[data_idx];									 // delta decompressed value, vs original value
            temp=fabs(temp);
            if (temp < least_error)  {
            	least_error = temp;
               best_factor = try_factor;
            }
       	}
         assert(best_factor <= 7);
         assert(best_factor >=-8);
         																	 
         float dfactor = best_factor;
           
         outdata[data_idx] = (dfactor * filter) + last_value;

         int pi = data_idx>>1;  
         
			if (data_idx & 1) {
				best_factor<<=4;
				pPacket[pi]&=0x0f;
				pPacket[pi]|=best_factor;
			}
			else {
			  	best_factor&=0xf;
				pPacket[pi]&=0xf0;
				pPacket[pi]|=best_factor;
			}
      }
             
      // END BRUTE FORCE IT
      
      error+=fabs(outdata[data_idx] - indata[data_idx]); 
      
      last_value = outdata[data_idx];
      	
   }		
   
   return( error );
	
} // compress	
                      

bool VectorChannelClass::SaveAdaptiveDelta(ChunkSaveClass & csave, BitChannelClass *binmov)
{
	uint32 channelsize  = sizeof(W3dAdaptiveDeltaAnimChannelStruct);
	int packetsize   = sizeof(AdaptiveDeltaPacketStruct);
	int numpackets   = (MaxFrames + 15) / 16;
	channelsize		 += packetsize * numpackets * VectorLen;
	channelsize		 -= sizeof(char);
	channelsize     += VectorLen * sizeof(float);

	W3dAdaptiveDeltaAnimChannelStruct * chn = (W3dAdaptiveDeltaAnimChannelStruct *)malloc(channelsize);
	
	if (chn == NULL) {
		return false;
	}

// Brute Force Compressor

	// Initialize Chan Data
	memset(chn, 0, channelsize);
	chn->NumFrames = MaxFrames;
	chn->Pivot = ID;
	chn->VectorLen = VectorLen;
	chn->Flags = Flags;
	chn->Scale = 0.0f;
	
	assert(VectorLen <= 4);	// otherwise temp vector won't have room

	float	*initial = (float *)&chn->Data[0];
	float work[4];

	// Fetch initial value
	int i=0;
	for (; i < VectorLen; i++) {
		work[i] = initial[i] = get_value(0, i);
	}

	float  original_packet[16];
	float  decompressed_packet[16];
	float  scale = 0.0f;
	float  last_value=0.0f;
	int	 frame = 1;
	float delta;

	// Compute Scale

	for (int fidx=1; fidx < MaxFrames; fidx++) {
		for (i=0; i<VectorLen; i++) {
			
			delta = fabs( get_value(fidx, i) - get_value(fidx - 1, i));

			if (delta > scale) scale = delta;
		}
	}

	scale/=7.0f;	// range of the delta encode is +- 7 units, in which ever filter range
						// the smaller the scale, the better our precision is going to be
						
	// End Compute Scale

	for (i=0; i < numpackets; i++) {
		for (int vi=0; vi<VectorLen; vi++) {
			last_value = work[vi];
			// Copy Original Data into the original packet
			int temp_frame = frame;

			for(int gi=0; gi<16; gi++) {
				
				if (temp_frame < MaxFrames) {
					last_value = original_packet[gi] = get_value(temp_frame, vi);
				}
				else {
					original_packet[gi] = last_value;
				}

				temp_frame++;

			} // for gi
			
         // Brute Force Filter	
         
         int   best_filter = 2 * FILTER_TABLE_SIZE;
         float least_error = 999999999.9f;  		  
         last_value = work[vi];
            
         for(int try_filter=0; try_filter < FILTER_TABLE_SIZE; try_filter++) {
         
         	float temp_error = test_compress(try_filter, scale, last_value, original_packet, decompressed_packet);
                      			 
            if (temp_error < least_error)  {
            	best_filter = try_filter;
               least_error = temp_error;
            }                   
                              
         }	 
         
         assert(best_filter < FILTER_TABLE_SIZE);
			
         //log->printf("\nvi= %d frames %d to %d : error = %f ",vi, frame, frame+15, least_error);		
               
			// Encode current packet

			unsigned char * pPacket;

			pPacket = (unsigned char *) &chn->Data[0];			// beginning of data struct
			pPacket+= (VectorLen * sizeof(float));					// skip over initial values
			pPacket+= (sizeof(AdaptiveDeltaPacketStruct) * VectorLen * ((frame-1)>>4));  // skip up to the appropriate packet
			pPacket+=  sizeof(AdaptiveDeltaPacketStruct) * vi;	// skip up the appropriate vector index
					  
         compress(best_filter, scale, last_value, original_packet, pPacket, decompressed_packet);

			// update work[vi];
			work[vi] = decompressed_packet[15];

		}	// for vi
		frame+=16;
	}  // for numpackets


	// print how big we are vs non-compressed
	float rawsize = sizeof(W3dAnimChannelStruct);
	rawsize += (VectorLen * sizeof(float32) * (MaxFrames)) - sizeof(float32);

	float percent = ((float)channelsize) / rawsize;
	percent*=100.0f;

	ExportLog::printf("%.0f", percent);


	// update final scale
	chn->Scale = scale;

	if (csave.Write(chn,channelsize) != channelsize) {
		return false;
	}
	
	if (chn != NULL) {
		free(chn);
	}

	if (!csave.End_Chunk()) {
		return false;
	}
	
	return true;

} // SaveAdaptiveDelta

			
bool VectorChannelClass::Save(ChunkSaveClass & csave, BitChannelClass *binmov)
{
	if (IsEmpty) return true;
 	
	compute_range();
	if (End<Begin) {
		IsEmpty = true;
		return true;
	}

	if (CompressAnimation) {
		// Save the Channel Data compressed
		// TIMECODED

		if (!csave.Begin_Chunk(W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL)) {
			return false;
		}

		switch (CompressAnimationFlavor) 
		{
			case ANIM_FLAVOR_TIMECODED:
				{
					return(SaveTimeCoded(csave, binmov));

					break;
				}

			case ANIM_FLAVOR_ADAPTIVE_DELTA:
				{
					return(SaveAdaptiveDelta(csave, binmov));

					break;
				}

			default:
				assert(0);		// unknown compressed format
				return false;
		}

	}
	else {
		// Classic Non Compressed Data

		if (!csave.Begin_Chunk(W3D_CHUNK_ANIMATION_CHANNEL)) {
			return false;
		}


		unsigned int channelsize = sizeof(W3dAnimChannelStruct);
		channelsize += VectorLen * sizeof(float32) * (MaxFrames) - sizeof(float32);

		W3dAnimChannelStruct * chn = (W3dAnimChannelStruct *)malloc(channelsize);
		
		if (chn == NULL) {
			return false;
		}
  
		chn->FirstFrame = Begin;
		chn->LastFrame = End;
		chn->VectorLen = VectorLen;
		chn->Pivot = ID;
		chn->Flags = Flags;
		
		for (int fcount=0; fcount < End-Begin+1; fcount++) {
			for (int vidx=0; vidx < VectorLen; vidx++) {

				int writeidx = fcount * VectorLen + vidx;

				chn->Data[writeidx] = get_value(Begin + fcount,vidx);
			}
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

void VectorChannelClass::SetSaveOptions(bool compress, int flavor, float Terr, float Rerr, bool reduce, int reduce_percent)
{

	ReduceAnimation = reduce;
	ReduceAnimationPercent = reduce_percent;
	CompressAnimation = compress;
	CompressAnimationFlavor = flavor;
	CompressAnimationTranslationError = Terr;
	CompressAnimationRotationError = DEG_TO_RAD(Rerr);

} // SetSaveOptions

//
//  Set data in motion channel to identity vector
//  R2 - set invisible data to repeat last known position, the previous algorthm caused problems with 
//  the current movie assets
//
void VectorChannelClass::ClearInvisibleData(BitChannelClass *vis)
{
	float *tvec;

	assert(VectorLen <= 8);

	bool prev_state = vis->Get_Bit( 0 );

	tvec = Get_Vector( 0 );

	for (int idx=0; idx < MaxFrames; idx++) {

		bool cur_state = vis->Get_Bit( idx );

		if (cur_state != prev_state) {
			prev_state = cur_state;
			tvec = Get_Vector( idx );
		}

		if (false == cur_state) {
			//Set_Vector( idx, IdentVect );
			Set_Vector( idx, tvec );
		}
	}
} // ClearInvisibleData


void VectorChannelClass::set_value(int framenum,int vindex,float32 val)
{
	assert(framenum >= 0);
	assert(framenum < MaxFrames);
	assert(vindex >= 0);
	assert(vindex < VectorLen);

	Data[framenum * VectorLen + vindex] = val;
}

float32 VectorChannelClass::get_value(int framenum,int vindex)
{
	assert(framenum >= 0);
	assert(framenum < MaxFrames);
	assert(vindex >= 0);
	assert(vindex < VectorLen);

	return Data[framenum * VectorLen + vindex];
}

bool VectorChannelClass::is_identity(float32 * vec)
{
	const double ERROR_TOLERANCE = 0.00005 * 0.00005;
	double dist = 0.0;

	for (int vi=0; vi<VectorLen; vi++) {
 		dist += (vec[vi] - IdentVect[vi])*(vec[vi] - IdentVect[vi]);
	}

	// if distance from identity is very small, it is identity...
	if (dist < ERROR_TOLERANCE) {
		return true;
	} else {
		return false;
	}
}

void VectorChannelClass::compute_range(void)
{
	Begin = 0;
	while ((Begin < MaxFrames) && (is_identity(Get_Vector(Begin)))) {
		Begin++;
	}

	End = MaxFrames-1;
	while ((End >= 0) && (is_identity(Get_Vector(End)))) {
		End--;
	}
}	// compute_range


//
//  Remove a packet from a W3dTimeCodedAnimChanelStruct
//
void VectorChannelClass::remove_packet(W3dTimeCodedAnimChannelStruct * c, uint32 packet_idx)
{												
	assert( c );
	assert( c->NumTimeCodes > 1 );
	
	uint32 packet_size = c->VectorLen + 1;
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
//  Take a non-compressed TimeCoded Motion Channel
//  and compress the packets
//
void VectorChannelClass::compress(W3dTimeCodedAnimChannelStruct * c)
{	
	
	assert( c );


	// Standard Error Threshold Compression

	double Terr = CompressAnimationTranslationError;
	double Rerr = CompressAnimationRotationError;

	float TimeCodes_ct = c->NumTimeCodes;

	switch( c->Flags )
	{
		case ANIM_CHANNEL_X:
		case ANIM_CHANNEL_Y:
		case ANIM_CHANNEL_Z: {
			
			while(1) {
				
				uint32 idx = find_useless_packet( c, Terr );
     
				if (PACKETS_ALL_USEFUL == idx) break;
    
				remove_packet( c, idx );
    			
			}	

			break;
		}
									
		case ANIM_CHANNEL_XR:
		case ANIM_CHANNEL_YR:
		case ANIM_CHANNEL_ZR: {

			while(1) {
				
				uint32 idx = find_useless_packet( c, Rerr );
     
				if (PACKETS_ALL_USEFUL == idx) break;
    
				remove_packet( c, idx );
    			
			}	

			break;
		}

		case ANIM_CHANNEL_Q: {
			
			while(1) {
				
				uint32 idx = find_useless_packetQ( c, Rerr );
     
				if (PACKETS_ALL_USEFUL == idx) break;
    
				remove_packet( c, idx );
    			
			}	

			break;
		}

		default:				// undefined channel
			assert(0);
			break;
	}

	// Forced Reduction Phase

	if (ReduceAnimation)	 {
		if (ReduceAnimationPercent) {

			float pct = ReduceAnimationPercent;

			pct *= (0.01f);

			pct = 1.0f - pct;

			// if out of range, don't even try
			if (pct <= 0.0f) return;
			if (pct >= 1.0f) return;

			pct*=TimeCodes_ct;

			pct+=0.5f;

			uint32 maxFrames = pct;

			if (maxFrames < 2) maxFrames = 2;

			if (maxFrames >= c->NumTimeCodes) return;	// desired minimum already attained


			switch( c->Flags )
			{
				case ANIM_CHANNEL_X:
				case ANIM_CHANNEL_Y:
				case ANIM_CHANNEL_Z: 
				case ANIM_CHANNEL_XR:
				case ANIM_CHANNEL_YR:
				case ANIM_CHANNEL_ZR: { 

					while(maxFrames < c->NumTimeCodes) {
						
						uint32 idx = find_least_useful_packet( c );
						if (PACKETS_ALL_USEFUL == idx) break;
						remove_packet( c, idx );
					}	

					break;
				}

				case ANIM_CHANNEL_Q: {
					
					while(maxFrames < c->NumTimeCodes) {
						
						uint32 idx = find_least_useful_packetQ( c );
						if (PACKETS_ALL_USEFUL == idx) break;
						remove_packet( c, idx );
					}	

					break;
				}

				default:				// undefined channel
					assert(0);
					break;
			}
		} // if ReducePercent

	} // if Reduce

} // compress



//
// find a packet that isn't needed, and return the index
// if all packets are necessary, then return back PACKETS_ALL_USEFUL
// a useless packet is defined, as a packet that can be recreated
// via interpolation
// 
// Make Sure we NEVER get rid of binary movement packets
// The rule is, you can't interpolate TOO a binary movement, but you can
// interpolate FROM
//#define W3D_TIMECODED_BINARY_MOVEMENT_FLAG  0x80000000
//      
uint32 VectorChannelClass::find_useless_packet(W3dTimeCodedAnimChannelStruct * c, double tolerance)
{												
#define MAX_VECTOR_SIZE 8
static  float32 tempvec[MAX_VECTOR_SIZE];		
	
  assert( c );	// make sure pointer exists
  assert( c->NumTimeCodes );	// make sure some packets exist
  assert( c->VectorLen <= MAX_VECTOR_SIZE );
  		 
  uint32 packet_size = c->VectorLen + 1;
                       
  if (c->NumTimeCodes > 1) {
  	if (c->NumTimeCodes > 2)  {
  						 
  		float32 *pVecSrc, *pVecDst, *pVecOriginal;
      uint32	*pTcSrc,  *pTcDst,  *pTcOriginal;
              
      for(uint32 try_idx = 0; try_idx < (c->NumTimeCodes - 2); try_idx++)  {
      	
        // Src Pointers
        pTcSrc  = (uint32 *) &c->Data[ try_idx * packet_size ];
        pVecSrc	= (float32 *) pTcSrc+1;
        				 
        // Original Vector we're trying to recreate         
        pTcOriginal  = (uint32 *) &c->Data[ (try_idx + 1) * packet_size ];
        pVecOriginal = (float32 *) pTcOriginal+1;
        
        // Dst Pointers
        pTcDst  = (uint32 *) &c->Data[ (try_idx + 2 ) * packet_size ];
        pVecDst =	(float32 *) pTcDst+1;	 
        
        // Skip automagically, if binary movement involved		
        if (*pTcOriginal & W3D_TIMECODED_BINARY_MOVEMENT_FLAG) {
				continue;	// can't get rid of this guy	
        }			  
        if (*pTcDst & W3D_TIMECODED_BINARY_MOVEMENT_FLAG)  {
        		continue;	// can't get rid of this guy either
        }
            
        // Linear Interpolate between Src, and Dst, to recreate the
        // Original
        
        float32 tStart    = ((*pTcSrc) & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG);				// upgrade to floats
        float32 tRecreate = *pTcOriginal;
        float32 tEnd      = *pTcDst; 
        float32 tRatio    = (tRecreate - tStart) / (tEnd - tStart);
        																		
        uint32 idx=0;
        for (; idx < c->VectorLen; idx++)  {
        
        	tempvec[ idx ] = WWMath::Lerp(pVecSrc[idx], pVecDst[idx], tRatio);
          	
        }                                    
        
        // Compare Original to our re-creation
        
        bool close_enough = true;
        
        for (idx=0; idx < c->VectorLen; idx++)  {
        	
          float32 delta;
          
          delta = fabs(pVecOriginal[idx] - tempvec[idx]);
          
          if (delta > tolerance)  {
          	close_enough = false;
            break;
          }
          
        }													 
                                   
        // If our Recreation is very close to the original,
        // then discard the original
        
        if (true == close_enough)  {
        		return (try_idx + 1);
        }
        
        // else continue
               
      }	// for 
        
  	}	
    else  {
    	// Special Case, when there are only 2 time codes
      // Check to see if they are equal, value
      // if so, then return the 2nd timecode as useless
       	
      float32 *pVecSrc = (float32 *) &c->Data[ 1 ];
      float32 *pVecDst = (float32 *) &c->Data[ packet_size + 1 ];  
       	
      bool identical = true;  
        
      if ((c->Data[ packet_size ] & W3D_TIMECODED_BINARY_MOVEMENT_FLAG) == 0) {
      	
	      for(uint32 idx=0; idx < c->VectorLen; idx++) {
	      
	      	float32 delta;
	        
	        delta = fabs(pVecDst[idx] - pVecSrc[idx]);
	        
	        if (delta > tolerance)  {
	       		identical = false;
	          break;	
	        }	
	        
	      }	 
	      
	      if (identical) return( 1 );
         
      }
    }
  }
	
	return( PACKETS_ALL_USEFUL );
	
} // find_useless_packet	


//
// Special Case for Quaternion Packets
//
// Make Sure we NEVER get rid of binary movement packets
// The rule is, you can't interpolate TOO a binary movement, but you can
// interpolate FROM
//#define W3D_TIMECODED_BINARY_MOVEMENT_FLAG  0x80000000
//
uint32 VectorChannelClass::find_useless_packetQ(W3dTimeCodedAnimChannelStruct * c, double tolerance)
{												
	
  assert( c );	// make sure pointer exists
  assert( c->NumTimeCodes );	// make sure some packets exist
  assert( c->VectorLen == 4);
  		 
  uint32 packet_size = c->VectorLen + 1;
                       
  if (c->NumTimeCodes > 1) {
  	if (c->NumTimeCodes > 2)  {
  						 
  		float32 *pVecSrc, *pVecDst, *pVecOrg;
      uint32	*pTcSrc,  *pTcDst,  *pTcOrg;
              
		for(uint32 try_idx = 0; try_idx < (c->NumTimeCodes - 2); try_idx++)  {
      	
			// Src Pointers
			pTcSrc  = (uint32 *) &c->Data[ try_idx * packet_size ];
			pVecSrc	= (float32 *) pTcSrc+1;
        				 
			// Original Vector we're trying to recreate         
			pTcOrg  = (uint32 *) &c->Data[ (try_idx + 1) * packet_size ];
			pVecOrg = (float32 *) pTcOrg+1;
        
			// Dst Pointers
			pTcDst  = (uint32 *) &c->Data[ (try_idx + 2 ) * packet_size ];
			pVecDst = (float32 *) pTcDst+1;	 
        
			// Sphereical Linear Interpolate between Src, and Dst, to recreate the
			// Original
         
        	// Skip automagically, if binary movement involved		
        	if (*pTcOrg & W3D_TIMECODED_BINARY_MOVEMENT_FLAG) {
				continue;	// can't get rid of this guy	
        	}			  
       	if (*pTcDst & W3D_TIMECODED_BINARY_MOVEMENT_FLAG)  {
        		continue;	// can't get rid of this guy either
        	}
        
			float32 tStart    = ((*pTcSrc) & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG); // upgrade to floats
			float32 tRecreate = *pTcOrg;
			float32 tEnd      = *pTcDst; 
			float32 tRatio    = (tRecreate - tStart) / (tEnd - tStart);
        				  
		  
			Quaternion qSrc(1);
			qSrc.Set(pVecSrc[0],pVecSrc[1],pVecSrc[2],pVecSrc[3]);

			Quaternion qOrg(1);
			qOrg.Set(pVecOrg[0],pVecOrg[1],pVecOrg[2],pVecOrg[3]);

			Quaternion qDst(1);
			qDst.Set(pVecDst[0],pVecDst[1],pVecDst[2],pVecDst[3]);

			Quaternion q = Slerp( qSrc, qDst, tRatio );

			Quaternion Delta(1);
			Delta =  Inverse(qOrg) * q; 
 
			double angle = acosf( fabs( Delta.W ) ) * 2.0;
				
			if (angle <= tolerance ) {
				return (try_idx + 1);
			}

			// else continue
               
		}	// for
		
	}
        
  	}	
    else  {
    	// Special Case, when there are only 2 time codes
      // Check to see if they are equal, value
      // if so, then return the 2nd timecode as useless
      
		float32 *pVecSrc = (float32 *) &c->Data[ 1 ];
      float32 *pVecDst = (float32 *) &c->Data[ packet_size + 1 ];  
       
      if ((c->Data[ packet_size ] & W3D_TIMECODED_BINARY_MOVEMENT_FLAG) == 0)  {
      	
			Quaternion qSrc(1);
			qSrc.Set(pVecSrc[0], pVecSrc[1], pVecSrc[2], pVecSrc[3]);

			Quaternion qDst(1);
			qDst.Set(pVecDst[0], pVecDst[1], pVecDst[2], pVecDst[3]);

			Quaternion Delta(1);
			Delta = Inverse(qSrc) * qDst;

			double angle = acosf( fabs( Delta.W ) ) * 2.0;
				
			if (angle <= tolerance ) {
				return (1);
			}
      }
  }
	
	return( PACKETS_ALL_USEFUL );
	
} // find_useless_packetQ	


//
//	Instead of using a fixed error threshold, find the packet
// that generates the least amount of error, via removal
// 
// Make Sure we NEVER get rid of binary movement packets
// The rule is, you can't interpolate too a binary movement, but you can
// interpolate FROM
//#define W3D_TIMECODED_BINARY_MOVEMENT_FLAG  0x80000000
//
uint32 VectorChannelClass::find_least_useful_packet(W3dTimeCodedAnimChannelStruct *c)
{

static  float32 tempvec[MAX_VECTOR_SIZE];		
	
  assert( c );	// make sure pointer exists
  assert( c->NumTimeCodes );	// make sure some packets exist
  assert( c->VectorLen <= MAX_VECTOR_SIZE );
  		 
  uint32 packet_size = c->VectorLen + 1;
  
  double leasterror = 9999999.0f;
  uint32 ret_idx = PACKETS_ALL_USEFUL;
                       
	if (c->NumTimeCodes > 1) {
		if (c->NumTimeCodes > 2)  {
  						 
  			float32 *pVecSrc, *pVecDst, *pVecOriginal;
			uint32	*pTcSrc,  *pTcDst,  *pTcOriginal;
              
			for(uint32 try_idx = 0; try_idx < (c->NumTimeCodes - 2); try_idx++)  {
      	
				// Src Pointers
				pTcSrc  = (uint32 *) &c->Data[ try_idx * packet_size ];
				pVecSrc	= (float32 *) pTcSrc+1;
        				 
				// Original Vector we're trying to recreate         
				pTcOriginal  = (uint32 *) &c->Data[ (try_idx + 1) * packet_size ];
				pVecOriginal = (float32 *) pTcOriginal+1;
        
				// Dst Pointers
				pTcDst  = (uint32 *) &c->Data[ (try_idx + 2 ) * packet_size ];
				pVecDst =	(float32 *) pTcDst+1;	 
        		
	        	// Skip automagically, if binary movement involved		
	       	if (*pTcOriginal & W3D_TIMECODED_BINARY_MOVEMENT_FLAG) {
					continue;	// can't get rid of this guy	
	         }			  
	         if (*pTcDst & W3D_TIMECODED_BINARY_MOVEMENT_FLAG)  {
	        		continue;	// can't get rid of this guy either
	         }
                              
				// Linear Interpolate between Src, and Dst, to recreate the
				// Original
        
				float32 tStart    = ((*pTcSrc) & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG);				// upgrade to floats
				float32 tRecreate = *pTcOriginal;
				float32 tEnd      = *pTcDst; 
				float32 tRatio    = (tRecreate - tStart) / (tEnd - tStart);
        		 																	
				uint32 idx=0;
				for (; idx < c->VectorLen; idx++)  {
        
        		 tempvec[ idx ] = WWMath::Lerp(pVecSrc[idx], pVecDst[idx], tRatio);
          	 	
				}                                    
        
				// Compare Original to our re-creation

				double delta = 0.0;
				
        
				for (idx=0; idx < c->VectorLen; idx++)  {
        		 
					double tmp;

					tmp = pVecOriginal[idx] - tempvec[idx];

					delta += (tmp * tmp);
          
				}													 
                   
				delta = sqrtf( delta );

				if (delta < leasterror)
				{

					// If our Recreation is very close to the original,
					// then discard the original
					leasterror = delta;
					ret_idx =(try_idx + 1);
				}
        
				// else continue
               
			}	// for 

			return( ret_idx );
        
		}	
		else  {
         
      	if ((c->Data[ packet_size ] & W3D_TIMECODED_BINARY_MOVEMENT_FLAG) == 0) {
				return( 1 );																				 
         }
		}
  }
			
  
  return( PACKETS_ALL_USEFUL );
         

}  // Find Least useful packet


//
//	Instead of using a fixed error threshold, find the packet
// that generates the least amount of error, via removal
// special case for Quaternion channel
// 
// Make Sure we NEVER get rid of binary movement packets
// The rule is, you can't interpolate FROM a binary movement, but you can
// interpolate TOO
//#define W3D_TIMECODED_BINARY_MOVEMENT_FLAG  0x80000000
//
uint32 VectorChannelClass::find_least_useful_packetQ(W3dTimeCodedAnimChannelStruct *c)
{

  assert( c );	// make sure pointer exists
  assert( c->NumTimeCodes );	// make sure some packets exist
  assert( c->VectorLen == 4);
  		 
  uint32 packet_size = c->VectorLen + 1;

  double leasterror = 9999999.0f;
  uint32 ret_idx = PACKETS_ALL_USEFUL;
  
  if (c->NumTimeCodes > 1) {
  	if (c->NumTimeCodes > 2)  {
  						 
  		float32 *pVecSrc, *pVecDst, *pVecOrg;
      uint32	*pTcSrc,  *pTcDst,  *pTcOrg;
              
		for(uint32 try_idx = 0; try_idx < (c->NumTimeCodes - 2); try_idx++)  {
      	
			// Src Pointers
			pTcSrc  = (uint32 *) &c->Data[ try_idx * packet_size ];
			pVecSrc	= (float32 *) pTcSrc+1;
        				 
			// Original Vector we're trying to recreate         
			pTcOrg  = (uint32 *) &c->Data[ (try_idx + 1) * packet_size ];
			pVecOrg = (float32 *) pTcOrg+1;
        
			// Dst Pointers
			pTcDst  = (uint32 *) &c->Data[ (try_idx + 2 ) * packet_size ];
			pVecDst = (float32 *) pTcDst+1;	 
        	
	      // Skip automagically, if binary movement involved		
	      if (*pTcOrg & W3D_TIMECODED_BINARY_MOVEMENT_FLAG) {
				continue;	// can't get rid of this guy	
	      }			  
	      if (*pTcDst & W3D_TIMECODED_BINARY_MOVEMENT_FLAG)  {
	      	continue;	// can't get rid of this guy either
	      }
                         
			// Spherical Linear Interpolate between Src, and Dst, to recreate the
			// Original
        
			float32 tStart    = ((*pTcSrc) & ~W3D_TIMECODED_BINARY_MOVEMENT_FLAG);				// upgrade to floats
			float32 tRecreate = *pTcOrg;
			float32 tEnd      = *pTcDst; 
			float32 tRatio    = (tRecreate - tStart) / (tEnd - tStart);
        				  
		  
			Quaternion qSrc(1);
			qSrc.Set(pVecSrc[0],pVecSrc[1],pVecSrc[2],pVecSrc[3]);

			Quaternion qOrg(1);
			qOrg.Set(pVecOrg[0],pVecOrg[1],pVecOrg[2],pVecOrg[3]);

			Quaternion qDst(1);
			qDst.Set(pVecDst[0],pVecDst[1],pVecDst[2],pVecDst[3]);

			Quaternion q = Slerp( qSrc, qDst, tRatio );

			Quaternion Delta(1);
			Delta =  Inverse(qOrg) * q; 
 
			double angle = acosf( fabs( Delta.W ) ) * 2;
				
			if (angle < leasterror ) {
				leasterror = angle;
				ret_idx = (try_idx + 1);
			}
        
			// else continue
               
		}	// for

		return( ret_idx );
		
	}
        
  	}	
    else  {
    	// Special Case, when there are only 2 time codes
      // Check to see if they are equal, value
      // if so, then return the 2nd timecode as useless
      
      	if ((c->Data[ packet_size ] & W3D_TIMECODED_BINARY_MOVEMENT_FLAG) == 0) {
				return( 1 );																				 
         }
  }

	 
  return( PACKETS_ALL_USEFUL );

} // find_least_useful_packetQ


// EOF - vchannel.cpp