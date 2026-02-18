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

// Win32 string functions
#ifndef lstrcpyn
#define lstrcpyn(dst, src, n) strncpy(dst, src, n)
#endif
#ifndef lstrcat
#define lstrcat(dst, src) strcat(dst, src)
#endif
#ifndef lstrcpy
#define lstrcpy(dst, src) strcpy(dst, src)
#endif
#ifndef lstrcmpi
#define lstrcmpi(a, b) strcasecmp(a, b)
#endif
#ifndef lstrlen
#define lstrlen(s) ((int)strlen(s))
#endif
#ifndef _isnan
#define _isnan(x) isnan(x)
#endif

// strupr — in-place uppercase (POSIX doesn't have it, not in string_compat.h)
#include <ctype.h>
#ifndef _STRUPR_DEFINED
#define _STRUPR_DEFINED
inline char *strupr(char *s) {
  for (char *p = s; *p; ++p)
    *p = (char)toupper((unsigned char)*p);
  return s;
}
inline char *_strupr(char *s) { return strupr(s); }
#endif
// strlwr is provided by Dependencies/Utility/string_compat.h

// Minimal filesystem stubs (return "not found" / empty)
#ifndef GetCurrentDirectory
#define GetCurrentDirectory(n, buf) ((buf)[0] = '\0', 0)
#endif
#ifndef GetFileAttributes
#define INVALID_FILE_ATTRIBUTES ((DWORD) - 1)
#define GetFileAttributes(path) INVALID_FILE_ATTRIBUTES
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

#endif // _WINDOWS_H_MACOS_SHIM_
