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

// FILE: versionUpdate.cpp //////////////////////////////////////////////////////
// Generals version number class updater
// Author: Matthew D. Campbell, November 2001

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN  // only bare bones windows stuff wanted
#include <windows.h>
#include <lmcons.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <trim.h>

// Local defines
#define VERSION_BUILDNUM "VERSION_LOCALBUILDNUM"
#define VERSION_STRING "LOCAL_BUILD_STRING"
#define VERSION_BUILDUSER "VERSION_BUILDUSER"
#define VERSION_BUILDLOC "VERSION_BUILDLOC"
#define FORMAT "#define " VERSION_STRING " \"%d\"\n"
#define COMMENTS "// Do not modify this file by hand. Auto-created and\n// Updated by versionUpdate.\n"
#define NUMFMT "#define %s %d\n"
#define STRFMT "#define %s \"%s\"\n"

static void writeVersion(char *file, int build)
{
	FILE *filePtr = fopen(file, "w");
	// Clobber the file.  Hey, this is a simple program.
	if (file)
	{
		if (filePtr)
		{
			unsigned long bufSize = UNLEN + 1;
			char userName[UNLEN + 1];
			if (!GetUserName(userName, &bufSize))
			{
				strcpy(userName, "unknown");
			}

			bufSize = MAX_COMPUTERNAME_LENGTH + 1;
			char computerName[MAX_COMPUTERNAME_LENGTH + 1];
			if (!GetComputerName(computerName, &bufSize))
			{
				strcpy(computerName, "unknown");
			}

			printf("Build is by %s at %s\n", userName, computerName);

			fprintf(filePtr, COMMENTS);
			fprintf(filePtr, FORMAT, build);
			fprintf(filePtr, NUMFMT, VERSION_BUILDNUM, build);
			fprintf(filePtr, STRFMT, VERSION_BUILDUSER, userName);
			fprintf(filePtr, STRFMT, VERSION_BUILDLOC, computerName);
			fclose(filePtr);
		}
		else
		{
			printf("Cannot write file\n");
		}
	}
	else
	{
		printf("No file to write\n");
	}
}

static void usage(char *progname)
{
	if (progname)
	{
		printf ("Usage: %s versionfile.h", progname);
	}
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	/*
	** Convert WinMain arguments to simple main argc and argv
	*/
	int argc = 1;
	char * argv[20];
	argv[0] = NULL;

	char * token = strtok(lpCmdLine, " ");
	while (argc < 20 && token != NULL)
	{
		argv[argc++] = strtrim(token);
		token = strtok(NULL, " ");
	}

	int build = 0;

	if (argc != 2)
	{
		usage(argv[0]);
	}
	else
	{
		char *target = argv[argc-1];
		FILE *filePtr;

		if (target) {
			filePtr = fopen(target, "r+");
			if (filePtr)
			{
				char buffer[256];
				char *stringPtr = NULL;

				while (!feof(filePtr))
				{
					fread(buffer, 256, 1, filePtr);
					if ((stringPtr = strstr(buffer, VERSION_STRING)) != NULL) 
					{
						char *ptr;

						// Looking for '#define VERSION "x.y.z"'
						ptr = strtok(stringPtr, " ");	// The VERSION
						ptr = strtok(NULL, "\n");			// The remainder
						
						if (*ptr == '\"')
						{
							ptr++; // Inc past the first "
							build = atoi(ptr);
							fclose(filePtr);

							build++;

							printf ("Local build is %d\n", build);
							writeVersion(target, build);
							break;
						} else
						{
							printf ("Local build is 0. Oops, didn't find a string of the format: '#define VERSION \"x.y.z\"'");
						}
					} // End if if (strstr
				} // End of while
			} // End of if filePtr
			else
			{
				// Didn't find the file, write a new one
				printf ("Local build is %d\n", build);
				writeVersion(target, build);
			}
		}
	}

	return 0;
}