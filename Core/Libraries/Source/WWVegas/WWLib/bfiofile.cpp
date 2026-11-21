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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Library/BFIOFILE.cpp                              $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"bfiofile.h"
#include	<string.h>


// The file is not opened. An assumption is made that the supplied filename is a constant string. A
// duplicate of the filename string is not created since it would be wasteful in that case.
BufferIOFileClass::BufferIOFileClass(char const * filename) :
	IsAllocated(false),
	IsOpen(false),
	IsDiskOpen(false),
	IsCached(false),
	IsChanged(false),
	UseBuffer(false),
	BufferRights(0),
	Buffer(0),
	BufferSize(0),
	BufferPos(0),
	BufferFilePos(0),
	BufferChangeBeg(-1),
	BufferChangeEnd(-1),
	FileSize(0),
	FilePos(0),
	TrueFileStart(0)
{
	BufferIOFileClass::Set_Name(filename);
}


BufferIOFileClass::BufferIOFileClass() :
	IsAllocated(false),
	IsOpen(false),
	IsDiskOpen(false),
	IsCached(false),
	IsChanged(false),
	UseBuffer(false),
	BufferRights(0),
	Buffer(0),
	BufferSize(0),
	BufferPos(0),
	BufferFilePos(0),
	BufferChangeBeg(-1),
	BufferChangeEnd(-1),
	FileSize(0),
	FilePos(0),
	TrueFileStart(0)
{
}


// Frees all memory allocated thru using Cache routines.
BufferIOFileClass::~BufferIOFileClass()
{
	Free();
}


// Load part or all of a file data into RAM.
// WARNING: This routine goes to disk for a potentially very long time.
bool BufferIOFileClass::Cache( long size, void * ptr )
{
	if (Buffer) {
		//
		// if trying to cache again with size or ptr fail
		//
		if (size || ptr) {
			return( false);
		} else {
			return( true);
		}
	}

	if (Is_Available()) {
		FileSize = Size();
	} else {
		FileSize = 0;
	}

	if (size) {
		//
		// minimum buffer size for performance
		//
		if (size < MINIMUM_BUFFER_SIZE) {
			size = MINIMUM_BUFFER_SIZE;

			/*
			**	Specifying a size smaller than the minimum is an error
			**	IF a buffer pointer was also specified. In such a case the
			**	system cannot use the buffer.
			*/
			if (ptr) {
				Error(EINVAL);
			}
		}

		BufferSize = size;
	} else {
		BufferSize = FileSize;
	}

	//
	// if size == 0 and a ptr to a buffer is specified then that is invalid.
	// if the BufferSize is 0 then this must be a new file and no size was
	// specified so exit.
	//
	if ((size == 0 && ptr) || !BufferSize) {
		return( false);
	}

	if (ptr) {
		Buffer = ptr;
	} else {
		Buffer = new char [BufferSize];
	}

	if (Buffer) {
		IsAllocated			= true;
		IsDiskOpen			= false;
		BufferPos			= 0;
		BufferFilePos		= 0;
		BufferChangeBeg	= -1;
		BufferChangeEnd	= -1;
		FilePos				= 0;
		TrueFileStart		= 0;

		//
		// the file was checked for availability then set the FileSize
		//
		if (FileSize) {
			long readsize;
			int opened = false;
			long prevpos = 0;


			if (FileSize <= BufferSize) {
				readsize = FileSize;
			} else {
				readsize = BufferSize;
			}

			if (Is_Open()) {
				//
				// get previous file position
				//
				prevpos = Seek(0);

				//
				// get true file position
				//
				if (BASECLASS::Is_Open()) {
					TrueFileStart = BASECLASS::Seek(0);
				} else {
					TrueFileStart = prevpos;
				}

				if (FileSize <= BufferSize) {
					//
					// if previous position is non-zero seek to the beginning
					//
					if (prevpos) {
						Seek(0, SEEK_SET);
					}

					//
					// set the buffer position for future reads/writes
					//
					BufferPos = prevpos;
				} else {
					BufferFilePos = prevpos;
				}

				FilePos = prevpos;
			} else {
				if (Open()) {
					TrueFileStart = BASECLASS::Seek(0);
					opened = true;
				}
			}

			long actual = Read(Buffer, readsize);

			if (actual != readsize) {
				Error(EIO);
			}

			if (opened) {
				Close();
			} else {
				//
				// seek to the previous position in the file
				//
				Seek(prevpos, SEEK_SET);
			}

			IsCached = true;
		}

		UseBuffer = true;
		return(true);
	}

	Error(ENOMEM);

	return(false);
}


