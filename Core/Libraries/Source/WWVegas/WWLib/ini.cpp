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
 *                     $Archive:: /Commando/Code/wwlib/ini.cpp                                $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/14/01 1:33a                                              $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/ini.cpp                                $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/14/01 1:33a                                              $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"b64pipe.h"
#include	"b64straw.h"
#include	"cstraw.h"
#include "INI.h"
#include	"readline.h"
#include	"trim.h"
#include	"win.h"
#include "XPIPE.h"
#include "XSTRAW.h"
#include <malloc.h>
#ifdef _UNIX
#include <ctype.h>
#endif
#include "RAWFILE.h"
#include "ffactory.h"

// recently transferred from ini.h
#include "inisup.h"
#include	"trect.h"
#include "WWFILE.h"
#include "PIPE.h"
#include	"wwstring.h"
#include "widestring.h"
#include "nstrdup.h"


// Instance of the static variables.
bool INIClass::KeepBlankEntries = false;
const int INIClass::MAX_LINE_LENGTH = 4096;


INIEntry::~INIEntry()
{
	free(Entry);
	Entry = nullptr;
	free(Value);
	Value = nullptr;
}

INISection::~INISection()
{
	free(Section);
	Section = nullptr;
	EntryList.Delete();
}

bool INIClass::Is_Loaded() const
{
	return(!SectionList->Is_Empty());
}

void INIClass::Initialize()
{
	SectionList = W3DNEW List<INISection *> ();
	SectionIndex = W3DNEW IndexClass<int, INISection *> ();
	Filename = nstrdup("<unknown>");
}

void INIClass::Shutdown()
{
	delete SectionList;
	delete SectionIndex;
	delete[] Filename;
}

INIClass::INIClass()
:	Filename(nullptr)
{
	Initialize();
}

INIClass::INIClass(FileClass & file)
:	Filename(nullptr)
{
	Initialize();
	Load(file);
}



INIClass::INIClass(const char *filename)
:	Filename(nullptr)
{
	Initialize();
	FileClass *file=_TheFileFactory->Get_File(filename);
	if ( file ) {
		Load(*file);
		_TheFileFactory->Return_File(file);
	}
}



INIClass::~INIClass()
{
	Clear();
	Shutdown();
}


// Clears out a section (or all sections) of the INI data. If no section is specified, then the
// entire INI data is cleared out. If an entry is specified, then only this specific entry (if
// found) will be cleared.
bool INIClass::Clear(char const * section, char const * entry)
{
	if (section == nullptr) {
		SectionList->Delete();
		SectionIndex->Clear();

		delete[] Filename;
		Filename = nstrdup("<unknown>");
	} else {
		INISection * secptr = Find_Section(section);
		if (secptr != nullptr) {
			if (entry != nullptr) {
				INIEntry * entptr = secptr->Find_Entry(entry);
				if (entptr != nullptr) {
					/*
					**	Remove the entry from the entry index list.
					*/
					secptr->EntryIndex.Remove_Index(entptr->Index_ID());

					delete entptr;
				}
			} else {
				/*
				**	Remove this section index from the section index list.
				*/
				SectionIndex->Remove_Index(secptr->Index_ID());

				delete secptr;
			}
		}
	}

	return(true);
}


// Returns the name of the INI file (if available - "<unknown>" otherwise).
const char * INIClass::Get_Filename () const
{
	return Filename;
}


// WARNING: This routine allocates memory.
int INIClass::Load(FileClass & file)
{
	FileStraw fs(file);
	delete[] Filename;
	Filename = nstrdup(file.File_Name());
	return(Load(fs));
}



// WARNING: This routine allocates memory.
int INIClass::Load(const char *filename)
{
	file_auto_ptr file(_TheFileFactory, filename);
	int retval=Load(*file);
	delete[] Filename;
	Filename = nstrdup(filename);

	return(retval);
}



