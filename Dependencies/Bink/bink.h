/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Subset of the binkw32.dll API as used by the W3D engine.
 *
 * @copyright This is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#pragma once

#if !defined _MSC_VER
#if !defined(__stdcall)
#if defined __has_attribute && __has_attribute(stdcall)
#define __stdcall __attribute__((stdcall))
#else
#define __stdcall
#endif
#endif /* !defined __stdcall */
#endif /* !defined COMPILER_MSVC */

#ifdef __cplusplus
extern "C" {
#endif

#define BINKSURFACE24 1
#define BINKSURFACE32 3
#define BINKSURFACE555 9
#define BINKSURFACE565 10
#define BINKPRELOADALL 0x00002000
#define BINKCOPYNOSCALING 0x70000000

typedef struct BINK
{
    unsigned int Width;
    unsigned int Height;
    unsigned int Frames;
    unsigned int FrameNum;
    unsigned int LastFrameNum;
    unsigned int FrameRate;
    unsigned int FrameRateDiv;
    /* Original struct has more members, but we only need these to match the ABI*/
} BINK, *HBINK;

typedef void *(__stdcall *SndOpenCallback)(unsigned long param);
typedef unsigned int u32;

HBINK __stdcall BinkOpen(const char *name, unsigned int flags);
void __stdcall BinkSetSoundTrack(unsigned int total_tracks, unsigned int *tracks);
int __stdcall BinkSetSoundSystem(SndOpenCallback open, unsigned long param);
void *__stdcall BinkOpenDirectSound(unsigned long param);
void __stdcall BinkClose(HBINK handle);
int __stdcall BinkWait(HBINK handle);
int __stdcall BinkDoFrame(HBINK handle);
int __stdcall BinkCopyToBuffer(
    HBINK handle, void *dest, int destpitch, unsigned int destheight, unsigned int destx, unsigned int desty, unsigned int flags);
void __stdcall BinkSetVolume(HBINK handle, unsigned int trackid, int volume);
void __stdcall BinkNextFrame(HBINK handle);
void __stdcall BinkGoto(HBINK handle, unsigned int frame, int flags);

#define BinkSoundUseDirectSound(x) BinkSetSoundSystem(BinkOpenDirectSound, (unsigned long)x)

#ifdef __cplusplus
} // extern "C"
#endif
