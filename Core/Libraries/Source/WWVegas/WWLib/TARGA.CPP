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

/****************************************************************************
*
*        C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S
*
*----------------------------------------------------------------------------
*
* FILE
*     targa.cpp
*
* DESCRIPTION
*     Targa image file class.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* DATE
*     August 8, 1995
*
*----------------------------------------------------------------------------
*
* PUBLIC
*     Open - Open Targa image file.
*     Close - Close Targa image file.
*     Load - Load Targa image file.
*     Save - Save a Targa Image File.
*     XFlip - X flip the image.
*     YFlip - Y flip the image.
*     SetImage - Set the image buffer.
*     GetImage - Get the current image buffer address.
*     SetPalette - Set the palette buffer.
*     GetPalette - Retrieve the current palette buffer address.
*     GetExtension - Get Extension data. (Targa 2.0 files only)
*
* PRIVATE
*     DecodeImage - Decompress Targa image data.
*     EncodeImage - Compress the image using targa RLE.
*     InvertImage - Invert TrueColor image data.
*
* MODIFICATIONS:
*     Converted to work with FileClass, FileFactory (changes are inside
*     ifdefs, so can be easily reversed). Naty Hoffman, January 25, 2001
*
****************************************************************************/

#include "TARGA.H"
#ifndef TGA_USES_WWLIB_FILE_CLASSES
#include <stdio.h>
#endif
#include <malloc.h>
#include <memory.h>
#include <string.h>
#ifdef TGA_USES_WWLIB_FILE_CLASSES
#include "WWFILE.H"
#include "ffactory.h"
#else
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#include <algorithm>
#include <utility>

/****************************************************************************
*
* NAME
*     Targa::Targa - Initialize a Targa instance.
*
* SYNOPSIS
*     Targa()
*
*     void Targa(void);
*
* FUNCTION
*     Initialize the targa class instance.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

Targa::Targa(void)
	{
	mImage = NULL;
	mPalette = NULL;
	Clear_File();
	mAccess = TGA_READMODE;
	mFlags = 0;
	memset(&Header, 0, sizeof(TGAHeader));
	memset(&mExtension, 0, sizeof(TGA2Extension));
	}


/****************************************************************************
*
* NAME
*     Targa::~Targa - Targa class destructor.
*
* SYNOPSIS
*     ~Targa()
*
*     void ~Targa(void);
*
* FUNCTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

Targa::~Targa(void)
{
	/* Close the file if has been left open. */
	Close();

	/* Free the palette buffer if we allocated it. */
	if ((mPalette != NULL) && (mFlags & TGAF_PAL))
		free(mPalette);

	/* Free the image buffer if we allocated it. */
	if ((mImage != NULL) && (mFlags & TGAF_IMAGE))
		free(mImage);
}


/****************************************************************************
*
* NAME
*     Targa::Open - Open Targa image file.
*
* SYNOPSIS
*     Error = Open(Name, Mode)
*
*     long Open(char *, long);
*
* FUNCTION
*     Open a Targa image file and read in its header. The file stream will
*     positioned after the ID field (if there is one).
*
* INPUTS
*     Name - Pointer to name of Targa file.
*     Mode - Access mode.
*
* RESULT
*     Error - Error code, 0 if okay.
*
****************************************************************************/

