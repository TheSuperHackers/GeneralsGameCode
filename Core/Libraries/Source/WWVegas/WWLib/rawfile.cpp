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
 *                     $Archive:: /Commando/Code/wwlib/rawfile.cpp                            $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/25/01 1:26p                                              $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// TheSuperHackers @build feliwir 11/04/2025 We removed Win32 API here and use plain C
#include	"always.h"
#include "RAWFILE.h"
#include	<stddef.h>
#include	<stdlib.h>
#include	<limits.h>
#include	<errno.h>
#include <sys/types.h>
#include <sys/stat.h>


#if 0		//#ifdef NEVER    (gth) the MAX sdk must #define NEVER! yikes :-)
	/*
	**	This is a duplicate of the error numbers. The error handler for the RawFileClass handles
	**	these errors. If the error routine is overridden and additional errors are defined, then
	**	use numbers starting with 100. Note that these errors here are listed in numerical order.
	**	These errors are defined in the standard header file "ERRNO.h".
	*/
	EZERO,				// Non-error.
	EINVFNC,				// Invalid function number.
	ENOFILE,				// File not found.
	ENOENT=ENOFILE,	// No such file or directory.
	ENOPATH,				// Path not found.
	EMFILE,				// Too many open files.
	EACCES,				// Permission denied.
	EBADF,				// Bad file number.
	ECONTR,				// Memory blocks destroyed.
	ENOMEM,				// Not enough core memory.
	EINVMEM,				// Invalid memory block address.
	EINVENV,				// Invalid environment.
	EINVFMT,				// Invalid format.
	EINVACC,				// Invalid access code.
	EINVDAT,				// Invalid data.
	EFAULT,				// Unknown error.
	EINVDRV,				// Invalid drive specified.
	ENODEV=EINVDRV,	// No such device.
	ECURDIR,				// Attempt to remove CurDir.
	ENOTSAM,				// Not same device.
	ENMFILE,				// No more files.
	EINVAL,				// Invalid argument.
	E2BIG,				// Argument list too long.
	ENOEXEC,				// exec format error.
	EXDEV,				// Cross-device link.
	ENFILE,				// Too many open files.
	ECHILD,				// No child process.
	ENOTTY,				// not used
	ETXTBSY,				// not used
	EFBIG,				// not used
	ENOSPC,				// No space left on device.
	ESPIPE,				// Illegal seek.
	EROFS,				// Read-only file system.
	EMLINK,				// not used
	EPIPE,				// Broken pipe.
	EDOM,					// Math argument.
	ERANGE,				// Result too large.
	EEXIST,				// File already exists.
	EDEADLOCK,			// Locking violation.
	EPERM,				// Operation not permitted.
	ESRCH,				// not used
	EINTR,				// Interrupted function call.
	EIO,					// Input/output error.
	ENXIO,				// No such device or address.
	EAGAIN,				// Resource temporarily unavailable.
	ENOTBLK,				// not used
	EBUSY,				// Resource busy.
	ENOTDIR,				// not used
	EISDIR,				// not used
	EUCLEAN,				// not used
#endif


// A null file object has no file handle or filename associated with it. In order to use a file
// object created in this fashion it must be assigned a name and then opened.
RawFileClass::RawFileClass() :
	Rights(READ),
	BiasStart(0),
	BiasLength(-1),
	Handle(NULL_HANDLE),
	Filename(""),
	Date(0),
	Time(0)
{
}



bool RawFileClass::Is_Open() const
{
	return(Handle != NULL_HANDLE);
}

// The error number is the same as the DOSERR.h error numbers. If it is allowed to retry, then
// pressing a key will return from this function. Otherwise, it will exit the program with "exit()".
// WARNING: This routine may not return at all. It handles being in text mode as well as if in a
// graphic mode.
void RawFileClass::Error(int, int, char const * )
{
}

// Returns the largest size a low level DOS read or write may perform. Larger file transfers are
// performed in chunks of this size or less.
int RawFileClass::Transfer_Block_Size()
{
	return (int)((unsigned)UINT_MAX)-16L;
}

// The file is not opened. An assumption is made that the supplied filename is a constant string. A
// duplicate of the filename string is not created since it would be wasteful in that case.
RawFileClass::RawFileClass(char const * filename) :
	Rights(0),
	BiasStart(0),
	BiasLength(-1),
	Handle(NULL_HANDLE),
	Filename(filename),
	Date(0),
	Time(0)
{
}

RawFileClass::~RawFileClass()
{
	Reset ();
}

// Closes the file handle and resets the object's state.
void RawFileClass::Reset()
{
	Close();
	Filename = "";
}

// This is a virtual function and thus can't be inlined.
char const * RawFileClass::File_Name() const
{
	return(Filename);
}

