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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

///////// StdLocalFileSystem.cpp /////////////////////////
// Stephan Vedder, April 2025
////////////////////////////////////////////////////////////

#include "Common/AsciiString.h"
#include "Common/GameMemory.h"
#include "Common/PerfTimer.h"
#include "StdDevice/Common/StdLocalFileSystem.h"
#include "StdDevice/Common/StdLocalFile.h"

#include <filesystem>

StdLocalFileSystem::StdLocalFileSystem() : LocalFileSystem() 
{
}

StdLocalFileSystem::~StdLocalFileSystem() {
}

//DECLARE_PERF_TIMER(StdLocalFileSystem_openFile)
File * StdLocalFileSystem::openFile(const Char *filename, Int access /* = 0 */) 
{
	//USE_PERF_TIMER(StdLocalFileSystem_openFile)
	StdLocalFile *file = newInstance( StdLocalFile );	

	// sanity check
	if (strlen(filename) <= 0) {
		file->deleteInstance();
		return NULL;
	}

	std::string fixedFilename(filename);

#ifndef _WIN32
	// Replace backslashes with forward slashes on unix
	std::replace(fixedFilename.begin(), fixedFilename.end(), '\\', '/');
#endif

	// Convert the filename to a std::filesystem::path and pass that
	// make_preferred does not convert from Windows to Linux (see https://en.cppreference.com/w/cpp/filesystem/path/make_preferred)
	std::filesystem::path path(fixedFilename);
	path = path.make_preferred();

#ifndef _WIN32
	// check if the file exists to see if fixup is required
	// if it's not found try to match disregarding case sensitivity
	if (!std::filesystem::exists(path)) {
		// Traverse path to try and match case-insensitively
		std::filesystem::path parent = path.parent_path();
		std::filesystem::path filename = path.filename();
		std::error_code ec;

		std::filesystem::path pathFixed;
		std::filesystem::path pathCurrent;
		for (auto& p : path)
		{
			std::filesystem::path pathFixedPart;
			if (pathCurrent.empty())
			{
				// Load the first part of the path
				pathFixed /= p;
				pathCurrent /= p;
				continue;
			}

			if (std::filesystem::exists(pathCurrent / p, ec))
			{
				pathFixedPart = p;
			}
			else if (std::filesystem::exists(pathFixed / p, ec))
			{
				pathFixedPart = p;
			}
			else
			{
				// Check if the subpath exists using case-insensitive comparison
				for (auto& entry : std::filesystem::directory_iterator(pathFixed, ec))
				{
					if (strcasecmp(entry.path().filename().string().c_str(), p.string().c_str()) == 0)
					{
						pathFixedPart = entry.path().filename();
						break;
					}
				}
			}

			if (pathFixedPart.empty())
			{
				// Required to allow creation of new files
				if (!(access & File::WRITE)) 
				{
					DEBUG_LOG(("StdLocalFileSystem::openFile - Error finding file %s\n", filename.string().c_str()));
					DEBUG_LOG(("StdLocalFileSystem::openFile - Got so far %s\n", pathCurrent.string().c_str()));	

					file->deleteInstance();
					return NULL;	
				}

				// Use the last known good path
				pathFixed = p;
			}

			// Copy of the current path to mirror the current depth
			pathFixed /= pathFixedPart;
			pathCurrent /= p;
		}
		path = pathFixed;
	}
#endif

	if (access & File::WRITE) {
		// if opening the file for writing, we need to make sure the directory is there
		// before we try to create the file.
		std::filesystem::path dir = path.parent_path();
		if (!std::filesystem::exists(dir)) {
			std::error_code ec;
			if(!std::filesystem::create_directories(dir,ec ) || ec) {
				DEBUG_LOG(("StdLocalFileSystem::openFile - Error creating directory %s\n", dir.string().c_str()));
				file->deleteInstance();
				return NULL;
			}
		}
	}
	
	if (file->open(path.string().c_str(), access) == FALSE) {
		file->close();
		file->deleteInstance();
		file = NULL;
	} else {
		file->deleteOnClose();
	}

// this will also need to play nice with the STREAMING type that I added, if we ever enable this

// srj sez: this speeds up INI loading, but makes BIG files unusable. 
// don't enable it without further tweaking.
//
// unless you like running really slowly.
//	if (!(access&File::WRITE)) {
//		// Return a ramfile.
//		RAMFile *ramFile = newInstance( RAMFile );
//		if (ramFile->open(file)) {
//			file->close(); // is deleteonclose, so should delete.
//			ramFile->deleteOnClose();
//			return ramFile;
//		}	else {
//			ramFile->close();
//			ramFile->deleteInstance();
//		}
//	}

	return file;
}

