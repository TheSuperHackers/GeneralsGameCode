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
 *                     $Archive:: /Commando/Code/ww3d2/proxy.h           $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "wwstring.h"
#include "matrix3d.h"


//////////////////////////////////////////////////////////////////////////////////
//
//	ProxyClass
//
//////////////////////////////////////////////////////////////////////////////////
class ProxyClass
{
public:

	///////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////
	ProxyClass (void) { }
	ProxyClass (const char *name, const Matrix3D &tm) :
		Name (name),
		Transform (tm) { }

	///////////////////////////////////////////////////////////
	//	Public operators
	///////////////////////////////////////////////////////////
	bool operator== (const ProxyClass &src);
	bool operator!= (const ProxyClass &src);

	///////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////

	//
	//	Accessors
	//
	const char *			Get_Name (void) const					{ return Name; }
	void						Set_Name (const char *name)			{ Name = name; }

	const Matrix3D &		Get_Transform (void) const				{ return Transform; }
	void						Set_Transform (const Matrix3D &tm)	{ Transform = tm; }

private:

	///////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////
	StringClass				Name;
	Matrix3D					Transform;
};


///////////////////////////////////////////////////////////
//	operator==
///////////////////////////////////////////////////////////
inline bool
ProxyClass::operator== (const ProxyClass &src)
{
	return ((Name == src.Name) && (Transform == src.Transform));
}


///////////////////////////////////////////////////////////
//	operator!=
///////////////////////////////////////////////////////////
inline bool
ProxyClass::operator!= (const ProxyClass &src)
{
	return ((Name != src.Name) || (Transform != src.Transform));
}
