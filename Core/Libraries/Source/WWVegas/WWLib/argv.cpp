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

/* $Header: /VSS_Sync/wwlib/argv.cpp 11    8/29/01 10:25p Vss_sync $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Library                                                      *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/wwlib/argv.cpp                                    $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CurrentPos -- Create an instance to parse argv with.                                      *
 *   ArgvClass::Free -- Release data allocated.                                                *
 *   ArgvClass::Load_File -- Load args from a file.                                            *
 *   *ArgvClass::Find_Value -- Find value of argument given prefix.                            *
 *   *ArgvClass::Get_Cur_Value -- Get value of current argugment.                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "argv.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ffactory.h"
#include "RAWFILE.H"
int   	 ArgvClass::Argc = 0;
char 		*ArgvClass::Argv[MAX_ARGC];

/***********************************************************************************************
 * CurrentPos -- Create an instance to parse argv with.                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *    	bool case_sensitive - Do you want to perform a case sensitive search (stricmp)?		  *
 *			bool exact_size     - Do you want string of same lenght (strncmp) ?						  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
ArgvClass::ArgvClass(bool case_sensitive, bool exact_size):
	Flags(0),
	LastArg(0),
	CurrentPos(-1)
{
	Case_Sensitive(case_sensitive);
	Exact_Size(exact_size);
}

/***********************************************************************************************
 * *ArgvClass::Find_Again -- Search for a string given the flags.                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      const char *arg - String to search for. If NULL, LastArg will be used.                 *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      const char *string found (null if not found)														  *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/17/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
const char *ArgvClass::Find_Again(const char *arg)
{
	if (arg) {
		LastArg = arg;
	} else {
		arg = LastArg;
	}

	// Make sure user has given us something to work with here.
	assert(LastArg);

	CurrentPos++;
	if (CurrentPos < Argc) {
		if (Is_Case_Sensitive()) {
			if (Is_Exact_Size()) {
				// Case Sensitive, Exact Size.
				for (; CurrentPos < Argc; CurrentPos++) {
					if (!strcmp(arg, Argv[CurrentPos])) {
						return Argv[CurrentPos];
					}
				}
			} else {
				// Case Sensitive, Match first strlen(arg).
				int len = strlen(arg);
				for (; CurrentPos < Argc; CurrentPos++) {
					if (!strncmp(arg, Argv[CurrentPos], len)) {
						return Argv[CurrentPos];
					}
				}
			}
		} else {
			if (Is_Exact_Size()) {
				// Note case sensitive, Exact Size.
				for (; CurrentPos < Argc; CurrentPos++) {
					if (!stricmp(arg, Argv[CurrentPos])) {
						return Argv[CurrentPos];
					}
				}
			} else {
				// Note case sensitive, Match first strlen(arg).
				int len = strlen(arg);
				for (; CurrentPos < Argc; CurrentPos++) {
					if (!strnicmp(arg, Argv[CurrentPos], len)) {
						return Argv[CurrentPos];
					}
				}
			}
		}
	}
	return NULL;
}

/***********************************************************************************************
 * ArgvClass::Init -- Setup the command line.                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *			LPSTR lpCmdLine - A string of white space seperated strings.  Quotes force spaces to  *
 *                         be ignored.                                                         *
 *			char *fileprefix - A prefix on an arguement telling system to load postfix file name  *
 *                          as command line params.                                            *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      This may be called multible times with different strings.                              *
 *      Once Argc reaches MAX_ARGC, no more will be added.                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/17/1999 SKB : Created.                                                                 *
 *   07/15/2001 SKB : Put file arguements in the correct order they were included.             *
 *=============================================================================================*/
int ArgvClass::Init(char *lpCmdLine, const char *fileprefix)
{
	// Get pointer to command line.
   char	*ptr = lpCmdLine;
	if (!ptr || !*ptr) {
		return 0;
	}

	int fp_cmp_len = (fileprefix) ? strlen(fileprefix) : 0;

	// Save original Argc for return.
	int origargc = Argc;

	while (*ptr) {
		char  *eos;
		char  save;

		// Keep anything within quotes as one string.
		if (*ptr == '"') {
			ptr++;
			eos = ptr;
			// Search for next " or a null.
			while (*eos && (*eos != '"')) {
				eos++;
			}
		} else if (isspace(*ptr)) {
			ptr++;
			continue;
		} else {
			eos = ptr + 1;
			// search for next white space or null.
			while (*eos && !isspace(*eos)) {
				eos++;
			}
		}

		// Null out end of string so string function work.
		// Save the end to restore later.
		save = *eos;
		*eos = 0;

		bool was_file = false;

		// See if we are to load a file with parameters in it.
		if (fp_cmp_len && !strncmp(fileprefix, ptr, fp_cmp_len)) {
			ptr += fp_cmp_len;
			if (*ptr) {
				was_file = Load_File(ptr);
			}
		}

		// If it was not the file or the load failed...then add parameter.
		if (!was_file) {
			// Copy string over and continue.
			Argv[Argc] = strdup(ptr);
			Argc++;
		}

		// If save is null, then we are at the end and we can bail out.
		if (!save) break;

		// resore whitespace.
		*eos = save;
		ptr = eos + 1;
	}

	// Return number of params read in.
	return(Argc - origargc);
}

