# ICU integration

`deps_icu` provides UTF conversions backed by ICU4C. Include `ICU/utf8.h` for conversions.

| Build configuration | ICU access | Fallback |
| --- | --- | --- |
| Non-Windows, ICU from vcpkg or the system (required) | Linked ICU C APIs | None needed |
| Windows, including VC6 | `IcuLoader` loads the OS `icu.dll` and resolves `u_strFromUTF8` and `u_strToUTF8WithSub` | Win32 `CP_UTF8` when the DLL or exports are unavailable |

Windows test builds also compile the conversions against ICU from vcpkg, when it is found. `icu_linked_googletest` runs the same `utf8_test.cpp` against linked ICU, so its results can be compared with `g_googletest` and `z_googletest`, which use `IcuLoader`.
