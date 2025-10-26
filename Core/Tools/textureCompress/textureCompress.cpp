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

// FILE: textureCompress.cpp //////////////////////////////////////////////////////
// Author: Matthew D. Campbell, Dec 2002

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN  // only bare bones windows stuff wanted
//#include <afxwin.h>
#include <windows.h>
#include <lmcons.h>
#include <stdlib.h>
#include <Utility/stdio_adapter.h>
#include <string.h>

#include "resource.h"
#include <map>
#include <string>
#include <set>
#include <cstdarg>
#include <io.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <trim.h>

static const char *nodxtPrefix[] = {
	"zhca",
	"caust",
	NULL,
};

static const char *nodxtAnywhere[] = {
	"userinterface",
	"controlbar",
	"commandbar",
	NULL,
};

#define LOG(x) logStuff x
static void logStuff(const char *fmt, ...)
{
	static char buffer[1024];
	va_list va;
	va_start( va, fmt );
	vsnprintf(buffer, 1024, fmt, va );
	va_end( va );

	puts(buffer);
	::MessageBox(NULL, buffer, "textureCompress", MB_OK);
}

#ifndef NDEBUG

class DebugMunkee
{
public:
	DebugMunkee(const char *fname = "debugLog.txt") { m_fp = fopen(fname, "w"); }
	~DebugMunkee() { if (m_fp) fclose(m_fp); m_fp = NULL; }

	FILE *m_fp;
};

static DebugMunkee *theDebugMunkee = NULL;

#define DEBUG_LOG(x) debugLog x
static void debugLog(const char *fmt, ...)
{
	static char buffer[1024];
	va_list va;
	va_start( va, fmt );
	vsnprintf(buffer, 1024, fmt, va );
	va_end( va );

	OutputDebugString( buffer );
	puts(buffer);
	if (theDebugMunkee)
		fputs(buffer, theDebugMunkee->m_fp);
}

#else

#define DEBUG_LOG(x) {}

#endif // NDEBUG


static void usage(const char *progname)
{
	if (!progname)
		progname = "textureCompress";
	LOG (("Usage: %s sourceDir destDir cacheDir outFile dxtOutFile\n", progname));
}

class FileInfo
{
public:
	FileInfo() {}
	~FileInfo() {}

	void set( const WIN32_FIND_DATA& info );

	std::string filename;
	time_t creationTime;
	time_t accessTime;
	time_t modTime;
	DWORD attributes;
	DWORD filesize;	// only care about 32 bits for our purposes

protected:
};

struct FileInfoComparator
{
	bool operator()(const FileInfo& a, const FileInfo& b) const
	{
		return a.filename < b.filename;
	}
};

//-------------------------------------------------------------------------------------------------

typedef std::set<FileInfo, FileInfoComparator> FileInfoSet;

//-------------------------------------------------------------------------------------------------

class Directory
{
public:
	Directory(const std::string& dirPath);
	~Directory() {}

	FileInfoSet* getFiles( void );
	FileInfoSet* getSubdirs( void );

protected:
	std::string m_dirPath;

	FileInfoSet m_files;
	FileInfoSet m_subdirs;
};

//-------------------------------------------------------------------------------------------------

static void TimetToFileTime( time_t t, FILETIME& ft )
{
	LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
	ft.dwLowDateTime = (DWORD) ll;
	ft.dwHighDateTime = ll >>32;
}

static time_t FileTimeToTimet( const FILETIME& ft )
{
	LONGLONG ll = (ft.dwHighDateTime << 32) + ft.dwLowDateTime - 116444736000000000;
	ll /= 10000000;
	return (time_t)ll;
}

//-------------------------------------------------------------------------------------------------

void FileInfo::set( const WIN32_FIND_DATA& info )
{
	filename = info.cFileName;
	for (int i=0; i<filename.size(); ++i)
	{
		char c[2] = { tolower(info.cFileName[i]), 0 };
		filename.replace(i, 1, c, 1);
	}
	creationTime = FileTimeToTimet(info.ftCreationTime);
	accessTime = FileTimeToTimet(info.ftLastAccessTime);
	modTime = FileTimeToTimet(info.ftLastWriteTime);
	attributes = info.dwFileAttributes;
	filesize = info.nFileSizeLow;

	struct stat origStat;
	stat( filename.c_str(), &origStat);
	modTime = origStat.st_mtime; // use stat(), since the LONGLONG code is unpredictable

	//DEBUG_LOG(("FileInfo::set(): fname=%s, size=%d, modTime=%d", filename.c_str(), filesize, modTime));
}