// Merges with any existing INI data. Fails on duplicate entry CRCs. Treats "foobar=" as a valid
// entry with value " " if KeepBlankEntries is set.
int INIClass::Load(Straw & ffile)
{
	bool end_of_file = false;
	char buffer[MAX_LINE_LENGTH];

	/*
	**	Determine if the INI database has preexisting entries. If it does,
	**	then the slower merging method of loading is required.
	*/
	bool merge = false;
	if (Section_Count() > 0) {
		merge = true;
	}

	CacheStraw file;
	file.Get_From(ffile);

	/*
	**	Prescan until the first section is found.
	*/
	while (!end_of_file) {
		Read_Line(file, buffer, sizeof(buffer), end_of_file);
		if (end_of_file) return(false);
		if (buffer[0] == '[' && strchr(buffer, ']') != nullptr) break;
	}

	if (merge) {

		/*
		**	Process a section. The buffer is prefilled with the section name line.
		*/
		while (!end_of_file) {

			/*
			**	Fetch the section name. Preserve it while the section's entries are
			**	being parsed.
			*/
			buffer[0] = ' ';
			char * ptr = strchr(buffer, ']');
			if (ptr != nullptr) *ptr = '\0';
			strtrim(buffer);
			char section[64];
			strlcpy(section, buffer, ARRAY_SIZE(section));

			/*
			**	Read in the entries of this section.
			*/
			while (!end_of_file) {

				/*
				**	If this line is the start of another section, then bail out
				**	of the entry loop and let the outer section loop take
				**	care of it.
				*/
				int len = Read_Line(file, buffer, sizeof(buffer), end_of_file);
				if (buffer[0] == '[' && strchr(buffer, ']') != nullptr) break;

				/*
				**	Determine if this line is a comment or blank line. Throw it out if it is.
				*/
				Strip_Comments(buffer);
				if (len == 0 || buffer[0] == ';' || buffer[0] == '=') continue;

				/*
				**	The line isn't an obvious comment. Make sure that there is the "=" character
				**	at an appropriate spot.
				*/
				char * divider = strchr(buffer, '=');
				if (!divider) continue;

				/*
				**	Split the line into entry and value sections. Be sure to catch the
				**	"=foobar" and "foobar=" cases. "=foobar" lines are ignored, while
				** "foobar=" lines are might be stored as has having " " as their value,
				** depending on the value of KeepBlankEntries.
				*/
				*divider++ = '\0';
				strtrim(buffer);
				if (!strlen(buffer)) continue;

				strtrim(divider);
				char space[] = " ";
				if (!strlen(divider)) {
					if (KeepBlankEntries)
						divider = space;
					else
						continue;
				}

				if (Put_String(section, buffer, divider) == false) {
					return(false);
				}
			}
		}

	} else {
		/*
		**	Process a section. The buffer is prefilled with the section name line.
		*/
		while (!end_of_file) {

			buffer[0] = ' ';
			char * ptr = strchr(buffer, ']');
			if (ptr != nullptr) *ptr = '\0';
			strtrim(buffer);
			INISection * secptr = W3DNEW INISection(strdup(buffer));
			if (secptr == nullptr) {
				Clear();
				return(false);
			}

			/*
			**	Read in the entries of this section.
			*/
			while (!end_of_file) {

				/*
				**	If this line is the start of another section, then bail out
				**	of the entry loop and let the outer section loop take
				**	care of it.
				*/
				int len = Read_Line(file, buffer, sizeof(buffer), end_of_file);
				if (buffer[0] == '[' && strchr(buffer, ']') != nullptr) break;

				/*
				**	Determine if this line is a comment or blank line. Throw it out if it is.
				*/
				Strip_Comments(buffer);
				if (len == 0 || buffer[0] == ';' || buffer[0] == '=') continue;

				/*
				**	The line isn't an obvious comment. Make sure that there is the "=" character
				**	at an appropriate spot.
				*/
				char * divider = strchr(buffer, '=');
				if (!divider) continue;

				/*
				**	Split the line into entry and value sections. Be sure to catch the
				**	"=foobar" and "foobar=" cases. "=foobar" lines are ignored, while
				** "foobar=" lines are might be stored as has having " " as their value,
				** depending on the value of KeepBlankEntries.
				*/
				*divider++ = '\0';
				strtrim(buffer);
				if (!strlen(buffer)) continue;

				strtrim(divider);
				char space[] = " ";
				if (!strlen(divider)) {
					if (KeepBlankEntries)
						divider = space;
					else
						continue;
				}


				INIEntry * entryptr = W3DNEW INIEntry(strdup(buffer), strdup(divider));
				if (entryptr == nullptr) {
					delete secptr;
					Clear();
					return(false);
				}

				// 12/09/97 EHC - check to see if an entry with this ID already exists
				if (secptr->EntryIndex.Is_Present(entryptr->Index_ID())) {
					DuplicateCRCError("INIClass::Load", secptr->Section, buffer);
					delete entryptr;
					continue;
				}

				secptr->EntryIndex.Add_Index(entryptr->Index_ID(), entryptr);
				secptr->EntryList.Add_Tail(entryptr);
			}

			/*
			**	All the entries for this section have been parsed. If this section is blank, then
			**	don't bother storing it.
			*/
			if (secptr->EntryList.Is_Empty()) {
				delete secptr;
			} else {
				SectionIndex->Add_Index(secptr->Index_ID(), secptr);
				SectionList->Add_Tail(secptr);
			}
		}
	}
	return(true);
}