long Targa::Open(const char* name, long mode)
{
	TGA2Footer	footer;
	long			size;
	long			error = 0;
	memset(&footer, 0, sizeof(footer));

	/* File already open? */
	if (Is_File_Open() && (mAccess == mode)) {
		return (0);
	}

	Close();

	/* Initialize the access mode. */
	mAccess = mode;
	mFlags &= ~TGAF_TGA2;

	switch (mode) {

		/* Open targa file for read. */
		case TGA_READMODE:
			if (File_Open_Read(name)) {

				/* Check for 2.0 targa file by loading the footer */
				if (File_Seek(-26, SEEK_END) == -1) {
					error = TGAERR_READ;
				}

				if (!error) {
					if (File_Read(&footer, sizeof(TGA2Footer)) != sizeof(TGA2Footer)) {
						error = TGAERR_READ;
					} else {
						/* If this a 2.0 file with an extension? */
						if (strncmp(footer.Signature, TGA2_SIGNATURE, 16) == 0) {
							if (footer.Extension != 0) {
								mFlags |= TGAF_TGA2;
							}
						}
					}
				}

				/* Read in Extension data */
				if (!error && (mFlags & TGAF_TGA2)) {

					if (File_Seek(footer.Extension, SEEK_SET) == -1) {
						error = TGAERR_READ;
					}

					if (!error) {
						if (File_Read(&mExtension, sizeof(TGA2Extension)) != sizeof(TGA2Extension)) {
							error = TGAERR_READ;
						}
					}
				}

				/* Read in header. */
				if (!error && (File_Seek(0, SEEK_SET) == -1)) {
					error = TGAERR_READ;
				} else {

					size = File_Read(&Header, sizeof(TGAHeader));
					if (size != sizeof(TGAHeader)) {
						error = TGAERR_READ;
					}
				}

				/* Skip the ID field */
				if (!error && (Header.IDLength != 0)) {
					if (File_Seek(Header.IDLength, SEEK_CUR) == -1) {
						error = TGAERR_READ;
					}
				}

			} else {
				error = TGAERR_OPEN;
			}
			break;

		/* Open targa file for write. */
		case TGA_WRITEMODE:
			if (!File_Open_Write(name)) {
				error = TGAERR_OPEN;
			} else {
//				printf("\r");
			}
			break;

		/* Open targa file for read/write.*/
		case TGA_RDWRMODE:
			if (File_Open_ReadWrite(name)) {

				/* Read in header. */
				size = File_Read(&Header, sizeof(TGAHeader));

				if (size != sizeof(TGAHeader)) {
					error = TGAERR_READ;
				}
				/* Skip the ID field */
				if (!error && (Header.IDLength != 0)) {
					if (File_Seek(Header.IDLength, SEEK_CUR) == -1) {
						error = TGAERR_READ;
					}
				}

			} else {
				error = TGAERR_OPEN;
			}
			break;
	}

	/* Close on any error! */
	if (error) {
		Close();
	}

	return (error);
}


/****************************************************************************
*
* NAME
*     Targa::Close - Close Targa image file.
*
* SYNOPSIS
*     Close()
*
*     void Close(void);
*
* FUNCTION
*     Close the Targa image file and free its handle.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void Targa::Close(void)
{
#ifdef TGA_USES_WWLIB_FILE_CLASSES
	if (TGAFile) {
		TGAFile->Close();
		_TheFileFactory->Return_File(TGAFile);
		TGAFile = NULL;
	}
#else
	/* Close the file if it is open. */
	if (mFH != -1) {
		close(mFH);
		mFH = -1;
	}
#endif
}


/****************************************************************************
*
* NAME
*     Targa::Load - Load Targa Image File into specified buffers.
*
* SYNOPSIS
*     Error = Load(Name, Palette, ImageBuffer)
*
*     long Load(char *, char *, char *);
*
* FUNCTION
*     Open and load the Targa into the specified buffers. If either buffer
*     pointer is NULL then that field will not be processed.
*
* INPUTS
*     Name        - Name of Targa image file to load.
*     Palette     - Pointer to buffer to load the palette into.
*     ImageBuffer - Pointer to buffer to load the image data into.
*
* RESULT
*     Error - 0 if successful, or TGAERR_??? error code.
*
****************************************************************************/

