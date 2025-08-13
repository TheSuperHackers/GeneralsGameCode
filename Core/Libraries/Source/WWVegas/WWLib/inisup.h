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
 *                 Project Name : WWLib                                                        *
 *                                                                                             *
 *                     $Archive:: /G/wwlib/inisup.h                                           $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/02/99 11:59a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/*
**	This header defines generally unused member structures used by the INI class.
**	Previously these were member structures of the INI class but they were separated
**	to help reduce header dependancies. -ehc
*/

#include	"LISTNODE.H"
#include	"INDEX.H"
#include "CRC.H"


/*
**	The value entries for the INI file are stored as objects of this type.
**	The entry identifier and value string are combined into this object.
*/
struct INIEntry : public Node<INIEntry *> {
	INIEntry(char * entry = NULL, char * value = NULL) : Entry(entry), Value(value) {}
	~INIEntry(void);
//	~INIEntry(void) {free(Entry);Entry = NULL;free(Value);Value = NULL;}
//	int Index_ID(void) const {return(CRCEngine()(Entry, strlen(Entry)));};
	int Index_ID(void) const { return CRC::String(Entry);};

	char * Entry;
	char * Value;
};

/*
**	Each section (bracketed) is represented by an object of this type. All entries
**	subordinate to this section are attached.
*/
struct INISection : public Node<INISection *> {
		INISection(char * section) : Section(section) {}
		~INISection(void);
//		~INISection(void) {free(Section);Section = 0;EntryList.Delete();}
		INIEntry * Find_Entry(char const * entry) const;
//		int Index_ID(void) const {return(CRCEngine()(Section, strlen(Section)));};
		int Index_ID(void) const { return CRC::String(Section); };

		char * Section;
		List<INIEntry *> EntryList;
		IndexClass<int, INIEntry *> EntryIndex;

	private:
		INISection(INISection const & rvalue);
		INISection& operator = (INISection const & rvalue);
};


