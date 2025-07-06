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

// FILE: buildVersionUpdate.cpp //////////////////////////////////////////////////////
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
#define VERSION_MAJOR "VERSION_MAJOR"
#define VERSION_MINOR "VERSION_MINOR"
#define VERSION_BUILDNUM "VERSION_BUILDNUM"
#define VERSION_STRING "VERSION_STRING"
#define FORMAT "#define " VERSION_STRING " \"%d.%d.%d\"\n"
#define COMMENTS "// Do not modify this file by hand. Auto-created and\n// Updated by buildVersionUpdate.\n"
#define NUMFMT "#define %s %d\n"
#define NUMFMT_MINOR "#define %s %d			///< This effects the replay version number.\n"

static void writeVersion(char *file, int major, int minor, int build)
{
	FILE *filePtr = fopen(file, "w");
	// Clobber the file.  Hey, this is a simple program.
	if (file)
	{
		if (filePtr)
		{
			fprintf(filePtr, COMMENTS);
			fprintf(filePtr, FORMAT, major, minor, build);
			fprintf(filePtr, NUMFMT, VERSION_MAJOR, major);
			fprintf(filePtr, NUMFMT_MINOR, VERSION_MINOR, minor);
			fprintf(filePtr, NUMFMT, VERSION_BUILDNUM, build);
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

	int major = 1;
	int minor = 0;
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
							ptr = strtok(ptr, ".");	// The first number
							major = atoi(ptr);
							ptr = strtok(NULL, ".");  // The second number
							minor = atoi(ptr);
							ptr = strtok(NULL, "\""); // The final number
							build = atoi(ptr);
							fclose(filePtr);

							writeVersion(target, major, minor, ++build);
							printf ("Build %d Version %d.%d.%d\n", build, major, minor, build);
							break;
						} else
						{
							printf ("Build 0. Oops, didn't find a string of the format: '#define VERSION \"x.y.z\"'");
						}
					} // End if if (strstr
				} // End of while
			} // End of if filePtr
			else
			{
				// Didn't find the file, write a new one
				writeVersion(target, major, minor, build);
			}
		}
	}

	return 0;
}