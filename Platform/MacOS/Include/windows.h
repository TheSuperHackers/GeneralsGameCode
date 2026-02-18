/*
**  macOS compatibility shim for <windows.h>
**  Provides the minimal set of Win32 types needed by DX8 SDK headers
**  and the game engine. This is NOT a full Win32 implementation.
**
**  Copyright 2025 TheSuperHackers
**  Licensed under GPL-3.0
*/
#pragma once
#ifndef _WINDOWS_H_MACOS_SHIM_
#define _WINDOWS_H_MACOS_SHIM_

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ── Calling conventions (no-ops on macOS) ──────────────────────────────
#ifndef WINAPI
#define WINAPI
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef STDAPI
#define STDAPI extern "C" HRESULT
#endif
#ifndef PASCAL
#define PASCAL
#endif
#ifndef FAR
#define FAR
#endif
#ifndef NEAR
#define NEAR
#endif
#ifndef CDECL
#define CDECL
#endif

// ── Basic integer types ────────────────────────────────────────────────
#ifndef _WINDEF_H // guard against real windef.h

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int UINT;
typedef int INT;
typedef long LONG;
typedef unsigned long ULONG;
typedef short SHORT;
typedef unsigned short USHORT;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef float FLOAT;
typedef void VOID;

typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;

// MSVC-specific type aliases — use typedef, NOT #define,
// because code like wwprofile.h does `typedef long long __int64`
// and #define would expand it to `typedef long long long long`.
typedef long long __int64;
typedef long long _int64;

#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

// MSVC string function aliases
#include <strings.h>
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif
#ifndef _snprintf
#define _snprintf snprintf
#endif
#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif
#ifndef _strdup
#define _strdup strdup
#endif

// Win32 string functions (inline so ::func() works)
#ifndef _WINSTRINGFUNCS_DEFINED
#define _WINSTRINGFUNCS_DEFINED
inline char *lstrcpyn(char *dst, const char *src, int n) {
  strncpy(dst, src, n);
  return dst;
}
inline char *lstrcat(char *dst, const char *src) { return strcat(dst, src); }
inline char *lstrcpy(char *dst, const char *src) { return strcpy(dst, src); }
inline int lstrcmpi(const char *a, const char *b) { return strcasecmp(a, b); }
inline int lstrlen(const char *s) { return (int)strlen(s); }
#endif
#ifndef _isnan
#define _isnan(x) isnan(x)
#endif

// strupr — in-place uppercase (POSIX doesn't have it, not in string_compat.h)
#include <ctype.h>
#ifndef _STRUPR_DEFINED
#define _STRUPR_DEFINED
#ifdef __cplusplus
extern "C" {
#endif
static inline char *strupr(char *s) {
  for (char *p = s; *p; ++p)
    *p = (char)toupper((unsigned char)*p);
  return s;
}
static inline char *_strupr(char *s) { return strupr(s); }
#ifdef __cplusplus
}
#endif
#endif
// strlwr is provided by Dependencies/Utility/string_compat.h

// Minimal filesystem stubs (return "not found" / empty)
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#endif
#ifndef _GETFILEATTR_DEFINED
#define _GETFILEATTR_DEFINED
inline DWORD GetCurrentDirectory(DWORD n, char *buf) {
  if (buf && n > 0)
    buf[0] = '\0';
  return 0;
}
inline DWORD GetCurrentDirectoryA(DWORD n, char *buf) {
  return GetCurrentDirectory(n, buf);
}
inline DWORD GetFileAttributes(const char *) { return INVALID_FILE_ATTRIBUTES; }
inline DWORD GetFileAttributesA(const char *p) { return GetFileAttributes(p); }
#endif

// Win32 memory/utility macros
#ifndef ZeroMemory
#define ZeroMemory(dst, len) memset((dst), 0, (len))
#endif
#ifndef CopyMemory
#define CopyMemory(dst, src, len) memcpy((dst), (src), (len))
#endif
#ifndef FillMemory
#define FillMemory(dst, len, val) memset((dst), (val), (len))
#endif

#ifndef MAKEFOURCC
#define MAKEFOURCC(a, b, c, d)                                                 \
  ((DWORD)(BYTE)(a) | ((DWORD)(BYTE)(b) << 8) | ((DWORD)(BYTE)(c) << 16) |     \
   ((DWORD)(BYTE)(d) << 24))
#endif

typedef uintptr_t UINT_PTR;
typedef intptr_t INT_PTR;
typedef uintptr_t ULONG_PTR;
typedef intptr_t LONG_PTR;
typedef size_t SIZE_T;

