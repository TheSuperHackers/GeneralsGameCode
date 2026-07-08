/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#include "W3DDevice/GameClient/W3DScreenshot.h"
#include "Common/GlobalData.h"
#include "GameClient/GameText.h"
#include "GameClient/InGameUI.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/surfaceclass.h"
#include <stb_image_write.h>

struct ScreenshotThreadData
{
	unsigned char* imageData;
	unsigned int width;
	unsigned int height;
	char pathname[_MAX_PATH];
	int quality;
	ScreenshotFormat format;
};

static DWORD WINAPI screenshotThreadFunc(LPVOID param)
{
	ScreenshotThreadData* data = (ScreenshotThreadData*)param;

	int result = 0;
	switch (data->format)
	{
		case SCREENSHOT_JPEG:
			result = stbi_write_jpg(data->pathname, data->width, data->height, 3, data->imageData, data->quality);
			break;
		case SCREENSHOT_PNG:
			result = stbi_write_png(data->pathname, data->width, data->height, 3, data->imageData, data->width * 3);
			break;
	}

	if (!result)
	{
		DEBUG_LOG(("Failed to write screenshot %s", data->pathname));
	}

	delete [] data->imageData;
	delete data;

	return 0;
}

void W3D_TakeCompressedScreenshot(ScreenshotFormat format, int quality)
{
	char leafname[_MAX_FNAME];
	char pathname[_MAX_PATH];
	const char* extension = (format == SCREENSHOT_JPEG) ? "jpg" : "png";

	SYSTEMTIME st;
	GetLocalTime(&st);
	sprintf(leafname, "sshot_%04d%02d%02d_%02d%02d%02d_%03d.%s",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, extension);
	strlcpy(pathname, TheGlobalData->getPath_UserData().str(), ARRAY_SIZE(pathname));
	strlcat(pathname, leafname, ARRAY_SIZE(pathname));

	// TheSuperHackers @bugfix xezon 21/05/2025 Get the back buffer and create a copy of the surface.
	// Originally this code took the front buffer and tried to lock it. This does not work when the
	// render view clips outside the desktop boundaries. It crashed the game.
	SurfaceClass* surface = DX8Wrapper::_Get_DX8_Back_Buffer();
	SurfaceClass::SurfaceDescription surfaceDesc;
	surface->Get_Description(surfaceDesc);

	SurfaceClass* surfaceCopy = NEW_REF(SurfaceClass, (DX8Wrapper::_Create_DX8_Surface(surfaceDesc.Width, surfaceDesc.Height, surfaceDesc.Format)));
	DX8Wrapper::_Copy_DX8_Rects(surface->Peek_D3D_Surface(), nullptr, 0, surfaceCopy->Peek_D3D_Surface(), nullptr);

	surface->Release_Ref();
	surface = nullptr;

	struct Rect
	{
		int Pitch;
		void* pBits;
	} lrect;

	lrect.pBits = surfaceCopy->Lock(&lrect.Pitch);
	if (lrect.pBits == nullptr)
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
	surfaceCopy = nullptr;

	if (quality <= 0 && format == SCREENSHOT_JPEG)
	{
		quality = TheGlobalData->m_jpegQuality;
	}

	ScreenshotThreadData* threadData = new ScreenshotThreadData();
	threadData->imageData = image;
	threadData->width = width;
	threadData->height = height;
	threadData->quality = quality;
	threadData->format = format;
	strlcpy(threadData->pathname, pathname, ARRAY_SIZE(threadData->pathname));

	DWORD threadId;
	HANDLE hThread = CreateThread(nullptr, 0, screenshotThreadFunc, threadData, 0, &threadId);
	if (hThread)
	{
		CloseHandle(hThread);

		UnicodeString ufileName;
		ufileName.translate(leafname);
		TheInGameUI->message(TheGameText->fetch("GUI:ScreenCapture"), ufileName.str());
	}
	else
	{
		delete [] threadData->imageData;
		delete threadData;
	}
}
