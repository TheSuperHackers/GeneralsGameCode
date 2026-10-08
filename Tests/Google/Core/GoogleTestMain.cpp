/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#include <windows.h>
#include <vector>

#include <gtest/gtest.h>

#include "Common/CommandLine.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#include "GameClient/ClientInstance.h"

// The game engine libraries expect the executable to define these.
HINSTANCE ApplicationHInstance = nullptr;
HWND ApplicationHWnd = nullptr;
const char *gAppPrefix = "GoogleTest_";
const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";

static void failTestOnCrash(const char *message)
{
	ADD_FAILURE() << message;
}

static int runTests(int argc, char **argv)
{
	// Google Test removes its flags from the argument array it gets, but the engine reads the process
	// arguments through __argv and __argc, so Google Test gets a copy that includes the terminating null.
	std::vector<char *> testArgs(argv, argv + argc + 1);
	::testing::InitGoogleTest(&argc, testArgs.data());
	DebugSetCrashHandler(failTestOnCrash);

	// Same startup as GameDebugInit does in logging builds, so that the tests run alike in every build.
	initMemoryManager();
	CommandLine::parseCommandLineForStartup();
	rts::ClientInstance::initialize();

	// The memory manager is not shut down, because Google Test frees its objects through it after main returns.
	return RUN_ALL_TESTS();
}

int main(int argc, char **argv)
{
	return runTests(argc, argv);
}

// The engine refers to WinMain when it dumps exception info, so the executable must define it.
Int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, Int nCmdShow)
{
	return runTests(__argc, __argv);
}