// ── Pointer types ──────────────────────────────────────────────────────
typedef void *PVOID;
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef BYTE *PBYTE;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef wchar_t *LPWSTR;
typedef const wchar_t *LPCWSTR;
typedef BYTE *LPBYTE;
typedef DWORD *LPDWORD;
typedef LONG *LPLONG;
typedef BOOL *LPBOOL;
typedef WORD *LPWORD;

#ifndef _TCHAR_DEFINED
typedef char TCHAR;
typedef const char *LPCTSTR;
typedef char *LPTSTR;
#define _TCHAR_DEFINED
#endif

// ── HRESULT ────────────────────────────────────────────────────────────
typedef LONG HRESULT;

#ifndef S_OK
#define S_OK ((HRESULT)0L)
#endif
#ifndef S_FALSE
#define S_FALSE ((HRESULT)1L)
#endif
#ifndef E_FAIL
#define E_FAIL ((HRESULT)0x80004005L)
#endif
#ifndef E_OUTOFMEMORY
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#endif
#ifndef E_INVALIDARG
#define E_INVALIDARG ((HRESULT)0x80070057L)
#endif
#ifndef E_NOINTERFACE
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#endif
#ifndef E_NOTIMPL
#define E_NOTIMPL ((HRESULT)0x80004001L)
#endif
#ifndef E_UNEXPECTED
#define E_UNEXPECTED ((HRESULT)0x8000FFFFL)
#endif
#ifndef E_POINTER
#define E_POINTER ((HRESULT)0x80004003L)
#endif
#ifndef E_ABORT
#define E_ABORT ((HRESULT)0x80004004L)
#endif
#ifndef D3D_OK
#define D3D_OK S_OK
#endif
#ifndef D3DERR_NOTFOUND
#define D3DERR_NOTFOUND ((HRESULT)0x88760866L)
#endif

#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif
#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev, fac, code)                                           \
  ((HRESULT)(((unsigned long)(sev) << 31) | ((unsigned long)(fac) << 16) |     \
             ((unsigned long)(code))))
#endif
#ifndef FACILITY_ITF
#define FACILITY_ITF 4
#endif
#ifndef SEVERITY_ERROR
#define SEVERITY_ERROR 1
#endif

// ── Boolean constants ──────────────────────────────────────────────────
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// ── Handle types (opaque pointers on macOS) ────────────────────────────
typedef void *HANDLE;
typedef void *HWND;
typedef void *HMONITOR;
typedef void *HDC;
typedef void *HMODULE;
typedef void *HINSTANCE;
typedef void *HICON;
typedef void *HCURSOR;
typedef void *HMENU;
typedef void *HBRUSH;
typedef void *HBITMAP;
typedef void *HRGN;
typedef void *HPEN;
typedef void *HPALETTE;
typedef void *HFONT;
typedef void *HGLOBAL;
typedef void *HGDIOBJ;
typedef void *HKEY;

#ifndef DECLARE_HANDLE
#define DECLARE_HANDLE(name) typedef void *name
#endif

typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef WORD ATOM;

// ── Geometry types ─────────────────────────────────────────────────────
#ifndef _RECT_DEFINED
#define _RECT_DEFINED
typedef struct tagRECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *LPRECT;
typedef const RECT *LPCRECT;
#endif

// ── RGNDATA (needed by d3d8.h) ─────────────────────────────────────────
#ifndef _RGNDATA_DEFINED
#define _RGNDATA_DEFINED
typedef struct _RGNDATAHEADER {
  DWORD dwSize;
  DWORD iType;
  DWORD nCount;
  DWORD nRgnSize;
  RECT rcBound;
} RGNDATAHEADER;
typedef struct _RGNDATA {
  RGNDATAHEADER rdh;
  char Buffer[1];
} RGNDATA, *LPRGNDATA;
#endif

#ifndef _POINT_DEFINED
#define _POINT_DEFINED
typedef struct tagPOINT {
  LONG x;
  LONG y;
} POINT, *LPPOINT;
#endif

#ifndef _SIZE_DEFINED
#define _SIZE_DEFINED
typedef struct tagSIZE {
  LONG cx;
  LONG cy;
} SIZE, *LPSIZE;
#endif

// ── DLL loading stubs ──────────────────────────────────────────────────
typedef void *HMODULE;
typedef void *HINSTANCE;
#ifndef _LOADLIBRARY_DEFINED
#define _LOADLIBRARY_DEFINED
inline HMODULE LoadLibrary(const char *) { return nullptr; }
inline HMODULE LoadLibraryA(const char *) { return nullptr; }
inline void *GetProcAddress(HMODULE, const char *) { return nullptr; }
inline BOOL FreeLibrary(HMODULE) { return FALSE; }
#endif

