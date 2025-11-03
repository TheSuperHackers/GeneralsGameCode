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

#include <stdlib.h>
#include <windows.h>
#include <io.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "W3DDevice/GameClient/W3DScreenshot.h"
#include "Common/GlobalData.h"
#include "GameClient/InGameUI.h"
#include "GameClient/GameText.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/surface.h"

struct ScreenshotThreadData
{
	unsigned char* imageData;
	unsigned int width;
	unsigned int height;
	char pathname[1024];
	char leafname[256];
	int quality;
	ScreenshotFormat format;
};

static DWORD WINAPI screenshotThreadFunc(LPVOID param)
{
	ScreenshotThreadData* data = (ScreenshotThreadData*)param;
	
	int result = 0;
	if (data->format == SCREENSHOT_JPEG)
	{
		result = stbi_write_jpg(data->pathname, data->width, data->height, 3, data->imageData, data->quality);
	}
	else if (data->format == SCREENSHOT_PNG)
	{
		result = stbi_write_png(data->pathname, data->width, data->height, 3, data->imageData, data->width * 3);
	}
	
	if (!result) {
		OutputDebugStringA("Failed to write screenshot\n");
	}
	
	delete [] data->imageData;
	delete data;
	
	return 0;
}

void W3D_TakeCompressedScreenshot(ScreenshotFormat format, int quality)
{
	char leafname[256];
	char pathname[1024];
	static int jpegFrameNumber = 1;
	static int pngFrameNumber = 1;
	
	int* frameNumber = (format == SCREENSHOT_JPEG) ? &jpegFrameNumber : &pngFrameNumber;
	const char* extension = (format == SCREENSHOT_JPEG) ? "jpg" : "png";

	Bool done = false;
	while (!done) {
		sprintf(leafname, "sshot%.3d.%s", (*frameNumber)++, extension);
		strcpy(pathname, TheGlobalData->getPath_UserData().str());
		strlcat(pathname, leafname, ARRAY_SIZE(pathname));
		if (_access(pathname, 0) == -1)
			done = true;
	}

	SurfaceClass* surface = DX8Wrapper::_Get_DX8_Back_Buffer();
	SurfaceClass::SurfaceDescription surfaceDesc;
	surface->Get_Description(surfaceDesc);

	SurfaceClass* surfaceCopy = NEW_REF(SurfaceClass, (DX8Wrapper::_Create_DX8_Surface(surfaceDesc.Width, surfaceDesc.Height, surfaceDesc.Format)));
	DX8Wrapper::_Copy_DX8_Rects(surface->Peek_D3D_Surface(), NULL, 0, surfaceCopy->Peek_D3D_Surface(), NULL);

	surface->Release_Ref();
	surface = NULL;

	struct Rect
	{
		int Pitch;
		void* pBits;
	} lrect;

	lrect.pBits = surfaceCopy->Lock(&lrect.Pitch);
	if (lrect.pBits == NULL)
	{
		surfaceCopy->Release_Ref();
		return;
	}

	unsigned int x, y, index, index2;
	unsigned int width = surfaceDesc.Width;
	unsigned int height = surfaceDesc.Height;

	unsigned char* image = new unsigned char[3 * width * height];

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			index = 3 * (x + y * width);
			index2 = y * lrect.Pitch + 4 * x;

			image[index]     = *((unsigned char*)lrect.pBits + index2 + 2);
			image[index + 1] = *((unsigned char*)lrect.pBits + index2 + 1);
			image[index + 2] = *((unsigned char*)lrect.pBits + index2 + 0);
		}
	}

	surfaceCopy->Unlock();
	surfaceCopy->Release_Ref();
	surfaceCopy = NULL;

	ScreenshotThreadData* threadData = new ScreenshotThreadData();
	threadData->imageData = image;
	threadData->width = width;
	threadData->height = height;
	threadData->quality = quality;
	threadData->format = format;
	strcpy(threadData->pathname, pathname);
	strcpy(threadData->leafname, leafname);

	DWORD threadId;
	HANDLE hThread = CreateThread(NULL, 0, screenshotThreadFunc, threadData, 0, &threadId);
	if (hThread) {
		CloseHandle(hThread);
	}

	UnicodeString ufileName;
	ufileName.translate(leafname);
	TheInGameUI->message(TheGameText->fetch("GUI:ScreenCapture"), ufileName.str());
}

