#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/AudioAffect.h"
#include "Common/GameAudio.h"
#include "Common/GameMemory.h"
#include "Common/LocalFileSystem.h"
#include "Common/file.h"
#include "SDL3Device/Common/SDL3BIGFile.h"
#include "SDL3Device/Common/SDL3BIGFileSystem.h"
#include "Utility/endian_compat.h"

#include <string>

namespace
{
	constexpr char BIGFileIdentifier[] = "BIGF";
	constexpr size_t MAX_BIG_PATH_LENGTH = 4096;
}

SDL3BIGFileSystem::SDL3BIGFileSystem()
	: ArchiveFileSystem()
{
}

SDL3BIGFileSystem::~SDL3BIGFileSystem() = default;

void SDL3BIGFileSystem::init()
{
	DEBUG_ASSERTCRASH(TheLocalFileSystem != nullptr,
		("TheLocalFileSystem must be initialized before TheArchiveFileSystem."));
	if (TheLocalFileSystem == nullptr)
		return;

	// SDL3LocalFileSystem roots relative paths at SDL_GetBasePath(), so the
	// executable directory is the complete asset root. Preserve the original
	// recursive archive discovery behavior without registry lookup.
	loadBigFilesFromDirectory("", "*.big");
}

void SDL3BIGFileSystem::reset()
{
}

void SDL3BIGFileSystem::update()
{
}

void SDL3BIGFileSystem::postProcessLoad()
{
}

ArchiveFile *SDL3BIGFileSystem::openArchiveFile(const Char *filename)
{
	if (filename == nullptr || TheLocalFileSystem == nullptr)
		return nullptr;

	File *file = TheLocalFileSystem->openFile(filename, File::READ | File::BINARY);
	if (file == nullptr)
	{
		DEBUG_LOG(("SDL3BIGFileSystem::openArchiveFile - could not open %s", filename));
		return nullptr;
	}

	char identifier[sizeof(BIGFileIdentifier)]{};
	if (file->read(identifier, 4) != 4 || std::memcmp(identifier, BIGFileIdentifier, 4) != 0)
	{
		DEBUG_LOG(("SDL3BIGFileSystem::openArchiveFile - invalid BIGF header in %s", filename));
		file->close();
		return nullptr;
	}

	UnsignedInt archiveFileSize = 0;
	Int numLittleFiles = 0;
	if (file->read(&archiveFileSize, sizeof(archiveFileSize)) != sizeof(archiveFileSize) ||
		file->read(&numLittleFiles, sizeof(numLittleFiles)) != sizeof(numLittleFiles))
	{
		file->close();
		return nullptr;
	}

	numLittleFiles = betoh(numLittleFiles);
	if (numLittleFiles < 0)
	{
		file->close();
		return nullptr;
	}

	if (file->seek(0x10, File::START) < 0)
	{
		file->close();
		return nullptr;
	}

	AsciiString archiveFileName = filename;
	archiveFileName.toLower();
	ArchiveFile *archiveFile = NEW SDL3BIGFile(filename, filename);
	ArchivedFileInfo fileInfo;

	for (Int i = 0; i < numLittleFiles; ++i)
	{
		UnsignedInt fileOffset = 0;
		UnsignedInt fileSize = 0;
		if (file->read(&fileOffset, sizeof(fileOffset)) != sizeof(fileOffset) ||
			file->read(&fileSize, sizeof(fileSize)) != sizeof(fileSize))
		{
			delete archiveFile;
			file->close();
			return nullptr;
		}

		fileOffset = betoh(fileOffset);
		fileSize = betoh(fileSize);
		fileInfo.m_archiveFilename = archiveFileName;
		fileInfo.m_offset = fileOffset;
		fileInfo.m_size = fileSize;

		std::string storedPath;
		storedPath.reserve(64);
		bool terminated = false;
		for (size_t pathLength = 0; pathLength < MAX_BIG_PATH_LENGTH; ++pathLength)
		{
			char character = 0;
			if (file->read(&character, 1) != 1)
				break;
			if (character == '\0')
			{
				terminated = true;
				break;
			}
			storedPath.push_back(character);
		}

		if (!terminated || storedPath.empty())
		{
			delete archiveFile;
			file->close();
			return nullptr;
		}

		const size_t separator = storedPath.find_last_of("\\/");
		const std::string path = separator == std::string::npos
			? std::string()
			: storedPath.substr(0, separator);
		const std::string leafName = separator == std::string::npos
			? storedPath
			: storedPath.substr(separator + 1);
		if (leafName.empty())
		{
			delete archiveFile;
			file->close();
			return nullptr;
		}

		fileInfo.m_filename = leafName.c_str();
		fileInfo.m_filename.toLower();
		archiveFile->addFile(AsciiString(path.c_str()), &fileInfo);
	}

	archiveFile->attachFile(file);
	return archiveFile;
}

void SDL3BIGFileSystem::closeArchiveFile(const Char *filename)
{
	if (filename == nullptr)
		return;

	ArchiveFileMap::iterator iterator = m_archiveFileMap.find(filename);
	if (iterator == m_archiveFileMap.end())
		return;

	if (stricmp(filename, MUSIC_BIG) == 0 && TheAudio != nullptr)
		TheAudio->stopAudio(AudioAffect_Music);

	delete iterator->second;
	m_archiveFileMap.erase(iterator);
}

void SDL3BIGFileSystem::closeAllArchiveFiles()
{
}

void SDL3BIGFileSystem::closeAllFiles()
{
}

Bool SDL3BIGFileSystem::loadBigFilesFromDirectory(AsciiString dir, AsciiString fileMask, Bool overwrite)
{
	if (TheLocalFileSystem == nullptr)
		return FALSE;

	FilenameList filenameList;
	TheLocalFileSystem->getFileListInDirectory(dir, AsciiString::TheEmptyString,
		fileMask, filenameList, TRUE);

	Bool actuallyAdded = FALSE;
	for (FilenameListIter iterator = filenameList.begin(); iterator != filenameList.end(); ++iterator)
	{
		// Some installations contain the same Zero Hour INIZH archive in both
		// Run and Run\Data\INI. Loading both creates duplicate INI entries.
		if (iterator->endsWithNoCase("Data\\INI\\INIZH.big") ||
			iterator->endsWithNoCase("Data/INI/INIZH.big"))
			continue;

		ArchiveFile *archiveFile = openArchiveFile(iterator->str());
		if (archiveFile == nullptr)
			continue;

		DEBUG_LOG(("SDL3BIGFileSystem::loadBigFilesFromDirectory - loading %s", iterator->str()));
		loadIntoDirectoryTree(archiveFile, overwrite);
		m_archiveFileMap[*iterator] = archiveFile;
		actuallyAdded = TRUE;
	}

	return actuallyAdded;
}
