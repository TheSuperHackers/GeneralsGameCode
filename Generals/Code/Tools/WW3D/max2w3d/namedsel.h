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

/* $Header: /Commando/Code/Tools/max2w3d/namedsel.h 4     10/28/97 6:08p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - WWSkin                                      * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/namedsel.h                     $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/21/97 2:05p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 4                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef NAMEDSEL_H
#define NAMEDSEL_H


#include "max.h"

/*
** This is a class for containing bit arrays for
** the named selection sets.  I stole it from
** the Edit Mesh modifier code...
** It is basically a dynamically sized array
** of bitarrays.
*/
class NamedSelSetList 
{
public:
	Tab<BitArray*>			Sets;
	Tab<TSTR*>				Names;
	
	~NamedSelSetList();

	BitArray & operator[](int i) { return *Sets[i]; }
	int Count() { return Sets.Count(); }
	
	int  Find_Set(TSTR & setname);
	void Delete_Set(int i);
	void Delete_Set(TSTR & setname);
	void Reset(void);
	void Append_Set(BitArray & nset,TSTR & setname);
	
	IOResult Load(ILoad * iload);
	IOResult Save(ISave * isave);
	IOResult Load_Set(ILoad * iload);
	
	void Set_Size(int size);
	NamedSelSetList & operator=(NamedSelSetList & from);

	enum {
		NAMED_SEL_SET_CHUNK =   0x0021,
		NAMED_SEL_BITS_CHUNK =	0x0022,
		NAMED_SEL_NAME_CHUNK =	0x0023
	};
};


#endif /*NAMEDSEL_H*/