long Targa::Load(const char* name, char* palette, char* image,bool invert_image)
{
	long size;
	long depth;
	long error = 0;

	/* Open the Targa */
	if (Open(name, TGA_READMODE) == NULL) {

		/* Process ColorMap (palette) */
		if (Header.ColorMapType == 1) {

			depth = (Header.CMapDepth >> 3);
			size = (Header.CMapLength * depth);

			/* Load the palette from the TGA if a palette buffer is provided
			 * otherwise we will skip it.
			 */
			if ((palette != NULL) && (Header.CMapLength > 0)) {

				/* Adjust palette to the starting color entry. */
				palette += (Header.CMapStart * depth);

				/* Read in the palette. */
				if (File_Read(palette, size) != size) {
					error = TGAERR_READ;
				}

			} else {
				if (File_Seek(size, SEEK_CUR) == -1) {
					error = TGAERR_READ;
				}
			}
		}

		/* Load the image data from the TGA if an image buffer is provided
		 * otherwise we are done.
		 */
		if (!error && (image != NULL)) {

			depth = TGA_BytesPerPixel(Header.PixelDepth);
			size = ((Header.Width * Header.Height) * depth);

			switch (Header.ImageType) {
				case TGA_CMAPPED:
					if (File_Read(image, size) != size) {
						error = TGAERR_READ;
					}
					break;

				case TGA_TRUECOLOR:
					if (File_Read(image, size) == size) {
						if (invert_image) InvertImage();
					} else {
						error = TGAERR_READ;
					}
					break;

				case TGA_MONO:
					if (File_Read(image, size) != size) {
						error = TGAERR_READ;
					}
					break;

				case TGA_CMAPPED_ENCODED:
					error = DecodeImage();
					break;

				case TGA_TRUECOLOR_ENCODED:
					if ((error = DecodeImage()) == NULL) {
						if (invert_image) InvertImage();
					}
					break;

				default:
					error = TGAERR_NOTSUPPORTED;
					break;
			}

			/* Arrange the image so that the origin position (coordinate 0,0)
			 * is the upperleft hand corner of the image.
			 */
			if (!error) {
				if ( Header.ImageDescriptor & TGAIDF_XORIGIN ) {
					XFlip();
					Header.ImageDescriptor &= ~TGAIDF_XORIGIN;
				}

				// Mod (IML) : Locate the origin at the bottom-left corner instead. This
				// will make ot consistent with .TGA's that have been generated with our
				// existing software.
				//	if (( Header.ImageDescriptor & TGAIDF_YORIGIN ) == 0){
				if ( Header.ImageDescriptor & TGAIDF_YORIGIN ) {
					YFlip();
					//	Bug fix (IML) : Clear this flag to indicate to the targa reader
					// that the Y-origin is at the bottom of the image.
					Header.ImageDescriptor &= ~TGAIDF_YORIGIN;
				}
			}
		}

		/* Close the Targa */
		Close();

	} else {
		error = TGAERR_OPEN;
	}

	return (error);
}


/****************************************************************************
*
* NAME
*     Targa::Load - Load Targa Image File. (Auto buffer allocation).
*
* SYNOPSIS
*     Error = Load(Name, Flags)
*
*     long Load(char, long);
*
* FUNCTION
*     Open and load the Targa into buffers allocated by this function.
*
* INPUTS
*     Name  - Name of Targa image file to load.
*     Flags -
*
* RESULT
*     Error - 0 if successful, or TGAERR_??? error code.
*
****************************************************************************/

long Targa::Load(const char* name, long flags, bool invert_image)
{
	long size;
	long error = 0;

	/* Open the file to get the header. */
	if (Open(name, TGA_READMODE) == 0) {

		/* Allocate palette memory if requested to and the targa has one. */
		if ((flags & TGAF_PAL) && (Header.ColorMapType == 1)) {

			/* Dispose of any previous palette. */
			if ((mPalette != NULL) && (mFlags & TGAF_PAL)) {
				free(mPalette);
				mPalette = NULL;
				mFlags &= ~TGAF_PAL;
			}

			/* Only allocate a palette if the client hasn't assigned one. */
			if ((mPalette == NULL) && !(mFlags & TGAF_PAL)) {

				/* Compute the size of the palette from the targa header. */
				size = (Header.CMapLength * (Header.CMapDepth >> 3));

				if (size != 0) {
					/* Allocate memory for the palette. */
					if ((mPalette = (char *)malloc(size)) != NULL) {
						mFlags |= TGAF_PAL; /* We allocated the palette. */
					} else {
						error = TGAERR_NOMEM;
					}
				}
			}
		}

		/* Allocate image memory if requested to. */
		if (!error && (flags & TGAF_IMAGE)) {

			/* Dispose of any previous image. */
			if ((mImage != NULL) && (mFlags & TGAF_IMAGE)) {
				free(mImage);
				mImage = NULL;
				mFlags &= ~TGAF_IMAGE;
			}

			/* Only allocate an image if the client hasn't assigned one. */
			if ((mImage == NULL) && !(mFlags & TGAF_IMAGE)) {

				/* Compute the size of the image data from the targa header. */
				size = ((Header.Width * Header.Height) * TGA_BytesPerPixel(Header.PixelDepth));
				if (size != 0) {
					/* Allocate memory for the image. */
					if ((mImage = (char *)malloc(size)) != NULL) {
						mFlags |= TGAF_IMAGE; /* We allocated the image. */
					} else {
						error = TGAERR_NOMEM;
					}
				}
			}
		}

		/* Read in the file contents. */
		if (!error) {
			error = Load(name, mPalette, mImage, invert_image);
		}

		/* Close the file. */
		Close();

	} else {
		error = TGAERR_OPEN;
	}

	return (error);
}