// Setting the name in this fashion doesn't affect the closed or opened state of the file.
char const * RawFileClass::Set_Name(char const * filename)
{
	Bias(0);

	Filename=filename;

	/*
	** If this is a UNIX build, fix the filename from the DOS-like name passed in
	*/
	#ifdef _UNIX
		for (int i=0; i<Filename.Get_Length(); i++)
		{
			if (Filename[i]=='\\')
				Filename[i]='/';
			Filename[i]=tolower(Filename[i]);  // don't preserve case
		}
	#endif

	return(Filename);
}


// If the file object was already open, then it will be closed first. If the file object was
// previously assigned a filename, then it will be replaced with the new name. The open is designed
// to never return unless it succeeded.
int RawFileClass::Open(char const * filename, int rights)
{
	Set_Name(filename);
	return(Open(rights));
}


// This only works if the file has already been assigned a filename. It is guaranteed, by the error
// handler, that this routine will always return with success.
int RawFileClass::Open(int rights)
{
	Close();

	/*
	**	Verify that there is a filename associated with this file object. If not, then this is a
	**	big error condition.
	*/
	if (Filename.Is_Empty()) {
		Error(ENOENT, false);
	}

	/*
	**	Record the access rights used for this open call. These rights will be used if the
	**	file object is duplicated.
	*/
	Rights = rights;

	/*
	**	Repetitively try to open the file. Abort if a fatal error condition occurs.
	*/
	for (;;) {

		/*
		**	Try to open the file according to the access rights specified.
		*/
		switch (rights) {

			/*
			**	If the access rights are not recognized, then report this as
			**	an invalid access code.
			*/
			default:
				errno = EINVAL;
				break;

			case READ:
				Handle = fopen(Filename, "rb");
				break;

			case WRITE:
				Handle = fopen(Filename, "wb");
				break;

			case READ|WRITE:
				// First try to open an existing file in read/write mode.
				Handle = fopen(Filename, "rb+");
				// If that fails, try to create a new file in read/write mode.
				if(Handle == NULL_HANDLE) {
					Handle = fopen(Filename, "wb+");
				}
				break;
		}

		/*
		**	Biased files must be positioned past the bias start position.
		*/
		if (BiasStart != 0 || BiasLength != -1) {
			Seek(0, SEEK_SET);
		}

		/*
		**	If the handle indicates the file is not open, then this is an error condition.
		**	For the case of the file cannot be found, then allow a retry. All other cases
		**	are fatal.
		*/
		if (Handle == NULL_HANDLE) {
			return(false);

//			Error(errno, false, Filename);
//			continue;
		}
		break;
	}

	return(true);
}


// Use this routine before opening a file in order to make sure that it is available or to perform
// other necessary actions.
// WARNING: If forced is true, this routine keeps retrying until the file becomes available, and it
// may never return. If it doesn't become available, then the program will abort.
bool RawFileClass::Is_Available(int forced)
{
	if (Filename.Is_Empty()) return(false);

	/*
	**	If the file is already open, then is must have already passed the availability check.
	**	Return true in this case.
	*/
	if (Is_Open()) return(true);

	/*
	**	If this is a forced check, then go through the normal open channels, since those
	**	channels ensure that the file must exist.
	*/
	if (forced) {
		RawFileClass::Open(READ);
		RawFileClass::Close();
		return(true);
	}

	/*
	**	Perform a raw open of the file. If this open fails for ANY REASON, including a missing
	**	CD-ROM, this routine will return a failure condition. In all but the missing file
	**	condition, go through the normal error recover channels.
	*/
	for (;;) {

		Handle=fopen(Filename,"r");

		if (Handle == NULL_HANDLE) {
			return(false);
		}
		break;
	}

	/*
	**	Since the file could be opened, then close it and return that the file exists.
	*/
	int closeok;
	closeok=((fclose(Handle)==0)?true:false);
	if (! closeok) {
		Error(errno, false, Filename);
	}
	Handle = NULL_HANDLE;

	return(true);
}


// WARNING: Some rare error conditions may cause this routine to abort the program.
void RawFileClass::Close()
{
	/*
	**	If the file is open, then close it. If the file is already closed, then just return. This
	**	isn't considered an error condition.
	*/
	if (Is_Open()) {

		/*
		**	Try to close the file. If there was an error (who knows what that could be), then
		**	call the error routine.
		*/
		int closeok=(fclose(Handle)==0)?true:false;

		if (!closeok) {
			Error(errno, false, Filename);
		}

		/*
		**	At this point the file must have been closed. Mark the file as empty and return.
		*/
		Handle = NULL_HANDLE;
	}
}


