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
 *                     $Archive:: /Commando/Code/wwlib/readline.cpp                           $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/14/01 7:25p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   strtrim -- Trim leading and trailing white space off of string.                           *
 *   Read_Line -- Read a text line from the file.                                              *
 *   Read_Line -- Reads a text line from the straw object specified.                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"readline.h"
#include	"trim.h"
#include	"WWFILE.H"
#include	"XSTRAW.H"
//#include	<ctype.h>
#include	<string.h>


#if !defined(__BORLANDC__) && !defined(_MSC_VER)
// Disable the "temporary object used to initialize a non-constant reference" warning.
#pragma warning 665 9
#endif


/***********************************************************************************************
 * Read_Line -- Read a text line from the file.                                                *
 *                                                                                             *
 *    This will read a line of text from the file specified.                                   *
 *                                                                                             *
 * INPUT:   file  -- Reference to the file to read the line from.                              *
 *                                                                                             *
 *          buffer   -- Pointer to the location to store the line of text.                     *
 *                                                                                             *
 *          len   -- The length of the buffer to store the line.                               *
 *                                                                                             *
 *          eof   -- Reference to the EOF flag that will be set to true if the source          *
 *                   file has been exhausted.                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the number of characters stored into the buffer.                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int Read_Line(FileClass & file, char * buffer, int len, bool & eof)
{
	FileStraw fs(file);
	return(Read_Line(fs, buffer, len, eof));
}


/***********************************************************************************************
 * Read_Line -- Reads a text line from the straw object specified.                             *
 *                                                                                             *
 *    This reads a line of text from the straw object.                                         *
 *                                                                                             *
 * INPUT:   file  -- Reference to the straw object to fetch the line of text from.             *
 *                                                                                             *
 *          buffer   -- Pointer to the buffer that will hold the line of text.                 *
 *                                                                                             *
 *          len   -- The size of the buffer.                                                   *
 *                                                                                             *
 *          eof   -- Reference to the EOF flag that will be set to true if the source          *
 *                   straw data has been exhausted.                                            *
 *                                                                                             *
 * OUTPUT:  Returns with the number of characters stored into the buffer.                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/06/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int Read_Line(Straw & file, char * buffer, int len, bool & eof)
{
	if (len == 0 || buffer == NULL) return(0);

	int count = 0;
	for (;;) {
		char c;
		if (file.Get(&c, sizeof(c)) != sizeof(c)) {
			eof = true;
			buffer[count] = '\0';
			break;
		}

		if (c == '\x0A') break;
		if (c != '\x0D' && count+1 < len) {
			buffer[count++] = c;
		}
	}
	buffer[count] = '\0';

	strtrim(buffer);
	return(strlen(buffer));
}

int Read_Line(Straw & file, wchar_t * buffer, int len, bool & eof)
{
	if (len == 0 || buffer == NULL) return(0);

	int count = 0;
	for (;;) {
		wchar_t c;
		if (file.Get(&c, sizeof(c)) != sizeof(c)) {
			eof = true;
			buffer[count] = L'\0';
			break;
		}

		if (c == L'\x0A') break;
		if (c != L'\x0D' && count+1 < len) {
			buffer[count++] = c;
		}
	}
	buffer[count] = '\0';

	wcstrim(buffer);
	return(wcslen(buffer));
}