// ── Window management stubs ────────────────────────────────────────────
#ifndef _WINSTUBS_DEFINED
#define _WINSTUBS_DEFINED
inline BOOL GetClientRect(HWND, RECT *r) {
  if (r) {
    r->left = r->top = 0;
    r->right = 800;
    r->bottom = 600;
  }
  return TRUE;
}
#define GWL_STYLE (-16)
#define GWL_EXSTYLE (-20)
inline LONG GetWindowLong(HWND, int) { return 0; }
inline LONG GetWindowLongA(HWND h, int n) { return GetWindowLong(h, n); }
inline BOOL AdjustWindowRect(RECT *, DWORD, BOOL) { return TRUE; }
#define HWND_TOPMOST ((HWND)(LONG_PTR) - 1)
#define HWND_NOTOPMOST ((HWND)(LONG_PTR) - 2)
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
inline BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT) { return TRUE; }
#define MONITOR_DEFAULTTOPRIMARY 1
typedef void *HMONITOR;
typedef struct {
  DWORD cbSize;
  RECT rcMonitor;
  RECT rcWork;
  DWORD dwFlags;
} MONITORINFO;
inline HMONITOR MonitorFromWindow(HWND, DWORD) { return nullptr; }
inline BOOL GetMonitorInfo(HMONITOR, MONITORINFO *mi) {
  if (mi) {
    mi->rcMonitor = {0, 0, 800, 600};
    mi->rcWork = mi->rcMonitor;
  }
  return TRUE;
}
inline BOOL GetMonitorInfoA(HMONITOR h, MONITORINFO *mi) {
  return GetMonitorInfo(h, mi);
}
#endif

// ── GDI stubs (gamma ramp, device contexts) ────────────────────────────
typedef void *HDC;
inline HWND GetDesktopWindow(void) { return nullptr; }
inline HDC GetDC(HWND) { return nullptr; }
inline int ReleaseDC(HWND, HDC) { return 0; }
inline BOOL SetDeviceGammaRamp(HDC, void *) { return FALSE; }
inline BOOL GetDeviceGammaRamp(HDC, void *) { return FALSE; }

// ── GDI geometry helpers ───────────────────────────────────────────────
inline BOOL SetRect(RECT *rc, int left, int top, int right, int bottom) {
  if (!rc)
    return FALSE;
  rc->left = left;
  rc->top = top;
  rc->right = right;
  rc->bottom = bottom;
  return TRUE;
}

// ── Global memory stubs ────────────────────────────────────────────────
#ifndef GMEM_FIXED
#define GMEM_FIXED 0x0000
#endif
#ifndef GMEM_MOVEABLE
#define GMEM_MOVEABLE 0x0002
#endif
#ifndef GMEM_ZEROINIT
#define GMEM_ZEROINIT 0x0040
#endif
#ifndef GHND
#define GHND (GMEM_MOVEABLE | GMEM_ZEROINIT)
#endif
inline void *GlobalAlloc(UINT flags, size_t sz) {
  (void)flags;
  return calloc(1, sz);
}
inline void *GlobalLock(void *h) { return h; }
inline BOOL GlobalUnlock(void *) { return TRUE; }
inline void *GlobalFree(void *h) {
  free(h);
  return NULL;
}
#ifndef GlobalAllocPtr
#define GlobalAllocPtr(flags, sz) calloc(1, sz)
#endif
#ifndef GlobalFreePtr
#define GlobalFreePtr(p) free(p)
#endif

// ── GDI font/text constants ────────────────────────────────────────────
#ifndef FW_NORMAL
#define FW_NORMAL 400
#define FW_BOLD 700
#endif
#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET 1
#define ANSI_CHARSET 0
#endif
#ifndef OUT_DEFAULT_PRECIS
#define OUT_DEFAULT_PRECIS 0
#endif
#ifndef CLIP_DEFAULT_PRECIS
#define CLIP_DEFAULT_PRECIS 0
#endif
#ifndef ANTIALIASED_QUALITY
#define ANTIALIASED_QUALITY 4
#endif
#ifndef VARIABLE_PITCH
#define VARIABLE_PITCH 2
#endif
#ifndef ETO_OPAQUE
#define ETO_OPAQUE 0x0002
#endif
#ifndef DIB_RGB_COLORS
#define DIB_RGB_COLORS 0
#endif
#ifndef LF_FACESIZE
#define LF_FACESIZE 32
#endif

// ── TEXTMETRIC ─────────────────────────────────────────────────────────
#ifndef _TEXTMETRIC_DEFINED
#define _TEXTMETRIC_DEFINED
typedef struct tagTEXTMETRIC {
  LONG tmHeight;
  LONG tmAscent;
  LONG tmDescent;
  LONG tmInternalLeading;
  LONG tmExternalLeading;
  LONG tmAveCharWidth;
  LONG tmMaxCharWidth;
  LONG tmWeight;
  LONG tmOverhang;
  LONG tmDigitizedAspectX;
  LONG tmDigitizedAspectY;
  char tmFirstChar;
  char tmLastChar;
  char tmDefaultChar;
  char tmBreakChar;
  BYTE tmItalic;
  BYTE tmUnderlined;
  BYTE tmStruckOut;
  BYTE tmPitchAndFamily;
  BYTE tmCharSet;
} TEXTMETRIC, *LPTEXTMETRIC;
#endif

