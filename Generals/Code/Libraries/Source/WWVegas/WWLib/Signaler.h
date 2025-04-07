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

/******************************************************************************
*
* FILE
*     $Archive: /Commando/Code/wwlib/Signaler.h $
*
* DESCRIPTION
*     Lightweight two-way notification system. This class allows loose coupling
*     communication between two classes. The only details that need to be know
*     by both classes is the Signaler class it self and the type of signal they
*     communicate to each other.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Modtime: 11/16/01 11:19a $
*     $Revision: 4 $
*
******************************************************************************/

#ifndef __SIGNALER_H__
#define __SIGNALER_H__

template<typename T> class Signaler
	{
	public:
		void SignalMe(Signaler<T>& target)
			{if (mConnection != &target) {Disconnect(); Connect(target); target.Connect(*this);}}

		void StopSignaling(Signaler<T>& target)
			{if (mConnection == &target) {Disconnect();}}

		void SendSignal(T& signal)
			{if (mConnection) {mConnection->ReceiveSignal(signal);}}

		virtual void ReceiveSignal(T&)
			{}

		virtual void SignalDropped(Signaler<T>& signaler)
			{mConnection = NULL;}

	protected:
		Signaler() :
				mConnection(NULL)
			{}
	
		virtual ~Signaler()
			{Disconnect();}

		void Connect(Signaler<T>& source)
			{mConnection = &source;}

		void Disconnect(void)
			{if (mConnection) {mConnection->SignalDropped(*this);} mConnection = NULL;}

		// Prevent copy and assignment
		Signaler(const Signaler&);
		const Signaler& operator=(const Signaler&);

	private:
		Signaler<T>* mConnection;
	};

#endif // __SIGNALER_H__


