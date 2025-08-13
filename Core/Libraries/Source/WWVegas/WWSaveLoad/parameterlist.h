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
 *                     $Archive:: /Commando/Code/wwsaveload/parameterlist.h                   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/06/99 2:52p                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif




#ifndef __PARAMETER_LIST_H
#define __PARAMETER_LIST_H

#include "always.h"
#include "Vector.H"
#include "parameter.h"
#include "wwdebug.h"

//////////////////////////////////////////////////////////////////////////////////
//
//	ParameterListClass
//
//////////////////////////////////////////////////////////////////////////////////
class ParameterListClass : public DynamicVectorClass<ParameterClass *>
{
	using DynamicVectorClass<ParameterClass *>::Vector;

public:

	/////////////////////////////////////////////////////////////////////
	// Public constructurs/destructors
	/////////////////////////////////////////////////////////////////////
	~ParameterListClass (void);

	/////////////////////////////////////////////////////////////////////
	// Public methods
	/////////////////////////////////////////////////////////////////////
	void			Add (void *data, const char *param_name, ParameterClass::Type type);
	void			Add (ParameterClass *parameter);

protected:

	/////////////////////////////////////////////////////////////////////
	// Protected methods
	/////////////////////////////////////////////////////////////////////
	void			Free_Parameters (void);

private:

	/////////////////////////////////////////////////////////////////////
	// Private member data
	/////////////////////////////////////////////////////////////////////
	//DynamicVectorClass<ParameterClass *>		m_Parameters;
};


/////////////////////////////////////////////////////////////////////
// ~ParameterListClass
/////////////////////////////////////////////////////////////////////
inline
ParameterListClass::~ParameterListClass (void)
{
	Free_Parameters ();
	return ;
}

/////////////////////////////////////////////////////////////////////
// Add
/////////////////////////////////////////////////////////////////////
inline void
ParameterListClass::Add (void *data, const char *param_name, ParameterClass::Type type)
{
	//
	//	Create a new parameter object
	//
	ParameterClass *new_param = ParameterClass::Construct (type, data, param_name);

	//
	//	Add the new paramter object to our list
	//
	WWASSERT (new_param != NULL);
	if (new_param != NULL) {
		DynamicVectorClass<ParameterClass *>::Add (new_param);
	}

	return ;
}

/////////////////////////////////////////////////////////////////////
// Add
/////////////////////////////////////////////////////////////////////
inline void
ParameterListClass::Add (ParameterClass *new_param)
{
	//
	//	Add the new paramter object to our list
	//
	if (new_param != NULL) {
		DynamicVectorClass<ParameterClass *>::Add (new_param);
	}

	return ;
}

/////////////////////////////////////////////////////////////////////
// Free_Parameters
/////////////////////////////////////////////////////////////////////
inline void
ParameterListClass::Free_Parameters (void)
{
	for (int index = 0; index < Count (); index ++) {
		ParameterClass *param = Vector[index];

		//
		//	Free the parameter object
		//
		if (param != NULL) {
			delete param;
		}
	}

	Delete_All();
//	m_Parameters.Delete_All ();
	return ;
}


#endif //__PARAMETER_LIST_H