// ── HGDIOBJ / HFONT / HBITMAP types ────────────────────────────────────
#ifndef _GDI_TYPES_DEFINED
#define _GDI_TYPES_DEFINED
typedef void *HGDIOBJ;
typedef void *HFONT;
typedef void *HBITMAP;
#endif

// ── LARGE_INTEGER ──────────────────────────────────────────────────────
#ifndef _LARGE_INTEGER_DEFINED
#define _LARGE_INTEGER_DEFINED
typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG HighPart;
  };
  LONGLONG QuadPart;
} LARGE_INTEGER;
typedef union _ULARGE_INTEGER {
  struct {
    DWORD LowPart;
    DWORD HighPart;
  };
  ULONGLONG QuadPart;
} ULARGE_INTEGER;
#endif

// ── PALETTEENTRY ───────────────────────────────────────────────────────
#ifndef _PALETTEENTRY_DEFINED
#define _PALETTEENTRY_DEFINED
typedef struct tagPALETTEENTRY {
  BYTE peRed;
  BYTE peGreen;
  BYTE peBlue;
  BYTE peFlags;
} PALETTEENTRY, *LPPALETTEENTRY;
#endif

// ── LOGFONT (needed by D3DX Font) ──────────────────────────────────────
#ifndef _LOGFONT_DEFINED
#define _LOGFONT_DEFINED
#define LF_FACESIZE 32
typedef struct tagLOGFONTA {
  LONG lfHeight;
  LONG lfWidth;
  LONG lfEscapement;
  LONG lfOrientation;
  LONG lfWeight;
  BYTE lfItalic;
  BYTE lfUnderline;
  BYTE lfStrikeOut;
  BYTE lfCharSet;
  BYTE lfOutPrecision;
  BYTE lfClipPrecision;
  BYTE lfQuality;
  BYTE lfPitchAndFamily;
  CHAR lfFaceName[LF_FACESIZE];
} LOGFONTA, *LPLOGFONTA;
typedef LOGFONTA LOGFONT;
typedef LPLOGFONTA LPLOGFONT;
#endif

// ── Color (GDI) ────────────────────────────────────────────────────────
typedef DWORD COLORREF;
#ifndef RGB
#define RGB(r, g, b)                                                           \
  ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) |                         \
              (((DWORD)(BYTE)(b)) << 16)))
#endif
#ifndef GetRValue
#define GetRValue(rgb) ((BYTE)(rgb))
#define GetGValue(rgb) ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb) ((BYTE)((rgb) >> 16))
#endif

// ── Bitmap structures (BMP) ────────────────────────────────────────────
#ifndef BI_RGB
#define BI_RGB 0
#endif
#ifndef _BITMAPSTRUCTS_DEFINED
#define _BITMAPSTRUCTS_DEFINED
#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER {
  WORD bfType;
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER;
#pragma pack(pop)
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER;
typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;
typedef struct tagBITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColors[1];
} BITMAPINFO;
#endif

// ── GDI function stubs (after BMP/COLORREF defs) ───────────────────────
#ifndef _GDI_FUNC_STUBS_DEFINED
#define _GDI_FUNC_STUBS_DEFINED
inline HFONT CreateFont(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD,
                        DWORD, DWORD, DWORD, DWORD, const char *) {
  return NULL;
}
inline HFONT CreateFontA(int h, int w, int e, int o, int wt, DWORD i, DWORD u,
                         DWORD s, DWORD c, DWORD op, DWORD cp, DWORD q,
                         DWORD pf, const char *f) {
  return CreateFont(h, w, e, o, wt, i, u, s, c, op, cp, q, pf, f);
}
inline HGDIOBJ SelectObject(HDC, HGDIOBJ) { return NULL; }
inline BOOL DeleteObject(HGDIOBJ) { return TRUE; }
inline HDC CreateCompatibleDC(HDC) { return NULL; }
inline BOOL DeleteDC(HDC) { return TRUE; }
inline HBITMAP CreateDIBSection(HDC, const BITMAPINFO *, UINT, void **, void *,
                                DWORD) {
  return NULL;
}
inline COLORREF SetBkColor(HDC, COLORREF) { return 0; }
inline COLORREF SetTextColor(HDC, COLORREF) { return 0; }
inline BOOL ExtTextOutW(HDC, int, int, UINT, const RECT *, const wchar_t *,
                        UINT, const int *) {
  return FALSE;
}
inline BOOL GetTextExtentPoint32W(HDC, const wchar_t *, int, SIZE *) {
  return FALSE;
}
inline BOOL GetTextMetrics(HDC, LPTEXTMETRIC) { return FALSE; }
#endif

