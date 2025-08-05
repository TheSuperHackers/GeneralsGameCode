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
 *                     $Archive:: /Commando/Code/WWAudio/sound2dhandle.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/23/01 5:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "sound2dhandle.h"
#include "AudibleSound.h"


//////////////////////////////////////////////////////////////////////
//
//	Sound2DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound2DHandleClass::Sound2DHandleClass (void)	:
	SampleHandle ((HSAMPLE)INVALID_MILES_HANDLE)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~Sound2DHandleClass
//
//////////////////////////////////////////////////////////////////////
Sound2DHandleClass::~Sound2DHandleClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Initialize (SoundBufferClass *buffer)
{
	SoundHandleClass::Initialize (buffer);

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {

		//
		// Make sure this handle is fresh
		//
		::AIL_init_sample (SampleHandle);

		//
		// Pass the actual sound data onto the sample
		//
		if (Buffer != NULL) {
			::AIL_set_named_sample_file (SampleHandle, (char *)Buffer->Get_Filename (),
					Buffer->Get_Raw_Buffer (), Buffer->Get_Raw_Length (), 0);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Start_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Start_Sample (void)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_start_sample (SampleHandle);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Stop_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Stop_Sample (void)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_stop_sample (SampleHandle);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Resume_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Resume_Sample (void)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_resume_sample (SampleHandle);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	End_Sample
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::End_Sample (void)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_end_sample (SampleHandle);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_Pan (S32 pan)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		// TheSuperHackers @fix xezon 05/04/2025 Upgrades miles call from legacy AIL_set_sample_pan.
		// TheSuperHackers @todo Perhaps use float natively.
		float fVolume = 0.0F;
		::AIL_sample_volume_pan (SampleHandle, &fVolume, NULL);
		float fPan = pan / 127.0F;
		::AIL_set_sample_volume_pan (SampleHandle, fVolume, fPan);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Pan
//
//////////////////////////////////////////////////////////////////////
S32
Sound2DHandleClass::Get_Sample_Pan (void)
{
	S32 retval = 0;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		// TheSuperHackers @fix xezon 05/04/2025 Upgrades miles call from legacy AIL_sample_pan.
		float fPan = 0.5F;
		::AIL_sample_volume_pan (SampleHandle, NULL, &fPan);
		retval = fPan * 127;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_Volume (S32 volume)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		// TheSuperHackers @fix xezon 05/04/2025 Upgrades miles call from legacy AIL_set_sample_volume.
		// TheSuperHackers @todo Perhaps use float natively.
		float fPan = 0.5F;
		::AIL_sample_volume_pan (SampleHandle, NULL, &fPan);
		float fVolume = volume / 127.0F;
		::AIL_set_sample_volume_pan (SampleHandle, fVolume, fPan);
	}
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Volume
//
//////////////////////////////////////////////////////////////////////
S32
Sound2DHandleClass::Get_Sample_Volume (void)
{
	S32 retval = 0;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		// TheSuperHackers @fix xezon 05/04/2025 Upgrades miles call from legacy AIL_sample_volume.
		float fVolume = 0.0F;
		::AIL_sample_volume_pan (SampleHandle, &fVolume, NULL);
		retval = fVolume * 127;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_Loop_Count (U32 count)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_loop_count (SampleHandle, count);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Loop_Count
//
//////////////////////////////////////////////////////////////////////
U32
Sound2DHandleClass::Get_Sample_Loop_Count (void)
{
	U32 retval = 0;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_loop_count (SampleHandle);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_MS_Position (U32 ms)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_ms_position (SampleHandle, ms);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_MS_Position
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Get_Sample_MS_Position (S32 *len, S32 *pos)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_sample_ms_position (SampleHandle, len, pos);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_User_Data (S32 i, U32 val)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_user_data (SampleHandle, i, val);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_User_Data
//
//////////////////////////////////////////////////////////////////////
U32
Sound2DHandleClass::Get_Sample_User_Data (S32 i)
{
	U32 retval = 0;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_user_data (SampleHandle, i);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Get_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
S32
Sound2DHandleClass::Get_Sample_Playback_Rate (void)
{
	S32 retval = 0;

	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		retval = ::AIL_sample_playback_rate (SampleHandle);
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Sample_Playback_Rate
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Sample_Playback_Rate (S32 rate)
{
	if (SampleHandle != (HSAMPLE)INVALID_MILES_HANDLE) {
		::AIL_set_sample_playback_rate (SampleHandle, rate);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Set_Miles_Handle
//
//////////////////////////////////////////////////////////////////////
void
Sound2DHandleClass::Set_Miles_Handle (uint32 handle)
{
	SampleHandle = (HSAMPLE)handle;
	return ;
}
