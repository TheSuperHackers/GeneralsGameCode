/* macOS shim: shellapi.h — Shell API stubs */
#pragma once
#include "windows.h"

#ifndef CSIDL_PERSONAL
#define CSIDL_PERSONAL 0x0005
#endif

typedef struct _SHFILEOPSTRUCTA {
  HWND hwnd;
  UINT wFunc;
  const char *pFrom;
  const char *pTo;
  WORD fFlags;
  BOOL fAnyOperationsAborted;
  void *hNameMappings;
  const char *lpszProgressTitle;
} SHFILEOPSTRUCT, *LPSHFILEOPSTRUCT;

/* Shell operations */
#define FO_DELETE 0x0003
#define FO_COPY 0x0002
#define FOF_NOCONFIRMATION 0x0010
#define FOF_SILENT 0x0004

inline int SHFileOperation(LPSHFILEOPSTRUCT) {
  return 1; /* fail: not supported */
}
inline HRESULT SHGetFolderPath(HWND, int, void *, DWORD, char *) {
  return E_FAIL;
}
inline HINSTANCE ShellExecute(HWND, const char *, const char *, const char *,
                              const char *, int) {
  return NULL;
}
