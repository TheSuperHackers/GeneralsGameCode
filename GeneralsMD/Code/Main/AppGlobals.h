#pragma once

#include "Lib/BaseType.h"

class Win32Mouse;

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
extern HINSTANCE ApplicationHInstance;
extern HWND ApplicationHWnd;
extern DWORD TheMessageTime;
#endif

#if defined(RTS_HAS_SDL3)
void showMainWindow();
#endif

extern const Char *g_strFile;
extern const Char *g_csfFile;
extern const char *gAppPrefix;
