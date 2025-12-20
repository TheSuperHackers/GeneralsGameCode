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
 *                 Project Name : LevelEdit																	  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/LevelEdit/TGAToDXT.cpp       $*
 *                                                                                             *
 *                       Author:: Ian Leslie			                                            *
 *                                                                                             *
 *                     $Modtime:: 8/29/01 5:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma message ("(gth) disabling TGAtoDXTClass temporarily so I can test the WW libs merge...")
#if 0

#include "always.h"
#include "nvdxtlib.h"
#include "TARGA.h"
#include "tgatodxt.h"
#include "wwdebug.h"
#include <io.h>
#include	<stdlib.h>

// Singletons.
TGAToDXTClass _TGAToDXTConverter;


///////////////////////////////////////////////////////////////////////////////
//
//	TGAToDXTClass
//
///////////////////////////////////////////////////////////////////////////////
TGAToDXTClass::TGAToDXTClass()
	: WriteTimePtr (NULL),
	  BufferSize (1024),
	  BufferCount (0)
{
	Buffer = new unsigned char [BufferSize];
	WWASSERT (Buffer != NULL);
}


///////////////////////////////////////////////////////////////////////////////
//
//	~TGAToDXTClass
//
///////////////////////////////////////////////////////////////////////////////
TGAToDXTClass::~TGAToDXTClass()
{
	// Clean-up.
	delete [] Buffer;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Convert
//
///////////////////////////////////////////////////////////////////////////////
TGAToDXTClass::ErrorCode TGAToDXTClass::Convert (const char *inputpathname, const char *outputpathname, FILETIME *writetimeptr, bool &redundantalpha, bool dontcheckalpha)
{
	ErrorCode error_code;
	Targa targa;
	long	error;

	WriteTimePtr = writetimeptr;
	redundantalpha = false;

	error_code=TGA_LOAD_ERROR;
	error = targa.Load (inputpathname, TGAF_IMAGE, false);
	if (error == 0) {

		bool validbitdepth, validsize, validaspect;

		// Check that the targa is in the right format.
		// In order to be valid it must adhere to the following:
		// 1. Pixel depth must be 24 or 32 (compressor has no support for lower bit depths) or 8-bit not paletted
		// 2. Dimensions >= 4 (DDS block size is 4x4).
		// 3. Aspect ratio <= 1:8 (some H/W will not render textures above this ratio).
		// 4. Dimensions must be power of 2 (see below).
		validbitdepth = ((targa.Header.PixelDepth == 24) || (targa.Header.PixelDepth == 32)
							|| ( (targa.Header.PixelDepth == 8) && (targa.Header.ColorMapType != 1) ) );
		validsize	  = (targa.Header.Width >= 4) && (targa.Header.Height >= 4);
		validaspect	  = ((float) MAX (targa.Header.Width, targa.Header.Height)) / ((float) MIN (targa.Header.Width, targa.Header.Height)) <= 8.0f;

		if (validbitdepth && validsize && validaspect) {

			unsigned char *byte;
			HRESULT			errorcode;

			targa.YFlip();

			// If TGA has an alpha channel...
			if (targa.Header.PixelDepth == 32) {

				// Analyse the alpha channel and ignore it if it contains redundant data (ie. is either all black or all white).
				if (!dontcheckalpha) {
					byte = (unsigned char*) targa.GetImage();
					if ((*(byte + 3) == 0x00) || (*(byte + 3) == 0xff)) {

						const unsigned char alpha = *(byte + 3);

						redundantalpha = true;
						for (unsigned p = 0; p < ((unsigned) targa.Header.Width) * ((unsigned) targa.Header.Height); p++) {
							redundantalpha &= (*(byte + 3) == alpha);
							byte += 4;
						}
					}
				}

				if (!redundantalpha) {

					errorcode = ::nvDXTcompress ((unsigned char*) targa.GetImage(), targa.Header.Width, targa.Header.Height, TF_DXT5, true, false, 4);

				} else {

					unsigned char *nonalphaimage, *nonalphabyte;

					// Remove the alpha channel and swizel the pixel data.
					nonalphaimage = new unsigned char [3 * ((unsigned) targa.Header.Width) * ((unsigned) targa.Header.Height)];
					nonalphabyte  = nonalphaimage;

					byte = (unsigned char*) targa.GetImage();
					for (unsigned p = 0; p < ((unsigned) targa.Header.Width) * ((unsigned) targa.Header.Height); p++) {

						*(nonalphabyte + 0) = *(byte + 0);
						*(nonalphabyte + 1) = *(byte + 1);
						*(nonalphabyte + 2) = *(byte + 2);
						nonalphabyte += 3;
						byte += 4;
					}

					errorcode = ::nvDXTcompress (nonalphaimage, targa.Header.Width, targa.Header.Height, TF_DXT1, true, false, 3);
					delete [] nonalphaimage;
				}

			} else if (targa.Header.PixelDepth == 8) {
				// 8-bit luminance
				unsigned char *image;

				// convert to 24-bit monochrome
				image = new unsigned char [3 * ((unsigned) targa.Header.Width) * ((unsigned) targa.Header.Height)];

				byte = (unsigned char*) targa.GetImage();
				for (unsigned p = 0; p < ((unsigned) targa.Header.Width) * ((unsigned) targa.Header.Height); p++) {
					image[3*p]=image[3*p+1]=image[3*p+2]=byte[p];
				}

				errorcode = ::nvDXTcompress (image, targa.Header.Width, targa.Header.Height, TF_DXT1, true, false, 3);
				delete [] image;

			} else {
				// 24-bit color
				errorcode = ::nvDXTcompress ((unsigned char*) targa.GetImage(), targa.Header.Width, targa.Header.Height, TF_DXT1, true, false, 3);
			}

			// Was the image compressed successfully?
			// NOTE: Any image that does not have power of 2 dimensions will not be compressed.
			if (errorcode >= 0) {
				Write (outputpathname);
				error_code = OK;
			} else {
				error_code = COMPRESSION_ERROR;
			}
		} else {
			if (!validbitdepth) error_code = INVALID_BIT_DEPTH;
			if (!validsize) error_code = INVALID_SIZE;
			if (!validaspect) error_code = INVALID_ASPECT_RATIO;
		}
	}

	return error_code;
}


///////////////////////////////////////////////////////////////////////////////
//
//	Write
//
///////////////////////////////////////////////////////////////////////////////
void TGAToDXTClass::Write (const char *outputpathname)
{
	HANDLE hfile;
	DWORD  bytecountwritten;

	hfile = ::CreateFile (outputpathname, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0L, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
      LockFile (hfile, 0, 0, BufferCount, 0);
      WriteFile (hfile, Buffer, BufferCount, &bytecountwritten, NULL);
      UnlockFile (hfile, 0, 0, BufferCount, 0);

		// Stamp the write time (if one has been supplied).
		if (WriteTimePtr != NULL) {
			SetFileTime (hfile, NULL, NULL, WriteTimePtr);
		}

		CloseHandle (hfile);
	}

	// Reset buffer.
	BufferCount = 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//	ReadDTXnFile
//
///////////////////////////////////////////////////////////////////////////////
void ReadDTXnFile (DWORD datacount, void *data)
{
	// Not implemented.
	WWASSERT (false);
}


///////////////////////////////////////////////////////////////////////////////
//
//	WriteDTXnFile
//
///////////////////////////////////////////////////////////////////////////////
void WriteDTXnFile (DWORD datacount, void *data)
{
	// Ensure that the buffer is large enough.
	if (_TGAToDXTConverter.BufferSize < _TGAToDXTConverter.BufferCount + datacount) {

		unsigned			newbuffersize;
		unsigned char *newbuffer;

		newbuffersize = MAX (_TGAToDXTConverter.BufferSize * 2, _TGAToDXTConverter.BufferCount + datacount);
		newbuffer	  = new unsigned char [newbuffersize];
		WWASSERT (newbuffer != NULL);
		memcpy (newbuffer, _TGAToDXTConverter.Buffer, _TGAToDXTConverter.BufferCount);
		delete [] _TGAToDXTConverter.Buffer;
		_TGAToDXTConverter.Buffer = newbuffer;
		_TGAToDXTConverter.BufferSize = newbuffersize;
	}

	// Write new data to buffer.
	memcpy (_TGAToDXTConverter.Buffer + _TGAToDXTConverter.BufferCount, data, datacount);
	_TGAToDXTConverter.BufferCount += datacount;
}


#endif