// ── Misc macros ────────────────────────────────────────────────────────
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif
#ifndef CONST
#define CONST const
#endif
#ifndef LOWORD
#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)((DWORD_PTR)(l) >> 16))
#endif
#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)((DWORD_PTR)(w) & 0xff))
#endif
#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)((DWORD_PTR)(w) >> 8))
#endif
#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#endif
#ifndef MAKEWORD
#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#endif
#ifndef DWORD_PTR
#define DWORD_PTR ULONG_PTR
#endif
#ifndef MAKEINTRESOURCE
#define MAKEINTRESOURCE(i) ((LPSTR)((ULONG_PTR)((WORD)(i))))
#endif

// ── Critical Section (stub) ────────────────────────────────────────────
typedef struct _CRITICAL_SECTION {
  void *pad[5];
} CRITICAL_SECTION, *LPCRITICAL_SECTION;

inline void InitializeCriticalSection(CRITICAL_SECTION *cs) {
  if (cs)
    memset(cs, 0, sizeof(*cs));
}
inline void DeleteCriticalSection(CRITICAL_SECTION *) {}
inline void EnterCriticalSection(CRITICAL_SECTION *) {}
inline void LeaveCriticalSection(CRITICAL_SECTION *) {}

// ── FILETIME / SYSTEMTIME ──────────────────────────────────────────────
typedef struct _FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
} FILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME, *LPSYSTEMTIME;

// ── Security (stubs) ───────────────────────────────────────────────────
typedef void *PSECURITY_DESCRIPTOR;
typedef void *LPSECURITY_ATTRIBUTES;

// ── Thread ID ──────────────────────────────────────────────────────────
// Note: on macOS, GetCurrentThreadId is provided by thread_compat.h in Utility

// ── WNDPROC (stub) ─────────────────────────────────────────────────────
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);

// ── OutputDebugString ──────────────────────────────────────────────────
#ifndef OutputDebugString
#define OutputDebugString(str) printf("%s\n", str)
#define OutputDebugStringA(str) printf("%s\n", str)
#endif

// ── Common stub functions ──────────────────────────────────────────────
#ifndef _INC_WINDOWS
#define _INC_WINDOWS
// Marker so code can check if "windows.h" was included
#endif

#endif // _WINDEF_H

// ── interface keyword (needed by d3d8.h) ───────────────────────────────
#ifndef interface
#define interface struct
#endif

// ── Multimedia / utility stubs ─────────────────────────────────────────
#ifndef _TIMEGETTIME_DEFINED
#define _TIMEGETTIME_DEFINED
#include <mach/mach_time.h>
inline DWORD timeGetTime(void) {
  static mach_timebase_info_data_t info = {0, 0};
  if (info.denom == 0)
    mach_timebase_info(&info);
  uint64_t t = mach_absolute_time();
  return (DWORD)((t * info.numer / info.denom) / 1000000ULL);
}
#endif

inline int MulDiv(int number, int numerator, int denominator) {
  if (denominator == 0)
    return -1;
  return (int)(((long long)number * numerator) / denominator);
}

// ── _stat / _S_IFDIR ──────────────────────────────────────────────────
#include <sys/stat.h>
#ifndef _stat
#define _stat stat
#endif
#ifndef _fstat
#define _fstat fstat
#endif
#ifndef _S_IFDIR
#define _S_IFDIR S_IFDIR
#endif

// ── Win32 API stubs ───────────────────────────────────────────────────
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <mach-o/dyld.h>
#include <time.h>
#include <unistd.h>

/* GetCommandLineA — on macOS we return empty string (args parsed elsewhere) */
inline const char *GetCommandLineA(void) {
  static char empty[] = "";
  return empty;
}

/* GetDoubleClickTime — return 500ms like Windows default */
inline UINT GetDoubleClickTime(void) { return 500; }

/* CreateDirectory — map to POSIX mkdir */
inline BOOL CreateDirectory(const char *path, void *) {
  if (!path)
    return FALSE;
  return (mkdir(path, 0755) == 0 || errno == EEXIST) ? TRUE : FALSE;
}
#define CreateDirectoryA CreateDirectory

