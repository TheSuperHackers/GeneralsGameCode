#include "AppGlobals.h"

#include "Common/CommandLine.h"
#include "Common/CriticalSection.h"
#include "Common/Debug.h"
#include "Common/GameDefines.h"
#include "Common/GameEngine.h"
#include "Common/GameMemory.h"
#include "Common/GlobalData.h"
#include "Common/MessageStream.h"
#include "Common/StackDump.h"
#include "Common/version.h"
#include "GameClient/ClientInstance.h"
#include "SDL3Device/Common/SDL3GameEngine.h"

#include "BuildVersion.h"
#include "GeneratedVersion.h"

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_properties.h>

#if defined(_WIN32)
#include <SDL3/SDL_system.h>
#include <crtdbg.h>
#include <direct.h>
#include <eh.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>

static SDL_Window *g_applicationWindow = nullptr;
static SDL_Window *g_splashWindow = nullptr;

#if defined(_WIN32)
#ifndef SDL_PROP_WINDOW_WIN32_HWND_POINTER
#define SDL_PROP_WINDOW_WIN32_HWND_POINTER "SDL.window.win32.hwnd"
#endif
#ifndef SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER
#define SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER "SDL.window.win32.instance"
#endif
#endif

// TheSuperHackers @feature denysmitin 03/12/2025 Replace WinMain-based bootstrap with portable SDL3 entry point.

#if !defined(_WIN32)
static std::string gCommandLineBuffer;

extern "C" const char *GetCommandLineA(void)
{
	return gCommandLineBuffer.c_str();
}

static void buildCommandLineBuffer(int argc, char **argv)
{
	gCommandLineBuffer.clear();
	for (int i = 0; i < argc; ++i)
	{
		if (i > 0)
		{
			gCommandLineBuffer.push_back(' ');
		}

		const char *argument = argv[i] != nullptr ? argv[i] : "";
		const bool needsQuotes = std::strchr(argument, ' ') != nullptr;
		if (needsQuotes)
		{
			gCommandLineBuffer.push_back('"');
		}
		gCommandLineBuffer.append(argument);
		if (needsQuotes)
		{
			gCommandLineBuffer.push_back('"');
		}
	}
}
#endif

#if defined(_WIN32)
static LONG WINAPI unhandledExceptionFilter(struct _EXCEPTION_POINTERS *info)
{
	DumpExceptionInfo(info->ExceptionRecord->ExceptionCode, info);
	return EXCEPTION_EXECUTE_HANDLER;
}

static void setWorkingDirectoryToExecutable()
{
	Char buffer[_MAX_PATH] = {0};
	if (GetModuleFileName(nullptr, buffer, sizeof(buffer)) == 0)
	{
		return;
	}

	if (Char *lastSlash = strrchr(buffer, '\\'))
	{
		*lastSlash = 0;
		SetCurrentDirectory(buffer);
	}
}

#else
static void setWorkingDirectoryToExecutable(const char *argv0)
{
	if (argv0 == nullptr)
	{
		return;
	}

	std::error_code errorCode;
	std::filesystem::path executablePath = std::filesystem::absolute(argv0, errorCode);
	if (errorCode)
	{
		if (char *basePath = SDL_GetBasePath())
		{
			chdir(basePath);
			SDL_free(basePath);
		}
		return;
	}

	std::filesystem::path directory = executablePath.parent_path();
	std::filesystem::current_path(directory, errorCode);
	if (errorCode && std::filesystem::exists(directory))
	{
		if (char *basePath = SDL_GetBasePath())
		{
			chdir(basePath);
			SDL_free(basePath);
		}
	}
}
#endif

static CriticalSection gCriticalSectionAscii;
static CriticalSection gCriticalSectionUnicode;
static CriticalSection gCriticalSectionDma;
static CriticalSection gCriticalSectionMemoryPool;
static CriticalSection gCriticalSectionDebugLog;