// All existing data in the file, if it was present, is replaced.
int INIClass::Save(FileClass & file) const
{
	FilePipe fp(file);
	delete[] Filename;
	Filename = nstrdup(file.File_Name());
	return(Save(fp));
}


// All existing data in the file, if it was present, is replaced.
int INIClass::Save(const char *filename) const
{
	FileClass *file=_TheWritingFileFactory->Get_File(filename);
	int retval=0;
	if ( file ) {
		retval=Save(*file);
		_TheWritingFileFactory->Return_File(file);
	}
	file=nullptr;

	delete[] Filename;
	Filename = nstrdup(filename);

	return(retval);
}



int INIClass::Save(Pipe & pipe) const
{
	int total = 0;

	#ifdef _UNIX
		const char *EOL="\n";
	#else
		const char *EOL="\r\n";
	#endif

	INISection * secptr = SectionList->First();
	while (secptr && secptr->Is_Valid()) {

		/*
		**	Output the section identifier.
		*/
		total += pipe.Put("[", 1);
		total += pipe.Put(secptr->Section, strlen(secptr->Section));
		total += pipe.Put("]", 1);
		total += pipe.Put(EOL, strlen(EOL));

		/*
		**	Output all the entries and values in this section.
		*/
		INIEntry * entryptr = secptr->EntryList.First();
		while (entryptr && entryptr->Is_Valid()) {
			total += pipe.Put(entryptr->Entry, strlen(entryptr->Entry));
			total += pipe.Put("=", 1);
			total += pipe.Put(entryptr->Value, strlen(entryptr->Value));
			total += pipe.Put(EOL, strlen(EOL));

			entryptr = entryptr->Next();
		}

		/*
		**	After the last entry in this section, output an extra
		**	blank line for readability purposes.
		*/
		total += pipe.Put(EOL, strlen(EOL));

		secptr = secptr->Next();
	}
	total += pipe.End();

	return(total);
}


INISection * INIClass::Find_Section(char const * section) const
{
	if (section != nullptr) {
//		long crc = CRCEngine()(section, strlen(section));
		long crc = CRC(section);

		if (SectionIndex->Is_Present(crc)) {
			return((*SectionIndex)[crc]);
		}
	}
	return(nullptr);
}


int INIClass::Section_Count() const
{
	return(SectionIndex->Count());
}


int INIClass::Entry_Count(char const * section) const
{
	INISection * secptr = Find_Section(section);
	if (secptr != nullptr) {
		return(secptr->EntryIndex.Count());
	}
	return(0);
}


INIEntry * INIClass::Find_Entry(char const * section, char const * entry) const
{
	INISection * secptr = Find_Section(section);
	if (secptr != nullptr) {
		return(secptr->Find_Entry(entry));
	}
	return(nullptr);
}


// The entry identifier is the text that appears to the left of the "=" character.
char const * INIClass::Get_Entry(char const * section, int index) const
{
	INISection * secptr = Find_Section(section);

	if (secptr != nullptr && index < secptr->EntryIndex.Count()) {
		INIEntry * entryptr = secptr->EntryList.First();

		while (entryptr != nullptr && entryptr->Is_Valid()) {
			if (index == 0) return(entryptr->Entry);
			index--;
			entryptr = entryptr->Next();
		}
	}
	return(nullptr);
}



// Count how many entries begin with a certain prefix followed by a range of numbers.
unsigned INIClass::Enumerate_Entries(const char *Section, const char * Entry_Prefix, unsigned StartNumber, unsigned EndNumber)
{
	unsigned count = StartNumber;
	bool present = false;
	char entry[256];
	do
	{
		snprintf(entry, ARRAY_SIZE(entry), "%s%d", Entry_Prefix, count);
		present = Is_Present(Section, entry);
		if(present)
			count++;
	} while(present && (count < EndNumber));

	return (count - StartNumber);
}



