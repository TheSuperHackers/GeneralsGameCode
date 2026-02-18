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
//  (c) 2001-2003 Electronic Arts Inc.
//  //
//																																						//
////////////////////////////////////////////////////////////////////////////////

///////// StdLocalFileSystem.cpp /////////////////////////
// Stephan Vedder, April 2025
////////////////////////////////////////////////////////////

#include "StdLocalFileSystem.h"
#include "Common/AsciiString.h"
#include "Common/GameMemory.h"
#include "Common/PerfTimer.h"
#include "StdLocalFile.h"

#include <algorithm>
#include <filesystem>
#include <strings.h>

StdLocalFileSystem::StdLocalFileSystem() : LocalFileSystem() {}

StdLocalFileSystem::~StdLocalFileSystem() {}

// DECLARE_PERF_TIMER(StdLocalFileSystem_openFile)
static std::filesystem::path fixFilenameFromWindowsPath(const Char *filename,
                                                        Int access) {
  std::string fixedFilename(filename);

#ifndef _WIN32
  // Replace backslashes with forward slashes on unix
  std::replace(fixedFilename.begin(), fixedFilename.end(), '\\', '/');
#endif

  std::error_code ec;
  std::filesystem::path p(fixedFilename);

  if (std::filesystem::exists(p, ec)) {
    return p;
  }

  // File doesn't exist as-is. Try to match components case-insensitively.
  std::filesystem::path currentPath = ".";
  bool isAbsolute = p.is_absolute();
  if (isAbsolute) {
    currentPath = p.root_path();
  }

  for (auto const &component : p) {
    if (component.empty() || component == "/" || component == "." ||
        component == "..")
      continue;

    bool found = false;
    // Check if the component exists in currentPath case-insensitively
    if (std::filesystem::exists(currentPath, ec) &&
        std::filesystem::is_directory(currentPath, ec)) {
      printf("DEBUG: fixPath: Iterating directory '%s' looking for '%s'\n",
             currentPath.string().c_str(), component.string().c_str());
      for (auto const &entry :
           std::filesystem::directory_iterator(currentPath, ec)) {
        std::string filenameStr = entry.path().filename().string();
        printf("DEBUG: fixPath:   found '%s'\n", filenameStr.c_str());
        if (strcasecmp(filenameStr.c_str(), component.string().c_str()) == 0) {

          currentPath /= entry.path().filename();
          found = true;
          printf("DEBUG: fixPath:   MATCH FOUND: '%s' -> '%s'\n",
                 component.string().c_str(), filenameStr.c_str());
          break;
        }
      }
      if (!found) {
        printf("DEBUG: fixPath:   NO MATCH for '%s' in '%s'\n",
               component.string().c_str(), currentPath.string().c_str());
      }
    } else {
      printf("DEBUG: fixPath:   NOT A DIRECTORY or NOT EXISTS: '%s' (ec=%d)\n",
             currentPath.string().c_str(), ec.value());
    }
    fflush(stdout);

    if (!found) {
      if (access & File::WRITE) {
        // If writing, we can create the rest of the path
        currentPath /= component;
      } else {
        // For reading, if one component is missing, the whole path is invalid
        DEBUG_LOG(("StdLocalFileSystem::fixFilenameFromWindowsPath - FAILED to "
                   "find %s (stuck at %s, looking for %s)",
                   filename, currentPath.string().c_str(),
                   component.string().c_str()));
        return std::filesystem::path();
      }
    }
  }

  return currentPath;
}

File *StdLocalFileSystem::openFile(const Char *filename, Int access,
                                   size_t bufferSize) {
  // USE_PERF_TIMER(StdLocalFileSystem_openFile)

  // sanity check
  if (strlen(filename) <= 0) {
    return nullptr;
  }

  std::filesystem::path path = fixFilenameFromWindowsPath(filename, access);

  if (path.empty()) {
    return nullptr;
  }

  if (access & File::WRITE) {
    // if opening the file for writing, we need to make sure the directory is
    // there before we try to create the file.
    std::filesystem::path dir = path.parent_path();
    if (!dir.empty()) {
      std::error_code ec;
      if (!std::filesystem::exists(dir, ec) || ec) {
        if (!std::filesystem::create_directories(dir, ec) || ec) {
          DEBUG_LOG(
              ("StdLocalFileSystem::openFile - Error creating directory %s",
               dir.string().c_str()));
          return nullptr;
        }
      }
    }
  }

  StdLocalFile *file = newInstance(StdLocalFile);

  if (file->open(path.string().c_str(), access, bufferSize) == FALSE) {
    deleteInstance(file);
    file = nullptr;
  } else {
    file->deleteOnClose();
  }

  // this will also need to play nice with the STREAMING type that I added, if
  // we ever enable this

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
  //			deleteInstance(ramFile);
  //		}
  //	}

  return file;
}

void StdLocalFileSystem::update() {}

void StdLocalFileSystem::init() {}

void StdLocalFileSystem::reset() {}

// DECLARE_PERF_TIMER(StdLocalFileSystem_doesFileExist)
Bool StdLocalFileSystem::doesFileExist(const Char *filename) const {
  std::filesystem::path path = fixFilenameFromWindowsPath(filename, 0);
  if (path.empty()) {
    return FALSE;
  }

  std::error_code ec;
  return std::filesystem::exists(path, ec);
}

