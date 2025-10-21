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
 *                     $Archive:: /Commando/Code/wwlib/ini.h                                  $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/14/01 1:32a                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef INI_H
#define INI_H

#include <wchar.h>

//#include	"listnode.h"
//#include	"trect.h"
//#include	"index.h"

//#include	"pipe.h"
//#include	"pk.h"
//#include	"straw.h"
//#include	"wwfile.h"

class FileClass;
class Straw;
class Pipe;
class StringClass;
class WideStringClass;
class FileFactoryClass;

struct INISection;
struct INIEntry;

template<class T> class TPoint3D;
template<class T> class TPoint2D;
template<class T> class TRect;
template<class T> class List;
template<class T, class U> class IndexClass;

#ifndef NULL
#define NULL 0L
#endif

/*
**	This is an INI database handler class. It handles a database with a disk format identical
**	to the INI files commonly used by Windows.
*/
class INIClass {
	public:
		INIClass(void);
		INIClass(FileClass & file);
		INIClass(const char *filename);

		virtual ~INIClass(void);

		/*
		** This setting allows you to control the behavior of loading blank entries.
		** If you set this to true, a blank entry (ie. "foo=") will be loaded with
		** a value of " ". If set to false, blank entries will be ignored.
		** The default behavior is to ignore blank entries.
		** This is a static method, because in general this is an application-level
		** decision as opposed to a per-ini file decision.
		*/
		static void Keep_Blank_Entries(bool keep_blanks);

		/*
		**	Fetch and store INI data.
		*/
		int Load(FileClass & file);
		int Load(Straw & file);
		int Load(const char *filename);
		int Save(FileClass & file) const;
		int Save(Pipe & file) const;
		int Save(const char *filename) const;

		/*
		** Fetch the name of the INI file (if it wasn't created from a Straw).
		*/
		const char * Get_Filename(void) const;

		/*
		**	Erase all data within this INI file manager.
		*/
		bool Clear(char const * section = NULL, char const * entry = NULL);

//		int Line_Count(char const * section) const;
		bool Is_Loaded(void) const;
		int Size(void) const;
		bool Is_Present(char const * section, char const * entry = NULL) const {if (entry == 0) return(Find_Section(section) != 0);return(Find_Entry(section, entry) != 0);}

		/*
		**	Fetch the number of sections in the INI file or verify if a specific
		**	section is present.
		*/
		int Section_Count(void) const;
		bool Section_Present(char const * section) const {return(Find_Section(section) != NULL);}

		/*
		**	Fetch the number of entries in a section or get a particular entry in a section.
		*/
		int Entry_Count(char const * section) const;
		char const * Get_Entry(char const * section, int index) const;

		/*
		**	Cound how many entries with the indicated prefix followed by a number exist in the section.
		*/
		unsigned Enumerate_Entries(const char * section, const char * entry_prefix, unsigned startnumber = 0, unsigned endnumber = (unsigned) -1);

		/*
		**	Get the various data types from the section and entry specified.
		*/
		bool Get_Bool(char const * section, char const * entry, bool defvalue=false) const;
		float Get_Float(char const * section, char const * entry, float defvalue=0.0f) const;
		double Get_Double(char const * section, char const * entry, double defvalue=0.0) const;
		int Get_Hex(char const * section, char const * entry, int defvalue=0) const;
		int Get_Int(char const * section, char const * entry, int defvalue=0) const;
		int Get_String(char const * section, char const * entry, char const * defvalue, char * buffer, int size) const;
		const StringClass& Get_String(StringClass& new_string, char const * section, char const * entry, char const * defvalue="") const;
		const WideStringClass& Get_Wide_String(WideStringClass& new_string, char const * section, char const * entry, wchar_t const * defvalue=L"") const;
		int Get_List_Index(char const * section, char const * entry, int const defvalue, char *list[]);
		int *	Get_Alloc_Int_Array(char const * section, char const * entry, int listend);
		int Get_Int_Bitfield(char const * section, char const * entry, int defvalue, char *list[]);
		char *Get_Alloc_String(char const * section, char const * entry, char const * defvalue) const;
		int Get_TextBlock(char const * section, char * buffer, int len) const;
		int Get_UUBlock(char const * section, void * buffer, int len) const;
		int Get_UUBlock(char const * section, char const *entry, void * block, int len) const;
		TRect<int> const Get_Rect(char const * section, char const * entry, TRect<int> const & defvalue) const;
		TPoint3D<int> const Get_Point(char const * section, char const * entry, TPoint3D<int> const & defvalue) const;
		TPoint2D<int> const Get_Point(char const * section, char const * entry, TPoint2D<int> const & defvalue) const;
		TPoint3D<float> const Get_Point(char const * section, char const * entry, TPoint3D<float> const & defvalue) const;
		TPoint2D<float> const Get_Point(char const * section, char const * entry, TPoint2D<float> const & defvalue) const;



		/*
		**	Put a data type to the section and entry specified.
		*/
		bool Put_Bool(char const * section, char const * entry, bool value);
		bool Put_Float(char const * section, char const * entry, float number);
		bool Put_Double(char const * section, char const * entry, double number);
		bool Put_Hex(char const * section, char const * entry, int number);
		bool Put_Int(char const * section, char const * entry, int number, int format=0);
		bool Put_String(char const * section, char const * entry, char const * string);
		bool Put_TextBlock(char const * section, char const * text);
		bool Put_UUBlock(char const * section, void const * block, int len);
		bool Put_UUBlock(char const * section, char const *entry, void const * block, int len);
		bool Put_Rect(char const * section, char const * entry, TRect<int> const & value);
		bool Put_Point(char const * section, char const * entry, TPoint3D<int> const & value);
		bool Put_Point(char const * section, char const * entry, TPoint3D<float> const & value);
		bool Put_Point(char const * section, char const * entry, TPoint2D<int> const & value);
		bool Put_Wide_String(char const * section, char const * entry, wchar_t const * string);

//	protected:

		// Declared here, but assigned a value in ini.cpp. This is done so that we
		// can change this value in the future without needed to rebuild the entire world.
		static const int MAX_LINE_LENGTH;

		/*
		**	Access to the list of all sections within this INI file.
		*/
		List<INISection *> & Get_Section_List() { return * SectionList; }

		IndexClass<int, INISection *> & Get_Section_Index() { return * SectionIndex; }


		/*
		**	Utility routines to help find the appropriate section and entry objects.
		*/
		INISection * Find_Section(char const * section) const;
		INIEntry * Find_Entry(char const * section, char const * entry) const;
		static void Strip_Comments(char * buffer);

		// the CRC function is used to produce hopefully unique values for
		// each of the entries in a section. Debug messages will be produced
		// if redundant CRCs are generated for a particular INI file.
		static int CRC(const char *string);

		// the DuplicateCRCError function is used by Put_String and Load in the case
		// that an entry's CRC matches one already in the database.
		void DuplicateCRCError(const char *message, const char *section, const char *entry);

	private:

		/*
		**	These functions are used to allocate and free the section list and section index
		*/
		void Initialize(void);
		void Shutdown(void);

		/*
		**	This is the list of all sections within this INI file.
		*/
		List<INISection *> * SectionList;

		IndexClass<int, INISection *> * SectionIndex;

		/*
		**	Ensure that the copy constructor and assignment operator never exist.
		*/
		INIClass(INIClass const & rvalue);
		INIClass operator = (INIClass const & rvalue);

		/*
		** The name of the file we were loaded from (if applicable).
		*/
		mutable char * Filename;

		/*
		** The flag used to control the blank entry loading behavior.
		*/
		static bool KeepBlankEntries;
};

#endif