// By using this in conjunction with the Cache() function, one can maintain tight control of memory
// usage.
void BufferIOFileClass::Free()
{
	if (Buffer) {
		if (IsAllocated) {
			delete [] Buffer;
			IsAllocated = false;
		}

		Buffer = 0;
	}

	BufferSize		= 0;
	IsOpen			= false;
	IsCached			= false;
	IsChanged		= false;
	UseBuffer		= false;
}


// Writes the cache to the file if it has changed.
bool BufferIOFileClass::Commit()
{
	long size;


	if (UseBuffer) {
		if (IsChanged) {
			size = BufferChangeEnd - BufferChangeBeg;

			if (IsDiskOpen) {
				BASECLASS::Seek( TrueFileStart + BufferFilePos + BufferChangeBeg, SEEK_SET);
				BASECLASS::Write( Buffer, size);
				BASECLASS::Seek( TrueFileStart + FilePos, SEEK_SET);
			} else {
				BASECLASS::Open();
				BASECLASS::Seek( TrueFileStart + BufferFilePos + BufferChangeBeg, SEEK_SET);
				BASECLASS::Write( Buffer, size);
				BASECLASS::Close();
			}

			IsChanged = false;
			return( true);
		} else {
			return( false);
		}
	} else {
		return( false);
	}
}


// Checks for a previous filename and that it is cached. If so, then check the new filename against
// the old. If they are the same then return that filename. Otherwise, the file object's name is set
// with just the raw filename as passed to this routine.
char const * BufferIOFileClass::Set_Name(char const * filename)
{
	if (File_Name() && UseBuffer) {
		if (strcmp(filename, File_Name() ) == 0) {
			return( File_Name());
		} else {
			Commit();
			IsCached = false;
		}
	}

	BASECLASS::Set_Name(filename);
	return( File_Name());
}


// Checks for existence of file cached or on disk.
bool BufferIOFileClass::Is_Available(int )
{
	if (UseBuffer) {
		return(true);
	}

	return(BASECLASS::Is_Available());
}


// If part or all of the file is cached, then return that it is opened.
bool BufferIOFileClass::Is_Open() const
{
	if (IsOpen && UseBuffer) {
		return( true);
	}

	return(BASECLASS::Is_Open());
}


// If the file object was already open, then it will be closed first. If the file object was
// previously assigned a filename, then it will be replaced with the new name. The open is designed
// to never return unless it succeeded.
int BufferIOFileClass::Open(char const * filename, int rights)
{
	Set_Name(filename);
	return( BufferIOFileClass::Open( rights ));
}


// This only works if the file has already been assigned a filename. It is guaranteed, by the error
// handler, that this routine will always return with success.
int BufferIOFileClass::Open(int rights)
{
	BufferIOFileClass::Close();

	if (UseBuffer) {

		BufferRights = rights;		// save rights requested for checks later

		if (rights != READ ||
			 (rights == READ && FileSize > BufferSize)) {

			if (rights == WRITE) {
				BASECLASS::Open(rights);
				BASECLASS::Close();
				rights = READ | WRITE;
				TrueFileStart = 0;		// now writing to single file
			}

			if (TrueFileStart) {
				UseBuffer = false;
				Open( rights);
				UseBuffer = true;
			} else {
				BASECLASS::Open( rights);
			}

			IsDiskOpen = true;

			if (BufferRights == WRITE) {
				FileSize = 0;
			}

		} else {
			IsDiskOpen = false;
		}

		BufferPos			= 0;
		BufferFilePos		= 0;
		BufferChangeBeg	= -1;
		BufferChangeEnd	= -1;
		FilePos				= 0;
		IsOpen				= true;
	} else {
		BASECLASS::Open( rights);
	}

	return( true);
}


