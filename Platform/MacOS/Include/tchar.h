/* macOS shim: tchar.h — TCHAR mappings (always ANSI on macOS) */
#pragma once

#ifndef _TCHAR_H_MACOS_SHIM_
#define _TCHAR_H_MACOS_SHIM_

#include <stdio.h>
#include <string.h>

#ifndef _TCHAR_DEFINED
typedef char TCHAR;
#define _TCHAR_DEFINED
#endif

#define _T(x) x
#define _TEXT(x) x

/* String functions mapped to ANSI equivalents */
#define _tcscpy strcpy
#define _tcsncpy strncpy
#define _tcslen strlen
#define _tcscmp strcmp
#define _tcsicmp strcasecmp
#define _tcsncicmp strncasecmp
#define _tcscat strcat
#define _tcschr strchr
#define _tcsrchr strrchr
#define _tcsstr strstr
#define _tcstok strtok
#define _tcstol strtol
#define _tcstoul strtoul
#define _tcstod strtod
#define _tcsdup strdup

/* Formatted I/O */
#define _stprintf sprintf
#define _sntprintf snprintf
#define _vstprintf vsprintf
#define _vsntprintf vsnprintf
#define _tprintf printf
#define _ftprintf fprintf
#define _tscanf scanf

/* File I/O */
#define _tfopen fopen
#define _tgetenv getenv

/* Conversion */
#define _ttoi atoi
#define _tstoi atoi
#define _ttol atol

#endif /* _TCHAR_H_MACOS_SHIM_ */