/* GetModuleFileName — return path of current executable */
inline DWORD GetModuleFileName(void *hModule, char *lpFilename, DWORD nSize) {
  (void)hModule;
  if (!lpFilename || nSize == 0)
    return 0;
  char path[1024];
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) == 0) {
    char *real = realpath(path, NULL);
    if (real) {
      snprintf(lpFilename, nSize, "%s", real);
      free(real);
      return (DWORD)strlen(lpFilename);
    }
  }
  lpFilename[0] = '\0';
  return 0;
}
#define GetModuleFileNameA GetModuleFileName

/* FARPROC */
#ifndef _FARPROC_DEFINED
#define _FARPROC_DEFINED
typedef int (*FARPROC)(void);
#endif

/* __max / __min — MSVC built-in macros */
#ifndef __max
#define __max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef __min
#define __min(a, b) (((a) < (b)) ? (a) : (b))
#endif

/* GetLocalTime — fills SYSTEMTIME from current local time */
inline void GetLocalTime(SYSTEMTIME *st) {
  if (!st)
    return;
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  if (tm) {
    st->wYear = (WORD)(tm->tm_year + 1900);
    st->wMonth = (WORD)(tm->tm_mon + 1);
    st->wDayOfWeek = (WORD)tm->tm_wday;
    st->wDay = (WORD)tm->tm_mday;
    st->wHour = (WORD)tm->tm_hour;
    st->wMinute = (WORD)tm->tm_min;
    st->wSecond = (WORD)tm->tm_sec;
    st->wMilliseconds = 0;
  } else {
    memset(st, 0, sizeof(*st));
  }
}

/* _strlwr — in-place lowercase */
#ifndef _STRLWR_DEFINED
#define _STRLWR_DEFINED
#ifdef __cplusplus
extern "C" {
#endif
static inline char *_strlwr(char *s) {
  if (s)
    for (char *p = s; *p; ++p)
      *p = (char)tolower((unsigned char)*p);
  return s;
}
#ifdef __cplusplus
}
#endif
#endif

// ── Exception / Debug types ────────────────────────────────────────────
#ifndef _EXCEPTION_TYPES_DEFINED
#define _EXCEPTION_TYPES_DEFINED
typedef struct _EXCEPTION_RECORD {
  DWORD ExceptionCode;
  DWORD ExceptionFlags;
  struct _EXCEPTION_RECORD *ExceptionRecord;
  void *ExceptionAddress;
  DWORD NumberParameters;
  ULONG_PTR ExceptionInformation[15];
} EXCEPTION_RECORD, *PEXCEPTION_RECORD;

typedef struct _CONTEXT {
  DWORD ContextFlags;
  DWORD Eax, Ebx, Ecx, Edx, Esi, Edi;
  DWORD Eip, Esp, Ebp;
  DWORD EFlags;
  DWORD SegCs, SegSs, SegDs, SegEs, SegFs, SegGs;
  DWORD pad[32];
} CONTEXT, *PCONTEXT;

typedef struct _EXCEPTION_POINTERS {
  PEXCEPTION_RECORD ExceptionRecord;
  PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS, *LPEXCEPTION_POINTERS;
#endif

// ── Debug helper types (stubs) ─────────────────────────────────────────
#ifndef _DBGHELP_TYPES_DEFINED
#define _DBGHELP_TYPES_DEFINED
typedef DWORD *PDWORD;
typedef struct _ADDRESS {
  DWORD Offset;
  WORD Segment;
  int Mode;
} ADDRESS;
typedef struct _STACKFRAME {
  ADDRESS AddrPC;
  ADDRESS AddrReturn;
  ADDRESS AddrFrame;
  ADDRESS AddrStack;
  void *FuncTableEntry;
  DWORD Params[4];
  BOOL Far;
  BOOL Virtual;
  DWORD Reserved[3];
} STACKFRAME, *LPSTACKFRAME;
typedef struct _IMAGEHLP_SYMBOL {
  DWORD SizeOfStruct;
  DWORD Address;
  DWORD Size;
  char Name[256];
} IMAGEHLP_SYMBOL, *PIMAGEHLP_SYMBOL;
typedef struct _IMAGEHLP_LINE {
  DWORD SizeOfStruct;
  DWORD LineNumber;
  char *FileName;
  DWORD Address;
} IMAGEHLP_LINE, *PIMAGEHLP_LINE;
typedef int (*PREAD_PROCESS_MEMORY_ROUTINE)(void *, void *, void *, DWORD,
                                            DWORD *);
typedef void *(*PFUNCTION_TABLE_ACCESS_ROUTINE)(void *, DWORD);
typedef DWORD (*PGET_MODULE_BASE_ROUTINE)(void *, DWORD);
typedef DWORD (*PTRANSLATE_ADDRESS_ROUTINE)(void *, void *, void *);
typedef DWORD MINIDUMP_TYPE;
typedef void *PMINIDUMP_EXCEPTION_INFORMATION;
typedef void *PMINIDUMP_USER_STREAM_INFORMATION;
typedef void *PMINIDUMP_CALLBACK_INFORMATION;
#endif

// ── OS Version ─────────────────────────────────────────────────────────
#ifndef _OSVERSIONINFO_DEFINED
#define _OSVERSIONINFO_DEFINED
#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT 2
typedef struct _OSVERSIONINFO {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  char szCSDVersion[128];
} OSVERSIONINFO, *LPOSVERSIONINFO;

inline BOOL GetVersionEx(LPOSVERSIONINFO lpVersionInfo) {
  if (!lpVersionInfo)
    return FALSE;
  lpVersionInfo->dwMajorVersion = 10;
  lpVersionInfo->dwMinorVersion = 0;
  lpVersionInfo->dwBuildNumber = 0;
  lpVersionInfo->dwPlatformId = VER_PLATFORM_WIN32_NT;
  lpVersionInfo->szCSDVersion[0] = '\0';
  return TRUE;
}
#define GetVersionExA GetVersionEx
#endif

// ── File handling stubs ────────────────────────────────────────────────
#ifndef _WIN32_FIND_DATA_DEFINED
#define _WIN32_FIND_DATA_DEFINED
typedef struct _WIN32_FIND_DATA {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD nFileSizeHigh;
  DWORD nFileSizeLow;
  char cFileName[260];
  char cAlternateFileName[14];
} WIN32_FIND_DATA, *LPWIN32_FIND_DATA;
#define WIN32_FIND_DATAA WIN32_FIND_DATA

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR) - 1)
#define FILE_ATTRIBUTE_DIRECTORY 0x10