static void assignCriticalSections()
{
	TheAsciiStringCriticalSection = &gCriticalSectionAscii;
	TheUnicodeStringCriticalSection = &gCriticalSectionUnicode;
	TheDmaCriticalSection = &gCriticalSectionDma;
	TheMemoryPoolCriticalSection = &gCriticalSectionMemoryPool;
	TheDebugLogCriticalSection = &gCriticalSectionDebugLog;
}

static void releaseCriticalSections()
{
	TheAsciiStringCriticalSection = nullptr;
	TheUnicodeStringCriticalSection = nullptr;
	TheDmaCriticalSection = nullptr;
	TheMemoryPoolCriticalSection = nullptr;
	TheDebugLogCriticalSection = nullptr;
}

static int runGameLoop()
{
	Version *version = NEW Version;
	TheVersion = version;
	version->setVersion(
			VERSION_MAJOR,
			VERSION_MINOR,
			VERSION_BUILDNUM,
			VERSION_LOCALBUILDNUM,
			AsciiString(VERSION_BUILDUSER),
			AsciiString(VERSION_BUILDLOC),
			AsciiString(__TIME__),
			AsciiString(__DATE__));

	int exitCode = 1;

	if (!rts::ClientInstance::initialize())
	{
#if defined(_WIN32)
		HWND existingWindow = FindWindow(rts::ClientInstance::getFirstInstanceName(), nullptr);
		if (existingWindow)
		{
			SetForegroundWindow(existingWindow);
			ShowWindow(existingWindow, SW_RESTORE);
		}
#endif
		DEBUG_LOG(("Generals Zero Hour is already running...Bail!"));
		delete version;
		TheVersion = nullptr;
		return exitCode;
	}

	DEBUG_LOG(("Create Generals Zero Hour Mutex okay."));
	DEBUG_LOG(("CRC message is %d", GameMessage::MSG_LOGIC_CRC));

	exitCode = GameMain();

	delete version;
	TheVersion = nullptr;
	return exitCode;
}

static void configureSDLWindowFromGlobals(SDL_Window *window)
{
	if (!window)
	{
#if defined(_WIN32)
		ApplicationHInstance = GetModuleHandle(nullptr);
		ApplicationHWnd = nullptr;
#endif
		return;
	}

	SDL_SetWindowTitle(window, "Command and Conquer Generals Zero Hour");

#if defined(_WIN32)
	SDL_PropertiesID props = SDL_GetWindowProperties(window);
	ApplicationHWnd = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
	ApplicationHInstance = static_cast<HINSTANCE>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr));
#endif
}

static SDL_Window *createWindowFromSettings()
{
	if (TheGlobalData->m_headless)
	{
		return nullptr;
	}

	const int width = (TheGlobalData->m_xResolution > 0) ? TheGlobalData->m_xResolution : DEFAULT_DISPLAY_WIDTH;
	const int height = (TheGlobalData->m_yResolution > 0) ? TheGlobalData->m_yResolution : DEFAULT_DISPLAY_HEIGHT;

	Uint32 flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
	if (TheGlobalData->m_windowed)
	{
		flags |= SDL_WINDOW_RESIZABLE;
	}
	else
	{
		flags |= SDL_WINDOW_FULLSCREEN;
	}

	SDL_Window *window = SDL_CreateWindow("Command and Conquer Generals Zero Hour", width, height, flags | SDL_WINDOW_HIDDEN);
	if (!window)
	{
		DEBUG_LOG(("SDL_CreateWindow failed: %s", SDL_GetError()));
		return nullptr;
	}

	// Don't show window yet - it will be shown after engine initialization
	return window;
}

static Uint32 chooseSDLInitFlags()
{
	Uint32 flags = SDL_INIT_EVENTS;
	if (!TheGlobalData->m_headless)
	{
		flags |= SDL_INIT_VIDEO;
	}
	return flags;
}

