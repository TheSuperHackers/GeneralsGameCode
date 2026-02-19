/* macOS shim: shlobj.h — Shell objects (folder paths, etc.) */
#pragma once
#include "windows.h"

#ifndef _LPITEMIDLIST_DEFINED
#define _LPITEMIDLIST_DEFINED
typedef void *LPITEMIDLIST;
#endif

#ifndef CSIDL_PERSONAL
#define CSIDL_PERSONAL 0x0005
#endif
#ifndef CSIDL_APPDATA
#define CSIDL_APPDATA 0x001A
#endif
#ifndef CSIDL_DESKTOPDIRECTORY
#define CSIDL_DESKTOPDIRECTORY 0x0010
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
    } else if (csidl == CSIDL_DESKTOPDIRECTORY) {
      snprintf(pszPath, MAX_PATH, "%s/Desktop", home);
    } else {
      snprintf(pszPath, MAX_PATH, "%s", home);
    }
    return TRUE;
  }
  return FALSE;
}

#define SHGetSpecialFolderPathA SHGetSpecialFolderPath

/* SHGetSpecialFolderLocation + SHGetPathFromIDList pattern used by ReplayMenu
 */
inline HRESULT SHGetSpecialFolderLocation(HWND, int csidl,
                                          LPITEMIDLIST *ppidl) {
  /* store the csidl in the pidl pointer itself as a tag */
  if (ppidl)
    *ppidl = (LPITEMIDLIST)(intptr_t)csidl;
  return 0; /* S_OK */
}

inline BOOL SHGetPathFromIDList(LPITEMIDLIST pidl, char *pszPath) {
  int csidl = (int)(intptr_t)pidl;
  return SHGetSpecialFolderPath(nullptr, pszPath, csidl, FALSE);
}