inline HANDLE FindFirstFile(const char *, WIN32_FIND_DATA *) {
  return INVALID_HANDLE_VALUE;
}
#define FindFirstFileA FindFirstFile
inline BOOL FindNextFile(HANDLE, WIN32_FIND_DATA *) { return FALSE; }
#define FindNextFileA FindNextFile
inline BOOL FindClose(HANDLE) { return TRUE; }
#endif

inline BOOL CopyFile(const char *src, const char *dst, BOOL failIfExists) {
  (void)failIfExists;
  FILE *in = fopen(src, "rb");
  if (!in)
    return FALSE;
  FILE *out = fopen(dst, "wb");
  if (!out) {
    fclose(in);
    return FALSE;
  }
  char buf[4096];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
    fwrite(buf, 1, n, out);
  fclose(in);
  fclose(out);
  return TRUE;
}
#define CopyFileA CopyFile

inline BOOL SetCurrentDirectory(const char *path) {
  return chdir(path) == 0 ? TRUE : FALSE;
}
#define SetCurrentDirectoryA SetCurrentDirectory

// ── Locale / Date formatting ───────────────────────────────────────────
#define LOCALE_USER_DEFAULT 0x0400
#define DATE_SHORTDATE 0x00000001
#define TIME_NOSECONDS 0x00000002

inline int GetDateFormat(DWORD, DWORD, const SYSTEMTIME *st, const char *,
                         char *buf, int size) {
  if (!buf || size <= 0)
    return 0;
  if (st) {
    return snprintf(buf, size, "%04u-%02u-%02u", st->wYear, st->wMonth,
                    st->wDay) +
           1;
  }
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  return snprintf(buf, size, "%04d-%02d-%02d", tm->tm_year + 1900,
                  tm->tm_mon + 1, tm->tm_mday) +
         1;
}
#define GetDateFormatA GetDateFormat

inline int GetTimeFormat(DWORD, DWORD, const SYSTEMTIME *st, const char *,
                         char *buf, int size) {
  if (!buf || size <= 0)
    return 0;
  if (st) {
    return snprintf(buf, size, "%02u:%02u:%02u", st->wHour, st->wMinute,
                    st->wSecond) +
           1;
  }
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  return snprintf(buf, size, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min,
                  tm->tm_sec) +
         1;
}
#define GetTimeFormatA GetTimeFormat

// ── MessageBox ─────────────────────────────────────────────────────────
#define MB_OK 0x00000000
#define MB_OKCANCEL 0x00000001
#define MB_YESNO 0x00000004
#define MB_YESNOCANCEL 0x00000003
#define MB_ICONERROR 0x00000010
#define MB_ICONWARNING 0x00000030
#define MB_ICONINFORMATION 0x00000040
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

inline int MessageBox(HWND, const char *, const char *, UINT) { return IDOK; }
#define MessageBoxA MessageBox
inline int MessageBoxW(HWND, const wchar_t *, const wchar_t *, UINT) {
  return IDOK;
}