// The data is converted into displayable form and then broken into lines that are stored in
// sequence to the section. A section used to store data in this fashion can not be used for any
// other entries.
bool INIClass::Put_UUBlock(char const * section, void const * block, int len)
{
	if (section == nullptr || block == nullptr || len < 1) return(false);

	Clear(section);

	BufferStraw straw(block, len);
	Base64Straw bstraw(Base64Straw::ENCODE);
	bstraw.Get_From(straw);

	int counter = 1;

	for (;;) {
		char buffer[71];
		char sbuffer[32];

		int length = bstraw.Get(buffer, sizeof(buffer)-1);
		buffer[length] = '\0';
		if (length == 0) break;

		sprintf(sbuffer, "%d", counter);
		Put_String(section, sbuffer, buffer);
		counter++;
	}
	return(true);
}


// Decomposes all the entries in the specified section into a binary block of data. A section
// processed by this routine can contain no other entries than those put there by a previous call to
// Put_UUBlock.
// WARNING: If the number of bytes retrieved exactly matches the length of the buffer specified,
// then you might have a condition of buffer "overflow".
int INIClass::Get_UUBlock(char const * section, void * block, int len) const
{
	if (section == nullptr) return(0);

	Base64Pipe b64pipe(Base64Pipe::DECODE);
	BufferPipe bpipe(block, len);

	b64pipe.Put_To(&bpipe);

	int total = 0;
	int counter = Entry_Count(section);
	for (int index = 0; index < counter; index++) {
		char buffer[128];

		int length = Get_String(section, Get_Entry(section, index), "=", buffer, sizeof(buffer));
		int outcount = b64pipe.Put(buffer, length);
		total += outcount;
	}
	total += b64pipe.End();
	return(total);
}





const WideStringClass& INIClass::Get_Wide_String(WideStringClass& new_string, char const * section, char const * entry, unichar const * defvalue) const
{
	unichar out[1024];
	char buffer[1024];

	Base64Pipe b64pipe(Base64Pipe::DECODE);
	BufferPipe bpipe(out, sizeof(out));
	b64pipe.Put_To(&bpipe);

	int length = Get_String(section, entry, "", buffer, sizeof(buffer));
	if (length == 0) {
		new_string = defvalue;
	} else {
		int outcount = b64pipe.Put(buffer, length);
		outcount += b64pipe.End();
		new_string = out;
	}
	return(new_string);
}




bool INIClass::Put_Wide_String(char const * section, char const * entry, unichar const * string)
{
	if (section == nullptr || entry == nullptr || string == nullptr) {
		return(false);
	}

	WideStringClass temp_string(string, true);
	int len = temp_string.Get_Length();

	if (len == 0) {
		Put_String(section, entry, "");
	} else {

		char *buffer = (char*) _alloca((len * 8) + 32);

		BufferStraw straw(string, (len*2) + 2);		// Convert from shorts to bytes, plus 2 for terminator.
		Base64Straw bstraw(Base64Straw::ENCODE);
		bstraw.Get_From(straw);

		int new_length = 0;
		int added = 0;
		do {
			added = bstraw.Get(buffer + new_length, 16);
			new_length += added;
		} while (added);
		buffer[new_length] = 0;
		WWASSERT(new_length != 0);
		Put_String(section, entry, buffer);
	}
	return(true);
}





bool INIClass::Put_UUBlock(char const * section, char const *entry, void const * block, int len)
{
	if (section == nullptr || block == nullptr || len < 1) return(false);

	BufferStraw straw(block, len);
	Base64Straw bstraw(Base64Straw::ENCODE);
	bstraw.Get_From(straw);

	char *buffer = (char*) _alloca(len * 3);
	int length = bstraw.Get(buffer, (len * 3) - 1);

	buffer[length] = '\0';
	Put_String(section, entry, buffer);

	return(true);
}






int INIClass::Get_UUBlock(char const * section, char const *entry, void * block, int len) const
{
	if (section == nullptr) return(0);
	if (entry == nullptr) return(0);

	Base64Pipe b64pipe(Base64Pipe::DECODE);
	BufferPipe bpipe(block, len);

	b64pipe.Put_To(&bpipe);

	int total = 0;
	char *buffer = (char*) _alloca(len * 3);

	int length = Get_String(section, entry, "=", buffer, len*3);
	int outcount = b64pipe.Put(buffer, length);
	total += outcount;
	total += b64pipe.End();
	return(total);
}








