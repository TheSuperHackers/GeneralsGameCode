#pragma once

typedef const char* LPCSTR;
typedef char* LPSTR;

// String functions
#define _vsnprintf vsnprintf
#define _snprintf snprintf
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define strcmpi strcasecmp