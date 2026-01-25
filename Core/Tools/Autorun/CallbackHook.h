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

/******************************************************************************
*
* FILE
*     $Archive: /Renegade Setup/Autorun/CallbackHook.h $
*
* DESCRIPTION
*
* PROGRAMMER
*     Steven Clinard
*     $Author: Maria_l $
*
* VERSION INFO
*     $Modtime: 8/14/00 7:52p $
*     $Revision: 2 $
*
******************************************************************************/

#pragma once

class CallbackHook
	{
	public:
		CallbackHook()
			{}

		virtual ~CallbackHook()
			{}

		virtual bool DoCallback(void) const
			{return false;}

	protected:
		CallbackHook(const CallbackHook&);
		const CallbackHook& operator=(const CallbackHook&);
	};


template<class T> class Callback : public CallbackHook
	{
	public:
		Callback(bool (*callback)(T), T userdata)
			: mCallback(callback),
			  mUserData(userdata)
			{
			}

		virtual ~Callback()
			{
			}

		virtual bool DoCallback(void) const
			{
			if (mCallback != nullptr)
				{
				return mCallback(mUserData);
				}

			return false;
			}

	private:
		bool (*mCallback)(T);
		T mUserData;
	};
