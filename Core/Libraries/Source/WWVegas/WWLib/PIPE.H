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
 *                     $Archive:: /G/wwlib/PIPE.H                                             $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/02/99 12:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

/*
**	A "push through" pipe interface abstract class used for such purposes as compression
**	and translation of data. In STL terms, this is functionally similar to an output
**	iterator but with a few enhancements. A pipe class object that is not derived into
**	another useful class serves only as a pseudo null-pipe. It will accept data but
**	just throw it away but pretend that it sent it somewhere.
*/
class Pipe
{
	public:
		Pipe(void) : ChainTo(0), ChainFrom(0) {}
		virtual ~Pipe(void);

		virtual int Flush(void);
		virtual int End(void) {return(Flush());}
		virtual void Put_To(Pipe * pipe);
		void Put_To(Pipe & pipe) {Put_To(&pipe);}
		virtual int Put(void const * source, int slen);

		/*
		**	Pointer to the next pipe segment in the chain.
		*/
		Pipe * ChainTo;
		Pipe * ChainFrom;

	private:

		/*
		**	Disable the copy constructor and assignment operator.
		*/
		Pipe(Pipe & rvalue);
		Pipe & operator = (Pipe const & pipe);
};
