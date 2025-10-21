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
 *                     $Archive:: /Commando/Code/wwlib/ffactory.h                     $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 9/07/01 5:30p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef FFACTORY_H
#define FFACTORY_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#include "mutex.h"
#include "Vector.H"
#include "wwstring.h"

/*
**
*/
#include	"RAWFILE.H"
class	FileClass;

/*
** FileFactoryClass is a pure virtual class used to
** create FileClasses.
*/
class	FileFactoryClass {

public:
	virtual ~FileFactoryClass(void){};
	virtual FileClass * Get_File( char const *filename ) = 0;
	virtual void Return_File( FileClass *file ) = 0;
};




//
// Handy auto pointer class.  Prevents you from having to call Return_File manually
//
class file_auto_ptr
{
public:
	explicit	file_auto_ptr(FileFactoryClass *fac, const char *filename);
				~file_auto_ptr();

	operator FileClass*(void) const
		{return (get()); }

	FileClass& operator*() const
		{return (*get()); }

	FileClass *operator->() const
		{return (get()); }

	FileClass *get() const
		{return (_Ptr); }

private:
	// prevent these from getting auto-generated or used
						file_auto_ptr(const file_auto_ptr &other);
	file_auto_ptr	&operator=(const file_auto_ptr &other);


	FileClass			*_Ptr;
	FileFactoryClass	*_Fac;
};







/*
** RawFileFactoryClass is a derived FileFactoryClass which
** gives RawFileClass objects
*/
class	RawFileFactoryClass {
public:
	RawFileClass * Get_File( char const *filename );
	void Return_File( FileClass *file );
};

#define no_SIMPLE_FILE
/*
** SimpleFileFactoryClass is a slightly more capable derivative of
** FileFactoryClass which adds some simple extra functionality. It supports a
** current subdirectory, and also adds some logging capabilities. Note that
** it currently creates BufferedFileClass objects instead of RawFileClass
** objects.
*/
class	SimpleFileFactoryClass : public FileFactoryClass {

public:
	SimpleFileFactoryClass( void );
	~SimpleFileFactoryClass( void )	{}

	virtual FileClass *	Get_File( char const *filename );
	virtual void			Return_File( FileClass *file );

	// sub_directory may be a semicolon seperated search path.  New files will always
	//   go in the last dir in the path.
	void						Get_Sub_Directory( StringClass& new_dir ) const;
	void						Set_Sub_Directory( const char * sub_directory );
	void						Prepend_Sub_Directory( const char * sub_directory );
	void						Append_Sub_Directory( const char * sub_directory );
	bool						Get_Strip_Path( void ) const								{ return IsStripPath; }
	void						Set_Strip_Path( bool set )									{ IsStripPath = set; }
	void						Reset_Sub_Directory( void )								{ SubDirectory = ""; }

protected:
	StringClass				SubDirectory;
	bool						IsStripPath;

	// Mutex must be mutable because const functions lock on it.
	mutable CriticalSectionClass	Mutex;
};


extern FileFactoryClass	*	_TheFileFactory;
extern RawFileFactoryClass	*	_TheWritingFileFactory;

// No simple file factory.  jba.
// (gth) re-enabling this because w3d view uses it
extern SimpleFileFactoryClass	*	_TheSimpleFileFactory;

#endif