//-------------------------------------------------------------------------------------------------

Directory::Directory( const std::string& dirPath ) : m_dirPath(dirPath)
{
	WIN32_FIND_DATA			item;  // search item
	HANDLE							hFile;  // handle for search resources
	char								currDir[ MAX_PATH ];

	// sanity
	if( !m_dirPath.length() )
	{
		return;
	}

	// save the current directory
	GetCurrentDirectory( MAX_PATH, currDir );

	// switch into the directory provided
	if( SetCurrentDirectory( m_dirPath.c_str() ) == 0 )
	{
		return;
	}

	// go through each item in the output directory
	bool done = false;
	hFile = FindFirstFile( "*", &item);
	if( hFile == INVALID_HANDLE_VALUE )
	{
		done = true;
	}

	FileInfo info;

	while (!done)
	{
		// if this is a subdirectory keep the name around till the end
		if( item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if ( strcmp( item.cFileName, "." ) && strcmp( item.cFileName, ".." ) )
			{
				info.set(item);
				m_subdirs.insert( info );
			}
		}
		else
		{
			info.set(item);
			m_files.insert( info );
		}

		if ( FindNextFile( hFile, &item ) == 0 )
		{
			done = true;
		}
	}

	// close search
	FindClose( hFile );

	// restore the working directory to what it was when we started here
	SetCurrentDirectory( currDir );
}

FileInfoSet* Directory::getFiles( void )
{
	return &m_files;
}

FileInfoSet* Directory::getSubdirs( void )
{
	return &m_subdirs;
}

//-------------------------------------------------------------------------------------------------
typedef std::set<std::string> StringSet;

//-------------------------------------------------------------------------------------------------
void eraseCachedFiles(const std::string& sourceDirName, const std::string& targetDirName, const std::string& cacheDirName,
											StringSet& cachedFilesToErase)
{
	StringSet::const_iterator sit;
	for (sit = cachedFilesToErase.begin(); sit != cachedFilesToErase.end(); ++sit)
	{
		std::string src = cacheDirName;
		src.append("\\");
		src.append(*sit);

		DEBUG_LOG(("Erasing cached file: %s", src.c_str()));
		DeleteFile(src.c_str());
	}
}

//-------------------------------------------------------------------------------------------------
void copyCachedFiles(const std::string& sourceDirName, const std::string& targetDirName, const std::string& cacheDirName,
										 StringSet& cachedFilesToCopy)
{
	StringSet::const_iterator sit;
	for (sit = cachedFilesToCopy.begin(); sit != cachedFilesToCopy.end(); ++sit)
	{
		std::string src = cacheDirName;
		src.append("\\");
		src.append(*sit);

		std::string dest = targetDirName;
		dest.append("\\");
		dest.append(*sit);

		DEBUG_LOG(("Copying cached file: %s", src.c_str()));
		if (_chmod(dest.c_str(), _S_IWRITE | _S_IREAD) == -1)
		{
			DEBUG_LOG(("Cannot chmod '%s'", dest.c_str()));
		}
		CopyFile(src.c_str(), dest.c_str(), FALSE);
	}
}

