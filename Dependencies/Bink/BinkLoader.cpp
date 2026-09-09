/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "BinkLoader.h"
#include "bink.h"

// binkw32.dll exports decorated __stdcall names, which only exist in the 32 bit Windows build.
#if defined(_WIN32) && !defined(_WIN64)
#define BINK_LOADER_SUPPORTED 1
#else
#define BINK_LOADER_SUPPORTED 0
#endif

#if BINK_LOADER_SUPPORTED
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif


namespace
{

typedef HBINK (__stdcall *BinkOpen_t)(const char *name, unsigned int flags);
typedef void (__stdcall *BinkSetSoundTrack_t)(unsigned int total_tracks, unsigned int *tracks);
typedef int (__stdcall *BinkSetSoundSystem_t)(SndOpenCallback open, unsigned long param);
typedef void *(__stdcall *BinkOpenDirectSound_t)(unsigned long param);
typedef void (__stdcall *BinkClose_t)(HBINK handle);
typedef int (__stdcall *BinkWait_t)(HBINK handle);
typedef int (__stdcall *BinkDoFrame_t)(HBINK handle);
typedef int (__stdcall *BinkCopyToBuffer_t)(HBINK handle, void *dest, int destpitch, unsigned int destheight,
	unsigned int destx, unsigned int desty, unsigned int flags);
typedef void (__stdcall *BinkSetVolume_t)(HBINK handle, unsigned int trackid, int volume);
typedef void (__stdcall *BinkNextFrame_t)(HBINK handle);
typedef void (__stdcall *BinkGoto_t)(HBINK handle, unsigned int frame, int flags);

BinkOpen_t BinkOpenPtr = nullptr;
BinkSetSoundTrack_t BinkSetSoundTrackPtr = nullptr;
BinkSetSoundSystem_t BinkSetSoundSystemPtr = nullptr;
BinkOpenDirectSound_t BinkOpenDirectSoundPtr = nullptr;
BinkClose_t BinkClosePtr = nullptr;
BinkWait_t BinkWaitPtr = nullptr;
BinkDoFrame_t BinkDoFramePtr = nullptr;
BinkCopyToBuffer_t BinkCopyToBufferPtr = nullptr;
BinkSetVolume_t BinkSetVolumePtr = nullptr;
BinkNextFrame_t BinkNextFramePtr = nullptr;
BinkGoto_t BinkGotoPtr = nullptr;

int ReferenceCount = 0;
bool Failed = false;
unsigned long LastError = 0;

#if BINK_LOADER_SUPPORTED
HMODULE Module = HMODULE(nullptr);

// Resolves one export into its matching function pointer. The exported names are decorated,
// so they carry the __stdcall argument byte count and must be spelled out verbatim.
#define BINK_RESOLVE(name, decorated) \
	name##Ptr = reinterpret_cast<name##_t>(::GetProcAddress(Module, decorated))

static void resolveAll()
{
	BINK_RESOLVE(BinkOpen, "_BinkOpen@8");
	BINK_RESOLVE(BinkSetSoundTrack, "_BinkSetSoundTrack@8");
	BINK_RESOLVE(BinkSetSoundSystem, "_BinkSetSoundSystem@8");
	BINK_RESOLVE(BinkOpenDirectSound, "_BinkOpenDirectSound@4");
	BINK_RESOLVE(BinkClose, "_BinkClose@4");
	BINK_RESOLVE(BinkWait, "_BinkWait@4");
	BINK_RESOLVE(BinkDoFrame, "_BinkDoFrame@4");
	BINK_RESOLVE(BinkCopyToBuffer, "_BinkCopyToBuffer@28");
	BINK_RESOLVE(BinkSetVolume, "_BinkSetVolume@12");
	BINK_RESOLVE(BinkNextFrame, "_BinkNextFrame@4");
	BINK_RESOLVE(BinkGoto, "_BinkGoto@12");
}
#undef BINK_RESOLVE

static void freeResources()
{
	if (Module != HMODULE(nullptr)) {
		::FreeLibrary(Module);
		Module = HMODULE(nullptr);
	}

	BinkOpenPtr = nullptr;
	BinkSetSoundTrackPtr = nullptr;
	BinkSetSoundSystemPtr = nullptr;
	BinkOpenDirectSoundPtr = nullptr;
	BinkClosePtr = nullptr;
	BinkWaitPtr = nullptr;
	BinkDoFramePtr = nullptr;
	BinkCopyToBufferPtr = nullptr;
	BinkSetVolumePtr = nullptr;
	BinkNextFramePtr = nullptr;
	BinkGotoPtr = nullptr;
}
#endif // BINK_LOADER_SUPPORTED

} // namespace