/****************************************************************************
*
* NAME
*     Targa::Save - Save a Targa Image File.
*
* SYNOPSIS
*     Error = Save(Name, Flags)
*
*     long Save(char *, long);
*
* FUNCTION
*
* INPUTS
*     Name  - Pointer to name of file to save.
*     Flags -
*
* RESULT
*     Error - 0 if successful, or TGAERR_??? error code.
*
****************************************************************************/

long Targa::Save(const char* name, long flags, bool addextension)
	{
	long size;
	long depth;
	char *palette;
	char *temppal;
	char *ptr;
	//long i,n;
	//char c;
	long error = 0;
	TGA2Footer footer;

	/* Open the Targa for write. */
	if (Open(name, TGA_WRITEMODE) == NULL)
		{
		Header.IDLength = 0;

		/* Set the ImageType for compression. */
		if (flags & TGAF_COMPRESS)
			{
			switch (Header.ImageType)
				{
				case TGA_CMAPPED:
				case TGA_TRUECOLOR:
				case TGA_MONO:
					Header.ImageType += 8;
				break;

				case TGA_CMAPPED_ENCODED:
				case TGA_TRUECOLOR_ENCODED:
				case TGA_MONO_ENCODED:
				break;

				/* Turn off compression for unknown types. */
				default:
					flags &= ~TGAF_COMPRESS;
				break;
				}
			}

		/*-----------------------------------------------------------------------
		 * WRITE THE HEADER DATA SECTION
		 *---------------------------------------------------------------------*/
		if (File_Write(&Header, sizeof(TGAHeader)) != sizeof(TGAHeader))
			error = TGAERR_WRITE;

		/*-----------------------------------------------------------------------
		 * WRITE THE COLORMAP (PALETTE) DATA SECTION
		 *---------------------------------------------------------------------*/
		if (!error && (flags & TGAF_PAL) && (mPalette != NULL)
				&& (Header.CMapLength > 0))
			{
			/* Adjust palette to the starting color entry. */
			depth = (Header.CMapDepth >> 3);
			palette = mPalette + (Header.CMapStart * depth);
			size = (Header.CMapLength * depth);

			/* Allocate temporary buffer for palette manipulation. */
			if ((temppal = (char *)malloc(size)) != NULL)
				{
				memcpy(temppal, palette, size);
				ptr = temppal;

				#if(0)
				/* Swap the byte ordering of the palette entries. */
				for (i = 0; i < Header.CMapLength; i++)
					{
					c = *ptr;
					*ptr = *(ptr + (depth - 1));
					*(ptr + (depth - 1)) = c;

					/* Next entry */
					palette += depth;
					}
				#endif

				/* Write the palette. */
				if (File_Write(temppal, size) != size)
					error = TGAERR_WRITE;

				/* Free temporary palette buffer. */
				free(temppal);
				}
			else
				error = TGAERR_NOMEM;
			}

		/*-----------------------------------------------------------------------
		 * WRITE THE IMAGE DATA SECTION
		 *---------------------------------------------------------------------*/
		if (!error && (flags & TGAF_IMAGE) && (mImage != NULL))
			{

			bool imageinverted;

			/* Invert truecolor data. */
			if ((Header.ImageType == TGA_TRUECOLOR) || (Header.ImageType == TGA_TRUECOLOR_ENCODED)) {
				InvertImage();
				imageinverted = true;
			} else {
				imageinverted = false;
			}

			/* Write the image. */
			if (flags & TGAF_COMPRESS)
				EncodeImage();
			else
				{
				depth = TGA_BytesPerPixel(Header.PixelDepth);
				size = (((Header.Width * Header.Height)) * depth);

				if (File_Write(mImage, size) != size)
					error = TGAERR_WRITE;
				}

			// Bug fix (IML) : If the image was inverted, invert it again to restore it to its prior state.
			if (imageinverted) InvertImage();
			}

		/*-----------------------------------------------------------------------
		 * WRITE THE EXTENSION DATA SECTION
		 *---------------------------------------------------------------------*/

		// Mod (IML) Optionally add an extension to the file.
		if (addextension) {
			if (!error) {

				mExtension.ExtSize = 495;
				strlcpy(mExtension.SoftID, "Denzil's Targa Code", sizeof(mExtension.SoftID));
				mExtension.SoftVer.Number = (1 * 100);
				mExtension.SoftVer.Letter = 0;

				/* Save position of extension area. */
				if ((footer.Extension = File_Seek(0, SEEK_CUR)) == -1)
					error = TGAERR_WRITE;

				if (!error && (File_Write(&mExtension, sizeof(TGA2Extension))
						!= sizeof(TGA2Extension)))
					error = TGAERR_WRITE;
			}
		} else {
			footer.Extension = 0;
		}

		/*-----------------------------------------------------------------------
		 * WRITE THE FOOTER DATA SECTION
		 *---------------------------------------------------------------------*/
		if (!error)
			{
			footer.Developer = 0;
			strlcpy(footer.Signature, TGA2_SIGNATURE, sizeof(footer.Signature));
			footer.RsvdChar = '.';
			footer.BZST = 0;

			if (File_Write(&footer, sizeof(TGA2Footer)) != sizeof(TGA2Footer))
				error = TGAERR_WRITE;
			}

		/* Close targa file. */
		Close();
		}
	else
		error = TGAERR_OPEN;

	return (error);
	}


