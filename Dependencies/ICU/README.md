# ICU integration

`core_icu` provides UTF conversions and the ICU headers available to the selected toolchain.
Include `ICU/utf8.h` for conversions and `ICU/IcuSupport.h` for linked ICU APIs.

| Build configuration | ICU access | Fallback |
| --- | --- | --- |
| Full ICU package, including Windows | Linked ICU C APIs and available C++ APIs | None needed |
| Windows SDK ICU | Linked, delay-loaded C APIs; `IcuLoader` checks the conversion exports first | Win32 `CP_UTF8` when the DLL or exports are unavailable |
| VC6 and Windows builds without an import library | `IcuLoader` loads `icu.dll` and resolves `u_strFromUTF8` and `u_strToUTF8WithSub` | Win32 `CP_UTF8` when the DLL or exports are unavailable |

VC6 does use ICU when these exports are available. It does not compile against modern ICU headers or expose the full ICU C++ API.

`IcuLoader` initializes once, with synchronization that also works on VC6. It owns the DLL reference returned by `LoadLibraryA`, releases it immediately if an export is missing, and otherwise releases it at normal process shutdown. The SDK delay loader owns any additional reference it acquires independently. Conversion workers must finish before static destruction begins.

The loader uses the normal Windows DLL search order, matching SDK delay loading and allowing an app-local `icu.dll`. There is no separate probe that discards a DLL handle.
