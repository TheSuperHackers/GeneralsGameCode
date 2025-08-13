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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Library/STIMER.H                                  $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef STIMER_H
#define STIMER_H

/****************************************************************************
**	Timer constants. These are used when setting the countdown timer.
**	Note that this is based upon a timer that ticks every 60th of a second.
*/
#define	TIMER_SECOND			60
#define	TIMER_MINUTE			(TIMER_SECOND*60)

#define	FADE_PALETTE_FAST		(TIMER_SECOND/8)
#define	FADE_PALETTE_MEDIUM	(TIMER_SECOND/4)
#define	FADE_PALETTE_SLOW		(TIMER_SECOND/2)

#define	TICKS_PER_SECOND		15
#define	TICKS_PER_MINUTE		(TICKS_PER_SECOND * 60)
#define	TICKS_PER_HOUR			(TICKS_PER_MINUTE * 60)

#define	GRAYFADETIME			(1 * TICKS_PER_SECOND)


class SystemTimerClass
{
	public:
		long operator () (void) const;
		operator long (void) const;
};

#endif
