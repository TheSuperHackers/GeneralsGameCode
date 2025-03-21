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
 *                     $Archive:: /Commando/Library/SHASTRAW.H                                $* 
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

#ifndef SHASTRAW_H
#define SHASTRAW_H


#include	"sha.h"
#include	"STRAW.H"

/*
**	This class serves as a straw that generates a Secure Hash from the data stream that flows
**	through it. It doesn't modify the data stream in any fashion.
*/
class SHAStraw : public Straw
{
	public:
		SHAStraw(void) : IsDisabled(false) {}
		virtual int Get(void * source, int slen);

		void Disable(void) {IsDisabled = true;}
		void Enable(void) {IsDisabled = false;}

		// Fetch the SHA hash value (stored in result buffer -- 20 bytes long).
		int Result(void * result) const;

	protected:
		bool IsDisabled;

		SHAEngine SHA;

	private:
		SHAStraw(SHAStraw & rvalue);
		SHAStraw & operator = (SHAStraw const & straw);
};


#endif
