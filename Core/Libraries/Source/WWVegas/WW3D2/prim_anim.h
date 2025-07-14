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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/prim_anim.h                            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/29/01 5:43p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PRIM_ANIM_H
#define __PRIM_ANIM_H


#include "simplevec.h"
#include "chunkio.h"


// Forward declarations
class ChunkSaveClass;
class ChunkLoadClass;


/////////////////////////////////////////////////////////////////////
//
//	PrimitiveAnimationChannelClass
//
//	This template class provides animated 'channels' of data for the 
// RingRenderObjClass and SphereRenderObjClass objects.
//
/////////////////////////////////////////////////////////////////////
template<class T>
class PrimitiveAnimationChannelClass
{
public:

	/////////////////////////////////////////////////////////
	//	Public constructors/destructors
	/////////////////////////////////////////////////////////
	PrimitiveAnimationChannelClass (void)
		:	m_LastIndex (0)									{ }
	virtual ~PrimitiveAnimationChannelClass (void)	{ Reset (); }

	/////////////////////////////////////////////////////////
	//	Public data types
	/////////////////////////////////////////////////////////
	class KeyClass
	{
	public:
		KeyClass (void)
			:	m_Time (0) {}

		KeyClass (const T &value, float time)
			:	m_Value (value),
				m_Time (time) {}

		float			Get_Time (void) const		{ return m_Time; }
		const T &	Get_Value (void) const		{ return m_Value; }
		T &			Get_Value (void)				{ return m_Value; }

		float			Set_Time (float time)		{ m_Time = time; }
		void			Set_Value (const T &value)	{ m_Value = value; }

	private:
		T				m_Value;
		float			m_Time;
	};

	/////////////////////////////////////////////////////////
	//	Public operators
	/////////////////////////////////////////////////////////
	const PrimitiveAnimationChannelClass<T> &operator= (const PrimitiveAnimationChannelClass<T> &src);
	const KeyClass &		operator[] (int index)	{ return Get_Key (index); }
	
	/////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////
	virtual T				Evaluate (float time) = 0;

	int						Get_Key_Count (void) const;
	const KeyClass &		Get_Key (int index) const;
	void						Set_Key (int index, const T &value, float time);
	void						Set_Key_Value (int index, const T &value);
	void						Add_Key (const T &value, float time);
	void						Insert_Key (int index, const T &value, float time);
	void						Delete_Key (int index);
	void						Reset (void);
	
	virtual void			Save (ChunkSaveClass &csave);
	virtual void			Load (ChunkLoadClass &cload);

protected:

	/////////////////////////////////////////////////////////
	//	Protected constants
	/////////////////////////////////////////////////////////
	enum
	{
		CHUNKID_VARIABLES		= 0x03150809,
	};

	enum
	{
		VARID_KEY				= 1,
	};

	/////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////
	void						Load_Variables (ChunkLoadClass &cload);

protected:

	/////////////////////////////////////////////////////////
	//	Protected  member data
	/////////////////////////////////////////////////////////
	SimpleDynVecClass< KeyClass >	m_Data;
	int									m_LastIndex;
};


/////////////////////////////////////////////////////////////////////
//
//	LERPAnimationChannelClass
//
//	This template class provides a simple LERP implementation of the
// Evaluate () method.
//
/////////////////////////////////////////////////////////////////////
template<class T>
class LERPAnimationChannelClass : public PrimitiveAnimationChannelClass<T>
{
	using PrimitiveAnimationChannelClass<T>::m_Data;
	using PrimitiveAnimationChannelClass<T>::m_LastIndex;
public:
	using typename PrimitiveAnimationChannelClass<T>::KeyClass;

public:

	/////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////
	virtual T Evaluate (float time);
};


/////////////////////////////////////////////////////////
//	Set_Key
/////////////////////////////////////////////////////////
template<class T>
int PrimitiveAnimationChannelClass<T>::Get_Key_Count (void) const
{
	return m_Data.Count ();
}

/////////////////////////////////////////////////////////
//	Set_Key_Value
/////////////////////////////////////////////////////////
template<class T>
const typename PrimitiveAnimationChannelClass<T>::KeyClass &PrimitiveAnimationChannelClass<T>::Get_Key (int index) const
{
	return m_Data[index];
}

/////////////////////////////////////////////////////////
//	Set_Key
/////////////////////////////////////////////////////////
template<class T>
void PrimitiveAnimationChannelClass<T>::Set_Key (int index, const T &value, float time)
{
	m_Data[index].Set_Value (value);
	m_Data[index].Set_Time (time);
	return ;
}