// If buffer or size is null, no read is performed. It is legal to request more bytes than are in
// the file, in which case fewer bytes are read.
int RawFileClass::Read(void * buffer, int size)
{
	long	bytesread = 0;			// Running count of the number of bytes read into the buffer.
	int	opened = false;		// Was the file opened by this routine?

	/*
	**	If the file isn't opened, open it. This serves as a convenience
	**	for the programmer.
	*/
	if (!Is_Open()) {

		/*
		**	The error check here is moot. Open will never return unless it succeeded.
		*/
		if (!Open(READ)) {
			return(0);
		}
		opened = true;
	}

	/*
	**	A biased file has the requested read length limited to the bias length of
	**	the file.
	*/
	if (BiasLength != -1) {
		int remainder = BiasLength - Seek(0);
		size = size < remainder ? size : remainder;
	}

	long total = 0;
	while (size > 0) {
		bytesread = 0;

		int readok=true;
		bytesread=fread(buffer,1,size,Handle);
		if ((bytesread == 0)&&( ! feof(Handle)))
			readok=ferror(Handle);

		if (! readok) {
			size -= bytesread;
			total += bytesread;
			Error(errno, true, Filename);
			continue;
		}
		size -= bytesread;
		total += bytesread;
		if (bytesread == 0) break;
	}
	bytesread = total;

	/*
	**	Close the file if it was opened by this routine and return
	**	the actual number of bytes read into the buffer.
	*/
	if (opened) Close();
	return(bytesread);
}


// A disk full condition is caught, so this routine will always return with the number matching the
// size request.
// WARNING: A fatal file condition could cause this routine to never return.
int RawFileClass::Write(void const * buffer, int size)
{
	long	byteswritten = 0;
	int	opened = false;		// Was the file manually opened?

	/*
	**	Check to open status of the file. If the file is open, then merely write to
	**	it. Otherwise, open the file for writing and then close the file when the
	**	output is finished.
	*/
	if (!Is_Open()) {
		if (!Open(WRITE)) {
			return(0);
		}
		opened = true;
	}

	int writeok=true;
	byteswritten = fwrite(buffer, 1, size, Handle);
	if (byteswritten != size)
		writeok = false;

	if (! writeok) {
		Error(errno, false, Filename);
	}

	/*
	**	Fixup the bias length if necessary.
	*/
	if (BiasLength != -1) {
		if (Raw_Seek(0) > BiasStart+BiasLength) {
			BiasLength = Raw_Seek(0) - BiasStart;
		}
	}

	/*
	**	If this routine had to open the file, then close it before returning.
	*/
	if (opened) {
		Close();
	}

	/*
	**	Return with the number of bytes written. This will always be the number of bytes
	**	requested, since the case of the disk being full is caught by this routine.
	*/
	return(byteswritten);
}


// This routine will only return if it successfully performed the seek.
// WARNING: If there was a file error, then this routine might never return.
int RawFileClass::Seek(int pos, int dir)
{

	/*
	**	A file that is biased will have a seek operation modified so that the file appears to
	**	exist only within the bias range. All bytes outside of this range appear to be
	**	non-existent.
	*/
	if (BiasLength != -1) {
		switch (dir) {
			case SEEK_SET:
				if (pos > BiasLength) {
					pos = BiasLength;
				}
				pos += BiasStart;
				break;

			case SEEK_CUR:
				break;

			case SEEK_END:
				dir = SEEK_SET;
				pos += BiasStart + BiasLength;
//				pos = (pos <= BiasStart+BiasLength) ? pos : BiasStart+BiasLength;
//				pos = (pos >= BiasStart) ? pos : BiasStart;
				break;
		}

		/*
		**	Perform the modified raw seek into the file.
		*/
		long newpos = Raw_Seek(pos, dir) - BiasStart;

		/*
		**	Perform a final double check to make sure the file position fits with the bias range.
		*/
		if (newpos < 0) {
			newpos = Raw_Seek(BiasStart, SEEK_SET) - BiasStart;
		}
		if (newpos > BiasLength) {
			newpos = Raw_Seek(BiasStart+BiasLength, SEEK_SET) - BiasStart;
		}
		return(newpos);
	}

	/*
	**	If the file is not biased in any fashion, then the normal seek logic will
	**	work just fine.
	*/
	return(Raw_Seek(pos, dir));
}


// The file must exist or this is an error condition.
// WARNING: This routine handles error conditions and will not return unless the file exists and can
// successfully be queried for file length.
int RawFileClass::Size()
{
	int size = 0;

	/*
	**	A biased file already has its length determined.
	*/
	if (BiasLength != -1) {
		return(BiasLength);
	}

	/*
	**	If the file is open, then proceed normally.
	*/
	if (Is_Open()) {

		size_t curpos;
		curpos = ftell(Handle);

		fseek(Handle,0,SEEK_END);
		size = ftell(Handle);

		fseek(Handle,curpos,SEEK_SET);

		/*
		**	If there was in internal error, then call the error function.
		*/
		if (size == 0xFFFFFFFF) {
			Error(errno, false, Filename);
		}

	} else {

		/*
		**	If the file wasn't open, then open the file and call this routine again. Count on
		**	the fact that the open function must succeed.
		*/
		if (Open()) {
			size = Size();

			/*
			**	Since we needed to open the file we must remember to close the file when the
			**	size has been determined.
			*/
			Close();
		}
	}

	BiasLength = size-BiasStart;
	return(BiasLength);
}