// Writes data to the file cache.
int BufferIOFileClass::Write(void const * buffer, int size)
{
	int opened = false;

	if (!Is_Open()) {
		if (!Open(WRITE)) {
			return(0);
		}
		TrueFileStart = BASECLASS::Seek(0);
		opened = true;
	}

	if (UseBuffer) {
		int sizewritten = 0;

		if (BufferRights != READ) {
			while (size) {
				int sizetowrite;

				if (size >= (BufferSize - BufferPos)) {
					sizetowrite = (BufferSize - BufferPos);
				} else {
					sizetowrite = size;
				}

				if (sizetowrite != BufferSize) {

					if (!IsCached) {
						int readsize;

						if (FileSize < BufferSize) {
							readsize = FileSize;
							BufferFilePos = 0;
						} else {
							readsize = BufferSize;
							BufferFilePos = FilePos;
						}

						if (TrueFileStart) {
							UseBuffer = false;
							Seek( FilePos, SEEK_SET);
							Read( Buffer, BufferSize);
							Seek( FilePos, SEEK_SET);
							UseBuffer = true;
						} else {
							BASECLASS::Seek( BufferFilePos, SEEK_SET);
							BASECLASS::Read( Buffer, readsize);
						}

						BufferPos			= 0;
						BufferChangeBeg	= -1;
						BufferChangeEnd	= -1;

						IsCached = true;
					}
				}

				memmove((char *)Buffer + BufferPos, (char *)buffer + sizewritten, sizetowrite);

				IsChanged = true;
				sizewritten += sizetowrite;
				size -= sizetowrite;

				if (BufferChangeBeg == -1) {
					BufferChangeBeg = BufferPos;
					BufferChangeEnd = BufferPos;
				} else {
					if (BufferChangeBeg > BufferPos) {
						BufferChangeBeg = BufferPos;
					}
				}

				BufferPos += sizetowrite;

				if (BufferChangeEnd < BufferPos) {
					BufferChangeEnd = BufferPos;
				}

				FilePos = BufferFilePos + BufferPos;

				if (FileSize < FilePos) {
					FileSize = FilePos;
				}

				//
				// end of buffer reached?
				//
				if (BufferPos == BufferSize) {
					Commit();

					BufferPos = 0;
					BufferFilePos = FilePos;
					BufferChangeBeg = -1;
					BufferChangeEnd = -1;

					if (size && FileSize > FilePos) {
						if (TrueFileStart) {
							UseBuffer = false;
							Seek( FilePos, SEEK_SET);
							Read( Buffer, BufferSize);
							Seek( FilePos, SEEK_SET);
							UseBuffer = true;
						} else {
							BASECLASS::Seek( FilePos, SEEK_SET);
							BASECLASS::Read( Buffer, BufferSize);
						}
					} else {
						IsCached = false;
					}
				}
			}
		} else {
			Error(EACCES);
		}

		size = sizewritten;
	} else {
		size = BASECLASS::Write(buffer, size);
	}

	if (opened) {
		Close();
	}

	return( size);
}


