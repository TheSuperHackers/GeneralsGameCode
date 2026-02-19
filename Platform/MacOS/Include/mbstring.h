/* macOS shim: mbstring.h — Multibyte string stubs */
#pragma once

#include <cctype>
#include <cstring>

/* _ismbblead: On macOS with UTF-8, no single byte is a MBCS lead byte in
   the Windows DBCS sense, so always return 0. */
inline int _ismbblead(unsigned int c) { return 0; }

/* _mbsstr: equivalent to strstr for single-byte locales */
inline unsigned char *_mbsstr(const unsigned char *str,
                              const unsigned char *substr) {
  return (unsigned char *)strstr((const char *)str, (const char *)substr);
}
