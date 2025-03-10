# Documenting the Inline Assembly

This document includes **all** assembly blocks. I am aware that many will not be ported and many others will not be
included at all, but as the repository is now, I will document all available inline assembly.

Thanks to [xezon](https://github.com/xezon) for suggesting using [godbolt](https://godbolt.org/) to compare assembly
outputs.

All generated code uses the latest version of MSVC 19 x86, as available in godbolt.

## List of Documented Inline ASM

<details>
<summary>Generals/Code/Tools/timingTest/timingTest.cpp</summary>

This file includes the following assembly block:

```c++
void GetPrecisionTimer(INT64* t)
{
	// CPUID is needed to force serialization of any previous instructions. 
	__asm 
	{
		RDTSC
		MOV ECX,[t]
		MOV [ECX], EAX
		MOV [ECX+4], EDX
	}
}
```

This function is attempting to use the [RDTSC](https://www.aldeid.com/wiki/X86-assembly/Instructions/rdtsc) instruction
and saving its value into `t`.

My goto equivalent is:

```c++
#include <intrin.h>

void GetPrevisionTimer(__int64* t)
{
    *t = (__int64)__rdtsc();
}
```

The generated assemblies are:

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
_t$ = 8                                       ; size = 4
void GetPrecisionTimer(__int64 *) PROC                  ; GetPrecisionTimer
        push    ebp
        mov     ebp, esp
        rdtsc
        mov     ecx, DWORD PTR _t$[ebp]
        mov     DWORD PTR [ecx], eax
        mov     DWORD PTR [ecx+4], edx
        pop     ebp
        ret     0
void GetPrecisionTimer(__int64 *) ENDP                  ; GetPrecisionTimer
```

</td>
<td>

```asm
_t$ = 8                                       ; size = 4
void GetPrevisionTimer(__int64 *) PROC                  ; GetPrevisionTimer
        push    ebp
        mov     ebp, esp
        rdtsc
        mov     ecx, DWORD PTR _t$[ebp]
        mov     DWORD PTR [ecx], eax
        mov     DWORD PTR [ecx+4], edx
        pop     ebp
        ret     0
void GetPrevisionTimer(__int64 *) ENDP                  ; GetPrevisionTimer
```

</td>
</table>

These are **not** different.

</details>
