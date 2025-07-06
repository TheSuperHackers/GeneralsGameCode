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
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwsaveload/postloadable.h                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/09/01 11:46a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef POSTLOADABLE_H
#define POSTLOADABLE_H


//////////////////////////////////////////////////////////////////////////////////
//
//	PostLoadableClass
//
//	PostLoadableClass allows a lower level for non-Persist objects to On_Post_Load
// without requiring Get_Factory or other requirements of PersistClass.  Objects
// derived from this class can be added to the post-load list in the SaveLoadSystem
//
//////////////////////////////////////////////////////////////////////////////////
class PostLoadableClass
{
public:
	PostLoadableClass(void) : IsPostLoadRegistered(false)						{ }
	virtual ~PostLoadableClass(void)													{ }

	virtual void						On_Post_Load (void)							{ }

	bool									Is_Post_Load_Registered(void)				{ return IsPostLoadRegistered; }
	void									Set_Post_Load_Registered(bool onoff)	{ IsPostLoadRegistered = onoff; }

private:

	bool									IsPostLoadRegistered;
};



#endif

