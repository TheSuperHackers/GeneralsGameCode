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

/****************************************************************************
*
*         C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S
*
*----------------------------------------------------------------------------
*
* FILE
*     Targa.h
*
* DESCRIPTION
*     Targa image file class definitions.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* DATE
*     July 15, 1998
*
****************************************************************************/

#pragma pack(push, 1)

// If you wish to display loading error messages call targa functions inside of
// the following macro - for example TARGA_ERROR_HANDLER(targa.Open(filename, TGA_READMODE));
// The error code is returned back from the handler so it can be used in an expression.
long Targa_Error_Handler(long error_code,const char* filename);
#define TARGA_ERROR_HANDLER(call,filename) Targa_Error_Handler(call,filename)

/*---------------------------------------------------------------------------
 * STRUCTURES AND RELATED DEFINITIONS
 *-------------------------------------------------------------------------*/

/* TGAHeader - Targa Image File header.
 *
 * IDLength        - Size of Image ID field
 * ColorMapType    - Color map type.
 * ImageType       - Image type code.
 * CMapStart       - Color map origin.
 * CMapLength      - Color map length.
 * CMapDepth       - Depth of color map entries.
 * XOffset         - X origin of image.
 * YOffset         - Y origin of image.
 * Width           - Width of image.
 * Height          - Height of image.
 * PixelDepth      - Image pixel size
 * ImageDescriptor - Image descriptor byte.
 */
typedef struct _TGAHeader
	{
	char  IDLength;
	char  ColorMapType;
	char  ImageType;
	short CMapStart;
	short CMapLength;
	char  CMapDepth;
	short XOffset;
	short YOffset;
	short Width;
	short Height;
	char  PixelDepth;
	char  ImageDescriptor;
	} TGAHeader;

/* ImageType definiton */
#define TGA_NOIMAGE           0  /* No image data included in file */
#define TGA_CMAPPED           1  /* Color-mapped image data */
#define TGA_TRUECOLOR         2  /* Truecolor image data */
#define TGA_MONO              3  /* Monochrome image data */
#define TGA_CMAPPED_ENCODED   9  /* Color-mapped image data (Encoded) */
#define TGA_TRUECOLOR_ENCODED 10 /* Truecolor image data (Encoded) */
#define TGA_MONO_ENCODED      11 /* Monochrome image data (Encoded) */

/* ImageDescriptor definition */
#define TGAIDF_ATTRIB_BITS (0x0F<<0) /* Number of attribute bits per pixel */
#define TGAIDF_XORIGIN     (1<<4)
#define TGAIDF_YORIGIN     (1<<5)

/* Access modes. */
#define TGA_READMODE  0
#define TGA_WRITEMODE 1
#define TGA_RDWRMODE  2

/* Error codes */
#define TGAERR_OPEN         -1
#define TGAERR_READ         -2
#define TGAERR_WRITE        -3
#define TGAERR_SYNTAX       -4
#define TGAERR_NOMEM        -5
#define TGAERR_NOTSUPPORTED -6

/* Flags definitions */
#define TGAF_IMAGE    (1<<0)
#define TGAF_PAL      (1<<1)
#define TGAF_COMPRESS (1<<2)
#define TGAF_TGA2     (1<<3)

/* Macro definitions */
#define TGA_BytesPerPixel(a) ((a+7) >> 3)

/*---------------------------------------------------------------------------
 * TARGA 2.0 DEFINITIONS
 *-------------------------------------------------------------------------*/

#define TGA2_SIGNATURE "TRUEVISION-XFILE"

/* TGA2Footer - Targa 2.0 footer
 *
 * Extension - Offset to the Extension area from start of file.
 * Developer - Offset to the Developer area from start of file.
 * Signature - 16 byte Targa 2.0 signature "TRUEVISION-XFILE"
 * RsvdChar  - Reserved character, must be ASCII "." (period)
 * BZST      - Binary Zero String Terminator.
 */
typedef struct _TGA2Footer
	{
	long Extension;
	long Developer;
	char Signature[16];
	char RsvdChar;
	char BZST;
	_TGA2Footer() {}
	} TGA2Footer;

/* TGA2DateStamp - A series of 3 WORD values which define the integer value
 *                 for the date the image was saved.
 *
 * Month - Month number (1 - 12)
 * Day   - Day number (1 - 31)
 * Year  - Year number (4 digit, ie. 1989)
 */
typedef struct _TGA2DateStamp
	{
	short Month;
	short Day;
	short Year;
	} TGA2DateStamp;

/* TGA2TimeStamp - A series of 3 WORD values which define the integer value
 *                 for the time the image was saved.
 *
 * Hour   - Hour number, military time (0 - 23)
 * Minute - Minute number (0 - 59)
 * Second - Second number (0 - 59)
 */
