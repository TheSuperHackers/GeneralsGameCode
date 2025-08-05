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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/bchannel.h                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/30/00 5:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef BCHANNEL_H
#define BCHANNEL_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#ifndef BITTYPE_H
#include "BITTYPE.H"
#endif

#ifndef CHUNKIO_H
#include "chunkio.h"
#endif

#ifndef VECTOR_H
#include "Vector.H"
#endif

#ifndef W3D_FILE_H
#include "w3d_file.h"
#endif

class LogDataDialogClass;

class BitChannelClass
{
public:

	BitChannelClass(uint32 id,int maxframes,uint32 chntype,bool def_val);
	~BitChannelClass(void);

	void		Set_Bit(int framenumber,bool bit);
	void		Set_Bits(BooleanVectorClass & bits);
	bool		Get_Bit(int frameidx);
	bool		Is_Empty(void) { return IsEmpty; }
	bool		Save(ChunkSaveClass & csave, bool compress);

private:

	uint32					ID;
	uint32					ChannelType;
	int						MaxFrames;
	bool						IsEmpty;

	bool						DefaultVal;
	BooleanVectorClass	Data;
	int						Begin;
	int						End;

	// Test a bit against the "default" bit
	bool is_default(bool bit);

	// This function finds the start and end of the "non-default" data
	void compute_range(void);

  // compress functions
	void remove_packet(W3dTimeCodedBitChannelStruct * c, uint32 packet_idx);
	uint32 find_useless_packet(W3dTimeCodedBitChannelStruct * c);
	void compress(W3dTimeCodedBitChannelStruct * c);


};


#endif
