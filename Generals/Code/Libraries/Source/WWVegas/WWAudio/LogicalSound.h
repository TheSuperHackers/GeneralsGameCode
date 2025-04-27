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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/LogicalSound.h                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/01/99 10:18a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __LOGICAL_SOUND_H
#define __LOGICAL_SOUND_H

#include "SoundSceneObj.h"
#include "bittype.h"
#include "vector3.h"
#include "matrix3d.h"

/////////////////////////////////////////////////////////////////////////////////
//
//	LogicalSoundClass
//
//	This class represents 'logical' sounds that affect gameplay but do not
// actually make an audible sound
//
class LogicalSoundClass : public SoundSceneObjClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public friends
		//////////////////////////////////////////////////////////////////////
		friend class SoundSceneClass;

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		LogicalSoundClass (void);
		virtual ~LogicalSoundClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		//	LogicalSoundClass specific
		//////////////////////////////////////////////////////////////////////		
		virtual bool			Is_Single_Shot (void) const			{ return m_IsSingleShot; }
		virtual void			Set_Single_Shot (bool single_shot)	{ m_IsSingleShot = single_shot; }

		virtual void			Set_Type_Mask (uint32 mask = 0)	{ m_TypeMask = mask; }
		virtual uint32			Get_Type_Mask (void) const			{ return m_TypeMask; }

		virtual float			Get_Notify_Delay (void) const		{ return (float)m_NotifyDelayInMS / 1000.0F; }
		virtual void			Set_Notify_Delay (float secs)		{ m_NotifyDelayInMS = uint32(secs * 1000.0F); }
		virtual bool			Allow_Notify (uint32 timestamp);

		virtual uint32			Get_Listener_Timestamp (void) const		{ return m_OldestListenerTimestamp; }
		virtual void			Set_Listener_Timestamp (int timestamp)	{ m_OldestListenerTimestamp = timestamp; }

		//////////////////////////////////////////////////////////////////////
		//	Update methods
		//////////////////////////////////////////////////////////////////////
		virtual bool			On_Frame_Update (unsigned int milliseconds = 0);

		//////////////////////////////////////////////////////////////////////
		//	Position/direction methods
		//////////////////////////////////////////////////////////////////////		
		virtual void			Set_Position (const Vector3 &position)		{ m_Position = position; }
		virtual Vector3		Get_Position (void) const						{ return m_Position; }

		virtual void			Set_Transform (const Matrix3D &transform) { m_Position = transform.Get_Translation (); }
		virtual Matrix3D		Get_Transform (void) const						{ Matrix3D tm(1); tm.Set_Translation (m_Position); return tm; }

		//////////////////////////////////////////////////////////////////////
		//	Culling methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Cull_Sound (bool culled = true)	{ };
		virtual bool			Is_Sound_Culled (void) const		{ return false; };

		//////////////////////////////////////////////////////////////////////
		//	Scene integration
		//////////////////////////////////////////////////////////////////////
		virtual void			Add_To_Scene (bool start_playing = true);
		virtual void			Remove_From_Scene (void);

		//////////////////////////////////////////////////////////////////////
		//	Attenuation settings
		//////////////////////////////////////////////////////////////////////

		//
		//	This is the distance where the sound can not be heard any longer.  (its vol is 0)
		//
		virtual void			Set_DropOff_Radius (float radius = 1)	{ m_DropOffRadius = radius; }
		virtual float			Get_DropOff_Radius (void) const			{ return m_DropOffRadius; }

		//////////////////////////////////////////////////////////////////////
		//	From PersistClass
		//////////////////////////////////////////////////////////////////////				
		bool									Save (ChunkSaveClass &csave);
		bool									Load (ChunkLoadClass &cload);
		const PersistFactoryClass &	Get_Factory (void) const;

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		float					m_DropOffRadius;
		bool					m_IsSingleShot;
		uint32					m_TypeMask;
		Vector3					m_Position;
		uint32					m_OldestListenerTimestamp;
		int						m_MaxListeners;
		uint32					m_NotifyDelayInMS;
		uint32					m_LastNotification;
};


#endif //__LOGICAL_SOUND_H

