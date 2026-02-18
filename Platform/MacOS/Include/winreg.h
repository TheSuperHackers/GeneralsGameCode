/* macOS shim: winreg.h — Windows Registry API stubs */
#pragma once
#include "windows.h"

typedef void *HKEY;
#define HKEY_LOCAL_MACHINE ((HKEY)(ULONG_PTR)0x80000002)
#define HKEY_CURRENT_USER ((HKEY)(ULONG_PTR)0x80000001)

#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define KEY_ALL_ACCESS 0xF003F

#define REG_SZ 1
#define REG_DWORD 4
#define REG_BINARY 3

inline LONG RegOpenKeyEx(HKEY, const char *, DWORD, DWORD, HKEY *phk) {
  if (phk)
    *phk = NULL;
  return 2L; /* ERROR_FILE_NOT_FOUND */
}
#define RegOpenKeyExA RegOpenKeyEx

inline LONG RegQueryValueEx(HKEY, const char *, DWORD *, DWORD *, BYTE *,
                            DWORD *) {
  return 2L;
}
#define RegQueryValueExA RegQueryValueEx

inline LONG RegSetValueEx(HKEY, const char *, DWORD, DWORD, const BYTE *,
                          DWORD) {
  return 5L; /* ERROR_ACCESS_DENIED */
}
#define RegSetValueExA RegSetValueEx

inline LONG RegCreateKeyEx(HKEY, const char *, DWORD, char *, DWORD, DWORD,
                           void *, HKEY *phk, DWORD *) {
  if (phk)
    *phk = NULL;
  return 5L;
}
#define RegCreateKeyExA RegCreateKeyEx

inline LONG RegCloseKey(HKEY) { return 0L; }
inline LONG RegDeleteKey(HKEY, const char *) { return 5L; }
#define RegDeleteKeyA RegDeleteKey