/***********************************************************************************************
 * ArgvClass::Load_File -- Load args from a file.                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *     	const char *fname - file to load.                                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
bool ArgvClass::Load_File(const char *fname)
{
	FILE *fp = fopen(fname, "r");

	if (fp)  {
		while (Argc < MAX_ARGC) {
			const int maxstrlen = 255;
			char string[maxstrlen + 1];

			// Get next line in file.
			if (!fgets(string, maxstrlen - 1, fp)) {
				break;
			}

			// Check for comments.
			if ((*string != '#') && (*string != ';'))  {
				// Make sure null terminated.
				string[maxstrlen - 1] = '\0';

				char *ptr = string + (strlen(string) - 1);
				while (*ptr <= ' ')  {
					*ptr = 0;

					// Is it just a blank line?
					if (ptr == string) {
						break;
					}
					ptr--;
				}

				// If there is anyting in the string. (NAK: old code used to fail for 1 char options)
				if (strlen(string)) {
					Argv[Argc] = strdup(string);
					Argc++;
				}
			}
		}
		fclose(fp);
		return(true);
	}
	return(false);
}

/***********************************************************************************************
 * ArgvClass::Free -- Release data allocated.                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
void ArgvClass::Free()
{
	for (int lp = 0; lp < Argc; lp++) {
		free(Argv[lp]);
		Argv[lp] = 0;
	}
	Argc = -1;
}


/***********************************************************************************************
 * *ArgvClass::Find_Value -- Find value of argument given prefix.                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/23/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
const char *ArgvClass::Find_Value(const char *arg)
{
	if (arg && *arg) {
		const char *ptr = Find(arg);
		if (ptr) {
			return(Get_Cur_Value(strlen(arg)));
		}
	}
	return(NULL);
}

/***********************************************************************************************
 * *ArgvClass::Get_Cur_Value -- Get value of current argugment.                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/23/1999 SKB : Created.                                                                 *
 *   06/25/2001 SKB : add flag user can check to see if value was extracted from next location.*
 *=============================================================================================*/
const char *ArgvClass::Get_Cur_Value(unsigned prefixlen, bool * val_in_next)
{
	if (val_in_next) *val_in_next = false;
	if (CurrentPos < 0) {
		return NULL;
	}
	char *ptr = Argv[CurrentPos];

	if (strlen(ptr) < prefixlen) {
		return(NULL);
	}

	ptr += prefixlen;

	// Look for non white space (or eol).
	while (*ptr && !isgraph(*ptr)) {
		ptr++;
	}
	if (*ptr) {
		return ptr;
	}

	// Goto next line to handle '-P data' case on command line.
	ptr = Argv[CurrentPos + 1];
	if (!ptr) {
		return NULL;
	}

	while (*ptr) {
		if (isgraph(*ptr)) {
			if (val_in_next) *val_in_next = true;
			return ptr;
		}
		ptr++;
	}
	return (NULL);
}




/***********************************************************************************************
 * void ArgvClass::Update_Value -- Add/Replace a value                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *			attrib = cmd line attrib to add/replace                                               *
 *			value = new value for attrib                                                          *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:    Not Tested!                                                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/13/1999 NAK : Created.                                                                 *
 *=============================================================================================*/
void ArgvClass::Update_Value(const char *attrib, const char *value)
{
	if ((Find_Value(attrib))!=NULL)
	{
		if (((CurrentPos+1) < Argc) && (Argv[CurrentPos+1][0] != '-'))  // update old value
		{
			free(Argv[CurrentPos+1]);
			Argv[CurrentPos+1]=strdup(value);
		}
		else  // add new value
		{
			// shift vals down to make room
			memmove(&(Argv[CurrentPos+2]),&(Argv[CurrentPos+1]),sizeof(char *) * (MAX_ARGC-CurrentPos-2));
			Argv[CurrentPos+1]=strdup(value);
			Argc++;
		}
	}
	else  // just add the new stuff
		Add_Value(attrib, value);
}


/***********************************************************************************************
 * void ArgvClass::Add_Value -- Add a value                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *			attrib = thing to add                                                                 *
 *			value = new optional value                                                            *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/13/1999 NAK : Created.                                                                 *
 *=============================================================================================*/
void ArgvClass::Add_Value(const char *attrib, const char *value)
{
	if (attrib)
	{
		Argv[Argc]=strdup(attrib);
		Argc++;

		if (value)
		{
			Argv[Argc]=strdup(value);
			Argc++;
		}
	}
}


/***********************************************************************************************
 * bool ArgvClass::Remove_Value -- Remove a value                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *			attrib = thing to remove                                                              *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *       THIS CONTAINS A POTENTIAL BUG - I don't want to fix it because it might be a desired  *
 *			behavior.  Given:																							  *
 *				Argv[0] = "-i test" "*.txt" as values in Argv, 															  *
 *       calling Remove_Value("-i") will remove *.txt	as well.											  *
 *																															  *
 * HISTORY:                                                                                    *
 *   12/13/1999 NAK : Created.                                                                 *
 *   06/25/2001 SKB : WARNINGS message                                                         *
 *=============================================================================================*/
bool ArgvClass::Remove_Value(const char *attrib)
{
	int        removeCount=1;

	if ((Find_Value(attrib))!=NULL)
	{
		free(Argv[CurrentPos]);
		if (((CurrentPos+1) < Argc)&&(Argv[CurrentPos+1][0]!='-'))  // value for this arg
		{
			free(Argv[CurrentPos+1]);
			removeCount=2;
		}
		memmove(&(Argv[CurrentPos]),&(Argv[CurrentPos+removeCount]),sizeof(char *) * (MAX_ARGC-CurrentPos-removeCount));

		Argc-=removeCount;

		return(true);
	}
	return(false);
}











