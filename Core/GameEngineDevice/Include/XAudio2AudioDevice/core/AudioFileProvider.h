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

#include <cstdint>

///
/// AudioFileProvider — abstract file I/O interface for audio backends.
///
/// Audio backends use this to load sound files without knowing anything about
/// the game's virtual file system (mix archives, pak files, loose files, etc.).
/// Each game supplies its own implementation at AudioSystem construction time.
///
/// Lifetime: the provider must outlive the AudioSystem that uses it.
///
class AudioFileProvider
{
public:
	virtual ~AudioFileProvider() = default;

	/// Opaque file handle returned by open().
	struct FileHandle
	{
		void *opaque = nullptr;
		bool isValid() const { return opaque != nullptr; }
	};

	/// Open a file by name. Returns an invalid handle on failure.
	virtual FileHandle open(const char *filename) = 0;

	/// Read up to `size` bytes into `buf`. Returns bytes actually read, or -1 on error.
	virtual int read(FileHandle handle, void *buf, int size) = 0;

	/// Seek to `offset` using standard whence values (SEEK_SET, SEEK_CUR, SEEK_END).
	/// Returns the new position, or -1 on error.
	virtual int64_t seek(FileHandle handle, int64_t offset, int whence) = 0;

	/// Get the total file size in bytes.
	virtual int64_t size(FileHandle handle) = 0;

	/// Close the file and release resources.
	virtual void close(FileHandle handle) = 0;
};


