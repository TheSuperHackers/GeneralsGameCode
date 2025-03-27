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

class BIGFile : public Win32BIGFileSystem,
{
public:
		BIGFile();
		~BIGFile();

		void init() override;

		// Archive operations
		Bool loadBigFileFromDirectory(AsciiString dir, AsciiString fileMask, Bool overwrite = FALSE) override;

		// File operations
		File* openFile(const Char* filename, Int access = 0) override;
		void closeAllFiles() override;

protected:
		virtual void loadIntoDirectoryTree(const ArchiveFile* archiveFile, const AsciiString& archiveFilename, Bool overwrite = FALSE) override;

private:
		ArchiveFileMap m_archiveFileMap;
		ArchivedDirectoryInfo m_rootDirectory;
};



#endif //GENZH_BIGFILE_H
