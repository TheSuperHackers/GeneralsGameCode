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
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwlib/timer.h                                $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 5/04/01 8:08p                                               $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   BasicTimerClass<T>::BasicTimerClass -- Constructor for basic timer class.                 *
 *   BasicTimerClass<T>::operator () -- Function operator for timer object.                    *
 *   BasicTimerClass<T>::operator long -- Conversion to long operator.                         *
 *   BasicTimerClass<T>::~BasicTimerClass -- Destructor for basic timer object.                *
 *   TTimerClass<T>::Is_Active -- Checks to see if the timer is counting.                      *
 *   TTimerClass<T>::Start -- Starts (resumes) a stopped timer.                                *
 *   TTimerClass<T>::Stop -- Stops the current timer from incrementing.                        *
 *   TTimerClass<T>::TTimerClass -- Constructor for timer class object.                        *
 *   TTimerClass<T>::operator () -- Function operator for timer object.                        *
 *   TTimerClass<T>::operator long -- Conversion operator for timer object.                    *
 *   CDTimerClass<T>::CDTimerClass -- Constructor for count down timer.                        *
 *   CDTimerClass<T>::Is_Active -- Checks to see if the timer object is active.                *
 *   CDTimerClass<T>::Start -- Starts (resumes) the count down timer.                          *
 *   CDTimerClass<T>::Stop -- Stops (pauses) the count down timer.                             *
 *   CDTimerClass<T>::operator () -- Function operator for the count down timer.               *
 *   CDTimerClass<T>::operator long -- Conversion to long operator function.                   *
 *   CDTimerClass<T>::~CDTimerClass -- Destructor for the count down timer object.             *
 *   TTimerClass<T>::Value -- Returns with the current value of the timer.                     *
 *   CDTimerClass<T>::Value -- Fetches the current value of the countdown timer.               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef TIMER_H
#define TIMER_H

#include	"noinit.h"

/*
**	This is a timer class that watches a constant rate timer (specified by the parameter
**	type class) and provides basic timer functionality. It is possible to set the start value
**	WITHOUT damaging or otherwise affecting any other timer that may be built upon the same
**	specified timer class object. Treat an object of this type as if it were a "magic" integral
**	long that automatically advances at the speed of the timer class object controlling it.
*/
// Let lint know that non-virtual destructor is OK for this class.
//lint -esym(1509,BasicTimerClass)
template<class T>
class BasicTimerClass {
 	public:
		// Constructor allows assignment as if class was integral 'long' type.
		BasicTimerClass(int set=0);
		BasicTimerClass(NoInitClass const & );

		virtual ~BasicTimerClass(void);

		// Fetch current value of timer.
		int Value(void) const {return(Timer()-Started);}

		// Conversion operator to allow consistent treatment with integral types.
		operator int(void) const;

		// Function operator to allow timer object definition to be cascaded.
		int operator () (void) const;

	protected:
		T Timer;			// Timer regulator (ticks at constant rate).
		int Started;	// Time started.
};


template<class T>
inline BasicTimerClass<T>::BasicTimerClass(NoInitClass const & )
{
};


/***********************************************************************************************
 * BasicTimerClass<T>::BasicTimerClass -- Constructor for basic timer class.                   *
 *                                                                                             *
 *    This is the constructor for the basic timer class object. It sets the timer counting     *
 *    up from zero at the rate of the controlling timer class object.                          *
 *                                                                                             *
 * INPUT:   set   -- Alternate initial start value for the counter. If not specified, then     *
 *                   the timer is assumed to start at zero and count upwards.                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
//lint -esym(1403,BasicTimerClass<class FrameTimerClass>::Timer)
//lint -esym(1403,BasicTimerClass<class SystemTimerClass>::Timer)
template<class T>
inline BasicTimerClass<T>::BasicTimerClass(int set) :
	Started(Timer()-set)
{
}


/***********************************************************************************************
 * BasicTimerClass<T>::~BasicTimerClass -- Destructor for basic timer object.                  *
 *                                                                                             *
 *    The destructor for the basic timer object doesn't have to do anything.                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline BasicTimerClass<T>::~BasicTimerClass(void)
{
}


//template<class T>
//inline unsigned long BasicTimerClass<T>::Value(void) const
//{
//	return(Timer()-Started);
//}


/***********************************************************************************************
 * BasicTimerClass<T>::operator long -- Conversion to long operator.                           *
 *                                                                                             *
 *    This conversion operator allows the basic timer object to function in much the same      *
 *    manner as the integral "long" type. One can assign a long with a timer object and the    *
 *    actual value of the timer is extracted from the object and used.                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the timer value expressed as a long.                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline BasicTimerClass<T>::operator int(void) const
{
	return(Timer()-Started);
}


/***********************************************************************************************
 * BasicTimerClass<T>::operator () -- Function operator for timer object.                      *
 *                                                                                             *
 *    This function operator allows the timer to also serve as the parameter type class for    *
 *    additional timer objects. This allows one to instantiate a controlling timer class that  *
 *    can control (e.g., turn on or off) all timers that are based upon it.                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the current timer value expressed as a long.                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline int BasicTimerClass<T>::operator () (void) const
{
	return(Timer()-Started);
}


/*
**	This timer class functions similarly to the basic timer class. In addition to the
**	normal timer operation, this class has the ability to be stopped and started at
**	will. If you have no need to start or stop the timer, then use the basic timer
**	class instead.
*/
template<class T>
class TTimerClass : public BasicTimerClass<T>
{
	using BasicTimerClass<T>::Started;
	using BasicTimerClass<T>::Timer;

