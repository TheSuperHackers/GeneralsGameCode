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
	ScreenshotThreadData()
		: pixelData(nullptr)
	{
	}

	~ScreenshotThreadData()
	{
		delete [] pixelData;
	}

	unsigned char* pixelData;
	unsigned int width;
	unsigned int height;
	bool is16Bit;
	char userDataDirectory[_MAX_PATH];
	char leafname[_MAX_FNAME];
	int quality;
	ScreenshotFormat format;
};

static DWORD WINAPI screenshotThreadFunc(LPVOID param)
{
	ScreenshotThreadData* data = (ScreenshotThreadData*)param;

	// TheSuperHackers @feature bobtista 08/07/2026 Save screenshots into a Screenshots subfolder
	// to keep them out of the User Data root.
	char pathname[_MAX_PATH];
	strlcpy(pathname, data->userDataDirectory, ARRAY_SIZE(pathname));
	strlcat(pathname, "Screenshots\\", ARRAY_SIZE(pathname));
	CreateDirectory(pathname, nullptr);
	strlcat(pathname, data->leafname, ARRAY_SIZE(pathname));

	const unsigned int width = data->width;
	const unsigned int height = data->height;
	unsigned int x, y, index;

	// Convert to R8G8B8 for stb_image_write.
	unsigned char* image = new unsigned char[3 * width * height];

	if (!data->is16Bit)
	{
		// Convert A8R8G8B8/X8R8G8B8 to R8G8B8
		for (y = 0; y < height; y++)
		{
			const unsigned char* srcLine = data->pixelData + y * width * 4;
			for (x = 0; x < width; x++)
			{
				index = 3 * (x + y * width);
				image[index + 0] = srcLine[4 * x + 2];
				image[index + 1] = srcLine[4 * x + 1];
				image[index + 2] = srcLine[4 * x + 0];
			}
		}
	}
	else
	{
		// Convert R5G6B5 to R8G8B8
		for (y = 0; y < height; y++)
		{
			const unsigned short* srcLine = (const unsigned short*)(data->pixelData + y * width * 2);
			for (x = 0; x < width; x++)
			{
				const unsigned short rgb = srcLine[x];
				index = 3 * (x + y * width);
				image[index + 0] = (unsigned char)((rgb & 0xF800) >> 8);
				image[index + 1] = (unsigned char)((rgb & 0x07E0) >> 3);
				image[index + 2] = (unsigned char)((rgb & 0x001F) << 3);
			}
		}
	}

	int success = 0;
	switch (data->format)
	{
		case SCREENSHOT_JPEG:
			success = stbi_write_jpg(pathname, width, height, 3, image, data->quality);
			break;
		case SCREENSHOT_PNG:
			success = stbi_write_png(pathname, width, height, 3, image, width * 3);
			break;
	}

	if (!success)
	{
		DEBUG_LOG(("Failed to write screenshot %s", pathname));
	}

	delete [] image;
	delete data;

	return success;
}

void W3D_TakeCompressedScreenshot(ScreenshotFormat format, Int jpegQuality)
{
	static const char* const ScreenshotFormatExtensions[] = { "jpg", "png" };
	static_assert(ARRAY_SIZE(ScreenshotFormatExtensions) == SCREENSHOT_FORMAT_COUNT, "Incorrect array size");

	// The filename is created here because the success message below shows it.
	char leafname[_MAX_FNAME];
	const char* extension = ScreenshotFormatExtensions[format];

	SYSTEMTIME st;
	GetLocalTime(&st);
	sprintf(leafname, "sshot_%04d%02d%02d_%02d%02d%02d_%03d.%s",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, extension);

	// TheSuperHackers @bugfix xezon 21/05/2025 Get the back buffer and create a copy of the surface.
	// Originally this code took the front buffer and tried to lock it. This does not work when the
	// render view clips outside the desktop boundaries. It crashed the game.
	SurfaceClass* surface = DX8Wrapper::_Get_DX8_Back_Buffer();
	SurfaceClass::SurfaceDescription surfaceDesc;
	surface->Get_Description(surfaceDesc);

	// TheSuperHackers @bugfix bobtista 08/07/2026 Support the 16 bit back buffer format that the
	// game uses when running in 16 bit color mode. Reading it with the 32 bit stride read garbage.
	const bool is32Bit = surfaceDesc.Format == WW3D_FORMAT_A8R8G8B8 || surfaceDesc.Format == WW3D_FORMAT_X8R8G8B8;
	const bool is16Bit = surfaceDesc.Format == WW3D_FORMAT_R5G6B5;

	if (!is32Bit && !is16Bit)
	{
		DEBUG_LOG(("Screenshot does not support back buffer format %d", (int)surfaceDesc.Format));
		surface->Release_Ref();
		return;
	}

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

	unsigned int y;
	unsigned int width = surfaceDesc.Width;
	unsigned int height = surfaceDesc.Height;

	// Copy the surface rows into a tightly packed buffer. The pixel conversion and all file
	// operations are done on the screenshot thread to keep the main thread cheap.
	const unsigned int bytesPerPixel = is32Bit ? 4 : 2;
	unsigned char* pixels = new unsigned char[bytesPerPixel * width * height];

	for (y = 0; y < height; y++)
	{
		memcpy(pixels + y * width * bytesPerPixel, (const unsigned char*)lrect.pBits + y * lrect.Pitch, width * bytesPerPixel);
	}

	surfaceCopy->Unlock();
	surfaceCopy->Release_Ref();
	surfaceCopy = nullptr;

	ScreenshotThreadData* threadData = new ScreenshotThreadData();
	threadData->pixelData = pixels;
	threadData->width = width;
	threadData->height = height;
	threadData->is16Bit = is16Bit;
	threadData->quality = jpegQuality;
	threadData->format = format;
	strlcpy(threadData->userDataDirectory, TheGlobalData->getPath_UserData().str(), ARRAY_SIZE(threadData->userDataDirectory));
	strlcpy(threadData->leafname, leafname, ARRAY_SIZE(threadData->leafname));

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
		delete threadData;
	}
}