void StdLocalFileSystem::update() 
{
}

void StdLocalFileSystem::init() 
{
}

void StdLocalFileSystem::reset() 
{
}

//DECLARE_PERF_TIMER(StdLocalFileSystem_doesFileExist)
Bool StdLocalFileSystem::doesFileExist(const Char *filename) const
{
	// Convert to host path
	std::filesystem::path path(filename);
	path = path.make_preferred();

	return std::filesystem::exists(path);
}

void StdLocalFileSystem::getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList & filenameList, Bool searchSubdirectories) const
{

	char search[_MAX_PATH];
	AsciiString asciisearch;
	asciisearch = originalDirectory;
	asciisearch.concat(currentDirectory);
		auto searchExt = std::filesystem::path(searchName.str()).extension();
	if (asciisearch.isEmpty()) {
				asciisearch = ".";
	}
		strcpy(search, asciisearch.str());

	Bool done = FALSE;
	std::error_code ec;

	auto iter = std::filesystem::directory_iterator(search, ec);
	done = iter == std::filesystem::directory_iterator();

	if (ec) {
		DEBUG_LOG(("StdLocalFileSystem::getFileListInDirectory - Error opening directory %s\n", search));
		return;
	}

	while (!done)	{
				if (!iter->is_directory() && iter->path().extension() == searchExt &&
				(strcmp(iter->path().filename().string().c_str(), ".") && strcmp(iter->path().string().c_str(), ".."))) {
			// if we haven't already, add this filename to the list.
				// a stl set should only allow one copy of each filename
				AsciiString newFilename = iter->path().string().c_str();
				if (filenameList.find(newFilename) == filenameList.end()) {
					filenameList.insert(newFilename);
				}
		}

		iter++;
		done = iter == std::filesystem::directory_iterator();
	}

	if (searchSubdirectories) {
		AsciiString subdirsearch;
		subdirsearch = originalDirectory;
		subdirsearch.concat(currentDirectory);
		if (subdirsearch.isEmpty()) {
			subdirsearch = ".";
		}

		auto iter = std::filesystem::directory_iterator(subdirsearch.str(), ec);

		if (ec) {
			DEBUG_LOG(("StdLocalFileSystem::getFileListInDirectory - Error opening subdirectory %s\n", subdirsearch.str()));
			return;
		}

		done = iter == std::filesystem::directory_iterator();

		while (!done) {
			if(iter->is_directory() && 
				(strcmp(iter->path().filename().string().c_str(), ".") && strcmp(iter->path().string().c_str(), ".."))) {
					AsciiString tempsearchstr;
					tempsearchstr = iter->path().filename().string().c_str();
					
					// recursively add files in subdirectories if required.
					getFileListInDirectory(tempsearchstr, originalDirectory, searchName, filenameList, searchSubdirectories);
			}

			iter++;
			done = iter == std::filesystem::directory_iterator();
		}
	}
}

Bool StdLocalFileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const 
{
		// Convert to host path
		std::filesystem::path path(filename.str());
		path = path.make_preferred();
	
		std::error_code ec;
		auto file_size = std::filesystem::file_size(path, ec);
		if (ec)
		{
				return FALSE;
		}

	auto write_time = std::filesystem::last_write_time(path, ec);
		if (ec)
		{
				return FALSE;
		}

	// TODO: fix this to be win compatible (time since 1601)
	auto time = write_time.time_since_epoch().count();
	fileInfo->timestampHigh = time >> 32;
	fileInfo->timestampLow = time & UINT32_MAX;
	fileInfo->sizeHigh      = file_size >> 32;
	fileInfo->sizeLow  = file_size & UINT32_MAX;

	return TRUE;
}

Bool StdLocalFileSystem::createDirectory(AsciiString directory) 
{
	bool result = FALSE;
	if ((directory.getLength() > 0) && (directory.getLength() < _MAX_DIR)) {
		// Convert to host path
		std::filesystem::path path(directory.str());
		path = path.make_preferred();

		std::error_code ec;
		result = std::filesystem::create_directory(directory.str(), ec);
		if (ec) {
			result = FALSE;
		}
	}
	return result;
}
