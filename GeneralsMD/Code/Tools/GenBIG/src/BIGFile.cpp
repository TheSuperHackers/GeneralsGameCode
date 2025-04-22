#include "BIGFile.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>

BIGFile::BIGFile()
{
	init();
}

BIGFile::~BIGFile()
{
	if (m_archiveFile != nullptr) {
		m_archiveFile->close();
		delete m_archiveFile;
		m_archiveFile = nullptr;
	}
	if (m_paths != nullptr) {
		delete[] m_paths;
		m_paths = nullptr;
	}
	if (m_outputDir != nullptr) {
		delete[] m_outputDir;
		m_outputDir = nullptr;
	}
	if (m_archiveName != nullptr) {
		delete[] m_archiveName;
		m_archiveName = nullptr;
	}
}

void BIGFile::init(void) {
	DEBUG_ASSERTCRASH(TheLocalFileSystem != NULL, ("TheLocalFileSystem must be initialized before TheArchiveFileSystem."));
	if (TheLocalFileSystem == NULL) {
		return;
	}
}

bool BIGFile::addToArchive() {
	if (m_archiveName == nullptr || m_outputDir == nullptr || m_paths == nullptr) {
		return false;
	}

	m_archiveFile = TheArchiveFileSystem->openArchiveFile(m_archiveName);

	// logic to add files to the archive

	m_archiveFile->close();
	printf("Failed to Add file to archive.\n");
	return false;
}

bool BIGFile::deleteFromArchive(const char *path) {
	if (path == nullptr) {
		return false;
	}

	// logic to delete file from the archive
	// function to use: doesFileExist

	printf("File not found in archive.\n");
	return false;
}

bool BIGFile::listArchive() {
	if (m_archiveName == nullptr) {
		return false;
	}
	// todo m_archiveFileSystem = ?
	m_archiveFile =  m_archiveFileSystem->openArchiveFile(m_archiveName);
	FilenameList filenameList;
	m_archiveFile->getFileListInDirectory(AsciiString(""), AsciiString(""), AsciiString("*"), filenameList, TRUE);
	for (const auto &filename : filenameList) {
		std::cout << filename.str() << std::endl;
	}
	m_archiveFile->close();
	return true;
}

bool BIGFile::extractArchive(const char *path) {
	// logic to extract file from the archive
	// function to use: openFile, addFile,
	return false;
}

bool BIGFile::setArchiveFileName(const char *path) {
	if (path == nullptr) {
		return false;
	}
	// logic to handle relative paths
	// function to use: loadIntoDirectoryTree,
	m_archiveName = path;

	return true;
}

bool BIGFile::setOutputDir(const char *path) {
	return false;
}

bool BIGFile::setPaths(const char *path) {
	// logic to handle relative paths of files to be added to/remove from the archive
	return false;
}