// The text is broken up into lines and each line is then stored as a numbered entry in the
// specified section. A section used to store text in this way can not be used to hold any other
// entries. The text is presumed to contain space characters scattered throughout it and that one
// space between words and sentences is natural.
bool INIClass::Put_TextBlock(char const * section, char const * text)
{
	if (section == nullptr) return(false);

	Clear(section);

	int index = 1;
	while (text != nullptr && *text != 0) {

		char buffer[128];

		strlcpy(buffer, text, 76);

		char b[32];
		sprintf(b, "%d", index);

		/*
		**	Scan backward looking for a good break position.
		*/
		int count = strlen(buffer);
		if (count > 0) {
			if (count >= 75) {
				while (count) {
					char c = buffer[count];

					if (isspace(c)) break;
					count--;
				}

				if (count == 0) {
					break;
				} else {
					buffer[count] = '\0';
				}
			}

			strtrim(buffer);
			Put_String(section, b, buffer);
			index++;
			text = ((char  *)text) + count;
		} else {
			break;
		}
	}
	return(true);
}


// Formats all entries in the specified section into a block of normalized text. That is, text with
// single spaces between each concatenated line. Use Put_TextBlock to build the entries in the
// section. The text will, at most, fill the buffer with the last character being forced to null.
// The trailing null is not counted in the returned length.
int INIClass::Get_TextBlock(char const * section, char * buffer, int len) const
{
	if (len <= 0) return(0);

	buffer[0] = '\0';
	if (len <= 1) return(0);

	int elen = Entry_Count(section);
	int total = 0;
	for (int index = 0; index < elen; index++) {

		/*
		**	Add spacers between lines of fetched text.
		*/
		if (index > 0) {
			*buffer++ = ' ';
			len--;
			total++;
		}

		Get_String(section, Get_Entry(section, index), "", buffer, len);

		int partial = strlen(buffer);
		total += partial;
		buffer += partial;
		len -= partial;
		if (len <= 1) break;
	}
	return(total);
}


// Stores a signed integer into the INI data base. The format is generally only a cosmetic effect:
// 0 is plain decimal, 1 is hexadecimal with trailing "h", 2 is hexadecimal with leading "$". The
// Get_Int operation will interpret the value the same regardless of what format was used.
bool INIClass::Put_Int(char const * section, char const * entry, int number, int format)
{
	char buffer[64];

	switch (format) {
		default:
		case 0:
			sprintf(buffer, "%d", number);
			break;

		case 1:
			sprintf(buffer, "%Xh", number);
			break;

		case 2:
			sprintf(buffer, "$%X", number);
			break;
	}
	return(Put_String(section, entry, buffer));
}


// All the formats written by Put_Int are understood.
int INIClass::Get_Int(char const * section, char const * entry, int defvalue) const
{
	/*
	**	Verify that the parameters are nominally correct.
	*/
	if (section == nullptr || entry == nullptr) return(defvalue);

	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr && entryptr->Value != nullptr) {

		if (*entryptr->Value == '$') {
			sscanf(entryptr->Value, "$%x", &defvalue);
		} else {
			if (tolower(entryptr->Value[strlen(entryptr->Value)-1]) == 'h') {
				sscanf(entryptr->Value, "%xh", &defvalue);
			} else {
				defvalue = atoi(entryptr->Value);
			}
		}
	}
	return(defvalue);
}



bool INIClass::Put_Rect(char const * section, char const * entry, Rect const & value)
{
	char buffer[64];

	sprintf(buffer, "%d,%d,%d,%d", value.X, value.Y, value.Width, value.Height);
	return(Put_String(section, entry, buffer));
}


Rect const INIClass::Get_Rect(char const * section, char const * entry, Rect const & defvalue) const
{
	char buffer[64];

	if (Get_String(section, entry, "0,0,0,0", buffer, sizeof(buffer))) {
		Rect retval = defvalue;
		sscanf(buffer, "%d,%d,%d,%d", &retval.X, &retval.Y, &retval.Width, &retval.Height);
		return(retval);
	}
	return(defvalue);
}


bool INIClass::Put_Hex(char const * section, char const * entry, int number)
{
	char buffer[64];

	sprintf(buffer, "%X", number);
	return(Put_String(section, entry, buffer));
}