//-------------------------------------------------------------------------------------------------
void compressOrigFiles(const std::string& sourceDirName, const std::string& targetDirName, const std::string& cacheDirName,
											 StringSet& origFilesToCompress, const std::string& dxtOutFname)
{
	char tmpPath[_MAX_PATH] = "C:\\temp\\";
	char tmpFname[_MAX_PATH] = "C:\\temp\\tmp.txt";
	GetTempPath(_MAX_PATH, tmpPath);
	GetTempFileName(tmpPath, "tex", 0, tmpFname);
	HANDLE h = CreateFile(tmpFname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	if (!h)
	{
		DEBUG_LOG(("Could not create temp file '%s'!  Unable to compress textures!", tmpFname));
	}

	StringSet::const_iterator sit;
	for (sit = origFilesToCompress.begin(); sit != origFilesToCompress.end(); ++sit)
	{
		std::string tmp = sourceDirName;
		tmp.append("\\");
		tmp.append(*sit);
		tmp.append("\n");
		DEBUG_LOG(("Compressing file: %s", tmp.c_str()));
		DWORD len;
		WriteFile(h, tmp.c_str(), tmp.length(), &len, NULL);
	}
	CloseHandle(h);

	std::string commandLine;
	commandLine = "\\projects\\rts\\build\\nvdxt -list ";
	commandLine.append(tmpFname);
	commandLine.append(" -24 dxt1c -32 dxt5 -full -outdir ");
	commandLine.append(cacheDirName);
	commandLine.append(" > ");
	commandLine.append(dxtOutFname);

	DEBUG_LOG(("Compressing textures with command line of '%s'", commandLine.c_str()));
	int ret = system(commandLine.c_str());
	DEBUG_LOG(("system(%s) returned %d", commandLine.c_str(), ret));
	DeleteFile(tmpFname);

	// now copy compressed file to target dir
	for (sit = origFilesToCompress.begin(); sit != origFilesToCompress.end(); ++sit)
	{
		std::string orig = sourceDirName;
		orig.append("\\");
		orig.append(*sit);

		struct stat origStat;
		stat( orig.c_str(), &origStat);

		struct _utimbuf utb;
		utb.actime = origStat.st_atime;
		utb.modtime = origStat.st_mtime;

		std::string src = cacheDirName;
		src.append("\\");
		src.append(*sit);
		src.replace(src.size()-4, 4, ".dds");

		_utime(src.c_str(), &utb);

		std::string dest = targetDirName;
		dest.append("\\");
		dest.append(*sit);
		dest.replace(dest.size()-4, 4, ".dds");

		DEBUG_LOG(("Copying new file from %s to %s", src.c_str(), dest.c_str()));

		if (_chmod(dest.c_str(), _S_IWRITE | _S_IREAD) == -1)
		{
			DEBUG_LOG(("Cannot chmod '%s'", dest.c_str()));
		}
		BOOL ret = CopyFile(src.c_str(), dest.c_str(), FALSE);
		if (!ret)
		{
			DEBUG_LOG(("Could not copy file!"));
		}

		_utime(dest.c_str(), &utb);
	}
}

//-------------------------------------------------------------------------------------------------
void copyOrigFiles(const std::string& sourceDirName, const std::string& targetDirName, const std::string& cacheDirName,
									 StringSet& origFilesToCopy)
{
	StringSet::const_iterator sit;
	for (sit = origFilesToCopy.begin(); sit != origFilesToCopy.end(); ++sit)
	{
		std::string src = sourceDirName;
		src.append("\\");
		src.append(*sit);

		std::string dest = targetDirName;
		dest.append("\\");
		dest.append(*sit);

		if (_chmod(dest.c_str(), _S_IWRITE | _S_IREAD) == -1)
		{
			DEBUG_LOG(("Cannot chmod '%s'", dest.c_str()));
		}
		BOOL res = CopyFile(src.c_str(), dest.c_str(), FALSE);
		DEBUG_LOG(("Copying file: %s returns %d", src.c_str(), res));
	}
}

//-------------------------------------------------------------------------------------------------
static void scanDir( const std::string& sourceDirName, const std::string& targetDirName, const std::string& cacheDirName, const std::string& dxtOutFname )
{
	DEBUG_LOG(("Scanning '%s'", sourceDirName.c_str()));
	Directory sourceDir(sourceDirName);

	DEBUG_LOG(("Scanning '%s'", targetDirName.c_str()));
	Directory targetDir(targetDirName);

	DEBUG_LOG(("Scanning '%s'", cacheDirName.c_str()));
	Directory cacheDir(cacheDirName);

	FileInfoSet *sourceFiles = sourceDir.getFiles();
	FileInfoSet *cacheFiles = cacheDir.getFiles();
	FileInfoSet *targetFiles = targetDir.getFiles();

	StringSet cachedFilesToErase;
	StringSet cachedFilesToCopy;
	StringSet origFilesToCompress;
	StringSet origFilesToCopy;

	DEBUG_LOG(("Emptying targetDir"));
	for (FileInfoSet::iterator targetIt = targetFiles->begin(); targetIt != targetFiles->end(); ++targetIt)
	{
		FileInfo f = *targetIt;
		std::string fname = f.filename;
		f.filename.replace(f.filename.size()-4, 4, ".tga");
		FileInfoSet::iterator fit = sourceFiles->find(f);
		if (fit == sourceFiles->end())
		{
			// look for pre-existing dds files too
			f.filename.replace(f.filename.size()-4, 4, ".dds");
			FileInfoSet::iterator ddsfit = sourceFiles->find(f);
			if (ddsfit == sourceFiles->end())
			{
				fname.insert(0, "\\");
				fname.insert(0, targetDirName);
				DEBUG_LOG(("Deleting now-removed file '%s'", fname.c_str()));
				DeleteFile(fname.c_str());
			}
		}
	}

	for (FileInfoSet::iterator cacheIt = cacheFiles->begin(); cacheIt != cacheFiles->end(); ++cacheIt)
	{
		FileInfo f = *cacheIt;
		int len = f.filename.size();
		if (len < 5)
		{
			cachedFilesToErase.insert(f.filename);
			continue;
		}
		std::string fname = f.filename;
		f.filename.replace(len-4, 4, ".tga");
		FileInfoSet::iterator fit = sourceFiles->find(f);
		if (fit != sourceFiles->end())
		{
			FileInfo sf = *fit;
			if (f.modTime < sf.modTime)
			{
				/**
				std::string orig = sourceDirName;
				orig.append("\\");
				orig.append(sf.filename);

				struct stat origStat;
				stat( orig.c_str(), &origStat);

				struct _utimbuf utb;
				utb.actime = origStat.st_atime;
				utb.modtime = origStat.st_mtime;

				std::string dest = cacheDirName;
				dest.append("\\");
				dest.append(f.filename);
				dest.replace(dest.size()-4, 4, ".dds");

				_utime(dest.c_str(), &utb);

				cachedFilesToCopy.insert(fname);
				/**/
				cachedFilesToErase.insert(fname);
			}
			else
			{
				f.filename = fname; // back to .dds
				FileInfoSet::iterator it = targetFiles->find(f);
				if (it == targetFiles->end())
					cachedFilesToCopy.insert(fname);
			}
		}
		else
		{
			cachedFilesToErase.insert(fname);
		}
	}

	for (FileInfoSet::iterator sourceIt = sourceFiles->begin(); sourceIt != sourceFiles->end(); ++sourceIt)
	{
		FileInfo f = *sourceIt;

		std::string fname = f.filename;
		const char *s = fname.c_str();
		int index = 0;
		const char *check = nodxtPrefix[0];
		bool shouldSkip = false;
		while (check)
		{
			if (fname.find(check) == 0)
			{
				shouldSkip = true;
				break;
			}
			check = nodxtPrefix[++index];
		}

		index = 0;
		check = nodxtAnywhere[0];
		while (check && !shouldSkip)
		{
			if (fname.find(check) != fname.npos)
			{
				shouldSkip = true;
				break;
			}
			check = nodxtAnywhere[++index];
		}

		if (!shouldSkip)
		{
			// check for preexisting .dds files so we can just copy them
			if (fname.find(".dds") != fname.npos)
			{
				shouldSkip = true;
			}
		}

		if (shouldSkip)
		{
			origFilesToCopy.insert(s);
		}
		else
		{
			int len = f.filename.size();
			f.filename.replace(len-4, 4, ".dds");
			FileInfoSet::iterator fit = cacheFiles->find(f);
			if (fit != cacheFiles->end())
			{
				FileInfo cf = *fit;
				if (cf.modTime < f.modTime)
				{
					origFilesToCompress.insert(fname);
				}
			}
			else
			{
				origFilesToCompress.insert(fname);
			}
		}
	}

	// now dump our files
	eraseCachedFiles (sourceDirName, targetDirName, cacheDirName, cachedFilesToErase);
	copyCachedFiles  (sourceDirName, targetDirName, cacheDirName, cachedFilesToCopy);
	copyOrigFiles    (sourceDirName, targetDirName, cacheDirName, origFilesToCopy);
	compressOrigFiles(sourceDirName, targetDirName, cacheDirName, origFilesToCompress, dxtOutFname);
}

//-------------------------------------------------------------------------------------------------
#define USE_WINMAIN
#ifdef USE_WINMAIN
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
#else
int main(int argc, const char **argv)
{
#endif // USE_WINMAIN

	if (argc != 6)
	{
		usage(argv[0]);
	}
	else
	{
		const char *sourceDir = argv[1];
		const char *targetDir = argv[2];
		const char *cacheDir  = argv[3];

#ifndef NDEBUG
		theDebugMunkee = new DebugMunkee(argv[4]);
#endif

		//setUpLoadWindow();
		scanDir(sourceDir, targetDir, cacheDir, argv[5]);
		//setLoadWindowText("Writing to file...");
		//printSet( noAlphaChannel, "No Alpha Channel" );
		//printSet( noAlpha, "Not Using Alpha Channel" );
		//printSet( hasAlpha, "Using Alpha Channel" );
		//tearDownLoadWindow();

#ifndef NDEBUG
		delete theDebugMunkee;
		theDebugMunkee = NULL;
#endif
	}

	return 0;
}
