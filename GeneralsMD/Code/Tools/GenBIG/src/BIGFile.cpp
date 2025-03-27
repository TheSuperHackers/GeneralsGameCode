#include "BIGFile.h"
#include <iostream>
#include <fstream>
#include <cstring>

static const char *BIGFileIdentifier = "BIGF";

BIGFile::BIGFile()
{
}

BIGFile::~BIGFile()
{
}

void BIGFile::init()
{
	std::cout << "BIGFile system initialized." << std::endl;
	// Initialize additional resources if needed
}

File* BIGFile::openFile(const Char* filename, Int access)
{
	// Open a file from an archive
	for (auto& pair : m_archiveFileMap)
	{
		File* file = pair.second->openFile(filename, access);
		if (file)
		{
			return file;
		}
	}

	std::cerr << "File not found in any archive: " << filename << std::endl;
	return nullptr;
}

void BIGFile::closeAllFiles()
{
	for (auto& pair : m_archiveFileMap)
	{
		pair.second->closeAllFiles();
	}
	std::cout << "Closed all files in all archives." << std::endl;
}

