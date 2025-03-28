#pragma once

// WCHAR
typedef wchar_t WCHAR;
typedef const WCHAR* LPCWSTR;
typedef WCHAR* LPWSTR;

#define _wcsicmp wcscasecmp
#define wcsicmp wcscasecmp
#define _vsnwprintf vswprintf

// MultiByteToWideChar
#define CP_ACP 0
#define MultiByteToWideChar(cp, flags, mbstr, cb, wcstr, cch) mbstowcs(wcstr, mbstr, cch)
#define WideCharToMultiByte(cp, flags, wcstr, cch, mbstr, cb, defchar, used) wcstombs(mbstr, wcstr, cb)