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
 *                 Project Name : wwaudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/soundstreamhandle.h                  $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/13/01 3:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "soundhandle.h"


//////////////////////////////////////////////////////////////////////
//
//	SoundStreamHandleClass
//
//////////////////////////////////////////////////////////////////////
class SoundStreamHandleClass	: public SoundHandleClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	SoundStreamHandleClass  ();
	~SoundStreamHandleClass ();

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	RTTI
	//
	SoundStreamHandleClass *	As_SoundStreamHandleClass ()	{ return this; }

	//
	//	Handle access
	//
	HSAMPLE						Get_HSAMPLE ()		{ return SampleHandle; }
	HSTREAM						Get_HSTREAM ()		{ return StreamHandle; }

	//
	//	Inherited
	//
	void							Set_Miles_Handle (uint32 handle);
	void							Initialize (SoundBufferClass *buffer);
	void							Start_Sample ();
	void							Stop_Sample ();
	void							Resume_Sample ();
	void							End_Sample ();
	void							Set_Sample_Pan (S32 pan);
	S32							Get_Sample_Pan ();
	void							Set_Sample_Volume (S32 volume);
	S32							Get_Sample_Volume ();
	void							Set_Sample_Loop_Count (U32 count);
	U32							Get_Sample_Loop_Count ();
	void							Set_Sample_MS_Position (U32 ms);
	void							Get_Sample_MS_Position (S32 *len, S32 *pos);
	void							Set_Sample_User_Data (S32 i, void *val);
	void *							Get_Sample_User_Data (S32 i);
	S32							Get_Sample_Playback_Rate ();
	void							Set_Sample_Playback_Rate (S32 rate);

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	HSAMPLE		SampleHandle;
	HSTREAM		StreamHandle;
};
