#pragma once

typedef const char* LPCSTR;
typedef char* LPSTR;

// String functions
inline char *_strlwr(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] = tolower(str[i]);
  }
  return str;
}

#define strlwr _strlwr
#define _vsnprintf vsnprintf
#define _snprintf snprintf
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define strcmpi strcasecmp