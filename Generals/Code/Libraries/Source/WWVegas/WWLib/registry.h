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
 *                 Project Name : Commando / G Library                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/registry.h                             $*
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 8/16/01 11:28a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef REGISTRY_H
#define REGISTRY_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#include "Vector.H"
#include "wwstring.h"
#include "widestring.h"

/*
**
*/
class	RegistryClass {
public:
	// Constructor & Destructor
	RegistryClass( const char * sub_key );
	~RegistryClass( void );

	bool	Is_Valid( void )		{ return IsValid; }

	// Int data type access
	int	Get_Int( const char * name, int def_value = 0 );
	void	Set_Int( const char * name, int value );

	// Bool data type access
	bool	Get_Bool( const char * name, bool def_value = false );
	void	Set_Bool( const char * name, bool value );

	// Float data type access
	float	Get_Float( const char * name, float def_value = 0.0f );
	void	Set_Float( const char * name, float value );

	// String data type access
	char *Get_String( const char * name, char *value, int value_size,
      const char * default_string = NULL );
	void	Get_String( const char * name, StringClass &string, const char *default_string = NULL);
	void	Set_String( const char * name, const char *value );

	// Wide string data type access
	void	Get_String( const WCHAR * name, WideStringClass &string, const WCHAR *default_string = NULL);
	void	Set_String( const WCHAR * name, const WCHAR *value );

	// Binary data type access
	void	Get_Bin( const char * name, void *buffer, int buffer_size );
	int	Get_Bin_Size( const char * name );
	void	Set_Bin( const char * name, const void *buffer, int buffer_size );

	// Value enumeration support
	void	Get_Value_List( DynamicVectorClass<StringClass> &list );

	// Delete support
	void	Delete_Value( const char * name);
	void	Deleta_All_Values( void );

private:
	int	Key;
	bool	IsValid;
};

#endif // REGISTRY_H