bool BinkLoader::isLoaded()
{
#if BINK_LOADER_SUPPORTED
	return Module != HMODULE(nullptr);
#else
	return false;
#endif
}


bool BinkLoader::isFailed()
{
	return Failed;
}


unsigned long BinkLoader::getLastError()
{
	return LastError;
}


bool BinkLoader::load()
{
	// Always increment the reference count.
	++ReferenceCount;

	// Optimization: return early if it failed before.
	if (Failed)
		return false;

	// Return early if someone else already loaded it.
	if (ReferenceCount > 1)
		return true;

#if BINK_LOADER_SUPPORTED
	// Load binkw32.dll by name, so that the usual module search order applies.
	Module = ::LoadLibraryA("binkw32.dll");
	if (Module == HMODULE(nullptr)) {
		LastError = ::GetLastError();
		Failed = true;
		return false;
	}

	resolveAll();
	return true;
#else
	Failed = true;
	return false;
#endif
}


void BinkLoader::unload()
{
	if (ReferenceCount > 0)
		--ReferenceCount;

	if (ReferenceCount > 0)
		return;

#if BINK_LOADER_SUPPORTED
	freeResources();
#endif
	Failed = false;
	LastError = 0;
}


// The Bink functions below stand in for the imports of binkw32.dll. An unresolved function
// returns the same neutral value the Bink SDK stub library returned.

HBINK __stdcall BinkOpen(const char *name, unsigned int flags)
{
	return BinkOpenPtr != nullptr ? BinkOpenPtr(name, flags) : nullptr;
}

void __stdcall BinkSetSoundTrack(unsigned int total_tracks, unsigned int *tracks)
{
	if (BinkSetSoundTrackPtr != nullptr)
		BinkSetSoundTrackPtr(total_tracks, tracks);
}

int __stdcall BinkSetSoundSystem(SndOpenCallback open, unsigned long param)
{
	return BinkSetSoundSystemPtr != nullptr ? BinkSetSoundSystemPtr(open, param) : 0;
}

void *__stdcall BinkOpenDirectSound(unsigned long param)
{
	return BinkOpenDirectSoundPtr != nullptr ? BinkOpenDirectSoundPtr(param) : nullptr;
}

void __stdcall BinkClose(HBINK handle)
{
	if (BinkClosePtr != nullptr)
		BinkClosePtr(handle);
}

int __stdcall BinkWait(HBINK handle)
{
	return BinkWaitPtr != nullptr ? BinkWaitPtr(handle) : 0;
}

int __stdcall BinkDoFrame(HBINK handle)
{
	return BinkDoFramePtr != nullptr ? BinkDoFramePtr(handle) : 0;
}

int __stdcall BinkCopyToBuffer(HBINK handle, void *dest, int destpitch, unsigned int destheight,
	unsigned int destx, unsigned int desty, unsigned int flags)
{
	return BinkCopyToBufferPtr != nullptr
		? BinkCopyToBufferPtr(handle, dest, destpitch, destheight, destx, desty, flags)
		: 0;
}

void __stdcall BinkSetVolume(HBINK handle, unsigned int trackid, int volume)
{
	if (BinkSetVolumePtr != nullptr)
		BinkSetVolumePtr(handle, trackid, volume);
}

void __stdcall BinkNextFrame(HBINK handle)
{
	if (BinkNextFramePtr != nullptr)
		BinkNextFramePtr(handle);
}

void __stdcall BinkGoto(HBINK handle, unsigned int frame, int flags)
{
	if (BinkGotoPtr != nullptr)
		BinkGotoPtr(handle, frame, flags);
}