// Reads data from the file cache.
int BufferIOFileClass::Read(void * buffer, int size)
{
	int opened = false;

	if (!Is_Open()) {
		if (Open()) {
			TrueFileStart = BASECLASS::Seek(0);
			opened = true;
		}
	}

	if (UseBuffer) {
		long sizeread = 0;

		if (BufferRights != WRITE) {
			while (size) {
				long sizetoread;

				if (size >= (BufferSize - BufferPos)) {
					sizetoread = (BufferSize - BufferPos);
				} else {
					sizetoread = size;
				}

				if (!IsCached) {
					long readsize;

					if (FileSize < BufferSize) {
						readsize = FileSize;
						BufferFilePos = 0;
					} else {
						readsize = BufferSize;
						BufferFilePos = FilePos;
					}

					if (TrueFileStart) {
						UseBuffer = false;
						Seek( FilePos, SEEK_SET);
						Read( Buffer, BufferSize);
						Seek( FilePos, SEEK_SET);
						UseBuffer = true;
					} else {
						BASECLASS::Seek( BufferFilePos, SEEK_SET);
						BASECLASS::Read( Buffer, readsize);
					}

					BufferPos			= 0;
					BufferChangeBeg	= -1;
					BufferChangeEnd	= -1;

					IsCached = true;
				}

				memmove((char *)buffer + sizeread, (char *)Buffer + BufferPos, sizetoread);

				sizeread += sizetoread;
				size -= sizetoread;
				BufferPos += sizetoread;
				FilePos = BufferFilePos + BufferPos;

				//
				// end of buffer reached?
				//
				if (BufferPos == BufferSize) {
					Commit();

					BufferPos = 0;
					BufferFilePos = FilePos;
					BufferChangeBeg = -1;
					BufferChangeEnd = -1;

					if (size && FileSize > FilePos) {
						if (TrueFileStart) {
							UseBuffer = false;
							Seek( FilePos, SEEK_SET);
							Read( Buffer, BufferSize);
							Seek( FilePos, SEEK_SET);
							UseBuffer = true;
						} else {
							BASECLASS::Seek( FilePos, SEEK_SET);
							BASECLASS::Read( Buffer, BufferSize);
						}
					} else {
						IsCached = false;
					}
				}
			}
		} else {
			Error(EACCES);
		}

		size = sizeread;
	} else {
		size = BASECLASS::Read(buffer, size);
	}

	if (opened) {
		Close();
	}

	return( size);
}


// It follows the same rules as a normal Seek() does, but if the file is part of the mixfile system,
// then only the position value needs to be updated.
int BufferIOFileClass::Seek(int pos, int dir)
{
	if (UseBuffer) {
		bool adjusted = false;

		switch (dir) {
			case SEEK_END:
				FilePos = FileSize;
				break;

			case SEEK_SET:
				FilePos = 0;
				break;

			case SEEK_CUR:
			default:
				break;
		}

		if (TrueFileStart) {
			if (pos >= TrueFileStart) {
				pos -= TrueFileStart;
				adjusted = true;
			}
		}

		FilePos += pos;

		if (FilePos < 0) {
			FilePos = 0;
		}

		if (FilePos > FileSize) {
			FilePos = FileSize;
		}

		if (FileSize <= BufferSize) {
			BufferPos = FilePos;
		} else {
			if (FilePos >= BufferFilePos &&
				 FilePos < (BufferFilePos + BufferSize)) {
				BufferPos = FilePos - BufferFilePos;
			} else {
				Commit();
// check!!
				if (TrueFileStart) {
					UseBuffer = false;
					Seek(FilePos, SEEK_SET);
					UseBuffer = true;
				} else {
					BASECLASS::Seek(FilePos, SEEK_SET);
				}

				IsCached = false;
			}
		}

		if (TrueFileStart && adjusted) {
			return( FilePos + TrueFileStart);
		}

		return( FilePos);
	}

	return( BASECLASS::Seek(pos, dir));
}


// If part or all of the file is cached, then the size of the file is already determined and
// available. Otherwise, go to the low level system to find the file size.
int BufferIOFileClass::Size()
{
	if (IsOpen && UseBuffer) {
		return( FileSize);
	}

	return( BASECLASS::Size());
}


// Calls Commit() to write the buffer if the file is cached and the buffer has changed, then calls
// lower level Close().
void BufferIOFileClass::Close()
{
	if (UseBuffer) {
		Commit();

		if (IsDiskOpen) {

			if (TrueFileStart) {
				UseBuffer = false;
				Close();
				UseBuffer = true;
			} else {
				BASECLASS::Close();
			}

			IsDiskOpen = false;
		}

		IsOpen = false;
	} else {
		BASECLASS::Close();
	}
}

