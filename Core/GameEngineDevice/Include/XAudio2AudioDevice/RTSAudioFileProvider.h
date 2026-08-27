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

#pragma once

#include "core/AudioFileProvider.h"
#include "Common/File.h"
#include "Common/FileSystem.h"

///
/// RTS AudioFileProvider — bridges AudioFileProvider to TheFileSystem.
///
/// Used by Generals and Zero Hour to load audio files from the RTS file system.
///
class RTSAudioFileProvider : public AudioFileProvider
{
public:
	FileHandle open(const char *filename) override
	{
		FileHandle fh;
		if (!TheFileSystem || !filename)
			return fh;
		File *file = TheFileSystem->openFile(filename, File::READ);
		if (!file)
			return fh;
		fh.opaque = file;
		return fh;
	}

	int read(FileHandle handle, void *buf, int size) override
	{
		auto *file = static_cast<File *>(handle.opaque);
		return file->read(buf, size);
	}

	int64_t seek(FileHandle handle, int64_t offset, int whence) override
	{
		auto *file = static_cast<File *>(handle.opaque);
		File::seekMode mode = File::START;
		if (whence == SEEK_CUR) mode = File::CURRENT;
		else if (whence == SEEK_END) mode = File::END;
		if (file->seek(static_cast<Int>(offset), mode))
			return file->position();
		return -1;
	}

	int64_t size(FileHandle handle) override
	{
		auto *file = static_cast<File *>(handle.opaque);
		return file->size();
	}

	void close(FileHandle handle) override
	{
		auto *file = static_cast<File *>(handle.opaque);
		file->close();
	}
};


