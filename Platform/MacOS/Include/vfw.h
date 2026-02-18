/* macOS shim: vfw.h — Video for Windows stub types */
#pragma once
#include "windows.h"

/* AVI opaque handles */
typedef void *PAVIFILE;
typedef void *PAVISTREAM;

/* mmioFOURCC */
#ifndef mmioFOURCC
#define mmioFOURCC(c0, c1, c2, c3)                                             \
  ((DWORD)(BYTE)(c0) | ((DWORD)(BYTE)(c1) << 8) | ((DWORD)(BYTE)(c2) << 16) |  \
   ((DWORD)(BYTE)(c3) << 24))
#endif
#ifndef streamtypeVIDEO
#define streamtypeVIDEO mmioFOURCC('v', 'i', 'd', 's')
#endif

/* AVI stream info */
typedef struct {
  DWORD fccType;
  DWORD fccHandler;
  DWORD dwFlags;
  DWORD dwCaps;
  WORD wPriority;
  WORD wLanguage;
  DWORD dwScale;
  DWORD dwRate;
  DWORD dwStart;
  DWORD dwLength;
  DWORD dwInitialFrames;
  DWORD dwSuggestedBufferSize;
  DWORD dwQuality;
  DWORD dwSampleSize;
  RECT rcFrame;
  DWORD dwEditCount;
  DWORD dwFormatChangeCount;
  char szName[64];
} AVISTREAMINFO, *LPAVISTREAMINFO;

/* Constants */
#ifndef OF_WRITE
#define OF_WRITE 0x0001
#endif
#ifndef OF_CREATE
#define OF_CREATE 0x1000
#endif
#ifndef AVIIF_KEYFRAME
#define AVIIF_KEYFRAME 0x00000010
#endif
#ifndef GMEM_MOVEABLE
#define GMEM_MOVEABLE 0x0002
#endif

/* Global memory stubs */
#ifndef GlobalFreePtr
#define GlobalFreePtr(p) free(p)
#endif
#include <stdlib.h>

/* AVI function stubs (no-ops on macOS) */
inline void AVIFileInit(void) {}
inline void AVIFileExit(void) {}
inline HRESULT AVIFileOpen(PAVIFILE *, const char *, UINT, void *) {
  return E_FAIL;
}
inline HRESULT AVIFileCreateStream(PAVIFILE, PAVISTREAM *, LPAVISTREAMINFO) {
  return E_FAIL;
}
inline HRESULT AVIStreamSetFormat(PAVISTREAM, LONG, void *, LONG) {
  return E_FAIL;
}
inline HRESULT AVIStreamWrite(PAVISTREAM, LONG, LONG, void *, LONG, DWORD,
                              LONG *, LONG *) {
  return E_FAIL;
}
inline ULONG AVIStreamRelease(PAVISTREAM) { return 0; }
inline ULONG AVIFileRelease(PAVIFILE) { return 0; }