static void configureDebugHeap()
{
#if defined(_WIN32) && defined(RTS_DEBUG)
	int dbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	dbgFlags |= (_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
	dbgFlags &= ~_CRTDBG_CHECK_CRT_DF;
	_CrtSetDbgFlag(dbgFlags);
#endif
}

static void shutdownSDL(SDL_Window *window)
{
	if (window)
	{
		SDL_DestroyWindow(window);
	}
	SDL_Quit();
}

void showMainWindow()
{
	if (g_applicationWindow)
	{
		SDL_ShowWindow(g_applicationWindow);
		// Close splash screen now that the main window is ready
		if (g_splashWindow)
		{
			SDL_DestroyWindow(g_splashWindow);
			g_splashWindow = nullptr;
		}
	}
}

// TheSuperHackers @feature denysmitin 03/12/2025 SDL3 splash screen support.
static SDL_Window *createSplashScreenWindow()
{
	const int width = 800;
	const int height = 600;
	Uint32 flags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIGH_PIXEL_DENSITY;

	SDL_Window *window = SDL_CreateWindow("Command and Conquer Generals Zero Hour", width, height, flags);
	if (!window)
	{
		DEBUG_LOG(("SDL_CreateWindow (splash) failed: %s", SDL_GetError()));
		return nullptr;
	}

	// Center the window on screen
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(window);
	return window;
}

static void displaySplashScreen(SDL_Window *window)
{
	if (!window)
	{
		return;
	}

	// Load the splash screen bitmap
	SDL_Surface *surface = SDL_LoadBMP("Install_Final.bmp");
	if (!surface)
	{
		DEBUG_LOG(("Failed to load Install_Final.bmp: %s", SDL_GetError()));
		return;
	}

	// Create renderer for the window
	SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
	if (!renderer)
	{
		DEBUG_LOG(("SDL_CreateRenderer (splash) failed: %s", SDL_GetError()));
		SDL_DestroySurface(surface);
		return;
	}

	// Create texture from surface
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_DestroySurface(surface);

	if (!texture)
	{
		DEBUG_LOG(("SDL_CreateTextureFromSurface (splash) failed: %s", SDL_GetError()));
		SDL_DestroyRenderer(renderer);
		return;
	}

	// Clear and render the splash screen
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderTexture(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);

	// Process one frame of events to ensure window displays
	SDL_PumpEvents();

	// Cleanup
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
}

GameEngine *CreateGameEngine()
{
	return NEW SDL3GameEngine;
}

int main(int argc, char **argv)
{
#if defined(_WIN32)
	SetUnhandledExceptionFilter(unhandledExceptionFilter);
	setWorkingDirectoryToExecutable();
#else
	buildCommandLineBuffer(argc, argv);
	setWorkingDirectoryToExecutable(argc > 0 ? argv[0] : nullptr);
#endif

	assignCriticalSections();
	initMemoryManager();
	configureDebugHeap();

	CommandLine::parseCommandLineForStartup();

	if (TheGlobalData->m_headless)
	{
		SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
	}

	if (!SDL_Init(chooseSDLInitFlags()))
	{
		DEBUG_LOG(("SDL_Init failed: %s", SDL_GetError()));
		releaseCriticalSections();
		shutdownMemoryManager();
		return EXIT_FAILURE;
	}

#if !defined(_WIN32)
	ApplicationHInstance = nullptr;
#endif

	// Display splash screen during initialization
	if (!TheGlobalData->m_headless)
	{
		g_splashWindow = createSplashScreenWindow();
		displaySplashScreen(g_splashWindow);
	}

	SDL_Window *window = createWindowFromSettings();
	g_applicationWindow = window;
	configureSDLWindowFromGlobals(window);

	// Keep splash alive until showMainWindow() is called from GameMain.

	int exitCode = runGameLoop();

	shutdownSDL(window);
	// Clear globals on shutdown
	g_applicationWindow = nullptr;
	g_splashWindow = nullptr;

#if defined(MEMORYPOOL_DEBUG)
	TheMemoryPoolFactory->debugMemoryReport(REPORT_POOLINFO | REPORT_POOL_OVERFLOW | REPORT_SIMPLE_LEAKS, 0, 0);
#endif
#if defined(RTS_DEBUG)
	TheMemoryPoolFactory->memoryPoolUsageReport("AAAMemStats");
#endif

	shutdownMemoryManager();
	releaseCriticalSections();

	return exitCode;
}