typedef struct _TGA2TimeStamp
	{
	short Hour;
	short Minute;
	short Second;
	} TGA2TimeStamp;

/* TGA2SoftVer - Define the version of the software used to generate file.
 *
 * Number - Version number * 100
 * Letter - Version letter
 */
typedef struct _TGA2SoftVer
	{
	short Number;
	char  Letter;
	} TGA2SoftVer;

/* TGA2Ratio - Numerator and denominator which when taken together specify
 *             a ratio.
 *
 * Numer - Numerator
 * Denom - Denominator (a value of zero indicates no ratio specified)
 */
typedef struct _TGA2Ratio
	{
	short Numer;
	short Denom;
	} TGA2Ratio;

/* TGA2Extension - Extension area, provided for additional file information.
 *                 This data is pointed to by the Extension offset in the
 *                 TGA2Footer.
 *
 * ExtSize     - Extension area size. (495 bytes for 2.0)
 * AuthName    - Name of the person who created image (null-terminated ASCII)
 * AuthComment - Comments of the author (null-terminated ASCII)
 * DateStamp   - Date the file was created. (See TGA2DateStamp)
 * TimeStamp   - Time the file was created. (See TGA2TimeStamp)
 * JobName     - Name of job image belongs to (null-terminated ASCII)
 * JobTime     - Elapsed time of the job.
 * SoftID      - ID of software used to create image (null-terminated ASCII)
 * SoftVer     - Version number of software used.
 * KeyColor    - Tranparent color value.
 * Aspect      - Pixel aspect ratio.
 * Gamma       - Fractional gamma value.
 * ColorCor    - Color correction table offset.
 * PostStamp   - Postage stamp image offset.
 * ScanLine    - Scan line table offset.
 * Attributes  - Alpha channel attributes. (Set defines below)
 */
typedef struct _TGA2Extension
	{
	short         ExtSize;
	char          AuthName[41];
	char          AuthComment[324];
	TGA2DateStamp Date;
	TGA2TimeStamp Time;
	char          JobName[41];
	TGA2TimeStamp JobTime;
	char          SoftID[41];
	TGA2SoftVer   SoftVer;
	long          KeyColor;
	TGA2Ratio     Aspect;
	TGA2Ratio     Gamma;
	long          ColorCor;
	long          PostStamp;
	long          ScanLine;
	char          Attributes;
	} TGA2Extension;

/* Alpha channel attributes (Extension Area) */
#define EXTA_NOALPHA 0  /* No alpha data included */
#define EXTA_IGNORE  1  /* Undefined alpha data, can ignore */
#define EXTA_RETAIN  2  /* Undefined alpha data, should retain */
#define EXTA_USEFUL  3  /* Useful alpha channel */
#define EXTA_PREMULT 4  /* Pre-Multiplied alpha data */

#pragma pack(pop)

/*
** This define changes this code from code that works with standard IO calls,
** to code that uses FileClass and FileFactoryClass.
*/
#define TGA_USES_WWLIB_FILE_CLASSES

#ifdef TGA_USES_WWLIB_FILE_CLASSES
class FileClass;
#endif

/*---------------------------------------------------------------------------
 * CLASS DEFINITION
 *-------------------------------------------------------------------------*/

class Targa
	{
	public:
		/* Constructor/destructor */
		Targa(void);
		~Targa();

		/* Function prototypes. */
		long Open(const char* name, long mode);
		void Close(void);

		long Load(const char* name, char* palette, char* image,bool invert_image=true);
		long Load(const char* name, long flags, bool invert_image=true);
		long Save(const char* name, long flags, bool addextension = false);

		void XFlip(void);
		void YFlip(void);

		char* SetImage(char* buffer);
		char* GetImage(void) const {return (mImage);}

		char* SetPalette(char* buffer);
		char* GetPalette(void) const {return (mPalette);}

		bool IsCompressed(void);

		TGA2Extension* GetExtension(void);

		TGAHeader Header;

	protected:
#ifdef TGA_USES_WWLIB_FILE_CLASSES
		FileClass *TGAFile;
#else
		long mFH;
#endif
		long mAccess;
		long mFlags;
		char* mImage;
		char* mPalette;
		TGA2Extension mExtension;

	private:
		// Utility functions
		long DecodeImage(void);
		long EncodeImage(void);
		void InvertImage(void);

		// These functions are for ease of ifdef'ing between standard io calls
		// and FileClass.
		void Clear_File(void);
		bool Is_File_Open(void);
		bool File_Open_Read(const char* name);
		bool File_Open_Write(const char* name);
		bool File_Open_ReadWrite(const char* name);
		int File_Seek(int pos, int dir);
		int File_Read(void *buffer, int size);
		int File_Write(void *buffer, int size);
	};
