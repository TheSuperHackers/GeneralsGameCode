//
// Created by User on 27/03/2025.
//

#include <iostream>
#include "main.h"

void printHelp()
{
	printf("Usage: GenBIG [options] <path> <path> ...\n\n");

	printf("Options:\n"
							 "  -a <path>  Add a file or directory to the archive\n"
							 "  -d <name>  Delete a file from the archive\n"
							 "  -l         List the contents of the archive\n"
							 "  -x         Extract the contents of the archive\n"
							 "  -o <path>  Output file\n"
							 "  -h         Display this help message\n\n");

	printf("Examples:"
							 "  Example 1: Add a file to the archive\n"
							 "  GenBIG -a file.txt -o archive.big\n\n"
							 "  Example 2: Add a directory to the archive\n"
							 "  GenBIG -a dir -o archive.big\n\n"
							 "  Example 3: Add multiple files and directories to the archive\n"
							 "  GenBIG -a file1.txt -a file2.txt -a dir1 -a dir2 -o archive.big\n\n"
							 "  Example 4: Extract the contents of the archive\n"
							 "  GenBIG -x -o archive.big\n\n"
							 "  Example 5: Extract the contents of the archive to a specific directory\n"
							 "  GenBIG -x -o your/path/archive.big\n\n");
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printHelp();
		return 1;
	}

	// Parse the command line arguments
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
				case 'a':
					break;
				case 'd':
					break;
				case 'l':
					break;
				case 'x':
					break;
				case 'o':
					break;
				case 'h':
					printHelp();
					break;
				default:
					printf("Invalid option: %s\n", argv[i]);
					printHelp();
					break;
			}
		}
		else if (i == 1)
			{
				printf("Invalid option: %s\n", argv[i]);
				printHelp();
				return 1;
			}
		}

	return 0;
}