void StdLocalFileSystem::getFileListInDirectory(
    const AsciiString &currentDirectory, const AsciiString &originalDirectory,
    const AsciiString &searchName, FilenameList &filenameList,
    Bool searchSubdirectories) const {

  AsciiString asciisearch;
  asciisearch = originalDirectory;
  asciisearch.concat(currentDirectory);
  auto searchExt = std::filesystem::path(searchName.str()).extension();
  if (asciisearch.isEmpty()) {
    asciisearch = ".";
  }

  std::filesystem::path fixedPath =
      fixFilenameFromWindowsPath(asciisearch.str(), 0);
  if (fixedPath.empty()) {
    printf("StdLocalFileSystem::getFileListInDirectory: path NOT found: %s\n",
           asciisearch.str());
    fflush(stdout);
    return;
  }
  std::string fixedDirectory = fixedPath.string();

  Bool done = FALSE;
  std::error_code ec;

  printf("StdLocalFileSystem::getFileListInDirectory: looking for %s in %s "
         "(fixed: %s)\n",
         searchName.str(), originalDirectory.str(), fixedDirectory.c_str());
  fflush(stdout);

  auto iter = std::filesystem::directory_iterator(fixedPath, ec);
  // The default iterator constructor creates an end iterator
  done = iter == std::filesystem::directory_iterator();

  if (ec) {
    DEBUG_LOG(("StdLocalFileSystem::getFileListInDirectory - Error opening "
               "directory %s (%s)",
               fixedDirectory.c_str(), ec.message().c_str()));
    printf("StdLocalFileSystem::getFileListInDirectory ERROR: %s\n",
           ec.message().c_str());
    fflush(stdout);
    return;
  }

  while (!done) {
    std::string filenameStr = iter->path().filename().string();
    auto ext = iter->path().extension();
    printf("  Checking file: %s (ext: %s, target: %s)\n", filenameStr.c_str(),
           ext.c_str(), searchExt.c_str());
    bool extMatch = strcasecmp(ext.c_str(), searchExt.c_str()) == 0;
    if (!iter->is_directory() && extMatch &&
        (strcmp(filenameStr.c_str(), ".") != 0 &&
         strcmp(filenameStr.c_str(), "..") != 0)) {
      // if we haven't already, add this filename to the list.
      // a stl set should only allow one copy of each filename
      std::string pathStr = iter->path().string();
      std::replace(pathStr.begin(), pathStr.end(), '/', '\\');
      AsciiString newFilename = pathStr.c_str();
      if (filenameList.find(newFilename) == filenameList.end()) {
        filenameList.insert(newFilename);
      }
    }

    iter++;
    done = iter == std::filesystem::directory_iterator();
  }

  if (searchSubdirectories) {
    auto iter = std::filesystem::directory_iterator(fixedDirectory, ec);

    if (ec) {
      DEBUG_LOG(("StdLocalFileSystem::getFileListInDirectory - Error opening "
                 "subdirectory %s",
                 fixedDirectory.c_str()));
      return;
    }

    // The default iterator constructor creates an end iterator
    done = iter == std::filesystem::directory_iterator();

    while (!done) {
      std::string filenameStr = iter->path().filename().string();
      if (iter->is_directory() && (strcmp(filenameStr.c_str(), ".") != 0 &&
                                   strcmp(filenameStr.c_str(), "..") != 0)) {
        AsciiString tempsearchstr = currentDirectory;
        if (!tempsearchstr.isEmpty())
          tempsearchstr.concat("/");
        tempsearchstr.concat(filenameStr.c_str());

        // recursively add files in subdirectories if required.
        getFileListInDirectory(tempsearchstr, originalDirectory, searchName,
                               filenameList, searchSubdirectories);
      }

      iter++;
      done = iter == std::filesystem::directory_iterator();
    }
  }
}

Bool StdLocalFileSystem::getFileInfo(const AsciiString &filename,
                                     FileInfo *fileInfo) const {
  std::filesystem::path path = fixFilenameFromWindowsPath(filename.str(), 0);

  if (path.empty()) {
    return FALSE;
  }

  std::error_code ec;
  auto file_size = std::filesystem::file_size(path, ec);
  if (ec) {
    return FALSE;
  }

  auto write_time = std::filesystem::last_write_time(path, ec);
  if (ec) {
    return FALSE;
  }

  // TODO: fix this to be win compatible (time since 1601)
  auto time = write_time.time_since_epoch().count();
  fileInfo->timestampHigh = time >> 32;
  fileInfo->timestampLow = time & UINT32_MAX;
  fileInfo->sizeHigh = file_size >> 32;
  fileInfo->sizeLow = file_size & UINT32_MAX;

  return TRUE;
}

Bool StdLocalFileSystem::createDirectory(AsciiString directory) {
  bool result = FALSE;

  std::string fixedDirectory(directory.str());

#ifndef _WIN32
  // Replace backslashes with forward slashes on unix
  std::replace(fixedDirectory.begin(), fixedDirectory.end(), '\\', '/');
#endif

  if ((!fixedDirectory.empty()) && (fixedDirectory.length() < _MAX_DIR)) {
    // Convert to host path
    std::filesystem::path path(std::move(fixedDirectory));

    std::error_code ec;
    result = std::filesystem::create_directory(path, ec);
    if (ec) {
      result = FALSE;
    }
  }
  return result;
}

AsciiString
StdLocalFileSystem::normalizePath(const AsciiString &filePath) const {
  std::string nonNormalized(filePath.str());
#ifndef _WIN32
  // Replace backslashes with forward slashes on non-Windows platforms
  std::replace(nonNormalized.begin(), nonNormalized.end(), '\\', '/');
#endif
  std::filesystem::path pathNonNormalized(nonNormalized);
  return AsciiString(pathNonNormalized.lexically_normal().string().c_str());
}
