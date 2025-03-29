/**
 * @file main.cpp
 *
 * @author DevGeniusCode
 *
 * @brief A CLI tool to handle BIG files for Command & Conquer Generals & Zero Hour
 *
 * @project GenBIG
 *
 * @date March 2025
 *
 * @version 1.0
 *
 * @copyright GenBIG is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */


#include <iostream>
#include "BIGFile.h"

void printHelp()
{
	printf("Usage: GenBIG [options] <path> <path> ...\n\n");

	printf("Options:\n"
							 "  -a <path>  Add a file or directory to the archive\n"
							 "  -d <name>  Delete a file from the archive\n"
							 "  -l         List the contents of the archive\n"
							 "  -x         Extract the contents of the archive\n"
							 "  -o <path>  Output dir\n"
							 "  -h         Display this help message\n\n");

	printf("Examples:"
							 "  Example 1: Add a file to the archive\n"
							 "  GenBIG -a file.txt -o archive.big\n\n"
							 "  Example 2: Add a directory to the archive\n"
							 "  GenBIG -a dir -o archive.big\n\n"
							 "  Example 3: Add multiple files and directories to the archive\n"
							 "  GenBIG -a file1.txt -a file2.txt -a dir1 -a dir2 -o archive.big\n\n"
							 "  Example 4: Extract the contents of the archive\n"
							 "  GenBIG -x archive.big -o dir\n\n"
							 "  Example 5: Extract the contents of the archive from specific directory\n"
							 "  GenBIG -x your/path/archive.big -o .\n\n"
							 "  Example 6: Delete a file from an archive\n"
							 "  GenBIG -d file.txt -o archive.big\n\n");
}

// GLOBALS ////////////////////////////////////////////////////////////////////
HINSTANCE ApplicationHInstance = NULL;  ///< our application instance
HWND ApplicationHWnd = NULL;  ///< our application window handle

const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";
const char *gAppPrefix = ""; /// So WB can have a different debug log file name.

// Main function to handle user input and options
int main(int argc, char *argv[]) {
	printf("GenBIG - BIG Archive Tool By TheSuperHackers v1.0\n\n");

	if (argc < 2) {
		printHelp();
		return 1;
	}

	BIGFile *bigFile = new BIGFile();

	int optionError = 0;

	for (int i = 1; i < argc; ++i) {
		switch (argv[i][0]) {
			case '-':
				switch (argv[i][1]) {
					case 'a':  // Add file or directory to the archive
					case 'd':  // Delete file from archive
						if (i + 1 < argc && argv[i + 1][0] != '-') {
							bigFile->setPaths(argv[i + 1]);
							i++;  // Skip the next argument
						} else {
							printf("Error: Missing path\n");
							optionError = 1;
						}
						break;
					case 'l':  // List the contents of the archive
					case 'x':  // Extract contents from the archive
						if (i + 1 < argc && argv[i + 1][0] != '-') {
							bigFile->setArchiveFileName(argv[i + 1]);
							i++;
						} else {
							printf("Error: Missing archive name\n");
							optionError = 1;
						}
						break;
					case 'o':  // Set output directory or BIG file name to create
						if (i + 1 < argc && argv[i + 1][0] != '-') {
							bigFile->setOutputDir(argv[i + 1]);
							i++;
						} else {
							printf("Error: Missing output directory after -o\n");
							optionError = 1;
						}
						break;
					case 'h':  // Display help message
						printHelp();
						return 0;

					default:
						printf("Error: Unknown option -%c\n", argv[i][1]);
						optionError = 1;
				}
				break;

			default:
				printf("Error: Invalid argument %s\n", argv[i]);
				optionError = 1;
		}
		if (optionError) {
			break;
		}
	}

	if (optionError || bigFile->getArchiveFileName() == NULL) {
		return 1;
	}

	// Perform the appropriate action based on the options
	if (strcmp(argv[1], "-a") == 0) {
		bigFile->addToArchive();
	}
	else if (strcmp(argv[1], "-d") == 0) {
		bigFile->deleteFromArchive();
	}
	else if (strcmp(argv[1], "-l") == 0) {
		bigFile->listArchive();
	}
	else if (strcmp(argv[1], "-x") == 0) {
		bigFile->extractArchive();
	}

	delete bigFile;
	return 0;
}