/****************************************************************************
*
* NAME
*     Targa::XFlip - X flip the image.
*
* SYNOPSIS
*     XFlip()
*
*     void XFlip();
*
* FUNCTION
*     Flip the image in memory on its X axis. (left to right)
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void Targa::XFlip(void)
	{
	char *ptr,*ptr1;
	long  x,y,d;
	char  v,v1;
	char  depth;

	/* Pixel depth in bytes. */
	depth = TGA_BytesPerPixel(Header.PixelDepth);

	for (y = 0; y < Header.Height; y++)
		{
		ptr = (mImage + ((Header.Width * depth) * y));
		ptr1 = (ptr + ((Header.Width * depth) - depth));

		for (x = 0; x < (Header.Width / 2); x++)
			{
			for (d = 0; d < depth; d++)
				{
				v = *(ptr + d);
				v1 = *(ptr1 + d);
				*(ptr + d) = v1;
				*(ptr1 + d) = v;
				}

			ptr += depth;
			ptr1 -= depth;
			}
		}
	}


/****************************************************************************
*
* NAME
*     Targa::YFlip - Y flip the image.
*
* SYNOPSIS
*     YFlip()
*
*     void YFlip();
*
* FUNCTION
*     Flip the image in memory on its Y axis. (top to bottom)
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

static __forceinline void _swapBytes(char *p1, char *p2, unsigned count)
{
#if defined(_MSC_VER) && _MSC_VER < 1300
  _asm
  {
    mov esi,[p1]
    mov edi,[p2]
    mov ebx,[count]
    mov ecx,ebx
    shr ecx,4
    jz  lessThan16
  lp:
    mov eax,dword ptr [esi]
    mov edx,dword ptr [esi+4]
    xchg eax,dword ptr [edi]
    xchg edx,dword ptr [edi+4]
    mov dword ptr [esi],eax
    mov dword ptr [esi+4],edx
    mov eax,dword ptr [esi+8]
    mov edx,dword ptr [esi+12]
    xchg eax,dword ptr [edi+8]
    xchg edx,dword ptr [edi+12]
    mov dword ptr [esi+8],eax
    mov dword ptr [esi+12],edx
    add esi,16
    add edi,16
    dec ecx
    jnz lp
  lessThan16:
    and ebx,15
  lpLessThan16:
    jz done
    mov al,byte ptr [esi]
    xchg al,byte ptr [edi]
    mov byte ptr [edi],al
    inc esi
    inc edi
    dec ebx
    jmp lpLessThan16
  done:
  }
#else
	std::swap_ranges(p1, p1 + count, p2);
#endif
}

void Targa::YFlip(void)
{
  /* old code left in for reference...
	char *ptr,*ptr1;
	long  x,y;
	char  v,v1;
	char  depth;

	/ * Pixel depth in bytes. * /
	depth = TGA_BytesPerPixel(Header.PixelDepth);

	for (y = 0; y < (Header.Height >> 1); y++)
	{
		/ * Compute address of lines to exchange. * /
		ptr = (mImage + ((Header.Width * y) * depth));
		ptr1 = (mImage + ((Header.Width * (Header.Height - 1)) * depth));
		ptr1 -= ((Header.Width * y) * depth);

		/ * Exchange all the pixels on this scan line. * /

		for (x = 0; x < (Header.Width * depth); x++)
		{
			v = *ptr;
			v1 = *ptr1;
			*ptr = v1;
			*ptr1 = v;
			ptr++;
			ptr1++;
		}
	}
  */

  unsigned stride=Header.Width*TGA_BytesPerPixel(Header.PixelDepth);
  char *ptrTop=mImage,
       *ptrBottom=mImage+stride*(Header.Height-1);
  for (unsigned y=Header.Height/2;y;--y,ptrTop+=stride,ptrBottom-=stride)
    _swapBytes(ptrTop,ptrBottom,stride);
}

