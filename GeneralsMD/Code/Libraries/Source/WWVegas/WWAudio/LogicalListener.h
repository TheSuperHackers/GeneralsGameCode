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
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/LogicalListener.h                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/01/99 10:18a                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __LOGICAL_LISTENER_H
#define __LOGICAL_LISTENER_H

#include "SoundSceneObj.h"
#include "bittype.h"
#include "vector3.h"
#include "matrix3d.h"

/////////////////////////////////////////////////////////////////////////////////
//
//	LogicalListenerClass
//
//	This class represents listeners in the 'world' that can hear and react to
// logical sounds.
//
/////////////////////////////////////////////////////////////////////////////////
class LogicalListenerClass : public SoundSceneObjClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		LogicalListenerClass (void);
		virtual ~LogicalListenerClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		//	LogicalSoundClass specific
		//////////////////////////////////////////////////////////////////////		
		virtual void			Set_Type_Mask (uint32 mask = 0)	{ m_TypeMask = mask; }
		virtual uint32			Get_Type_Mask (void) const			{ return m_TypeMask; }

		//////////////////////////////////////////////////////////////////////
		//	Position/direction methods
		//////////////////////////////////////////////////////////////////////		
		virtual void			Set_Position (const Vector3 &position)		{ m_Position = position; }
		virtual Vector3			Get_Position (void) const						{ return m_Position; }

		virtual void			Set_Transform (const Matrix3D &transform) { m_Position = transform.Get_Translation (); }
		virtual Matrix3D		Get_Transform (void) const						{ Matrix3D tm(1); tm.Set_Translation (m_Position); return tm; }

		//////////////////////////////////////////////////////////////////////
		//	Culling methods (not used for listeners)
		//////////////////////////////////////////////////////////////////////
		virtual void			Cull_Sound (bool culled = true)	{ };
		virtual bool			Is_Sound_Culled (void) const		{ return false; };

		//////////////////////////////////////////////////////////////////////
		//	Scene integration
		//////////////////////////////////////////////////////////////////////
		virtual void			Add_To_Scene (bool /*start_playing*/ = true);
		virtual void			Remove_From_Scene (void);

		//////////////////////////////////////////////////////////////////////
		//	Attenuation settings
		//////////////////////////////////////////////////////////////////////

		//
		//	This is the distance where the listener can no longer hear sounds.
		//
		virtual void			Set_Scale (float scale = 1.0F)	{ m_Scale = scale; }
		virtual float			Get_Scale (void) const				{ return m_Scale; }
		virtual float			Get_Effective_Scale (void) const	{ return m_Scale * m_GlobalScale; }

		static float			Get_Global_Scale (void)				{ return m_GlobalScale; }
		static void				Set_Global_Scale (float scale)	{ m_GlobalScale = scale; }

		virtual void			Set_DropOff_Radius (float radius = 1)	{}
		virtual float			Get_DropOff_Radius (void) const			{ return 1.0F; }

		//////////////////////////////////////////////////////////////////////
		//	From PersistClass
		//////////////////////////////////////////////////////////////////////				
		bool									Save (ChunkSaveClass &csave);
		bool									Load (ChunkLoadClass &cload);
		const PersistFactoryClass &	Get_Factory (void) const;


		//////////////////////////////////////////////////////////////////////
		//	Timestamp
		//////////////////////////////////////////////////////////////////////				
		uint32					Get_Timestamp (void) const		{ return m_Timestamp; }
		void					Set_Timestamp (int timestamp)	{ m_Timestamp = timestamp; }

		static uint32			Get_New_Timestamp (void)		{ return m_NewestTimestamp ++; }
		static uint32			Get_Newest_Timestamp (void)		{ return m_NewestTimestamp - 1; }

		static uint32			Get_Oldest_Timestamp (void)				{ return m_OldestTimestamp; }
		static void				Set_Oldest_Timestamp (uint32 timestamp)	{ WWASSERT (m_OldestTimestamp < timestamp); m_OldestTimestamp = timestamp; }

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Update methods
		//////////////////////////////////////////////////////////////////////

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		static float			m_GlobalScale;
		float						m_Scale;
		uint32					m_TypeMask;
		Vector3					m_Position;
		uint32					m_Timestamp;
		static uint32			m_OldestTimestamp;
		static uint32			m_NewestTimestamp;
};


#endif //__LOGICAL_LISTENER_H

