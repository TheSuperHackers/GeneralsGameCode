/* macOS shim: wininet.h — Windows Internet API stubs */
#pragma once

#define INTERNET_OPEN_TYPE_DIRECT 1
#define INTERNET_FLAG_RELOAD 0x80000000

typedef void *HINTERNET;

inline HINTERNET InternetOpen(const char *, int, const char *, const char *,
                              int) {
  return NULL;
}
inline HINTERNET InternetOpenUrl(HINTERNET, const char *, const char *, int,
                                 int, int) {
  return NULL;
}
inline int InternetReadFile(HINTERNET, void *, int, int *) { return 0; }
inline int InternetCloseHandle(HINTERNET) { return 0; }