/****************************************************************************
*
* NAME
*     Targa::SetImage - Set the image buffer.
*
* SYNOPSIS
*     OldImage = SetImage(Image)
*
*     char *SetImage(char *);
*
* FUNCTION
*     Set the image buffer to one provided by the caller.
*
* INPUTS
*     Image - Pointer to buffer to use for the image buffer.
*
* RESULT
*     OldImage - Previous caller assigned image buffer.
*
****************************************************************************/

char *Targa::SetImage(char *buffer)
{
	char *oldbuffer = NULL;

	/* Free any image buffer before assigning another. */
	if ((mImage != NULL) && (mFlags & TGAF_IMAGE))
	{
		free(mImage);
		mImage = NULL;
		mFlags &= ~TGAF_IMAGE;
	}

	/* Get the old user buffer. */
	if (mImage != NULL)
		oldbuffer = mImage;

	/* Assign the new image buffer. */
	mImage = buffer;

	return (oldbuffer);
}


/****************************************************************************
*
* NAME
*     Targa::SetPalette - Set the palette buffer.
*
* SYNOPSIS
*     OldPal = SetPalette(Pal)
*
*     char *SetPalette(char *);
*
* FUNCTION
*
* INPUTS
*     Pal - Pointer to buffer to use for palette.
*
* RESULT
*     OldPal - Pointer to previous user palette.
*
****************************************************************************/

char *Targa::SetPalette(char *buffer)
{
	char *oldbuffer = NULL;

	/* Free any image buffer before assigning another. */
	if ((mPalette != NULL) && (mFlags & TGAF_PAL))
	{
		free(mPalette);
		mPalette = NULL;
		mFlags &= ~TGAF_PAL;
	}

	/* Get the old user buffer. */
	if (mPalette != NULL)
		oldbuffer = mPalette;

	/* Assign the new image buffer. */
	mPalette = buffer;

	return (oldbuffer);
}


bool Targa::IsCompressed(void)
	{
	if (Header.ImageType > 8)
		return true;

	return false;
	}


/****************************************************************************
*
* NAME
*     Targa::GetExtension - Get Extension data. (Targa 2.0 files only)
*
* SYNOPSIS
*     Ext = GetExtension()
*
*     TGA2Extension *GetExtension(void);
*
* FUNCTION
*     Retrieve a pointer to the Targa 2.0 extension data area. If the file
*     version is 1.0 OR there is no extensio area then a NULL will be returned.
*
* INPUTS
*     NONE
*
* RESULT
*     Ext - Pointer to Extension data, NULL if not available.
*
****************************************************************************/

TGA2Extension *Targa::GetExtension(void)
	{
	if (mFlags & TGAF_TGA2)
		return (&mExtension);

	return (NULL);
	}


/****************************************************************************
*
* NAME
*     Targa::DecodeImage - Decompress Targa image data.
*
* SYNOPSIS
*     Error = DecodeImage()
*
*     long DecodeImage();
*
* FUNCTION
*     Decode the RLE compressed image data into the specified buffer from
*     the file I/O stream.
*
* INPUTS
*     NONE
*
* RESULT
*     Error - 0 if successful, or TGAERR_??? error code.
*
****************************************************************************/