int INIClass::Get_Hex(char const * section, char const * entry, int defvalue) const
{
	/*
	**	Verify that the parameters are nominally correct.
	*/
	if (section == nullptr || entry == nullptr) return(defvalue);

	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr && entryptr->Value != nullptr) {
		sscanf(entryptr->Value, "%x", &defvalue);
	}
	return(defvalue);
}


float INIClass::Get_Float(char const * section, char const * entry, float defvalue) const
{
	/*
	**	Verify that the parameters are nominally correct.
	*/
	if (section == nullptr || entry == nullptr) return(defvalue);

	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr != nullptr && entryptr->Value != nullptr) {
		float val = defvalue;
		sscanf(entryptr->Value, "%f", &val);
		defvalue = val;
		if (strchr(entryptr->Value, '%') != nullptr) {
			defvalue /= 100.0f;
		}
	}
	return(defvalue);
}


bool INIClass::Put_Float(char const * section, char const * entry, float number)
{
	char buffer[64];

	sprintf(buffer, "%f", number);
	return(Put_String(section, entry, buffer));
}


double INIClass::Get_Double(char const * section, char const * entry, double defvalue) const
{
	/*
	**	Verify that the parameters are nominally correct.
	*/
	if (section == nullptr || entry == nullptr) return(defvalue);

	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr != nullptr && entryptr->Value != nullptr) {
		double val = defvalue;
		sscanf(entryptr->Value, "%lf", &val);
		defvalue = val;
		if (strchr(entryptr->Value, '%') != nullptr) {
			defvalue /= 100.0f;
		}
	}
	return(defvalue);
}


bool INIClass::Put_Double(char const * section, char const * entry, double number)
{
	char buffer[64];

	sprintf(buffer, "%lf", number);
	return(Put_String(section, entry, buffer));
}


// Any previous matching entry will be replaced. On a duplicate entry CRC, the strings are checked
// for being identical.
bool INIClass::Put_String(char const * section, char const * entry, char const * string)
{
	if (section == nullptr || entry == nullptr) return(false);

	INISection * secptr = Find_Section(section);

	if (secptr == nullptr) {
		secptr = W3DNEW INISection(strdup(section));
		if (secptr == nullptr) return(false);
		SectionList->Add_Tail(secptr);
		SectionIndex->Add_Index(secptr->Index_ID(), secptr);
	}

	/*
	**	Remove the old entry if found and print debug message
	*/
	INIEntry * entryptr = secptr->Find_Entry(entry);
	if (entryptr != nullptr) {
      if (strcmp(entryptr->Entry, entry) != 0) {
         DuplicateCRCError("INIClass::Put_String", section, entry);
      } else {
#if 0
			OutputDebugString("INIClass::Put_String - Duplicate Entry \"");
	   	OutputDebugString(entry);
		   OutputDebugString("\"\n");
#endif
      }
   	secptr->EntryIndex.Remove_Index(entryptr->Index_ID());
	   delete entryptr;
	}

	/*
	**	Create and add the new entry.
	*/
	if (string != nullptr && strlen(string) > 0) {
		entryptr = W3DNEW INIEntry(strdup(entry), strdup(string));

		// If this assert fires, then the string will be truncated on load, because
		// there will not be enough room in the loading buffer!
		WWASSERT(strlen(string) < MAX_LINE_LENGTH);

		if (entryptr == nullptr) {
			return(false);
		}
		secptr->EntryList.Add_Tail(entryptr);
		secptr->EntryIndex.Add_Index(entryptr->Index_ID(), entryptr);
	}
	return(true);
}


// Retrieves the entire text to the right of the "=" character. The maximum string length that could
// be retrieved will be one less than the buffer size, due to the forced trailing zero.
int INIClass::Get_String(char const * section, char const * entry, char const * defvalue, char * buffer, int size) const
{
	/*
	**	Verify that the parameters are nominally legal.
	*/
//	if (buffer != nullptr && size > 0) {
//		buffer[0] = '\0';
//	}
	if (buffer == nullptr || size < 2 || section == nullptr || entry == nullptr) return(0);

	/*
	**	Fetch the entry string if it is present. If not, then the normal default
	**	value will be used as the entry value.
	*/
	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr != nullptr && entryptr->Value != nullptr) {
		defvalue = entryptr->Value;
	}

	/*
	**	Fill in the buffer with the entry value and return with the length of the string.
	*/
	if (defvalue == nullptr) {
		buffer[0] = '\0';
		return(0);
	} else {
		strlcpy(buffer, defvalue, size);
		strtrim(buffer);
		return(strlen(buffer));
	}
}


