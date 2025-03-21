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
 *                     $Archive:: /Commando/Code/wwlib/textfile.h                             $* 
 *                                                                                             * 
 *                      $Author:: Patrick                   $*
 *                                                                                             * 
 *                     $Modtime:: 5/31/00 9:22a                                               $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __TEXT_FILE_H
#define __TEXT_FILE_H


#include "RAWFILE.H"

///////////////////////////////////////////////////////////////////////////////
//	Forward declarations
///////////////////////////////////////////////////////////////////////////////
class StringClass;


///////////////////////////////////////////////////////////////////////////////
//
//	TextFileClass
//
//	This file class simply adds text file support to the RawFileClass object.
// It is used for reading/writing lines of text (looking for CR/LF or LF's).
//
///////////////////////////////////////////////////////////////////////////////
class TextFileClass : public RawFileClass
{
public:

	/////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	/////////////////////////////////////////////////////////////////
	TextFileClass (void);
	TextFileClass (char const *filename);	
	TextFileClass (const TextFileClass &src);
	virtual ~TextFileClass (void);

	/////////////////////////////////////////////////////////////////
	//	Public operators
	/////////////////////////////////////////////////////////////////
	const TextFileClass &operator= (const TextFileClass &src);

	/////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////
	bool			Read_Line (StringClass &string);
	bool			Write_Line (const StringClass &string);
};


#endif //__TEXT_FILE_H

