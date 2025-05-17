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

// FramGrab.cpp: implementation of the FrameGrabClass class.
//
//////////////////////////////////////////////////////////////////////

#include "framgrab.h"
#include <stdio.h>
#include <io.h>
//#include <errno.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FrameGrabClass::FrameGrabClass(const char *filename, MODE mode, int width, int height, int bitcount, float framerate)
{
	HRESULT          hr; 
	
	Mode = mode;
	Filename = filename;
	FrameRate = framerate;
	Counter = 0;

	Stream = 0;
	AVIFile = 0;

	if(Mode != AVI) return;

	AVIFileInit();          // opens AVIFile library  

	// find the first free file with this prefix
	int counter = 0;
	int result;
	char file[256];
	do {
		sprintf(file, "%s%d.AVI", filename, counter++);
		result = _access(file, 0);
	} while(result != -1);

	// Create new AVI file using AVIFileOpen. 
    hr = AVIFileOpen(&AVIFile, file, OF_WRITE | OF_CREATE, NULL); 
    if (hr != 0) {
		char buf[256];
		sprintf(buf, "Unable to open %s\n", Filename);
		OutputDebugString(buf);
		CleanupAVI();
		return;
	}
    

    // Create a stream using AVIFileCreateStream. 
	AVIStreamInfo.fccType = streamtypeVIDEO;
	AVIStreamInfo.fccHandler = mmioFOURCC('M','S','V','C');
	AVIStreamInfo.dwFlags = 0;
	AVIStreamInfo.dwCaps = 0;
	AVIStreamInfo.wPriority = 0;
	AVIStreamInfo.wLanguage = 0;
	AVIStreamInfo.dwScale = 1;
	AVIStreamInfo.dwRate = (int)FrameRate;
	AVIStreamInfo.dwStart = 0;
	AVIStreamInfo.dwLength = 0;
	AVIStreamInfo.dwInitialFrames = 0;
	AVIStreamInfo.dwSuggestedBufferSize = 0;
	AVIStreamInfo.dwQuality = 0;
	AVIStreamInfo.dwSampleSize = 0;
	SetRect(&AVIStreamInfo.rcFrame, 0, 0, width, height);  
	AVIStreamInfo.dwEditCount = 0;
	AVIStreamInfo.dwFormatChangeCount = 0;
	sprintf(AVIStreamInfo.szName,"G");

    hr = AVIFileCreateStream(AVIFile, &Stream, &AVIStreamInfo); 
    if (hr != 0) {   
		CleanupAVI();
		return;     
	}
	
    // Set format of new stream
	BitmapInfoHeader.biWidth = width;
	BitmapInfoHeader.biHeight = height; 
	BitmapInfoHeader.biBitCount = (unsigned short)bitcount;
    BitmapInfoHeader.biSizeImage = ((((UINT)BitmapInfoHeader.biBitCount * BitmapInfoHeader.biWidth + 31) & ~31) / 8) * BitmapInfoHeader.biHeight; 
	BitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER); // size of structure
	BitmapInfoHeader.biPlanes = 1; // must be set to 1
	BitmapInfoHeader.biCompression = BI_RGB; // uncompressed
 	BitmapInfoHeader.biXPelsPerMeter = 1; // not used
	BitmapInfoHeader.biYPelsPerMeter = 1; // not used
	BitmapInfoHeader.biClrUsed = 0; // all colors are used
	BitmapInfoHeader.biClrImportant = 0; // all colors are important

    hr = AVIStreamSetFormat(Stream, 0, &BitmapInfoHeader, sizeof(BitmapInfoHeader)); 
    if (hr != 0) {
		CleanupAVI();
		return;     
	}  

    Bitmap = (long *) GlobalAllocPtr(GMEM_MOVEABLE, BitmapInfoHeader.biSizeImage); 
}

FrameGrabClass::~FrameGrabClass()
{
	if(Mode == AVI) {
		CleanupAVI();
	}
}

void FrameGrabClass::CleanupAVI() {
	if(Bitmap != 0) { GlobalFreePtr(Bitmap); Bitmap = 0; }
	if(Stream != 0) { AVIStreamRelease(Stream); Stream = 0; }
	if(AVIFile != 0) { AVIFileRelease(AVIFile); AVIFile = 0; }
	
	AVIFileExit();
	Mode = RAW;
}

void FrameGrabClass::GrabAVI(void *BitmapPointer)
{
    // CompressDIB(&bi, lpOld, &biNew, lpNew);  

    // Save the compressed data using AVIStreamWrite. 
    HRESULT hr = AVIStreamWrite(Stream, Counter++, 1, BitmapPointer, BitmapInfoHeader.biSizeImage, AVIIF_KEYFRAME, NULL, NULL);     
	if(hr != 0) {
		char buf[256];
		sprintf(buf, "avi write error %x/%d\n", hr, hr);
		OutputDebugString(buf);
	} 
}

void FrameGrabClass::GrabRawFrame(void * /*BitmapPointer*/)
{

}


void FrameGrabClass::ConvertGrab(void *BitmapPointer) 
{
	ConvertFrame(BitmapPointer);
	Grab( Bitmap );
}


void FrameGrabClass::Grab(void *BitmapPointer) 
{
	if(Mode == AVI) 
		GrabAVI(BitmapPointer);
	else
		GrabRawFrame(BitmapPointer);
}


void FrameGrabClass::ConvertFrame(void *BitmapPointer) 
{

	int width = BitmapInfoHeader.biWidth;
	int height = BitmapInfoHeader.biHeight;
	long *image = (long *) BitmapPointer;

	// copy the data, doing a vertical flip & byte re-ordering of the pixel longwords
	int y = height;
	while(y--) {
		int x = width;
		int yoffset = y * width;
		int yoffset2 = (height - y) * width;
		while(x--) {
			long *source = &image[yoffset + x];
			long *dest = &Bitmap[yoffset2 + x];
			*dest = *source;
			unsigned char *c = (unsigned char *) dest;
			c[3] = c[0];
			c[0] = c[2];
			c[2] = c[3];
			c[3] = 0;
		}
	}
}