/*
** GetString
*/
const StringClass& INIClass::Get_String(StringClass& new_string, char const * section, char const * entry, char const * defvalue) const
{
	if (section == nullptr || entry == nullptr) {
		new_string="";
		return new_string;
	}

	/*
	**	Fetch the entry string if it is present. If not, then the normal default
	**	value will be used as the entry value.
	*/
	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr != nullptr) {
		defvalue = entryptr->Value;
	}

	if (defvalue == nullptr) {
		new_string="";
		return new_string;
	}
	new_string=defvalue;
	return new_string;
}



// Retrieves the entire text to the right of the "=" character.
char *INIClass::Get_Alloc_String(char const * section, char const * entry, char const * defvalue) const
{
	if (section == nullptr || entry == nullptr) return(nullptr);

	/*
	**	Fetch the entry string if it is present. If not, then the normal default
	**	value will be used as the entry value.
	*/
	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr != nullptr) {
		defvalue = entryptr->Value;
	}

	if (defvalue == nullptr) return nullptr;
	return(strdup(defvalue));
}

int INIClass::Get_List_Index(char const * section, char const * entry, int const defvalue, char *list[])
{
	if (section == nullptr || entry == nullptr) return(0);

	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr == nullptr || entryptr->Value == nullptr) {
		return defvalue;
	}

	for (int lp = 0; list[lp]; lp++) {
		if (stricmp(entryptr->Value, list[lp]) == 0) {
			return lp;
		}
		assert(lp < 1000);
	}
	return defvalue;
}
int INIClass::Get_Int_Bitfield(char const * section, char const * entry, int defvalue, char *list[])
{
	// if we can't find the entry or the entry is null just return the default value
	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr == nullptr || entryptr->Value == nullptr) {
		return defvalue;
	}

	// swim through the entry breaking it down into its token pieces and
	// get the bitfield value for each piece.
	// int count	= 0; (gth) initailized but not referenced...
	int retval	= 0;
	char *str	= strdup(entryptr->Value);

   int lp;
	for (char *token = strtok(str, "|+"); token; token = strtok(nullptr, "|+")) {
		for (lp = 0; list[lp]; lp++) {
			// if this list entry matches our string token then we need
			// to set this bit.
			if (stricmp(token, list[lp]) == 0) {
				retval |= (1 << lp);
				break;
			}
		}
		// if we reached the end of the list and found nothing then we need
		// to assert since we have an unidentified value
		if (list[lp] == nullptr) assert(lp < 1000);
	}
	free(str);
	return retval;
}

int *	INIClass::Get_Alloc_Int_Array(char const * section, char const * entry, int listend)
{
	int *retval = nullptr;

	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr == nullptr || entryptr->Value == nullptr) {
		retval = W3DNEWARRAY int[1];
		retval[0] = listend;

		return retval;
	}

	// count all the tokens in the string.  Each token should represent an
	// integer number.
	int count = 0;
	char *str = strdup(entryptr->Value);
	char *token;
	for (token = strtok(str, " "); token; token = strtok(nullptr, " ")) {
		count++;
	}
	free(str);

	// now that we know how many tokens there are in the string, allocate a int
	// array to hold the tokens and parse out the actual values.
	retval	= W3DNEWARRAY int[count+1];
	count		= 0;
	str		= strdup(entryptr->Value);
	for (token = strtok(str, " "); token; token = strtok(nullptr, " ")) {
		retval[count] = atoi(token);
		count++;
	}
	free(str);

	// arrays of integers are terminated with the listend variable passed in
	retval[count] = listend;

	// now that we have the allocated array with the results filled in lets return
	// the results.
	return retval;
}

// The boolean value will be stored as "yes" or "no".
bool INIClass::Put_Bool(char const * section, char const * entry, bool value)
{
	if (value) {
		return(Put_String(section, entry, "yes"));
	} else {
		return(Put_String(section, entry, "no"));
	}
}


