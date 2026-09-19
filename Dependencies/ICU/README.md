# ICU integration

`core_icu` provides UTF conversions and the ICU headers available to the selected toolchain.
Include `ICU/utf8.h` for conversions and `ICU/IcuSupport.h` for linked ICU APIs.

| Build configuration | ICU access | Fallback |
| --- | --- | --- |
| Full ICU package, including Windows | Linked ICU C APIs and available C++ APIs | None needed |
| Windows SDK ICU | `IcuLoader` resolves the conversion exports; other C APIs remain available through delay imports | Win32 `CP_UTF8` when the DLL or exports are unavailable |
| VC6 and Windows builds without an import library | `IcuLoader` loads `icu.dll` and resolves `u_strFromUTF8` and `u_strToUTF8WithSub` | Win32 `CP_UTF8` when the DLL or exports are unavailable |

VC6 does use ICU when these exports are available. It does not compile against modern ICU headers or expose the full ICU C++ API.

`IcuLoader` has paired `load()` and `unload()` calls, following `BinkLoader` and `MilesLoader`. Every load needs an unload, including failed loads. Overlapping callers share the result, and the last unload releases the DLL and clears the function pointers. A later load can retry. Reference changes are synchronized, including on VC6; callers must retain a reference while using the resolved functions.

`IcuScope` pairs these calls automatically. Each conversion holds a scope so another thread cannot unload its functions while they are running. Both games retain an additional scope through engine teardown to avoid repeated loading. Tools can retain a scope around batches of conversions too. Full linked ICU does not need explicit ownership.

Conversions in Windows SDK builds use the resolved function pointers, avoiding an extra delay-loader reference. If a caller uses other SDK ICU APIs directly, the SDK delay loader owns its reference independently.

The loader tries absolute paths in the executable directory and then the Windows system directory. It supports app-local ICU and Unicode installation paths without searching the working directory or `PATH`. A missing DLL or conversion export selects the Win32 fallback; a DLL with missing exports is released immediately.