long Targa::DecodeImage()
	{
	char          *image;
	char          *color;
	unsigned char count;
	unsigned char depth;
	unsigned long pixel_count;
	unsigned long size;
	unsigned long c,i;
	long          error = 0;

	/* Initialize */
	image = mImage;

	/* Compute pixel depth in bytes. */
	depth = TGA_BytesPerPixel(Header.PixelDepth);

	/* Total number of pixels compressed in this image. */
	pixel_count = (Header.Width * Header.Height);

	while ((pixel_count > 0) && !error)
		{
 		/* Read count. */
		if (File_Read(&count, 1) == 1)
			{
			/* If bit 8 of the count is set then we have a run of pixels,
			 * otherwise the data is raw pixels.
			 */
			if (count & 0x80)
				{
				count &= 0x7F;
				count++;

				/* Read in run pixel. */
				if (File_Read(image, depth) == depth)
					{
					color = image;
					image += depth;

					/* Repeat the pixel for the run count in the image buffer. */
					for (c = 1; c < count; c++)
						for (i = 0; i < depth; i++)
							*image++ = *(color + i);
					}
				else
					error = TGAERR_READ;
				}
			else
				{
				count++;
				size = (count * depth);

				/* Read in raw pixels. */
				if ((unsigned)File_Read(image, size) == size)
					image += size;
				else
					error = TGAERR_READ;
				}

			/* Adjust the pixel count. */
			pixel_count -= count;
			}
		else
			error = TGAERR_READ;
		}

	return (error);
	}


/****************************************************************************
*
* NAME
*     Targa::EncodeImage - Compress the image using targa RLE.
*
* SYNOPSIS
*     EncodeImage()
*
*     void EncodeImage(void);
*
* FUNCTION
*     Encode the image data using the RLE algorithm outlined in the TARGA
*     file specification.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

long Targa::EncodeImage()
	{
	char *packet;
	long packet_index;
	char *start;
	char *end;
	long depth;
	long pixels;
	long count;
	long match;
	long i;
	long error = 0;

	/* Initialize variables. */
	depth = TGA_BytesPerPixel(Header.PixelDepth);

	/* Allocate packet buffer to hold maximum encoded data run. */
	if ((packet = (char *)malloc(128 * depth)) != NULL)
		{
		pixels = Header.Width * Header.Height;
		start = mImage;
		end = start;
		count = 0;
		packet[0] = 0;
		packet_index = 1;

		while ((pixels != 0) && !error)
			{
			match = 1;

			/* Advance to the next pixel */
			end += depth;
			pixels--;

			/* Compare pixels. */
			for (i = 0; i < depth; i++)
				{
				if (start[i] != end[i])
					{
					match = 0;
					break;
					}
				}

			/* Run of pixels */
 			if (match == 1)
				{
				count++;

				/* Continue counting until the maximum has been reached. */
				if (count < 128)
					{
					if (packet[0] == 0)
						continue;
					}
				else
					count--;
				}

			/* If there is a count then write out the run. Otherwise, write
			 * the raw pixel to the packet.
			 */
			if ((count != 0) && (packet[0] == 0))
				{
				/* Run count */
				packet[0] = (count | 0x80);

				/* Run pixel */
				for (i = 0; i < depth; i++)
					packet[i + 1] = start[i];

				/* Write the run packet. */
				if (File_Write(packet, (depth + 1)) != (depth + 1))
					error = TGAERR_WRITE;

				/* Reposition start and reset. */
				start = end;
				count = 0;
				packet[0] = 0;
				}
			else
				{
				if (count == 0)
					{
					/* Copy the raw pixel to the packet. */
					for (i = 0; i < depth; i++)
						packet[packet_index + i] = start[i];

					/* Increment the raw packet count. */
					packet[0]++;

					/* Reposition start */
					start = end;
					packet_index += depth;
					}

				/* Write the raw packet if the packet is full or a run has started
				 * or all the pixels have been processed.
				 */
				if ((packet[0] == 127) || (count != 0) || (pixels == 0))
					{
					i = packet[0];
					packet[0]--;

					if (File_Write(packet, ((i * depth) + 1)) != ((i * depth) + 1))
						error = TGAERR_WRITE;

					packet_index = 1;
					packet[0] = 0;
					}
				}
			}

		/* Free the packet buffer. */
		free(packet);
		}
	else
		error = TGAERR_NOMEM;

	return (error);
	}