// The boolean value is interpreted using the standard boolean conventions. e.g., "Yes", "Y", "1",
// "True", "T" are all considered to be a TRUE boolean value.
bool INIClass::Get_Bool(char const * section, char const * entry, bool defvalue) const
{
	/*
	**	Verify that the parameters are nominally correct.
	*/
	if (section == nullptr || entry == nullptr) return(defvalue);

	INIEntry * entryptr = Find_Entry(section, entry);
	if (entryptr && entryptr->Value != nullptr) {
		switch (toupper(*entryptr->Value)) {
			case 'Y':
			case 'T':
			case '1':
				return(true);

			case 'N':
			case 'F':
			case '0':
				return(false);
		}
	}
	return(defvalue);
}


bool INIClass::Put_Point(char const * section, char const * entry, TPoint2D<int> const & value)
{
	char buffer[54];
	sprintf(buffer, "%d,%d", value.X, value.Y);
	return(Put_String(section, entry, buffer));
}


TPoint2D<int> const INIClass::Get_Point(char const * section, char const * entry, TPoint2D<int> const & defvalue) const
{
	char buffer[64];
	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		int x,y;
		sscanf(buffer, "%d,%d", &x, &y);
		return(TPoint2D<int>(x, y));
	}
	return(defvalue);
}


bool INIClass::Put_Point(char const * section, char const * entry, TPoint3D<int> const & value)
{
	char buffer[54];
	sprintf(buffer, "%d,%d,%d", value.X, value.Y, value.Z);
	return(Put_String(section, entry, buffer));
}


TPoint3D<int> const INIClass::Get_Point(char const * section, char const * entry, TPoint3D<int> const & defvalue) const
{
	char buffer[64];
	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		int x,y,z;
		sscanf(buffer, "%d,%d,%d", &x, &y, &z);
		return(TPoint3D<int>(x, y, z));
	}
	return(defvalue);
}


bool INIClass::Put_Point(char const * section, char const * entry, TPoint3D<float> const & value)
{
	char buffer[54];
	sprintf(buffer, "%f,%f,%f", (float)value.X, (float)value.Y, (float)value.Z);
	return(Put_String(section, entry, buffer));
}


TPoint3D<float> const INIClass::Get_Point(char const * section, char const * entry, TPoint3D<float> const & defvalue) const
{
	char buffer[64];
	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		float x,y,z;
		sscanf(buffer, "%f,%f,%f", &x, &y, &z);
		return(TPoint3D<float>(x, y, z));
	}
	return(defvalue);
}


TPoint2D<float> const INIClass::Get_Point(char const * section, char const * entry, TPoint2D<float> const & defvalue) const
{
	char buffer[64];
	if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
		float x,y;
		sscanf(buffer, "%f,%f", &x, &y);
		return(TPoint2D<float>(x, y));
	}
	return(defvalue);
}


// This is used for internal database maintenance.
INIEntry * INISection::Find_Entry(char const * entry) const
{
	if (entry != nullptr) {
//		int crc = CRCEngine()(entry, strlen(entry));
		int crc = CRC::String(entry);
		if (EntryIndex.Is_Present(crc)) {
			return(EntryIndex[crc]);
		}
	}
	return(nullptr);
}


// Leading and trailing blanks are also removed.
void INIClass::Strip_Comments(char * buffer)
{
	if (buffer != nullptr) {
		char * comment = strchr(buffer, ';');
		if (comment) {
			*comment = '\0';
			strtrim(buffer);
		}
	}
}


// Returns a (hopefully) unique 32-bit value for a string.
int INIClass::CRC(const char *string)
{
	// simply call the CRC class string evaluator.
	return CRC::String(string);
}


// Displays debug information when a duplicate entry is found in an INI file. In Release mode under
// Windows, a message box will be displayed.
// WARNING: Will assert(0) and exit program. INI errors are considered fatal at this time.
void INIClass::DuplicateCRCError(const char *message, const char *section, const char *entry)
{
	char buffer[512];
	snprintf(buffer, sizeof(buffer), "%s - Duplicate Entry \"%s\" in section \"%s\" (%s)\n", message,
		entry, section, Filename);

	OutputDebugString(buffer);
	assert(0);

#ifdef RTS_RELEASE
#ifdef _WIN32
	MessageBox(nullptr, buffer, "Duplicate CRC in INI file.", MB_ICONSTOP | MB_OK);
#endif
#endif
}


void	INIClass::Keep_Blank_Entries (bool keep_blanks)
{
	KeepBlankEntries = keep_blanks;
}

