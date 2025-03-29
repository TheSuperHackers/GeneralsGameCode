//
// Created by User on 27/03/2025.
//

#ifndef GENZH_BIGFILE_H
#define GENZH_BIGFILE_H

#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/file.h"
#include "Common/LocalFileSystem.h"
#include "Win32Device/Common/Win32BIGFile.h"
#include "Win32Device/Common/Win32BIGFileSystem.h"

class BIGFile : Win32BIGFileSystem
{
public:
		BIGFile();
		~BIGFile();

		// Initialize the archive file system
		void init( void ) override;

		// Handle archive operations
		bool addToArchive();
		bool addDirectoryToArchive(const char* path);
		bool deleteFromArchive(const char* name = NULL);
		bool listArchive();
		bool extractArchive(const char* path = NULL);

		// getters
		ArchiveFile* getArchiveFile() const { return m_archiveFile; }
		const char* getArchiveFileName() const { return m_archiveName; }
		const char* getOutputDir() const { return m_outputDir; }
		const char** getPaths() const { return m_paths; }
		int getPathCount() const { return m_pathCount; }

		// setters
		bool setArchiveFileName(const char* path);
		bool setOutputDir(const char* path);
		bool setPaths(const char* path);

private:
		Win32BIGFileSystem * m_archiveFileSystem; // or Win32BIGFileSystem?
		ArchiveFile* m_archiveFile;
		const char *m_archiveName;
		const char *m_outputDir;
		const char **m_paths;
		int m_pathCount;
		// list files to archive
		std::vector<std::string> m_files;
};

#endif //GENZH_BIGFILE_H