 	public:
		// Constructor allows assignment as if class was integral 'long' type.
		TTimerClass(int set=0);
		TTimerClass(NoInitClass const & x);

		~TTimerClass(void) {};

		// Fetches current value of timer.
		int Value(void) const;

		// Conversion operator to allow consistent treatment with integral types.
		operator int(void) const;

		// Function operator to allow timer object definition to be cascaded.
		int operator () (void) const;

		// Stops (pauses) the timer.
		void Stop(void);

		// Starts (resumes) the timer.
		void Start(void);

		// Queries whether the timer is currently active.
		bool Is_Active(void) const;

	private:
		int Accumulated;				//	Total accumulated ticks.
};


template<class T>
inline TTimerClass<T>::TTimerClass(NoInitClass const & x) :
	BasicTimerClass<T>(x)
{
}


/***********************************************************************************************
 * TTimerClass<T>::TTimerClass -- Constructor for timer class object.                          *
 *                                                                                             *
 *    This is the constructor for the advanced timer class object. This object class can start *
 *    or stop the timer under user control.                                                    *
 *                                                                                             *
 * INPUT:   set   -- The initial value to set the timer to. If no value is specified, then     *
 *                   the timer is assumed to start from zero.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline TTimerClass<T>::TTimerClass(int set) :
	BasicTimerClass<T>(set),
	Accumulated(0)
{
}


/***********************************************************************************************
 * TTimerClass<T>::Value -- Returns with the current value of the timer.                       *
 *                                                                                             *
 *    This routine will return with the current value of the timer. It takes into account      *
 *    whether the timer has stopped or not so as to always return the correct value regardless *
 *    of that condition.                                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current value of the timer.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline int TTimerClass<T>::Value(void) const
{
	int value = Accumulated;
	if (Started != -1) {
		value += BasicTimerClass<T>::Value();
	}
	return(value);
}


/***********************************************************************************************
 * TTimerClass<T>::operator long -- Conversion operator for timer object.                      *
 *                                                                                             *
 *    This conversion operator allows this timer object to function as an "rvalue" of a "long" *
 *    type. This is consistent with the integral "long" value. It is possible to assign a      *
 *    timer object to a long and have the long initialized with the current value of the       *
 *    timer.                                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current time value expressed as a long.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline TTimerClass<T>::operator int(void) const
{
	int value = Accumulated;
	if (Started != -1) {
		value += BasicTimerClass<T>::Value();
	}
	return(value);
}


/***********************************************************************************************
 * TTimerClass<T>::operator () -- Function operator for timer object.                          *
 *                                                                                             *
 *    This function operator for the timer class allows this timer class to be used as the     *
 *    template parameter for other timer class objects. With this ability, one can control     *
 *    several timers (e.g., start or stop them) by using a single controlling timer class      *
 *    that other timers are instantiated from.                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current time expressed as a long.                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline int TTimerClass<T>::operator () (void) const
{
	int value = Accumulated;
	if (Started != -1) {
		value += BasicTimerClass<T>::Value();
	}
	return(value);
}


/***********************************************************************************************
 * TTimerClass<T>::Stop -- Stops the current timer from incrementing.                          *
 *                                                                                             *
 *    This routine will stop (pause) the timer from further increments. To cause the timer     *
 *    to begin anew, call the Start() function.                                                *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/05/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
void TTimerClass<T>::Stop(void)
{
	if (Started != -1) {
		Accumulated += BasicTimerClass<T>::Value();
		Started = -1;
	}
}


/***********************************************************************************************
 * TTimerClass<T>::Start -- Starts (resumes) a stopped timer.                                  *
 *                                                                                             *
 *    This routine will resume a timer that was previously stopped with the Stop() function.   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
void TTimerClass<T>::Start(void)
{
	if (Started == -1) {
		Started = Timer();
	}
}


/***********************************************************************************************
 * TTimerClass<T>::Is_Active -- Checks to see if the timer is counting.                        *
 *                                                                                             *
 *    Since this timer can be paused, this routine is used to examine the timer to see if it   *
 *    is currently paused or active. If the timer is active, then the return value will be     *
 *    true.                                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is this timer currently active?                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool TTimerClass<T>::Is_Active(void) const
{
	return(Started != -1);
}


/*
**	This timer counts down from the specified (or constructed) value down towards zero.
**	The countdown rate is controlled by the timer object specified. This timer object can
**	be started or stopped. It can also be tested to see if it has expired or not. An expired
**	count down timer is one that has value of zero. You can treat this class object as if it
**	were an integral "magic" long that automatically counts down toward zero.
*/
template<class T>
class CDTimerClass : public BasicTimerClass<T>
{
	using BasicTimerClass<T>::Started;
	using BasicTimerClass<T>::Timer;

