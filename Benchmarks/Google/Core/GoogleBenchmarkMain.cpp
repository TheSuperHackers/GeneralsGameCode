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
#include <stdio.h>
#include <vector>

#include <benchmark/benchmark.h>

#include "Common/CommandLine.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#include "GameClient/ClientInstance.h"

// The game engine libraries expect the executable to define these.
HINSTANCE ApplicationHInstance = nullptr;
HWND ApplicationHWnd = nullptr;
const char *gAppPrefix = "GoogleBenchmark_";
const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";

static void printCrash(const char *message)
{
	fprintf(stderr, "%s\n", message);
}

static int runBenchmarks(int argc, char **argv)
{
	DebugSetCrashHandler(printCrash);

	// Same startup as GameDebugInit does in logging builds, so that the benchmarks run alike in every build.
	initMemoryManager();
	CommandLine::parseCommandLineForStartup();
	rts::ClientInstance::initialize();

	// Google Benchmark removes its flags from the argument array it gets, but the engine reads the process
	// arguments through __argv and __argc, so Google Benchmark gets a copy that includes the terminating null.
	std::vector<char *> benchmarkArgs(argv, argv + argc + 1);
	::benchmark::Initialize(&argc, benchmarkArgs.data());
	if (::benchmark::ReportUnrecognizedArguments(argc, benchmarkArgs.data()))
		return 1;
	::benchmark::RunSpecifiedBenchmarks();
	::benchmark::Shutdown();

	// The memory manager is not shut down, because Google Benchmark frees its objects through it after main returns.
	return 0;
}

int main(int argc, char **argv)
{
	return runBenchmarks(argc, argv);
}

// The engine refers to WinMain when it dumps exception info, so the executable must define it.
Int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, Int nCmdShow)
{
	return runBenchmarks(__argc, __argv);
}
