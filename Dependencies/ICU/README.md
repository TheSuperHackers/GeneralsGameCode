# ICU integration

`core_icu` provides UTF conversions and the ICU headers available to the selected toolchain.
Include `ICU/utf8.h` for conversions and `ICU/IcuSupport.h` for linked ICU APIs.

| Build configuration | ICU access | Fallback |
| --- | --- | --- |
| Full ICU package, including Windows | Linked ICU C APIs and available C++ APIs | None needed |
| Windows SDK ICU | `IcuLoader` resolves the conversion exports; other C APIs remain available through delay imports | Win32 `CP_UTF8` when the DLL or exports are unavailable |
| VC6 and Windows builds without an import library | `IcuLoader` loads `icu.dll` and resolves `u_strFromUTF8` and `u_strToUTF8WithSub` | Win32 `CP_UTF8` when the DLL or exports are unavailable |

VC6 does use ICU when these exports are available. It does not compile against modern ICU headers or expose the full ICU C++ API.

`IcuLoader` loads on the first conversion and caches both successful and failed attempts. Later calls reuse the result, including in WorldBuilder and other tools without an enclosing application scope. `unload()` releases the DLL, clears the exports, and permits a later call to retry. Both games explicitly unload after engine teardown; tools also have cleanup at normal module shutdown.

Loading, conversion calls, and unloading use a Windows `CRITICAL_SECTION`, the same primitive used by the engine's critical-section classes. It is available on VC6 and does not require a dependency on WWLib. Calls are serialized, and unload waits for any active conversion before freeing the DLL. Export pointers remain private to the loader. Conversion workers must stop before static destruction begins.

The critical section initializes on first use, including conversions triggered by startup logging before global constructors have run. An interlocked initialization guard also supports concurrent first calls on VC6; conversion calls use the native critical section. Cleanup is registered when the lock is initialized.

Conversions in Windows SDK builds use the resolved function pointers, avoiding an extra delay-loader reference. If a caller uses other SDK ICU APIs directly, the SDK delay loader owns its reference independently.

The loader tries absolute paths in the executable directory and then the Windows system directory. It supports app-local ICU and Unicode installation paths without searching the working directory or `PATH`. A missing DLL or conversion export selects the Win32 fallback; a DLL with missing exports is released immediately.
