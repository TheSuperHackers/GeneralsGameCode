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
 *                     $Archive:: /Commando/Code/WWAudio/soundhandle.cpp                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/13/01 12:18p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "soundhandle.h"
#include "Threads.h"


//////////////////////////////////////////////////////////////////////
//
//	SoundHandleClass
//
//////////////////////////////////////////////////////////////////////
SoundHandleClass::SoundHandleClass (void)	:
	Buffer (nullptr)
{
	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	~SoundHandleClass
//
//////////////////////////////////////////////////////////////////////
SoundHandleClass::~SoundHandleClass (void)
{
	//
	//	Delay the release of the buffer (fixes a sync bug
	// with Miles internals).
	//
	if (Buffer != nullptr) {
		WWAudioThreadsClass::Add_Delayed_Release_Object (Buffer);
		Buffer = nullptr;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////
void
SoundHandleClass::Initialize (SoundBufferClass *buffer)
{
	REF_PTR_SET (Buffer, buffer);
	return ;
}

