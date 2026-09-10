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
#include "WWLib/mpsc_intrusive_queue.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <span>
#include <vector>

import Graphics.Frame.Runtime;
import Graphics.Capture.FrameCapture;
import Video.Capture.ImageWriter;

struct ScreenshotThreadData
{
	// Retain the readback storage until the worker finishes encoding its frame view.
	Graphics::FrameCapture readback;
	Engine::Video::DecodedVideoFrame frame;
	std::string userDataDirectory;
	std::string leafname;
	int quality;
	ScreenshotFormat format;
};

// TheInGameUI is not thread safe, so the screenshot threads cannot show the success message
// themselves. Each thread pushes the written filename onto this queue and the main thread
// shows all pending messages in W3D_UpdateScreenshotMessages, so no message is lost when
// multiple screenshot threads finish within the same frame.
struct ScreenshotWrittenMessage
{
	ScreenshotWrittenMessage* next;
	char leafname[64];
};
static MPSCIntrusiveQueue<ScreenshotWrittenMessage> s_screenshotWrittenQueue;

static int SDLCALL screenshotThreadFunc(void *param)
{
	ScreenshotThreadData* data = static_cast<ScreenshotThreadData*>(param);

	// TheSuperHackers @feature bobtista 08/07/2026 Save screenshots into a Screenshots subfolder
	// to keep the user data root folder tidy.
	const std::filesystem::path screenshotDirectory = std::filesystem::path(data->userDataDirectory) / "Screenshots";
	std::filesystem::create_directories(screenshotDirectory);
	const std::string pathname = (screenshotDirectory / data->leafname).string();

	Engine::Video::FrameImageOptions options;
	options.format = data->format == SCREENSHOT_JPEG
		? Engine::Video::FrameImageFormat::JPEG : Engine::Video::FrameImageFormat::PNG;
	options.jpeg_quality = data->quality;
	const bool success = Engine::Video::Write_Frame_Image(pathname, data->frame, options);

	if (success)
	{
		ScreenshotWrittenMessage* message = new ScreenshotWrittenMessage;
		std::snprintf(message->leafname, sizeof(message->leafname), "%s", data->leafname.c_str());
		s_screenshotWrittenQueue.Push(message);
	}
	else
	{
		DEBUG_LOG(("Failed to write screenshot %s", pathname.c_str()));
	}

	delete data;

	return success;
}

void W3D_UpdateScreenshotMessages()
{
	ScreenshotWrittenMessage* message = s_screenshotWrittenQueue.Flush();
	while (message != nullptr)
	{
		UnicodeString ufileName;
		ufileName.translate(message->leafname);
		TheInGameUI->message(TheGameText->fetch("GUI:ScreenCapture"), ufileName.str());
		ScreenshotWrittenMessage* next = message->next;
		delete message;
		message = next;
	}
}

void W3D_TakeCompressedScreenshot(ScreenshotFormat format, Int jpegQuality)
{
	if (format < 0 || format >= SCREENSHOT_FORMAT_COUNT)
		return;

	static constexpr const char* const ScreenshotFormatExtensions[] = { "jpg", "png" };
	static_assert(ARRAY_SIZE(ScreenshotFormatExtensions) == SCREENSHOT_FORMAT_COUNT, "Incorrect array size");

	// The filename is created here so the timestamp matches the capture time.
	char leafname[64];
	const char* extension = ScreenshotFormatExtensions[format];

	SDL_Time currentTime;
	SDL_DateTime st{};
	SDL_GetCurrentTime(&currentTime);
	SDL_TimeToDateTime(currentTime, &st, true);
	sprintf(leafname, "sshot_%04d%02d%02d_%02d%02d%02d_%03d.%s",
		st.year, st.month, st.day, st.hour, st.minute, st.second, st.nanosecond / 1000000, extension);

	auto* device = Graphics::Shared_Frame_Device();
	if (device == nullptr)
		return;
	const auto target = device->Get_Swap_Chain().Backbuffer();
	auto* threadData = new ScreenshotThreadData();
	threadData->frame = threadData->readback.Read(*device, target.texture, target.width, target.height,
		Graphics::RHITextureFormat::BGRA8_UNorm);
	if (!threadData->frame.Is_Valid()) {
		delete threadData;
		return;
	}
	threadData->quality = jpegQuality;
	threadData->format = format;
	threadData->userDataDirectory = TheGlobalData->getPath_UserData().str();
	threadData->leafname = leafname;

	SDL_Thread *thread = SDL_CreateThread(screenshotThreadFunc, "Screenshot", threadData);
	if (thread)
	{
		SDL_DetachThread(thread);
	}
	else
	{
		delete threadData;
	}
}