/****************************************************************************
*
* NAME
*     Targa::InvertImage - Invert TrueColor image data.
*
* SYNOPSIS
*     InvertImage()
*
*     void InvertImage(void);
*
* FUNCTION
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void Targa::InvertImage(void)
	{
	char *buffer;
	long depth;
	long pixel_count;
	long i;
	char c;

	/* Initialize */
	buffer = mImage;

	/* Compute the pixel depth in bytes. */
	depth = TGA_BytesPerPixel(Header.PixelDepth);

	/* Total number of pixels in this image. */
	pixel_count = (Header.Width * Header.Height);

	/* 16-bit pixel layout is different that 24-bit and 32-bit. */
	if (depth > 2)
		{
		while (pixel_count > 0)
			{
			for (i = 0; i < (depth / 2); i++)
				{
				c = *(buffer + i);
				*(buffer + i) = *(buffer + ((depth - 1) - i));
				*(buffer + ((depth - 1) - i)) = c;
				}

			/* Next pixel */
			pixel_count--;
			buffer += depth;
			}
		}
	}


/*
** These functions are just for ease of ifdef'ing between standard io calls and FileClass.
*/
void Targa::Clear_File(void)
{
#ifdef TGA_USES_WWLIB_FILE_CLASSES
	TGAFile = NULL;
#else
	mFH = -1;
#endif
}
bool Targa::Is_File_Open(void)
{
#ifdef TGA_USES_WWLIB_FILE_CLASSES
	return (TGAFile != NULL);
#else
	return (mFH != -1);
#endif
}
bool Targa::File_Open_Read(const char* name)
{
#ifdef TGA_USES_WWLIB_FILE_CLASSES
	TGAFile = _TheFileFactory->Get_File(name);
	if (TGAFile && TGAFile->Is_Available()) {
		return (TGAFile->Open(FileClass::READ) != 0);
	} else {
		return false;
	}
#else
	mFH = open(name, (O_RDONLY|O_BINARY));
	return (mFH != -1);
#endif
}
bool Targa::File_Open_Write(const char* name)
{
#ifdef TGA_USES_WWLIB_FILE_CLASSES
	TGAFile = _TheWritingFileFactory->Get_File(name);
	if (TGAFile) {
		return (TGAFile->Open(FileClass::WRITE) != 0);
	} else {
		return false;
	}
#else
	mFH = open(name, (O_CREAT|O_TRUNC|O_WRONLY|O_BINARY), (S_IREAD|S_IWRITE));
	return (mFH != -1);
#endif
}
bool Targa::File_Open_ReadWrite(const char* name)
{
#ifdef TGA_USES_WWLIB_FILE_CLASSES
	TGAFile = _TheWritingFileFactory->Get_File(name);
	if (TGAFile && TGAFile->Is_Available()) {
		return (TGAFile->Open(FileClass::READ|FileClass::WRITE) != 0);
	} else {
		return false;
	}
#else
	mFH = open(name, (O_RDWR|O_BINARY), (S_IREAD|S_IWRITE));
	return (mFH != -1);
#endif
}
int Targa::File_Seek(int pos, int dir)
{
#ifdef TGA_USES_WWLIB_FILE_CLASSES
	return TGAFile->Seek(pos, dir);
#else
	return lseek(mFH, pos, dir);
#endif
}
int Targa::File_Read(void *buffer, int size)
{
#ifdef TGA_USES_WWLIB_FILE_CLASSES
	return TGAFile->Read(buffer, size);
#else
	return read(mFH, buffer, size);
#endif
}
int Targa::File_Write(void *buffer, int size)
{
#ifdef TGA_USES_WWLIB_FILE_CLASSES
	return TGAFile->Write(buffer, size);
#else
	return write(mFH, buffer, size);
#endif
}

// ----------------------------------------------------------------------------
//
// Output targa load error message.
//
// ----------------------------------------------------------------------------

long Targa_Error_Handler(long load_err,const char* filename)
{
	switch (load_err) {
	case 0:
		return 0;
	case TGAERR_OPEN:
		WWDEBUG_SAY(("Targa: Failed to open file \"%s\"", filename));
		break;

	case TGAERR_READ:
		WWDEBUG_SAY(("Targa: Failed to read file \"%s\"", filename));
		break;

	case TGAERR_NOTSUPPORTED:
		WWDEBUG_SAY(("Targa: File \"%s\" is an unsupported Targa type", filename));
		break;

	case TGAERR_NOMEM:
		WWDEBUG_SAY(("Targa: Failed to allocate memory for file \"%s\"", filename));
		break;

	default:
		WWDEBUG_SAY(("Targa: Unknown error when loading file \"%s\"", filename));
		break;
	}
	return load_err;
}