/////////////////////////////////////////////////////////
//	Set_Key_Value
/////////////////////////////////////////////////////////
template<class T>
void PrimitiveAnimationChannelClass<T>::Set_Key_Value (int index, const T &value)
{
	m_Data[index].Set_Value (value);
	return ;
}

/////////////////////////////////////////////////////////
//	Add_Key
/////////////////////////////////////////////////////////
template<class T>
void PrimitiveAnimationChannelClass<T>::Add_Key (const T &value, float time)
{
	m_Data.Add (KeyClass (value, time));
	return ;
}

/////////////////////////////////////////////////////////
//	Insert_Key
/////////////////////////////////////////////////////////
template<class T>
void PrimitiveAnimationChannelClass<T>::Insert_Key (int index, const T &value, float time)
{
	m_Data.Insert (index, KeyClass (value, time));
	return ;
}

/////////////////////////////////////////////////////////
//	Delete_Key
/////////////////////////////////////////////////////////
template<class T>
void PrimitiveAnimationChannelClass<T>::Delete_Key (int index)
{
	m_Data.Delete (index);
	return ;
}

/////////////////////////////////////////////////////////
//	Reset
/////////////////////////////////////////////////////////
template<class T>
void PrimitiveAnimationChannelClass<T>::Reset (void)
{
	m_Data.Delete_All ();
	m_LastIndex = 0;
	return ;
}

/////////////////////////////////////////////////////////////////////
//	operator=
/////////////////////////////////////////////////////////////////////
template<class T> const PrimitiveAnimationChannelClass<T> &
PrimitiveAnimationChannelClass<T>::operator= (const PrimitiveAnimationChannelClass<T> &src)
{
	Reset ();

	//
	//	Copy the data array
	//
	for (int index = 0; index < src.Get_Key_Count (); index ++) {		
		m_Data.Add (src.Get_Key (index));
	}

	m_LastIndex = src.m_LastIndex;
	return *this;
}

/////////////////////////////////////////////////////////////////////
//	Save
/////////////////////////////////////////////////////////////////////
template<class T> void
PrimitiveAnimationChannelClass<T>::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		
		//
		//	Save each key
		//
		for (int index = 0; index < m_Data.Count (); index ++) {
			KeyClass &value = m_Data[index];
			WRITE_MICRO_CHUNK (csave, VARID_KEY, value);
		}

	csave.End_Chunk ();

	return ;
}

/////////////////////////////////////////////////////////////////////
//	Load
/////////////////////////////////////////////////////////////////////
template<class T> void
PrimitiveAnimationChannelClass<T>::Load (ChunkLoadClass &cload)
{
	Reset ();

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return ;
}

/////////////////////////////////////////////////////////////////////
//	Load_Variables
/////////////////////////////////////////////////////////////////////
template<class T> void
PrimitiveAnimationChannelClass<T>::Load_Variables (ChunkLoadClass &cload)
{
	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			case VARID_KEY:
			{
				KeyClass value;
				cload.Read (&value, sizeof (value));
				m_Data.Add (value);
			}
			break;
		}

		cload.Close_Micro_Chunk ();
	}
	
	return ;
}

/////////////////////////////////////////////////////////////////////
//	Evaluate
/////////////////////////////////////////////////////////////////////
template<class T> T
LERPAnimationChannelClass<T>::Evaluate (float time)
{
	int key_count	= m_Data.Count ();
	T value			= m_Data[key_count - 1].Get_Value ();

	//
	//	Don't interpolate past the last keyframe
	//
	if (time < m_Data[key_count - 1].Get_Time ()) {

		// Check to see if the last key index is valid
		if (time < m_Data[m_LastIndex].Get_Time ()) {
			m_LastIndex = 0;
		}

		KeyClass *key1 = &m_Data[m_LastIndex];
		KeyClass *key2 = &m_Data[key_count - 1];

		//
		// Search, using last_key as our starting point
		//
		for (int keyidx = m_LastIndex; keyidx < (key_count - 1); keyidx ++) {

			if (time < m_Data[keyidx+1].Get_Time ()) {
				key1 = &m_Data[keyidx];
				key2 = &m_Data[keyidx+1];
				m_LastIndex = keyidx;
				break;
			}
		}

		// Calculate the linear percent between the two keys
		float percent	= (time - key1->Get_Time ()) / (key2->Get_Time () - key1->Get_Time ());

		// Interpolate the value
		value = (key1->Get_Value () + (key2->Get_Value () - key1->Get_Value ()) * percent);
	}

	return value;
}

#endif //__PRIM_ANIM_H