	public:
		// Constructor allows assignment as if class was integral 'long' type.
		CDTimerClass(int set=0);
		CDTimerClass(NoInitClass const & x);

		~CDTimerClass(void);

		// Fetches current value of count down timer.
		int Value(void) const;

		// Conversion operator to allow consistent treatment with integral types.
		operator int(void) const;

		// Function operator to allow timer object definition to be cascaded.
		int operator () (void) const;

		// Stops (pauses) the timer.
		void Stop(void);

		// Starts (resumes) the timer.
		void Start(void);

		// Queries whether the timer is currently active.
		bool Is_Active(void) const;

	private:
		int DelayTime;			// Ticks remaining before countdown timer expires.
};


template<class T>
inline CDTimerClass<T>::CDTimerClass(NoInitClass const & x) :
	BasicTimerClass<T>(x)
{
}


/***********************************************************************************************
 * CDTimerClass<T>::CDTimerClass -- Constructor for count down timer.                          *
 *                                                                                             *
 *    This is the constructor for the count down timer object. The optional starting value     *
 *    can be used to initiate the timer. Because of this constructor it is possible to assign  *
 *    a long to a count down timer object in order to begin the countdown process.             *
 *                                                                                             *
 * INPUT:   set   -- The initial starting value for the countdown timer.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline CDTimerClass<T>::CDTimerClass(int set) :
	BasicTimerClass<T>(0),
	DelayTime(set)
{
}


/***********************************************************************************************
 * CDTimerClass<T>::~CDTimerClass -- Destructor for the count down timer object.               *
 *                                                                                             *
 *    The destructor for the count down timer object does nothing.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline CDTimerClass<T>::~CDTimerClass(void)
{
}


/***********************************************************************************************
 * CDTimerClass<T>::Value -- Fetches the current value of the countdown timer.                 *
 *                                                                                             *
 *    Use this routine to fetch the current value of the timer. It takes into consideration    *
 *    whether the timer has been stopped or not. It returns the correct value regardless of    *
 *    this condition.                                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the correct value of this count down timer.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline int CDTimerClass<T>::Value(void) const
{
	int remain = DelayTime;
	if (Started != -1) {
		int value = BasicTimerClass<T>::Value();
		if (value < remain) {
			return(remain - value);
		} else {
			return(0);
		}
	}
	return(remain);
}


/***********************************************************************************************
 * CDTimerClass<T>::operator long -- Conversion to long operator function.                     *
 *                                                                                             *
 *    This conversion operator allows the count down timer object to be used as if it were     *
 *    a "magic" long that automatically counted downward at the controller class tick rate.    *
 *    The count down object can be used in any place that an rvalue long could be used.        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current count down time expressed in the form of a long value.    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline CDTimerClass<T>::operator int(void) const
{
	int remain = DelayTime;
	if (Started != 0xFFFFFFFFU) {
		int value = BasicTimerClass<T>::Value();
		if (value < remain) {
			return(remain - value);
		} else {
			return(0);
		}
	}
	return(remain);
}


/***********************************************************************************************
 * CDTimerClass<T>::operator () -- Function operator for the count down timer.                 *
 *                                                                                             *
 *    This is the function operator for the count down timer object. By supporting this        *
 *    function operator, this class (or one derived from this class) could be used as the      *
 *    controlling timer to the timer templates.                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current count down time expressed in the form of a long.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline int CDTimerClass<T>::operator () (void) const
{
	int remain = DelayTime;
	if (Started != -1) {
		int value = BasicTimerClass<T>::Value();
		if (value < remain) {
			return(remain - value);
		} else {
			return(0);
		}
	}
	return(remain);
}


/***********************************************************************************************
 * CDTimerClass<T>::Stop -- Stops (pauses) the count down timer.                               *
 *                                                                                             *
 *    This routine is used to stop (pause) the count down timer object. A timer object paused  *
 *    in this fashion will be resumed by a call to Start() or by assigning a new count down    *
 *    value to the timer.                                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
void CDTimerClass<T>::Stop(void)
{
	if (Started != -1) {
		DelayTime = *this;
		Started = -1;
	}
}


/***********************************************************************************************
 * CDTimerClass<T>::Start -- Starts (resumes) the count down timer.                            *
 *                                                                                             *
 *    This routine is used to start (resume) the count down timer that was previously stopped  *
 *    with the Stop() function. The timer will also resume when a new timer value is assigned. *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
void CDTimerClass<T>::Start(void)
{
	if (Started == -1) {
		Started = Timer();
	}
}


/***********************************************************************************************
 * CDTimerClass<T>::Is_Active -- Checks to see if the timer object is active.                  *
 *                                                                                             *
 *    Because the timer object counting can be stopped, this routine is used to determine      *
 *    if the timer is currently paused or active.                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is the timer currently active?                                               *
 *                                                                                             *
 * WARNINGS:   Note that if the timer has counted down to zero, then it may be active, but     *
 *             the value will, naturally, not change.                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
inline bool CDTimerClass<T>::Is_Active(void) const
{
	return(Started != -1);
}


#endif
