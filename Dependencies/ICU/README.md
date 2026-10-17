# ICU integration

`deps_icu` provides UTF conversions backed by ICU4C. Include `ICU/utf8.h` for conversions.

| Build configuration | ICU access | Fallback |
| --- | --- | --- |
| Full ICU package from vcpkg or the system (required on non-Windows) | Linked ICU C APIs | None needed |
| Windows, including VC6 | `IcuLoader` loads the OS `icu.dll` and resolves `u_strFromUTF8` and `u_strToUTF8WithSub` | Win32 `CP_UTF8` when the DLL or exports are unavailable |

Windows builds do not compile against ICU headers or link an ICU import library.

`IcuLoader` loads on the first conversion and caches both successful and failed attempts. Later calls reuse the result, including in WorldBuilder and other tools without an enclosing application scope. `unload()` releases the DLL, clears the exports, and permits a later call to retry. Both games explicitly unload after engine teardown; tools also have cleanup at normal module shutdown.

Loading, conversion calls, and unloading use a Windows `CRITICAL_SECTION`, the same primitive used by the engine's critical-section classes. It is available on VC6 and does not require a dependency on WWLib. Calls are serialized, and unload waits for any active conversion before freeing the DLL. Export pointers remain private to the loader. Conversion workers must stop before static destruction begins.

The critical section uses `Utility/lazy_static.h` for thread-safe first use on VC6, including startup logging before global constructors have run. The lock remains usable throughout static destruction. DLL cleanup is registered when the lock is initialized.

The loader tries absolute paths in the executable directory and then the Windows system directory. It supports app-local ICU and Unicode installation paths without searching the working directory or `PATH`. A missing DLL or conversion export selects the Win32 fallback; a DLL with missing exports is released immediately.