// ── SetWindowText ──────────────────────────────────────────────────────
inline BOOL SetWindowText(HWND, const char *) { return TRUE; }
#define SetWindowTextA SetWindowText
inline BOOL SetWindowTextW(HWND, const wchar_t *) { return TRUE; }

// ── MoveFile ───────────────────────────────────────────────────────────
inline BOOL MoveFile(const char *src, const char *dst) {
  return rename(src, dst) == 0 ? TRUE : FALSE;
}
#define MoveFileA MoveFile

// ── DeleteFile ─────────────────────────────────────────────────────────
inline BOOL DeleteFile(const char *path) {
  return remove(path) == 0 ? TRUE : FALSE;
}
#define DeleteFileA DeleteFile

// ── Process helpers ────────────────────────────────────────────────────
#include <pthread.h>
#include <sys/types.h>
inline HANDLE GetCurrentProcess(void) { return (HANDLE)(uintptr_t)getpid(); }
inline HANDLE GetCurrentThread(void) {
  return (HANDLE)(uintptr_t)pthread_self();
}
inline DWORD GetCurrentProcessId(void) { return (DWORD)getpid(); }

// ── _access ────────────────────────────────────────────────────────────
#ifndef _access
#define _access access
#endif

// ── _splitpath ─────────────────────────────────────────────────────────
inline void _splitpath(const char *path, char *drive, char *dir, char *fname,
                       char *ext) {
  if (drive)
    drive[0] = '\0';
  if (dir) {
    const char *slash = strrchr(path, '/');
    if (!slash)
      slash = strrchr(path, '\\');
    if (slash) {
      size_t len = slash - path + 1;
      memcpy(dir, path, len);
      dir[len] = '\0';
    } else {
      dir[0] = '\0';
    }
  }
  const char *base = strrchr(path, '/');
  if (!base)
    base = strrchr(path, '\\');
  base = base ? base + 1 : path;
  const char *dot = strrchr(base, '.');
  if (fname) {
    if (dot) {
      size_t len = dot - base;
      memcpy(fname, base, len);
      fname[len] = '\0';
    } else
      strcpy(fname, base);
  }
  if (ext) {
    if (dot)
      strcpy(ext, dot);
    else
      ext[0] = '\0';
  }
}

// ── Wide-char formatting ───────────────────────────────────────────────
#define TIME_FORCE24HOURFORMAT 0x00000008
#define TIME_NOTIMEMARKER 0x00000004

inline int GetDateFormatW(DWORD, DWORD, const SYSTEMTIME *st, const wchar_t *,
                          wchar_t *buf, int size) {
  if (!buf || size <= 0)
    return 0;
  char tmp[64];
  if (st)
    snprintf(tmp, sizeof(tmp), "%04u-%02u-%02u", st->wYear, st->wMonth,
             st->wDay);
  else {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(tmp, sizeof(tmp), "%04d-%02d-%02d", tm->tm_year + 1900,
             tm->tm_mon + 1, tm->tm_mday);
  }
  for (int i = 0; tmp[i] && i < size - 1; i++)
    buf[i] = (wchar_t)tmp[i];
  buf[strlen(tmp) < (size_t)size ? strlen(tmp) : size - 1] = L'\0';
  return (int)strlen(tmp) + 1;
}

inline int GetTimeFormatW(DWORD, DWORD, const SYSTEMTIME *st, const wchar_t *,
                          wchar_t *buf, int size) {
  if (!buf || size <= 0)
    return 0;
  char tmp[64];
  if (st)
    snprintf(tmp, sizeof(tmp), "%02u:%02u:%02u", st->wHour, st->wMinute,
             st->wSecond);
  else {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(tmp, sizeof(tmp), "%02d:%02d:%02d", tm->tm_hour, tm->tm_min,
             tm->tm_sec);
  }
  for (int i = 0; tmp[i] && i < size - 1; i++)
    buf[i] = (wchar_t)tmp[i];
  buf[strlen(tmp) < (size_t)size ? strlen(tmp) : size - 1] = L'\0';
  return (int)strlen(tmp) + 1;
}

// ── Debug symbols ──────────────────────────────────────────────────────
#ifndef SYMOPT_DEFERRED_LOADS
#define SYMOPT_DEFERRED_LOADS 0x00000004
#define SYMOPT_UNDNAME 0x00000002
#define SYMOPT_LOAD_LINES 0x00000010
#define SYMOPT_OMAP_FIND_NEAREST 0x00000020
#define CONTEXT_FULL 0x10000B
#define AddrModeFlat 3
#define IMAGE_FILE_MACHINE_I386 0x014c
#endif

// ── Registry extras ────────────────────────────────────────────────────
#ifndef REG_OPTION_NON_VOLATILE
#define REG_OPTION_NON_VOLATILE 0x00000000
#endif

#endif // _WINDOWS_H_MACOS_SHIM_