// The file object's filename must already have been assigned before this routine will function.
// WARNING: A fatal error condition could occur with this routine. Especially if the disk is full or
// a read-only media was selected.
int RawFileClass::Create()
{
	Close();
	if (Open(WRITE)) {

		/*
		**	A biased file must be at least as long as the bias offset. Seeking to the
		**	appropriate start offset has the effect of lengthening the file to the
		**	correct length.
		*/
		if (BiasLength != -1) {
			Seek(0, SEEK_SET);
		}

		Close();
		return(true);
	}
	return(false);
}


// Returns false if the file was already missing.
int RawFileClass::Delete()
{
	/*
	**	If the file was open, then it must be closed first.
	*/
	Close();

	/*
	**	If there is no filename associated with this object, then this indicates a fatal error
	**	condition. Report this and abort.
	*/
	if (!Filename) {
		Error(ENOENT, false);
	}

	/*
	**	Repetitively try to delete the file if possible. Either return with success, or
	**	abort the program with an error.
	*/
	for (;;) {

		/*
		**	If the file is already missing, then return with this fact. No action is necessary.
		**	This can occur as this section loops if the file exists on a floppy and the floppy
		**	was removed, the file deleted on another machine, and then the floppy was
		**	reinserted. Admittedly, this is a rare case, but is handled here.
		*/
		if (!Is_Available()) {
			return(false);
		}

		int deleteok=(unlink(Filename)==0)?true:false;

		if (! deleteok) {
			Error(errno, false, Filename);
			return(false);
		}
		break;
	}

	/*
	**	DOS reports that the file was successfully deleted. Return with this fact.
	*/
	return(true);
}

unsigned long RawFileClass::Get_Date_Time()
{
	// Return value
	unsigned long retval = 0;

	// Ensure we will work properly if the file is not open
	if (Is_Open()) {
		// If file is open proceed normally
		struct stat statbuf;
		stat(Filename, &statbuf);
		retval = statbuf.st_mtime;
	} else {
		// If file not open open it, if open succeeded proceed normally and then close to put
		// everything back the way we found it.
		if (Open()) {
			struct stat statbuf;
			stat(Filename, &statbuf);
			retval = statbuf.st_mtime;
			Close();

		}
	}

	return retval;
}


bool RawFileClass::Set_Date_Time(unsigned long datetime)
{
	DEBUG_ASSERTLOG(false, ("Set_Date_Time is not implemented"));
	return(false);
}


// By using this routine, it is possible to 'fool' the file into ignoring a header and trailing
// extra data. An example of this would be a file inside of a mixfile.
void RawFileClass::Bias(int start, int length)
{
	if (start == 0) {
		BiasStart = 0;
		BiasLength = -1;
		return;
	}

	BiasLength = RawFileClass::Size();
	BiasStart += start;
	if (length != -1) {
		BiasLength = BiasLength < length ? BiasLength : length;
	}
	BiasLength = BiasLength > 0 ? BiasLength : 0;

	/*
	**	Move the current file offset to a legal position if necessary and the
	**	file was open.
	*/
	if (Is_Open()) {
		RawFileClass::Seek(0, SEEK_SET);
	}
}


// Performs a seek on the file as if it were unbiased, in spite of any bias setting the file may
// have. The ability to perform a raw seek in this fashion is necessary to maintain the bias
// ability.
int RawFileClass::Raw_Seek(int pos, int dir)
{
	/*
	**	If the file isn't opened, then this is a fatal error condition.
	*/
	if (!Is_Open()) {
		Error(EBADF, false, Filename);
	}

	if (fseek(Handle, pos, dir) == 0) {
		pos = ftell(Handle);
	} else {
		pos = -1;
	}

	/*
	**	If there was an error in the seek, then bail with an error condition.
	*/
	if (pos == 0xFFFFFFFF) {
		Error(errno, false, Filename);
	}

	/*
	**	Return with the new position of the file. This will range between zero and the number of
	**	bytes the file contains.
	*/
	return(pos);
}

void RawFileClass::Attach (void *handle, int rights)
{
	Reset ();

	Rights = rights;
	BiasStart = 0;
	BiasLength = -1;
	Date = 0;
	Time = 0;

	Handle = (FILE *)handle;
}

// Does not close the handle.
void RawFileClass::Detach ()
{
	Rights = 0;
	BiasStart = 0;
	BiasLength = -1;
	Date = 0;
	Time = 0;
	Handle = NULL_HANDLE;
}

