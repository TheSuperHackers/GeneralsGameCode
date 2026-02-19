/* macOS shim: dsound.h — DirectSound stubs */
#pragma once

#include "windows.h"

/* DirectSound return codes */
#define DS_OK                 S_OK
#define DSERR_GENERIC         E_FAIL
#define DSERR_OUTOFMEMORY     E_OUTOFMEMORY
#define DSERR_INVALIDPARAM    E_INVALIDARG

/* Cooperative level constants */
#define DSSCL_NORMAL          0x00000001
#define DSSCL_PRIORITY        0x00000002
#define DSSCL_EXCLUSIVE       0x00000003
#define DSSCL_WRITEPRIMARY    0x00000004

/* Buffer flags */
#define DSBCAPS_PRIMARYBUFFER 0x00000001
#define DSBCAPS_STATIC        0x00000002
#define DSBCAPS_LOCHARDWARE   0x00000004
#define DSBCAPS_LOCSOFTWARE   0x00000008
#define DSBCAPS_CTRL3D        0x00000010
#define DSBCAPS_CTRLFREQUENCY 0x00000020
#define DSBCAPS_CTRLPAN       0x00000040
#define DSBCAPS_CTRLVOLUME    0x00000080
#define DSBCAPS_CTRLPOSITIONNOTIFY 0x00000100
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000

/* Buffer play flags */
#define DSBPLAY_LOOPING       0x00000001

/* 3D algorithm modes */
#define DS3DALG_DEFAULT       GUID()
#define DS3DALG_NO_VIRTUALIZATION GUID()

/* Speaker configuration constants */
#define DSSPEAKER_DIRECTOUT   0x00000000
#define DSSPEAKER_HEADPHONE   0x00000001
#define DSSPEAKER_MONO        0x00000002
#define DSSPEAKER_QUAD        0x00000003
#define DSSPEAKER_STEREO      0x00000004
#define DSSPEAKER_SURROUND    0x00000005
#define DSSPEAKER_5POINT1     0x00000006
#define DSSPEAKER_7POINT1     0x00000007

/* Speaker config extraction macro */
#define DSSPEAKER_CONFIG(x) ((BYTE)(x))

/* Wave format tag */
#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM       1
#endif

/* WAVEFORMATEX */
#ifndef _WAVEFORMATEX_DEFINED
#define _WAVEFORMATEX_DEFINED
typedef struct tWAVEFORMATEX {
  WORD  wFormatTag;
  WORD  nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD  nBlockAlign;
  WORD  wBitsPerSample;
  WORD  cbSize;
} WAVEFORMATEX, *LPWAVEFORMATEX;
typedef const WAVEFORMATEX *LPCWAVEFORMATEX;
#endif

/* Buffer description */
typedef struct _DSBUFFERDESC {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwBufferBytes;
  DWORD dwReserved;
  LPWAVEFORMATEX lpwfxFormat;
  GUID  guid3DAlgorithm;
} DSBUFFERDESC, *LPDSBUFFERDESC;
typedef const DSBUFFERDESC *LPCDSBUFFERDESC;

/* Capabilities */
typedef struct _DSCAPS {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwMinSecondarySampleRate;
  DWORD dwMaxSecondarySampleRate;
  DWORD dwPrimaryBuffers;
  DWORD dwMaxHwMixingAllBuffers;
  DWORD dwMaxHwMixingStaticBuffers;
  DWORD dwMaxHwMixingStreamingBuffers;
  DWORD dwFreeHwMixingAllBuffers;
  DWORD dwFreeHwMixingStaticBuffers;
  DWORD dwFreeHwMixingStreamingBuffers;
  DWORD dwMaxHw3DAllBuffers;
  DWORD dwMaxHw3DStaticBuffers;
  DWORD dwMaxHw3DStreamingBuffers;
  DWORD dwFreeHw3DAllBuffers;
  DWORD dwFreeHw3DStaticBuffers;
  DWORD dwFreeHw3DStreamingBuffers;
  DWORD dwTotalHwMemBytes;
  DWORD dwFreeHwMemBytes;
  DWORD dwMaxContigFreeHwMemBytes;
  DWORD dwUnlockTransferRateHwBuffers;
  DWORD dwPlayCpuOverheadSwBuffers;
} DSCAPS, *LPDSCAPS;

/* IDirectSound interface (minimal stub) */
struct IDirectSound {
  virtual HRESULT GetSpeakerConfig(DWORD *pdwSpeakerConfig) {
    if (pdwSpeakerConfig) *pdwSpeakerConfig = DSSPEAKER_STEREO;
    return DS_OK;
  }
  virtual HRESULT GetCaps(LPDSCAPS) { return DS_OK; }
  virtual HRESULT SetCooperativeLevel(HWND, DWORD) { return DS_OK; }
  virtual ~IDirectSound() {}
};
typedef IDirectSound *LPDIRECTSOUND;

/* Opaque interface pointers */
typedef void *LPDIRECTSOUNDBUFFER;
typedef void *LPDIRECTSOUND3DLISTENER;
typedef void *LPDIRECTSOUND3DBUFFER;

/* DirectSoundCreate stub */
inline HRESULT DirectSoundCreate(const GUID *, LPDIRECTSOUND *, void *) {
  return E_NOTIMPL;
}
