#pragma once

#include <SDL3/SDL.h>

class SDLPlatformWindow
{
public:
	SDLPlatformWindow() = default;
	~SDLPlatformWindow()
	{
		shutdown();
	}

	bool initialize(int width, int height, bool fullscreen);
	void show();
	static void hide();
	static int showMessageBox(const char *message, const char *title, int type);
	void shutdown();

	static void *window();
	static void *nativeHandle();
	static bool isFullscreen();
	static bool setFullscreen(bool fullscreen);
	static Uint32 fullscreenToggleEventType();
	void *nativeInstance() const;
	static void setTitle(const char *title);
	static bool isMinimized();
	static bool setWorkingDirectoryToExecutable();
	static bool restoreExistingInstance(const char *instanceName);
	static void initializeRuntimeModule();
	static void installCrashHandler();

private:
	SDL_Window *m_window = nullptr;
};
