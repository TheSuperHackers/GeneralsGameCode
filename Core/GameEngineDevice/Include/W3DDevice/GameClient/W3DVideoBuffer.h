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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:    Generals
//
// File name:  W3DDevice/GameClient/W3DVideoBuffer.h
//
// Created:    10/23/01
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __W3DDEVICE_GAMECLIENT_W3DVIDEOBUFFER_H_
#define __W3DDEVICE_GAMECLIENT_W3DVIDEOBUFFER_H_


//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "GameClient/VideoPlayer.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

class TextureClass;
class SurfaceClass;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// W3DVideoBuffer
//===============================
/**
  * Video buffer interface class to a W3D TextureClass
	*/
//===============================


class W3DVideoBuffer : public VideoBuffer
{
	protected:

		TextureClass	*m_texture;
		SurfaceClass	*m_surface;

	public:

		W3DVideoBuffer( Type format );
		virtual ~W3DVideoBuffer();

		virtual	Bool		allocate( UnsignedInt width, UnsignedInt height); ///< Allocates buffer
		virtual void		free( void);					///< Free buffer
		virtual	void*		lock( void );					///< Returns memory pointer to start of buffer
		virtual void		unlock( void );				///< Release buffer
		virtual Bool		valid( void );				///< Is the buffer valid to use

		TextureClass		*texture( void );			///< Returns texture object

		static WW3DFormat TypeToW3DFormat( VideoBuffer::Type format );
		static VideoBuffer::Type W3DFormatToType( WW3DFormat w3dFormat );
};


//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------

inline TextureClass* W3DVideoBuffer::texture( void ) { return m_texture; }

#endif // __W3DDEVICE_GAMECLIENT_W3DVIDEOBUFFER_H_
