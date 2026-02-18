/* macOS shim: shlobj.h — Shell objects (folder paths, etc.) */
#pragma once
#include "windows.h"

#ifndef CSIDL_PERSONAL
#define CSIDL_PERSONAL 0x0005
#endif
#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA 0x001A
#endif

inline BOOL SHGetSpecialFolderPath(HWND hwnd, char *pszPath, int csidl,
                                   BOOL fCreate) {
  if (pszPath) {
    const char *home = getenv("HOME");
    if (!home)
      home = "/tmp";
    if (csidl == CSIDL_PERSONAL) {
      snprintf(pszPath, MAX_PATH, "%s/Documents", home);
    } else if (csidl == CSIDL_APPDATA) {
      snprintf(pszPath, MAX_PATH, "%s/Library/Application Support", home);
    } else {
      snprintf(pszPath, MAX_PATH, "%s", home);
    }
    return TRUE;
  }
  return FALSE;
}
