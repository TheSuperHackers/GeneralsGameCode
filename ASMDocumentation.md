# Documenting the Inline Assembly

Thanks to [xezon](https://github.com/TheSuperHackers/GeneralsGameCode/pull/405#issuecomment-2709737713) for suggesting
using [godbolt](https://godbolt.org/) to compare assembly outputs.

All generated code uses the latest version of MSVC 19 x86, as available in godbolt.

As recommended by [tomson26](https://github.com/TheSuperHackers/GeneralsGameCode/pull/405#issuecomment-2710604104), I am
not including `blitblit`, `rlerle`, `lcw` or `mpu`.

The `crc` assembly block is already extensively documented in source.

> **NOTE**: For methods with `__forceinline`, this is removed in godbolt to allow the compiler to only emit the function
> assembly and not other, unrelated assembly to keep the function from being removed.

> **NOTE**: All the code is compiled **without** optimization. Optimized code will result in smaller and more optimized
> assemblies in modern compilers.

> **NOTE**: Classes and structures, as well as functions that don't exist, will be replicated in godbolt in their most
> basic necessity to compile, and the comparison from original to equivalent codes has the same stubs to ensure parity
> in the assembly that is being generated.

## List of Documented Inline ASM

---

<details>
<summary>GeneralsMD/Code/Libraries/Include/Lib/BaseType.h</summary>

This file includes the following assembly blocks:

<details>
<summary>fast_float2long_round</summary>

```c++
__forceinline long fast_float2long_round(float f)
{
	long i;

	__asm {
		fld [f]
		fistp [i]
	}

	return i;
}
```

This is using assembly to do a fast round and conversion from `float` to `long`.

My goto equivalent is:

```c++
#include <math.h>

__froceinline float fast_float_trunc(float f) {
    return lround(f);
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
_i$ = -4                                                ; size = 4
_f$ = 8                                       ; size = 4
long fast_float2long_round(float) PROC           ; fast_float2long_round
        push    ebp
        mov     ebp, esp
        push    ecx
        fld     DWORD PTR _f$[ebp]
        fistp   DWORD PTR _i$[ebp]
        mov     eax, DWORD PTR _i$[ebp]
        mov     esp, ebp
        pop     ebp
        ret     0
long fast_float2long_round(float) ENDP           ; fast_float2long_round
```

</td>
<td>

```asm
_f$ = 8                                       ; size = 4
long fast_float2long_round(float) PROC           ; fast_float2long_round
        push    ebp
        mov     ebp, esp
        cvtss2sd xmm0, DWORD PTR _f$[ebp]
        sub     esp, 8
        movsd   QWORD PTR [esp], xmm0
        call    _lround
        add     esp, 8
        pop     ebp
        ret     0
long fast_float2long_round(float) ENDP           ; fast_float2long_round
```

</td>
</table>

While the assemblies are different, the `lround` method is very efficient as well. Refer to its implementation for more
details.

```diff
@@ -1,13 +1,12 @@
-_i$ = -4                                                ; size = 4
 _f$ = 8                                       ; size = 4
 long fast_float2long_round(float) PROC           ; fast_float2long_round
         push    ebp
         mov     ebp, esp
-        push    ecx
-        fld     DWORD PTR _f$[ebp]
-        fistp   DWORD PTR _i$[ebp]
-        mov     eax, DWORD PTR _i$[ebp]
-        mov     esp, ebp
+        cvtss2sd xmm0, DWORD PTR _f$[ebp]
+        sub     esp, 8
+        movsd   QWORD PTR [esp], xmm0
+        call    _lround
+        add     esp, 8
         pop     ebp
         ret     0
 long fast_float2long_round(float) ENDP           ; fast_float2long_round
```

</details>

<details>
<summary>fast_float_trunc</summary>

```c++
__forceinline float fast_float_trunc(float f)
{
  _asm
  {
    mov ecx,[f]
    shr ecx,23
    mov eax,0xff800000
    xor ebx,ebx
    sub cl,127
    cmovc eax,ebx
    sar eax,cl
    and [f],eax
  }
  return f;
}
```

This is using assembly to do a fast truncation of the given float number.

My goto equivalent is:

```c++
__froceinline float fast_float_trunc(float f) {
    // Constants
    const int exponentBias = 127; // Bias for single precision float
    const int exponentMask = 0x7F800000; // Mask for exponent bits
    const int signMask = 0x80000000; // Mask for sign bit
    const int signShift = 23; // Shift for extracting the exponent
    // Extract exponent from the input float
    int exponent = (*(int*)&f & exponentMask) >> signShift;
    // Prepare the sign bit
    int sign = -8388608; // 0xff800000
    int temp = 0;
    // Calculate the shift amount
    exponent -= exponentBias; // Adjust exponent to find the actual exponent value
    if (exponent < 0) {
        temp = sign; // If the exponent is negative, set temp to sign
    }
    // Apply the sign if necessary
    temp = (temp & (*(int*)&f)); // Mask the input float with temp
    // Cast input to float and return
    return *(float*)&temp; // Return the truncated float
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
_f$ = 8                                       ; size = 4
float fast_float_trunc(float) PROC                      ; fast_float_trunc
        push    ebp
        mov     ebp, esp
        push    ebx
        mov     ecx, DWORD PTR _f$[ebp]
        shr     ecx, 23                             ; 00000017H
        mov     eax, -8388608                   ; ff800000H
        xor     ebx, ebx
        sub     cl, 127                             ; 0000007fH
        cmovb   eax, ebx
        sar     eax, cl
        and     DWORD PTR _f$[ebp], eax
        fld     DWORD PTR _f$[ebp]
        pop     ebx
        pop     ebp
        ret     0
float fast_float_trunc(float) ENDP                      ; fast_float_trunc
```

</td>
<td>

```asm
_signShift$ = -28                                 ; size = 4
_signMask$ = -24                                        ; size = 4
_exponentMask$ = -20                                    ; size = 4
_exponentBias$ = -16                                    ; size = 4
_sign$ = -12                                            ; size = 4
_exponent$ = -8                               ; size = 4
_temp$ = -4                                   ; size = 4
_f$ = 8                                       ; size = 4
float fast_float_trunc(float) PROC                      ; fast_float_trunc
        push    ebp
        mov     ebp, esp
        sub     esp, 28                             ; 0000001cH
        mov     DWORD PTR _exponentBias$[ebp], 127        ; 0000007fH
        mov     DWORD PTR _exponentMask$[ebp], 2139095040 ; 7f800000H
        mov     DWORD PTR _signMask$[ebp], -2147483648    ; 80000000H
        mov     DWORD PTR _signShift$[ebp], 23            ; 00000017H
        mov     eax, DWORD PTR _f$[ebp]
        and     eax, 2139095040                     ; 7f800000H
        sar     eax, 23                             ; 00000017H
        mov     DWORD PTR _exponent$[ebp], eax
        mov     DWORD PTR _sign$[ebp], -8388608         ; ff800000H
        mov     DWORD PTR _temp$[ebp], 0
        mov     ecx, DWORD PTR _exponent$[ebp]
        sub     ecx, 127                      ; 0000007fH
        mov     DWORD PTR _exponent$[ebp], ecx
        jns     SHORT $LN2@fast_float
        mov     edx, DWORD PTR _sign$[ebp]
        mov     DWORD PTR _temp$[ebp], edx
$LN2@fast_float:
        mov     eax, DWORD PTR _temp$[ebp]
        and     eax, DWORD PTR _f$[ebp]
        mov     DWORD PTR _temp$[ebp], eax
        fld     DWORD PTR _temp$[ebp]
        mov     esp, ebp
        pop     ebp
        ret     0
float fast_float_trunc(float) ENDP                      ; fast_float_trunc
```

</td>
</table>

While both assemblies are very different, they both reach the same result. I cannot see a better or shorter C++
implementation, suggestions are welcome if this method requires further investigation.

```diff
@@ -1,18 +1,37 @@
+_signShift$ = -28                                 ; size = 4
+_signMask$ = -24                                        ; size = 4
+_exponentMask$ = -20                                    ; size = 4
+_exponentBias$ = -16                                    ; size = 4
+_sign$ = -12                                            ; size = 4
+_exponent$ = -8                               ; size = 4
+_temp$ = -4                                   ; size = 4
 _f$ = 8                                       ; size = 4
 float fast_float_trunc(float) PROC                      ; fast_float_trunc
         push    ebp
         mov     ebp, esp
-        push    ebx
-        mov     ecx, DWORD PTR _f$[ebp]
-        shr     ecx, 23                             ; 00000017H
-        mov     eax, -8388608                   ; ff800000H
-        xor     ebx, ebx
-        sub     cl, 127                             ; 0000007fH
-        cmovb   eax, ebx
-        sar     eax, cl
-        and     DWORD PTR _f$[ebp], eax
-        fld     DWORD PTR _f$[ebp]
-        pop     ebx
+        sub     esp, 28                             ; 0000001cH
+        mov     DWORD PTR _exponentBias$[ebp], 127        ; 0000007fH
+        mov     DWORD PTR _exponentMask$[ebp], 2139095040 ; 7f800000H
+        mov     DWORD PTR _signMask$[ebp], -2147483648    ; 80000000H
+        mov     DWORD PTR _signShift$[ebp], 23            ; 00000017H
+        mov     eax, DWORD PTR _f$[ebp]
+        and     eax, 2139095040                     ; 7f800000H
+        sar     eax, 23                             ; 00000017H
+        mov     DWORD PTR _exponent$[ebp], eax
+        mov     DWORD PTR _sign$[ebp], -8388608         ; ff800000H
+        mov     DWORD PTR _temp$[ebp], 0
+        mov     ecx, DWORD PTR _exponent$[ebp]
+        sub     ecx, 127                      ; 0000007fH
+        mov     DWORD PTR _exponent$[ebp], ecx
+        jns     SHORT $LN2@fast_float
+        mov     edx, DWORD PTR _sign$[ebp]
+        mov     DWORD PTR _temp$[ebp], edx
+$LN2@fast_float:
+        mov     eax, DWORD PTR _temp$[ebp]
+        and     eax, DWORD PTR _f$[ebp]
+        mov     DWORD PTR _temp$[ebp], eax
+        fld     DWORD PTR _temp$[ebp]
+        mov     esp, ebp
         pop     ebp
         ret     0
 float fast_float_trunc(float) ENDP                      ; fast_float_trunc
```

</details>

</details>

---

<details>
<summary>Generals/Code/Libraries/Source/WWVegas/WWLib/cpudetect.cpp</summary>

This file includes the following assembly clocks:

<details>
<summary>ASM_RDTSC</summary>

```c++
#define ASM_RDTSC _asm _emit 0x0f _asm _emit 0x31
```

This code just emits the RDTSC instruction, which is a two byte instruction, `0x0F31`.

This is identical to:

```c++
#define ASM_RDTSC RDTSC
```

And it is identical, in Win32; to:

```c++
#include <intrin.h>

#define ASM_RDTSC __rdtsc();
```

This may have been done for obfuscation? Some parts of the code do use the `RDTSC` instruction directly and other parts
use the bytecodes instead, it's not exactly consistent.

</details>

<details>
<summary>Calculate_Processor_Speed</summary>

> **NOTE**: This code has been very slightly modified to allow it to work on modern MSVC inline assembly.

```c++
static unsigned Calculate_Processor_Speed(__int64& ticks_per_second)
{
	struct {
		unsigned timer0_h;
		unsigned timer0_l;
		unsigned timer1_h;
		unsigned timer1_l;
	} Time;

#ifdef WIN32
   __asm {
      ASM_RDTSC1:
      mov Time.timer0_h, eax
      mov Time.timer0_l, edx
   }
#elif defined(_UNIX)
      __asm__("rdtsc");
      __asm__("mov %eax, __Time.timer1_h");
      __asm__("mov %edx, __Time.timer1_l");
#endif

	unsigned start=TIMEGETTIME();
	unsigned elapsed;
	while ((elapsed=TIMEGETTIME()-start)<200) {
#ifdef WIN32
      __asm {
         ASM_RDTSC2:
         mov Time.timer1_h, eax
         mov Time.timer1_l, edx
      }
#elif defined(_UNIX)
      __asm__ ("rdtsc");
      __asm__("mov %eax, __Time.timer1_h");
      __asm__("mov %edx, __Time.timer1_l");
#endif
	}

	__int64 t=*(__int64*)&Time.timer1_h-*(__int64*)&Time.timer0_h;
	ticks_per_second=(__int64)((1000.0/(double)elapsed)*(double)t);	// Ticks per second
	return unsigned((double)t/(double)(elapsed*1000));
}
```

This is uses cross-platform inline assembly to calculate processor speed.

My goto equivalent is:

```c++
#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#elif defined(_UNIX)
#include <ia32intrin.h>
#endif

#define TIMEGETTIME timeGetTime

static unsigned Calculate_Processor_Speed(__int64& ticks_per_second)
{
	struct {
		unsigned timer0_h;
		unsigned timer0_l;
		unsigned timer1_h;
		unsigned timer1_l;
	} Time;

#ifdef _WIN32
    unsigned __int64 timeStampCounter = __rdtsc();
    Time.timer0_l = (unsigned)timeStampCounter;
    Time.timer0_h = (unsigned)(timeStampCounter >> 32);
#elif defined(_UNIX)
    unsigned long long tsc = __rdtsc();
    Time.timer1_l = (unsigned)timeStampCounter;
    Time.timer1_h = (unsigned)(timeStampCounter >> 32);
    // Is this a bug? is this supposed to be 0_l and 0_h? Because they are about to be overwritten
#endif

	unsigned start=TIMEGETTIME();
	unsigned elapsed;
	while ((elapsed=TIMEGETTIME()-start)<200) {
#ifdef _WIN32
    timeStampCounter = __rdtsc();
    Time.timer1_l = (unsigned)timeStampCounter;
    Time.timer1_h = (unsigned)(timeStampCounter >> 32);
#elif defined(_UNIX)
    tsc = __rdtsc();
    Time.timer1_l = (unsigned)timeStampCounter;
    Time.timer1_h = (unsigned)(timeStampCounter >> 32);
    // Is this a bug? is this supposed to be 0_l and 0_h? Because these just overwrote the previously set values
#endif
	}

	__int64 t=*(__int64*)&Time.timer1_h-*(__int64*)&Time.timer0_h;
	ticks_per_second=(__int64)((1000.0/(double)elapsed)*(double)t);	// Ticks per second
	return unsigned((double)t/(double)(elapsed*1000));
}
```

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
__xmm@41f00000000000000000000000000000 DB 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 0f0H, 'A'
__real@408f400000000000 DQ 0408f400000000000r   ; 1000
voltbl  SEGMENT
_volmd  DD  0ffffffffH
        DDSymXIndex:    FLAT:unsigned int Calculate_Processor_Speed(__int64 &)
        DD      0dH
        DD      0cfH
voltbl  ENDS

tv148 = -52                                   ; size = 8
_start$ = -44                                     ; size = 4
_t$ = -40                                         ; size = 8
tv140 = -32                                   ; size = 4
tv87 = -28                                          ; size = 4
_elapsed$ = -24                               ; size = 4
_Time$ = -20                                            ; size = 16
__$ArrayPad$ = -4                                 ; size = 4
_ticks_per_second$ = 8                              ; size = 4
unsigned int Calculate_Processor_Speed(__int64 &) PROC          ; Calculate_Processor_Speed
        push    ebp
        mov     ebp, esp
        sub     esp, 52                             ; 00000034H
        mov     eax, DWORD PTR ___security_cookie
        xor     eax, ebp
        mov     DWORD PTR __$ArrayPad$[ebp], eax
$ASM_RDTSC1$5:
        mov     DWORD PTR _Time$[ebp], eax
        mov     DWORD PTR _Time$[ebp+4], edx
        call    DWORD PTR __imp__timeGetTime@0
        mov     DWORD PTR _start$[ebp], eax
$LN2@Calculate_:
        call    DWORD PTR __imp__timeGetTime@0
        sub     eax, DWORD PTR _start$[ebp]
        mov     DWORD PTR _elapsed$[ebp], eax
        cmp     DWORD PTR _elapsed$[ebp], 200       ; 000000c8H
        jae     SHORT $LN3@Calculate_
$ASM_RDTSC2$6:
        mov     DWORD PTR _Time$[ebp+8], eax
        mov     DWORD PTR _Time$[ebp+12], edx
        jmp     SHORT $LN2@Calculate_
$LN3@Calculate_:
        mov     eax, DWORD PTR _Time$[ebp+8]
        sub     eax, DWORD PTR _Time$[ebp]
        mov     ecx, DWORD PTR _Time$[ebp+12]
        sbb     ecx, DWORD PTR _Time$[ebp+4]
        mov     DWORD PTR _t$[ebp], eax
        mov     DWORD PTR _t$[ebp+4], ecx
        mov     edx, DWORD PTR _elapsed$[ebp]
        mov     DWORD PTR tv87[ebp], edx
        cvtsi2sd xmm0, DWORD PTR tv87[ebp]
        mov     eax, DWORD PTR tv87[ebp]
        shr     eax, 31                             ; 0000001fH
        addsd   xmm0, QWORD PTR __xmm@41f00000000000000000000000000000[eax*8]
        movsd   xmm1, QWORD PTR __real@408f400000000000
        divsd   xmm1, xmm0
        mov     edx, DWORD PTR _t$[ebp+4]
        mov     ecx, DWORD PTR _t$[ebp]
        movsd   QWORD PTR tv148[ebp], xmm1
        call    __ltod3
        movsd   xmm1, QWORD PTR tv148[ebp]
        mulsd   xmm1, xmm0
        movaps  xmm0, xmm1
        call    __dtol3
        mov     ecx, DWORD PTR _ticks_per_second$[ebp]
        mov     DWORD PTR [ecx], eax
        mov     DWORD PTR [ecx+4], edx
        mov     edx, DWORD PTR _t$[ebp+4]
        mov     ecx, DWORD PTR _t$[ebp]
        call    __ltod3
        imul    edx, DWORD PTR _elapsed$[ebp], 1000
        mov     DWORD PTR tv140[ebp], edx
        cvtsi2sd xmm1, DWORD PTR tv140[ebp]
        mov     eax, DWORD PTR tv140[ebp]
        shr     eax, 31                             ; 0000001fH
        addsd   xmm1, QWORD PTR __xmm@41f00000000000000000000000000000[eax*8]
        divsd   xmm0, xmm1
        call    __dtol3
        mov     ecx, DWORD PTR __$ArrayPad$[ebp]
        xor     ecx, ebp
        call    @__security_check_cookie@4
        mov     esp, ebp
        pop     ebp
        ret     0
unsigned int Calculate_Processor_Speed(__int64 &) ENDP          ; Calculate_Processor_Speed
```

</td>
<td>

```asm
__xmm@41f00000000000000000000000000000 DB 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 0f0H, 'A'
__real@408f400000000000 DQ 0408f400000000000r   ; 1000
voltbl  SEGMENT
_volmd  DD  0ffffffffH
        DDSymXIndex:    FLAT:unsigned int Calculate_Processor_Speed(__int64 &)
        DD      0dH
        DD      0ffH
voltbl  ENDS

tv182 = -60                                   ; size = 8
_start$ = -52                                     ; size = 4
_t$ = -48                                         ; size = 8
tv174 = -40                                   ; size = 4
tv153 = -36                                   ; size = 4
_elapsed$ = -32                               ; size = 4
_timeStampCounter$ = -28                                ; size = 8
_Time$ = -20                                            ; size = 16
__$ArrayPad$ = -4                                 ; size = 4
_ticks_per_second$ = 8                              ; size = 4
unsigned int Calculate_Processor_Speed(__int64 &) PROC          ; Calculate_Processor_Speed
        push    ebp
        mov     ebp, esp
        sub     esp, 60                             ; 0000003cH
        mov     eax, DWORD PTR ___security_cookie
        xor     eax, ebp
        mov     DWORD PTR __$ArrayPad$[ebp], eax
        rdtsc
        mov     DWORD PTR _timeStampCounter$[ebp], eax
        mov     DWORD PTR _timeStampCounter$[ebp+4], edx
        mov     eax, DWORD PTR _timeStampCounter$[ebp]
        mov     DWORD PTR _Time$[ebp+4], eax
        mov     eax, DWORD PTR _timeStampCounter$[ebp]
        mov     edx, DWORD PTR _timeStampCounter$[ebp+4]
        mov     cl, 32                                    ; 00000020H
        call    __aullshr
        mov     DWORD PTR _Time$[ebp], eax
        call    DWORD PTR __imp__timeGetTime@0
        mov     DWORD PTR _start$[ebp], eax
$LN2@Calculate_:
        call    DWORD PTR __imp__timeGetTime@0
        sub     eax, DWORD PTR _start$[ebp]
        mov     DWORD PTR _elapsed$[ebp], eax
        cmp     DWORD PTR _elapsed$[ebp], 200       ; 000000c8H
        jae     SHORT $LN3@Calculate_
        rdtsc
        mov     DWORD PTR _timeStampCounter$[ebp], eax
        mov     DWORD PTR _timeStampCounter$[ebp+4], edx
        mov     ecx, DWORD PTR _timeStampCounter$[ebp]
        mov     DWORD PTR _Time$[ebp+12], ecx
        mov     eax, DWORD PTR _timeStampCounter$[ebp]
        mov     edx, DWORD PTR _timeStampCounter$[ebp+4]
        mov     cl, 32                                    ; 00000020H
        call    __aullshr
        mov     DWORD PTR _Time$[ebp+8], eax
        jmp     SHORT $LN2@Calculate_
$LN3@Calculate_:
        mov     edx, DWORD PTR _Time$[ebp+8]
        sub     edx, DWORD PTR _Time$[ebp]
        mov     eax, DWORD PTR _Time$[ebp+12]
        sbb     eax, DWORD PTR _Time$[ebp+4]
        mov     DWORD PTR _t$[ebp], edx
        mov     DWORD PTR _t$[ebp+4], eax
        mov     ecx, DWORD PTR _elapsed$[ebp]
        mov     DWORD PTR tv153[ebp], ecx
        cvtsi2sd xmm0, DWORD PTR tv153[ebp]
        mov     edx, DWORD PTR tv153[ebp]
        shr     edx, 31                             ; 0000001fH
        addsd   xmm0, QWORD PTR __xmm@41f00000000000000000000000000000[edx*8]
        movsd   xmm1, QWORD PTR __real@408f400000000000
        divsd   xmm1, xmm0
        mov     edx, DWORD PTR _t$[ebp+4]
        mov     ecx, DWORD PTR _t$[ebp]
        movsd   QWORD PTR tv182[ebp], xmm1
        call    __ltod3
        movsd   xmm1, QWORD PTR tv182[ebp]
        mulsd   xmm1, xmm0
        movaps  xmm0, xmm1
        call    __dtol3
        mov     ecx, DWORD PTR _ticks_per_second$[ebp]
        mov     DWORD PTR [ecx], eax
        mov     DWORD PTR [ecx+4], edx
        mov     edx, DWORD PTR _t$[ebp+4]
        mov     ecx, DWORD PTR _t$[ebp]
        call    __ltod3
        imul    edx, DWORD PTR _elapsed$[ebp], 1000
        mov     DWORD PTR tv174[ebp], edx
        cvtsi2sd xmm1, DWORD PTR tv174[ebp]
        mov     eax, DWORD PTR tv174[ebp]
        shr     eax, 31                             ; 0000001fH
        addsd   xmm1, QWORD PTR __xmm@41f00000000000000000000000000000[eax*8]
        divsd   xmm0, xmm1
        call    __dtol3
        mov     ecx, DWORD PTR __$ArrayPad$[ebp]
        xor     ecx, ebp
        call    @__security_check_cookie@4
        mov     esp, ebp
        pop     ebp
        ret     0
unsigned int Calculate_Processor_Speed(__int64 &) ENDP          ; Calculate_Processor_Speed
```

</td>
</table>

The generated assemblies are quite different due to the fact that C++ code is importing and using the `__rdtsc()`
intrinsics, but the result is the same. As with others, which one will be more efficient is to be tested. Or if with
optimizations assembly is even worth it with modern compilers.

> **NOTE**: Using modern C++ with the STL will probably be simply better for this.

```diff
@@ -5,28 +5,36 @@ voltbl  SEGMENT
 _volmd  DD  0ffffffffH
         DDSymXIndex:    FLAT:unsigned int Calculate_Processor_Speed(__int64 &)
         DD      0dH
-        DD      0cfH
+        DD      0ffH
 voltbl  ENDS
 
-tv148 = -52                                   ; size = 8
-_start$ = -44                                     ; size = 4
-_t$ = -40                                         ; size = 8
-tv140 = -32                                   ; size = 4
-tv87 = -28                                          ; size = 4
-_elapsed$ = -24                               ; size = 4
+tv182 = -60                                   ; size = 8
+_start$ = -52                                     ; size = 4
+_t$ = -48                                         ; size = 8
+tv174 = -40                                   ; size = 4
+tv153 = -36                                   ; size = 4
+_elapsed$ = -32                               ; size = 4
+_timeStampCounter$ = -28                                ; size = 8
 _Time$ = -20                                            ; size = 16
 __$ArrayPad$ = -4                                 ; size = 4
 _ticks_per_second$ = 8                              ; size = 4
 unsigned int Calculate_Processor_Speed(__int64 &) PROC          ; Calculate_Processor_Speed
         push    ebp
         mov     ebp, esp
-        sub     esp, 52                             ; 00000034H
+        sub     esp, 60                             ; 0000003cH
         mov     eax, DWORD PTR ___security_cookie
         xor     eax, ebp
         mov     DWORD PTR __$ArrayPad$[ebp], eax
-$ASM_RDTSC1$5:
+        rdtsc
+        mov     DWORD PTR _timeStampCounter$[ebp], eax
+        mov     DWORD PTR _timeStampCounter$[ebp+4], edx
+        mov     eax, DWORD PTR _timeStampCounter$[ebp]
+        mov     DWORD PTR _Time$[ebp+4], eax
+        mov     eax, DWORD PTR _timeStampCounter$[ebp]
+        mov     edx, DWORD PTR _timeStampCounter$[ebp+4]
+        mov     cl, 32                                    ; 00000020H
+        call    __aullshr
         mov     DWORD PTR _Time$[ebp], eax
-        mov     DWORD PTR _Time$[ebp+4], edx
         call    DWORD PTR __imp__timeGetTime@0
         mov     DWORD PTR _start$[ebp], eax
 $LN2@Calculate_:
@@ -35,30 +43,37 @@ $LN2@Calculate_:
         mov     DWORD PTR _elapsed$[ebp], eax
         cmp     DWORD PTR _elapsed$[ebp], 200       ; 000000c8H
         jae     SHORT $LN3@Calculate_
-$ASM_RDTSC2$6:
+        rdtsc
+        mov     DWORD PTR _timeStampCounter$[ebp], eax
+        mov     DWORD PTR _timeStampCounter$[ebp+4], edx
+        mov     ecx, DWORD PTR _timeStampCounter$[ebp]
+        mov     DWORD PTR _Time$[ebp+12], ecx
+        mov     eax, DWORD PTR _timeStampCounter$[ebp]
+        mov     edx, DWORD PTR _timeStampCounter$[ebp+4]
+        mov     cl, 32                                    ; 00000020H
+        call    __aullshr
         mov     DWORD PTR _Time$[ebp+8], eax
-        mov     DWORD PTR _Time$[ebp+12], edx
         jmp     SHORT $LN2@Calculate_
 $LN3@Calculate_:
-        mov     eax, DWORD PTR _Time$[ebp+8]
-        sub     eax, DWORD PTR _Time$[ebp]
-        mov     ecx, DWORD PTR _Time$[ebp+12]
-        sbb     ecx, DWORD PTR _Time$[ebp+4]
-        mov     DWORD PTR _t$[ebp], eax
-        mov     DWORD PTR _t$[ebp+4], ecx
-        mov     edx, DWORD PTR _elapsed$[ebp]
-        mov     DWORD PTR tv87[ebp], edx
-        cvtsi2sd xmm0, DWORD PTR tv87[ebp]
-        mov     eax, DWORD PTR tv87[ebp]
-        shr     eax, 31                             ; 0000001fH
-        addsd   xmm0, QWORD PTR __xmm@41f00000000000000000000000000000[eax*8]
+        mov     edx, DWORD PTR _Time$[ebp+8]
+        sub     edx, DWORD PTR _Time$[ebp]
+        mov     eax, DWORD PTR _Time$[ebp+12]
+        sbb     eax, DWORD PTR _Time$[ebp+4]
+        mov     DWORD PTR _t$[ebp], edx
+        mov     DWORD PTR _t$[ebp+4], eax
+        mov     ecx, DWORD PTR _elapsed$[ebp]
+        mov     DWORD PTR tv153[ebp], ecx
+        cvtsi2sd xmm0, DWORD PTR tv153[ebp]
+        mov     edx, DWORD PTR tv153[ebp]
+        shr     edx, 31                             ; 0000001fH
+        addsd   xmm0, QWORD PTR __xmm@41f00000000000000000000000000000[edx*8]
         movsd   xmm1, QWORD PTR __real@408f400000000000
         divsd   xmm1, xmm0
         mov     edx, DWORD PTR _t$[ebp+4]
         mov     ecx, DWORD PTR _t$[ebp]
-        movsd   QWORD PTR tv148[ebp], xmm1
+        movsd   QWORD PTR tv182[ebp], xmm1
         call    __ltod3
-        movsd   xmm1, QWORD PTR tv148[ebp]
+        movsd   xmm1, QWORD PTR tv182[ebp]
         mulsd   xmm1, xmm0
         movaps  xmm0, xmm1
         call    __dtol3
@@ -69,9 +84,9 @@ $LN3@Calculate_:
         mov     ecx, DWORD PTR _t$[ebp]
         call    __ltod3
         imul    edx, DWORD PTR _elapsed$[ebp], 1000
-        mov     DWORD PTR tv140[ebp], edx
-        cvtsi2sd xmm1, DWORD PTR tv140[ebp]
-        mov     eax, DWORD PTR tv140[ebp]
+        mov     DWORD PTR tv174[ebp], edx
+        cvtsi2sd xmm1, DWORD PTR tv174[ebp]
+        mov     eax, DWORD PTR tv174[ebp]
         shr     eax, 31                             ; 0000001fH
         addsd   xmm1, QWORD PTR __xmm@41f00000000000000000000000000000[eax*8]
         divsd   xmm0, xmm1
```

</details>

<details>
<summary>CPUDetectClass::Init_CPUID_Instruction</summary>

```c++
void CPUDetectClass::Init_CPUID_Instruction()
{
	unsigned long cpuid_available=0;

   // The pushfd/popfd commands are done using emits
   // because CodeWarrior seems to have problems with
   // the command (huh?)

#ifdef WIN32
   __asm
   {
      mov cpuid_available, 0	// clear flag
      push ebx
      pushfd
      pop eax
      mov ebx, eax
      xor eax, 0x00200000
      push eax
      popfd
      pushfd
      pop eax
      xor eax, ebx
      je done
      mov cpuid_available, 1
   done:
      push ebx
      popfd
      pop ebx
   }
#elif defined(_UNIX)
     __asm__(" mov $0, __cpuid_available");  // clear flag
     __asm__(" push %ebx");
     __asm__(" pushfd");
     __asm__(" pop %eax");
     __asm__(" mov %eax, %ebx");
     __asm__(" xor 0x00200000, %eax");
     __asm__(" push %eax");
     __asm__(" popfd");
     __asm__(" pushfd");
     __asm__(" pop %eax");
     __asm__(" xor %ebx, %eax");
     __asm__(" je done");
     __asm__(" mov $1, __cpuid_available");
     goto done;  // just to shut the compiler up
   done:
     __asm__(" push %ebx");
     __asm__(" popfd");
     __asm__(" pop %ebx");
#endif
	HasCPUIDInstruction=!!cpuid_available;
}
```

This code is attempting to check if the CPU has the `CPUID` instruction.

My goto equivalent is:

```c++
#if _WIN32
#include <intrin.h>
#endif

class CPUDetectClass {
    public:
    void Init_CPUID_Instruction();
    private:
    bool HasCPUIDInstruction = false;
};

void CPUDetectClass::Init_CPUID_Instruction()
{
	unsigned long cpuid_available=0;
    unsigned int mask = 0x00200000;

#ifdef _WIN32
    unsigned int eflags = __readeflags();
    unsigned int modifiedEflags = eflags ^ mask; // Flip the 21st bit (ID flag) of EFLAGS
    __writeeflags(modifiedEflags);
    
    // Compare the original and new EFLAGS value at bit 21
    cpuid_available = (unsigned long)((__readeflags() ^ eflags) & mask);
#elif defined(_UNIX)
    unsigned int eflags;
    // GCC/Clang require inline assembly, sorry :(
    // Save current EFLAGS to eflags
    asm volatile("push\n\t"
                 "pop %0"
                 : "=r"(eflags));

    unsigned int modifiedEflags = eflags ^ mask; // Flip the 21st bit (ID flag) of EFLAGS
    // Write the modified EFLAGS
    asm volatile("push %0\n\t"
                 "popf"
                 :
                 : "r"(modifiedEflags));

    // Read back the current EFLAGS to see if the ID flag changed
    unsigned int newEflags;
    asm volatile("pushf\n\t"
                 "pop %0"
                 : "=r"(newEflags));
    
    // Compare the original and new EFLAGS value at bit 21
    cpuid_available = (unsigned long)((newEflags ^ eflags) & mask);
#endif
	HasCPUIDInstruction=!!cpuid_available;
}
```

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
_this$ = -12                                            ; size = 4
tv66 = -8                                         ; size = 4
_cpuid_available$ = -4                              ; size = 4
void CPUDetectClass::Init_CPUID_Instruction(void) PROC     ; CPUDetectClass::Init_CPUID_Instruction
        push    ebp
        mov     ebp, esp
        sub     esp, 12                             ; 0000000cH
        push    ebx
        mov     DWORD PTR _this$[ebp], ecx
        mov     DWORD PTR _cpuid_available$[ebp], 0
        mov     DWORD PTR _cpuid_available$[ebp], 0
        push    ebx
        pushfd
        pop     eax
        mov     ebx, eax
        xor     eax, 2097152                          ; 00200000H
        push    eax
        popfd
        pushfd
        pop     eax
        xor     eax, ebx
        je      SHORT $done$5
        mov     DWORD PTR _cpuid_available$[ebp], 1
$done$5:
        push    ebx
        popfd
        pop     ebx
        cmp     DWORD PTR _cpuid_available$[ebp], 0
        je      SHORT $LN3@Init_CPUID
        mov     DWORD PTR tv66[ebp], 1
        jmp     SHORT $LN4@Init_CPUID
$LN3@Init_CPUID:
        mov     DWORD PTR tv66[ebp], 0
$LN4@Init_CPUID:
        mov     eax, DWORD PTR _this$[ebp]
        mov     cl, BYTE PTR tv66[ebp]
        mov     BYTE PTR [eax], cl
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret     0
void CPUDetectClass::Init_CPUID_Instruction(void) ENDP     ; CPUDetectClass::Init_CPUID_Instruction
```

</td>
<td>

```asm
_this$ = -24                                            ; size = 4
_modifiedEflags$ = -20                              ; size = 4
tv72 = -16                                          ; size = 4
_cpuid_available$ = -12                       ; size = 4
_mask$ = -8                                   ; size = 4
_eflags$ = -4                                     ; size = 4
void CPUDetectClass::Init_CPUID_Instruction(void) PROC     ; CPUDetectClass::Init_CPUID_Instruction
        push    ebp
        mov     ebp, esp
        sub     esp, 24                             ; 00000018H
        mov     DWORD PTR _this$[ebp], ecx
        mov     DWORD PTR _cpuid_available$[ebp], 0
        mov     DWORD PTR _mask$[ebp], 2097152            ; 00200000H
        pushfd
        pop     eax
        mov     DWORD PTR _eflags$[ebp], eax
        mov     ecx, DWORD PTR _eflags$[ebp]
        xor     ecx, DWORD PTR _mask$[ebp]
        mov     DWORD PTR _modifiedEflags$[ebp], ecx
        mov     edx, DWORD PTR _modifiedEflags$[ebp]
        push    edx
        popfd
        pushfd
        pop     eax
        xor     eax, DWORD PTR _eflags$[ebp]
        and     eax, DWORD PTR _mask$[ebp]
        mov     DWORD PTR _cpuid_available$[ebp], eax
        je      SHORT $LN3@Init_CPUID
        mov     DWORD PTR tv72[ebp], 1
        jmp     SHORT $LN4@Init_CPUID
$LN3@Init_CPUID:
        mov     DWORD PTR tv72[ebp], 0
$LN4@Init_CPUID:
        mov     ecx, DWORD PTR _this$[ebp]
        mov     dl, BYTE PTR tv72[ebp]
        mov     BYTE PTR [ecx], dl
        mov     esp, ebp
        pop     ebp
        ret     0
void CPUDetectClass::Init_CPUID_Instruction(void) ENDP     ; CPUDetectClass::Init_CPUID_Instruction
```

</td>
</table>

The assemblies are only slightly different due to the calls to Win32 intrinsics, but both accomplish the same. Modern
compilers with optimizations may be faster than only inline assembly.

> **NOTE** GCC/Clang don't have `EFLAGS` intrinsics and inline assembly is required for them.

```diff
@@ -1,41 +1,39 @@
-_this$ = -12                                            ; size = 4
-tv66 = -8                                         ; size = 4
-_cpuid_available$ = -4                              ; size = 4
+_this$ = -24                                            ; size = 4
+_modifiedEflags$ = -20                              ; size = 4
+tv72 = -16                                          ; size = 4
+_cpuid_available$ = -12                       ; size = 4
+_mask$ = -8                                   ; size = 4
+_eflags$ = -4                                     ; size = 4
 void CPUDetectClass::Init_CPUID_Instruction(void) PROC     ; CPUDetectClass::Init_CPUID_Instruction
         push    ebp
         mov     ebp, esp
-        sub     esp, 12                             ; 0000000cH
-        push    ebx
+        sub     esp, 24                             ; 00000018H
         mov     DWORD PTR _this$[ebp], ecx
         mov     DWORD PTR _cpuid_available$[ebp], 0
-        mov     DWORD PTR _cpuid_available$[ebp], 0
-        push    ebx
+        mov     DWORD PTR _mask$[ebp], 2097152            ; 00200000H
         pushfd
         pop     eax
-        mov     ebx, eax
-        xor     eax, 2097152                          ; 00200000H
-        push    eax
+        mov     DWORD PTR _eflags$[ebp], eax
+        mov     ecx, DWORD PTR _eflags$[ebp]
+        xor     ecx, DWORD PTR _mask$[ebp]
+        mov     DWORD PTR _modifiedEflags$[ebp], ecx
+        mov     edx, DWORD PTR _modifiedEflags$[ebp]
+        push    edx
         popfd
         pushfd
         pop     eax
-        xor     eax, ebx
-        je      SHORT $done$5
-        mov     DWORD PTR _cpuid_available$[ebp], 1
-$done$5:
-        push    ebx
-        popfd
-        pop     ebx
-        cmp     DWORD PTR _cpuid_available$[ebp], 0
+        xor     eax, DWORD PTR _eflags$[ebp]
+        and     eax, DWORD PTR _mask$[ebp]
+        mov     DWORD PTR _cpuid_available$[ebp], eax
         je      SHORT $LN3@Init_CPUID
-        mov     DWORD PTR tv66[ebp], 1
+        mov     DWORD PTR tv72[ebp], 1
         jmp     SHORT $LN4@Init_CPUID
 $LN3@Init_CPUID:
-        mov     DWORD PTR tv66[ebp], 0
+        mov     DWORD PTR tv72[ebp], 0
 $LN4@Init_CPUID:
-        mov     eax, DWORD PTR _this$[ebp]
-        mov     cl, BYTE PTR tv66[ebp]
-        mov     BYTE PTR [eax], cl
-        pop     ebx
+        mov     ecx, DWORD PTR _this$[ebp]
+        mov     dl, BYTE PTR tv72[ebp]
+        mov     BYTE PTR [ecx], dl
         mov     esp, ebp
         pop     ebp
         ret     0
```

</details>

<details>
<summary>CPUDetectClass::CPUID</summary>

```c++
bool CPUDetectClass::CPUID(
	unsigned& u_eax_,
	unsigned& u_ebx_,
	unsigned& u_ecx_,
	unsigned& u_edx_,
	unsigned cpuid_type)
{
	if (!Has_CPUID_Instruction()) return false;	// Most processors since 486 have CPUID...

	unsigned u_eax;
	unsigned u_ebx;
	unsigned u_ecx;
	unsigned u_edx;

#ifdef WIN32
   __asm
   {
      pushad
      mov	eax, [cpuid_type]
      xor	ebx, ebx
      xor	ecx, ecx
      xor	edx, edx
      cpuid
      mov	[u_eax], eax
      mov	[u_ebx], ebx
      mov	[u_ecx], ecx
      mov	[u_edx], edx
      popad
   }
#elif defined(_UNIX)
   __asm__("pusha");
   __asm__("mov	__cpuid_type, %eax");
   __asm__("xor	%ebx, %ebx");
   __asm__("xor	%ecx, %ecx");
   __asm__("xor	%edx, %edx");
   __asm__("cpuid");
   __asm__("mov	%eax, __u_eax");
   __asm__("mov	%ebx, __u_ebx");
   __asm__("mov	%ecx, __u_ecx");
   __asm__("mov	%edx, __u_edx");
   __asm__("popa");
#endif

	u_eax_=u_eax;
	u_ebx_=u_ebx;
	u_ecx_=u_ecx;
	u_edx_=u_edx;

	return true;
}
```

This is attempting to get information from the `CPUID` instruction.

My goto equivalent is:

```c++
#if _WIN32
#include <intrin.h>
#elif defined(_UNIX)
#include <cpuid.h>
#endif

class CPUDetectClass {
    public:
    bool Has_CPUID_Instruction();
    bool CPUID(unsigned& u_eax_,
	            unsigned& u_ebx_,
	            unsigned& u_ecx_,
	            unsigned& u_edx_,
	            unsigned cpuid_type);

    private:
    bool HasCPUIDInstruction = false;
};

bool CPUDetectClass::CPUID(
	unsigned& u_eax_,
	unsigned& u_ebx_,
	unsigned& u_ecx_,
	unsigned& u_edx_,
	unsigned cpuid_type)
{
	if (!Has_CPUID_Instruction()) return false;	// Most processors since 486 have CPUID...

	unsigned u_eax;
	unsigned u_ebx;
	unsigned u_ecx;
	unsigned u_edx;

#ifdef WIN32
    int cpuInfo[4]; // The CPUID output
    __cpuid(cpuInfo, cpuid_type);
    u_eax = cpuInfo[0];
    u_ebx = cpuInfo[1];
    u_ecx = cpuInfo[2];
    u_edx = cpuInfo[3];
#elif defined(_UNIX)
    unsigned int eax, ebx, ecx, edx;
    if (__get_cpuid(cpuid_type, &eax, &ebx, &ecx, &edx))
    {
        u_eax = eax;
        u_ebx = ebx;
        u_ecx = ecx;
        u_edx = edx;
    }
    else
    {
        u_eax = u_ebx = u_ecx = u_edx = 0;
    }
#endif

	u_eax_=u_eax;
	u_ebx_=u_ebx;
	u_ecx_=u_ecx;
	u_edx_=u_edx;

	return true;
}
```

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
_u_edx$ = -20                                     ; size = 4
_u_ecx$ = -16                                     ; size = 4
_u_ebx$ = -12                                     ; size = 4
_u_eax$ = -8                                            ; size = 4
_this$ = -4                                   ; size = 4
_u_eax_$ = 8                                            ; size = 4
_u_ebx_$ = 12                                     ; size = 4
_u_ecx_$ = 16                                     ; size = 4
_u_edx_$ = 20                                     ; size = 4
_cpuid_type$ = 24                                 ; size = 4
bool CPUDetectClass::CPUID(unsigned int &,unsigned int &,unsigned int &,unsigned int &,unsigned int) PROC          ; CPUDetectClass::CPUID
        push    ebp
        mov     ebp, esp
        sub     esp, 20                             ; 00000014H
        mov     DWORD PTR _this$[ebp], ecx
        mov     ecx, DWORD PTR _this$[ebp]
        call    bool CPUDetectClass::Has_CPUID_Instruction(void) ; CPUDetectClass::Has_CPUID_Instruction
        movzx   eax, al
        test    eax, eax
        jne     SHORT $LN2@CPUID
        xor     al, al
        jmp     SHORT $LN1@CPUID
$LN2@CPUID:
        mov     ecx, DWORD PTR _u_eax_$[ebp]
        mov     edx, DWORD PTR _u_eax$[ebp]
        mov     DWORD PTR [ecx], edx
        mov     eax, DWORD PTR _u_ebx_$[ebp]
        mov     ecx, DWORD PTR _u_ebx$[ebp]
        mov     DWORD PTR [eax], ecx
        mov     edx, DWORD PTR _u_ecx_$[ebp]
        mov     eax, DWORD PTR _u_ecx$[ebp]
        mov     DWORD PTR [edx], eax
        mov     ecx, DWORD PTR _u_edx_$[ebp]
        mov     edx, DWORD PTR _u_edx$[ebp]
        mov     DWORD PTR [ecx], edx
        mov     al, 1
$LN1@CPUID:
        mov     esp, ebp
        pop     ebp
        ret     20                                        ; 00000014H
bool CPUDetectClass::CPUID(unsigned int &,unsigned int &,unsigned int &,unsigned int &,unsigned int) ENDP          ; CPUDetectClass::CPUID
```

</td>
<td>

```asm
_u_edx$ = -20                                     ; size = 4
_u_ecx$ = -16                                     ; size = 4
_u_ebx$ = -12                                     ; size = 4
_u_eax$ = -8                                            ; size = 4
_this$ = -4                                   ; size = 4
_u_eax_$ = 8                                            ; size = 4
_u_ebx_$ = 12                                     ; size = 4
_u_ecx_$ = 16                                     ; size = 4
_u_edx_$ = 20                                     ; size = 4
_cpuid_type$ = 24                                 ; size = 4
bool CPUDetectClass::CPUID(unsigned int &,unsigned int &,unsigned int &,unsigned int &,unsigned int) PROC          ; CPUDetectClass::CPUID
        push    ebp
        mov     ebp, esp
        sub     esp, 20                             ; 00000014H
        mov     DWORD PTR _this$[ebp], ecx
        mov     ecx, DWORD PTR _this$[ebp]
        call    bool CPUDetectClass::Has_CPUID_Instruction(void) ; CPUDetectClass::Has_CPUID_Instruction
        movzx   eax, al
        test    eax, eax
        jne     SHORT $LN2@CPUID
        xor     al, al
        jmp     SHORT $LN1@CPUID
$LN2@CPUID:
        mov     ecx, DWORD PTR _u_eax_$[ebp]
        mov     edx, DWORD PTR _u_eax$[ebp]
        mov     DWORD PTR [ecx], edx
        mov     eax, DWORD PTR _u_ebx_$[ebp]
        mov     ecx, DWORD PTR _u_ebx$[ebp]
        mov     DWORD PTR [eax], ecx
        mov     edx, DWORD PTR _u_ecx_$[ebp]
        mov     eax, DWORD PTR _u_ecx$[ebp]
        mov     DWORD PTR [edx], eax
        mov     ecx, DWORD PTR _u_edx_$[ebp]
        mov     edx, DWORD PTR _u_edx$[ebp]
        mov     DWORD PTR [ecx], edx
        mov     al, 1
$LN1@CPUID:
        mov     esp, ebp
        pop     ebp
        ret     20                                        ; 00000014H
bool CPUDetectClass::CPUID(unsigned int &,unsigned int &,unsigned int &,unsigned int &,unsigned int) ENDP          ; CPUDetectClass::CPUID
```

</td>
</table>

The generated assemblies have **no** difference.

</details>

</details>

---

<details>
<summary>GeneralsMD/Code/Libraries/Source/debug/debug_except.cpp</summary>

```c++
void DebugExceptionhandler::LogFPURegisters(Debug &dbg, struct _EXCEPTION_POINTERS *exptr)
{
  struct _CONTEXT &ctx=*exptr->ContextRecord;

  if (!(ctx.ContextFlags&CONTEXT_FLOATING_POINT))
  {
    dbg << "FP registers not available\n";
    return;
  }

  FLOATING_SAVE_AREA &flt=ctx.FloatSave;
  dbg << Debug::Bin() << Debug::FillChar('0')
      << "CW:" << Debug::Width(16) << (flt.ControlWord&0xffff) << "\n"
      << "SW:" << Debug::Width(16) << (flt.StatusWord&0xffff) << "\n"
      << "TW:" << Debug::Width(16) << (flt.TagWord&0xffff) << "\n"
      << Debug::Hex() 
      << "ErrOfs:      " << Debug::Width(8) << flt.ErrorOffset
      << " ErrSel:  "    << Debug::Width(8) << flt.ErrorSelector << "\n"
      << "DataOfs:     " << Debug::Width(8) << flt.DataOffset
      << " DataSel: "    << Debug::Width(8) << flt.DataSelector << "\n"
      << "Cr0NpxState: " << Debug::Width(8) << flt.Cr0NpxState << "\n";

  for (unsigned k=0;k<SIZE_OF_80387_REGISTERS/10;++k)
  {
    dbg << Debug::Dec() << "ST(" << k << ") ";
    dbg.SetPrefixAndRadix("",16);

    BYTE *value=flt.RegisterArea+k*10;
    for (unsigned i=0;i<10;i++)
      dbg << Debug::Width(2) << value[i];

    double fpVal;

    // convert from temporary real (10 byte) to double
    _asm
    {
      mov eax,value
      fld tbyte ptr [eax]
      fstp qword ptr [fpVal]
    }

    dbg << " " << fpVal << "\n";
  }
  dbg << Debug::FillChar() << Debug::Dec();
}
```

This is doing a fast conversion from a `long double` to a `double`.

> **NOTE**: I have extracted the assembly code to its own function:
> ```c++
> #include <string.h>
>
> void ConvertToDouble(const long double* value, double* fpVal) {
>   _asm
>   {
>   mov eax,value
>   fld tbyte ptr [eax]
>   fstp qword ptr [fpVal]
>   }
> }
>
> ```

My goto equivalent is:

```c++
#include <string.h>

void ConvertToDouble(const long double* value, double* fpVal) {
    *fpVal = static_cast<double>(*value);
}
```

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
_value$ = 8                                   ; size = 4
_fpVal$ = 12                                            ; size = 4
void ConvertToDouble(long double const *,double *) PROC                  ; ConvertToDouble
        push    ebp
        mov     ebp, esp
        mov     eax, DWORD PTR _value$[ebp]
        fld     TBYTE PTR [eax]
        fstp    QWORD PTR _fpVal$[ebp]
        pop     ebp
        ret     0
void ConvertToDouble(long double const *,double *) ENDP                  ; ConvertToDouble
```

</td>
<td>

```asm
_value$ = 8                                   ; size = 4
_fpVal$ = 12                                            ; size = 4
void ConvertToDouble(long double const *,double *) PROC                  ; ConvertToDouble
        push    ebp
        mov     ebp, esp
        mov     eax, DWORD PTR _fpVal$[ebp]
        mov     ecx, DWORD PTR _value$[ebp]
        movsd   xmm0, QWORD PTR [ecx]
        movsd   QWORD PTR [eax], xmm0
        pop     ebp
        ret     0
void ConvertToDouble(long double const *,double *) ENDP                  ; ConvertToDouble
```

I don't think I will be able to make this identical, but the modern compilers seem to do a great job at optimizing even
without optimization flags.

```diff
@@ -3,9 +3,10 @@ _fpVal$ = 12                                            ; size = 4
 void ConvertToDouble(long double const *,double *) PROC                  ; ConvertToDouble
         push    ebp
         mov     ebp, esp
-        mov     eax, DWORD PTR _value$[ebp]
-        fld     TBYTE PTR [eax]
-        fstp    QWORD PTR _fpVal$[ebp]
+        mov     eax, DWORD PTR _fpVal$[ebp]
+        mov     ecx, DWORD PTR _value$[ebp]
+        movsd   xmm0, QWORD PTR [ecx]
+        movsd   QWORD PTR [eax], xmm0
         pop     ebp
         ret     0
 void ConvertToDouble(long double const *,double *) ENDP                  ; ConvertToDouble
```

</td>
</table>

</details>

---

<details>
<summary>GeneralsMD/Code/Libraries/Source/debug/debug_stack.cpp</summary>

```c++
int DebugStackwalk::StackWalk(Signature &sig, struct _CONTEXT *ctx)
{
  InitDbghelp();

  sig.m_numAddr=0;

  // bail out if no stack walk available
  if (!gDbg._StackWalk)
    return 0;

	// Set up the stack frame structure for the start point of the stack walk (i.e. here).
	STACKFRAME stackFrame;
	memset(&stackFrame,0,sizeof(stackFrame));

	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;

	// Use the context struct if it was provided.
	if (ctx) 
  {
		stackFrame.AddrPC.Offset = ctx->Eip;
		stackFrame.AddrStack.Offset = ctx->Esp;
		stackFrame.AddrFrame.Offset = ctx->Ebp;
	}
  else
  {
    // walk stack back using current call chain
	  unsigned long reg_eip, reg_ebp, reg_esp;
	  __asm 
    {
    here:
		  lea	eax,here
		  mov	reg_eip,eax
		  mov	reg_ebp,ebp
		  mov	reg_esp,esp
	  };
	  stackFrame.AddrPC.Offset = reg_eip;
	  stackFrame.AddrStack.Offset = reg_esp;
	  stackFrame.AddrFrame.Offset = reg_ebp;
  }

	// Walk the stack by the requested number of return address iterations.
  bool skipFirst=!ctx;
  while (sig.m_numAddr<Signature::MAX_ADDR&&
		     gDbg._StackWalk(IMAGE_FILE_MACHINE_I386,GetCurrentProcess(),GetCurrentThread(),
                         &stackFrame,NULL,NULL,gDbg._SymFunctionTableAccess,gDbg._SymGetModuleBase,NULL))
  {
    if (skipFirst)
      skipFirst=false;
    else
      sig.m_addr[sig.m_numAddr++]=stackFrame.AddrPC.Offset;
  }

	return sig.m_numAddr;
}
```

This assembly is trying to get the values of the registers `EIP`, `ESP` and `EBP`.

> **NOTE**: I have extracted the assembly code to its own function:
> #include <windows.h>
> #include <dbghelp.h>
>
> void CaptureStackInfo(CONTEXT& threadContext, STACKFRAME& stackFrame) {
> // walk stack back using current call chain
> unsigned long reg_eip, reg_ebp, reg_esp;
> __asm
> {
> here:
> lea eax,here
> mov reg_eip,eax
> mov reg_ebp,ebp
> mov reg_esp,esp
> };
> stackFrame.AddrPC.Offset = reg_eip;
> stackFrame.AddrStack.Offset = reg_esp;
> stackFrame.AddrFrame.Offset = reg_ebp;
> }

My goto equivalent is:

```c++
#include <windows.h>
#include <dbghelp.h>

void CaptureStackInfo(CONTEXT& threadContext, STACKFRAME& stackFrame) {
    // Capture the current thread's context to extract EIP, ESP, and EBP registers.
    memset(&threadContext, 0, sizeof(CONTEXT));
    threadContext.ContextFlags = CONTEXT_CONTROL;

    RtlCaptureContext(&threadContext); // Retrieves the current thread's register state.

    // Assign values to the stack frame.
    stackFrame.AddrPC.Offset = threadContext.Eip;      // Program Counter (EIP)
    stackFrame.AddrPC.Mode = AddrModeFlat;

    stackFrame.AddrStack.Offset = threadContext.Esp;   // Stack Pointer (ESP)
    stackFrame.AddrStack.Mode = AddrModeFlat;

    stackFrame.AddrFrame.Offset = threadContext.Ebp;   // Base Pointer (EBP)
    stackFrame.AddrFrame.Mode = AddrModeFlat;
}
```

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
_reg_ebp$ = -12                               ; size = 4
_reg_esp$ = -8                                      ; size = 4
_reg_eip$ = -4                                      ; size = 4
_threadContext$ = 8                           ; size = 4
_stackFrame$ = 12                                 ; size = 4
void CaptureStackInfo(_CONTEXT &,_tagSTACKFRAME &) PROC ; CaptureStackInfo
        push    ebp
        mov     ebp, esp
        sub     esp, 12                             ; 0000000cH
$here$3:
        lea     eax, OFFSET $here$3
        mov     DWORD PTR _reg_eip$[ebp], eax
        mov     DWORD PTR _reg_ebp$[ebp], ebp
        mov     DWORD PTR _reg_esp$[ebp], esp
        mov     eax, DWORD PTR _stackFrame$[ebp]
        mov     ecx, DWORD PTR _reg_eip$[ebp]
        mov     DWORD PTR [eax], ecx
        mov     edx, DWORD PTR _stackFrame$[ebp]
        mov     eax, DWORD PTR _reg_esp$[ebp]
        mov     DWORD PTR [edx+36], eax
        mov     ecx, DWORD PTR _stackFrame$[ebp]
        mov     edx, DWORD PTR _reg_ebp$[ebp]
        mov     DWORD PTR [ecx+24], edx
        mov     esp, ebp
        pop     ebp
        ret     0
void CaptureStackInfo(_CONTEXT &,_tagSTACKFRAME &) ENDP ; CaptureStackInfo
```

</td>
<td>

```asm
_threadContext$ = 8                           ; size = 4
_stackFrame$ = 12                                 ; size = 4
void CaptureStackInfo(_CONTEXT &,_tagSTACKFRAME &) PROC ; CaptureStackInfo
        push    ebp
        mov     ebp, esp
        push    716                           ; 000002ccH
        push    0
        mov     eax, DWORD PTR _threadContext$[ebp]
        push    eax
        call    _memset
        add     esp, 12                             ; 0000000cH
        mov     ecx, DWORD PTR _threadContext$[ebp]
        mov     DWORD PTR [ecx], 65537                    ; 00010001H
        mov     edx, DWORD PTR _threadContext$[ebp]
        push    edx
        call    DWORD PTR __imp__RtlCaptureContext@4
        mov     eax, DWORD PTR _stackFrame$[ebp]
        mov     ecx, DWORD PTR _threadContext$[ebp]
        mov     edx, DWORD PTR [ecx+184]
        mov     DWORD PTR [eax], edx
        mov     eax, DWORD PTR _stackFrame$[ebp]
        mov     DWORD PTR [eax+8], 3
        mov     ecx, DWORD PTR _stackFrame$[ebp]
        mov     edx, DWORD PTR _threadContext$[ebp]
        mov     eax, DWORD PTR [edx+196]
        mov     DWORD PTR [ecx+36], eax
        mov     ecx, DWORD PTR _stackFrame$[ebp]
        mov     DWORD PTR [ecx+44], 3
        mov     edx, DWORD PTR _stackFrame$[ebp]
        mov     eax, DWORD PTR _threadContext$[ebp]
        mov     ecx, DWORD PTR [eax+180]
        mov     DWORD PTR [edx+24], ecx
        mov     edx, DWORD PTR _stackFrame$[ebp]
        mov     DWORD PTR [edx+32], 3
        pop     ebp
        ret     0
void CaptureStackInfo(_CONTEXT &,_tagSTACKFRAME &) ENDP ; CaptureStackInfo
```

I don't think I will be able to make this identical, but the modern compilers optimizations might help.

```diff
@@ -1,27 +1,37 @@
-_reg_ebp$ = -12                               ; size = 4
-_reg_esp$ = -8                                      ; size = 4
-_reg_eip$ = -4                                      ; size = 4
 _threadContext$ = 8                           ; size = 4
 _stackFrame$ = 12                                 ; size = 4
 void CaptureStackInfo(_CONTEXT &,_tagSTACKFRAME &) PROC ; CaptureStackInfo
         push    ebp
         mov     ebp, esp
-        sub     esp, 12                             ; 0000000cH
-$here$3:
-        lea     eax, OFFSET $here$3
-        mov     DWORD PTR _reg_eip$[ebp], eax
-        mov     DWORD PTR _reg_ebp$[ebp], ebp
-        mov     DWORD PTR _reg_esp$[ebp], esp
+        push    716                           ; 000002ccH
+        push    0
+        mov     eax, DWORD PTR _threadContext$[ebp]
+        push    eax
+        call    _memset
+        add     esp, 12                             ; 0000000cH
+        mov     ecx, DWORD PTR _threadContext$[ebp]
+        mov     DWORD PTR [ecx], 65537                    ; 00010001H
+        mov     edx, DWORD PTR _threadContext$[ebp]
+        push    edx
+        call    DWORD PTR __imp__RtlCaptureContext@4
         mov     eax, DWORD PTR _stackFrame$[ebp]
-        mov     ecx, DWORD PTR _reg_eip$[ebp]
-        mov     DWORD PTR [eax], ecx
-        mov     edx, DWORD PTR _stackFrame$[ebp]
-        mov     eax, DWORD PTR _reg_esp$[ebp]
-        mov     DWORD PTR [edx+36], eax
+        mov     ecx, DWORD PTR _threadContext$[ebp]
+        mov     edx, DWORD PTR [ecx+184]
+        mov     DWORD PTR [eax], edx
+        mov     eax, DWORD PTR _stackFrame$[ebp]
+        mov     DWORD PTR [eax+8], 3
+        mov     ecx, DWORD PTR _stackFrame$[ebp]
+        mov     edx, DWORD PTR _threadContext$[ebp]
+        mov     eax, DWORD PTR [edx+196]
+        mov     DWORD PTR [ecx+36], eax
         mov     ecx, DWORD PTR _stackFrame$[ebp]
-        mov     edx, DWORD PTR _reg_ebp$[ebp]
-        mov     DWORD PTR [ecx+24], edx
-        mov     esp, ebp
+        mov     DWORD PTR [ecx+44], 3
+        mov     edx, DWORD PTR _stackFrame$[ebp]
+        mov     eax, DWORD PTR _threadContext$[ebp]
+        mov     ecx, DWORD PTR [eax+180]
+        mov     DWORD PTR [edx+24], ecx
+        mov     edx, DWORD PTR _stackFrame$[ebp]
+        mov     DWORD PTR [edx+32], 3
         pop     ebp
         ret     0
 void CaptureStackInfo(_CONTEXT &,_tagSTACKFRAME &) ENDP ; CaptureStackInfo
```

</td>
</table>

</details>

---

<details>
<summary>Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h</summary>

This file includes the following assembly blocks:

<details>
<summary>DX8Wrapper::Convert_Color</summary>

```c++
WWINLINE unsigned int DX8Wrapper::Convert_Color(const Vector3& color,float alpha)
{
	const float scale = 255.0;
	unsigned int col;

	// Multiply r, g, b and a components (0.0,...,1.0) by 255 and convert to integer. Or the integer values togerher
	// such that 32 bit ingeger has AAAAAAAARRRRRRRRGGGGGGGGBBBBBBBB.
	__asm
	{
		sub	esp,20					// space for a, r, g and b float plus fpu rounding mode

		// Store the fpu rounding mode

		fwait
		fstcw		[esp+16]				// store control word to stack
		mov		eax,[esp+16]		// load it to eax
		mov		edi,eax				// take copy
		and		eax,~(1024|2048)	// mask out certain bits
		or			eax,(1024|2048)	// or with precision control value "truncate"
		sub		edi,eax				// did it change?
		jz			skip					// .. if not, skip
		mov		[esp],eax			// .. change control word
		fldcw		[esp]
skip:

		// Convert the color

		mov	esi,dword ptr color
		fld	dword ptr[scale]

		fld	dword ptr[esi]			// r
		fld	dword ptr[esi+4]		// g
		fld	dword ptr[esi+8]		// b
		fld	dword ptr[alpha]		// a
		fld	st(4)
		fmul	st(4),st
		fmul	st(3),st
		fmul	st(2),st
		fmulp	st(1),st
		fistp	dword ptr[esp+0]		// a
		fistp	dword ptr[esp+4]		// b
		fistp	dword ptr[esp+8]		// g
		fistp	dword ptr[esp+12]		// r
		mov	ecx,[esp]				// a
		mov	eax,[esp+4]				// b
		mov	edx,[esp+8]				// g
		mov	ebx,[esp+12]			// r
		shl	ecx,24					// a << 24
		shl	ebx,16					// r << 16
		shl	edx,8						//	g << 8
		or		eax,ecx					// (a << 24) | b
		or		eax,ebx					// (a << 24) | (r << 16) | b
		or		eax,edx					// (a << 24) | (r << 16) | (g << 8) | b
		
		fstp	st(0)

		// Restore fpu rounding mode

		cmp	edi,0					// did we change the value?
		je		not_changed			// nope... skip now...
		fwait
		fldcw	[esp+16];
not_changed:
		add	esp,20

		mov	col,eax
	}
	return col;
}
```

This is converting a Vector3 color (RGB) and return an unsigned integer in the form ARGB.

My goto equivalent is:

```c++
#include <math.h>

WWINLINE unsigned int DX8Wrapper::Convert_Color(const Vector3& color,float alpha)
{
	const float scale = 255.0;

	// Multiply r, g, b and a components (0.0,...,1.0) by 255 and convert to integer. Or the integer values togerher
	// such that 32 bit ingeger has AAAAAAAARRRRRRRRGGGGGGGGBBBBBBBB.
    unsigned int r = (unsigned int)(floor(color.x * scale));
    unsigned int g = (unsigned int)(floor(color.y * scale));
    unsigned int b = (unsigned int)(floor(color.z * scale));
    unsigned int a = (unsigned int)(floor(alpha * scale));
    
    return (a << 24) | (r << 16) | (g << 8) | b;
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
__real@437f0000 DD 0437f0000r             ; 255

_this$ = -12                                            ; size = 4
_col$ = -8                                          ; size = 4
_scale$ = -4                                            ; size = 4
_color$ = 8                                   ; size = 4
_alpha$ = 12                                            ; size = 4
unsigned int DX8Wrapper::Convert_Color(Vector3 const &,float) PROC     ; DX8Wrapper::Convert_Color
        push    ebp
        mov     ebp, esp
        sub     esp, 12                             ; 0000000cH
        push    ebx
        push    esi
        push    edi
        mov     DWORD PTR _this$[ebp], ecx
        movss   xmm0, DWORD PTR __real@437f0000
        movss   DWORD PTR _scale$[ebp], xmm0
        sub     esp, 20                             ; 00000014H
        fwait
        fstcw   TBYTE PTR [esp+16]
        mov     eax, DWORD PTR [esp+16]
        mov     edi, eax
        and     eax, -3073                                ; fffff3ffH
        or      eax, 3072                     ; 00000c00H
        sub     edi, eax
        je      SHORT $skip$3
        mov     DWORD PTR [esp], eax
        fldcw   TBYTE PTR [esp]
$skip$3:
        mov     esi, DWORD PTR _color$[ebp]
        fld     DWORD PTR _scale$[ebp]
        fld     DWORD PTR [esi]
        fld     DWORD PTR [esi+4]
        fld     DWORD PTR [esi+8]
        fld     DWORD PTR _alpha$[ebp]
        fld     ST(4)
        fmul    ST(4), ST(0)
        fmul    ST(3), ST(0)
        fmul    ST(2), ST(0)
        fmulp   ST(1), ST(0)
        fistp   DWORD PTR [esp]
        fistp   DWORD PTR [esp+4]
        fistp   DWORD PTR [esp+8]
        fistp   DWORD PTR [esp+12]
        mov     ecx, DWORD PTR [esp]
        mov     eax, DWORD PTR [esp+4]
        mov     edx, DWORD PTR [esp+8]
        mov     ebx, DWORD PTR [esp+12]
        shl     ecx, 24                             ; 00000018H
        shl     ebx, 16                             ; 00000010H
        shl     edx, 8
        or      eax, ecx
        or      eax, ebx
        or      eax, edx
        fstp    ST(0)
        cmp     edi, 0
        je      SHORT $not_changed$4
        fwait
        fldcw   TBYTE PTR [esp+16]
$not_changed$4:
        add     esp, 20                             ; 00000014H
        mov     DWORD PTR _col$[ebp], eax
        mov     eax, DWORD PTR _col$[ebp]
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret     8
unsigned int DX8Wrapper::Convert_Color(Vector3 const &,float) ENDP     ; DX8Wrapper::Convert_Color
```

</td>
<td>

```asm
__real@437f0000 DD 0437f0000r             ; 255

_scale$ = -24                                     ; size = 4
_this$ = -20                                            ; size = 4
_b$ = -16                                         ; size = 4
_g$ = -12                                         ; size = 4
_r$ = -8                                                ; size = 4
_a$ = -4                                                ; size = 4
_color$ = 8                                   ; size = 4
_alpha$ = 12                                            ; size = 4
unsigned int DX8Wrapper::Convert_Color(Vector3 const &,float) PROC     ; DX8Wrapper::Convert_Color
        push    ebp
        mov     ebp, esp
        sub     esp, 24                             ; 00000018H
        mov     DWORD PTR _this$[ebp], ecx
        movss   xmm0, DWORD PTR __real@437f0000
        movss   DWORD PTR _scale$[ebp], xmm0
        mov     eax, DWORD PTR _color$[ebp]
        movss   xmm0, DWORD PTR [eax]
        mulss   xmm0, DWORD PTR __real@437f0000
        cvtss2sd xmm0, xmm0
        sub     esp, 8
        movsd   QWORD PTR [esp], xmm0
        call    _floor
        add     esp, 8
        call    __ftol2
        mov     DWORD PTR _r$[ebp], eax
        mov     ecx, DWORD PTR _color$[ebp]
        movss   xmm0, DWORD PTR [ecx+4]
        mulss   xmm0, DWORD PTR __real@437f0000
        cvtss2sd xmm0, xmm0
        sub     esp, 8
        movsd   QWORD PTR [esp], xmm0
        call    _floor
        add     esp, 8
        call    __ftol2
        mov     DWORD PTR _g$[ebp], eax
        mov     edx, DWORD PTR _color$[ebp]
        movss   xmm0, DWORD PTR [edx+8]
        mulss   xmm0, DWORD PTR __real@437f0000
        cvtss2sd xmm0, xmm0
        sub     esp, 8
        movsd   QWORD PTR [esp], xmm0
        call    _floor
        add     esp, 8
        call    __ftol2
        mov     DWORD PTR _b$[ebp], eax
        movss   xmm0, DWORD PTR _alpha$[ebp]
        mulss   xmm0, DWORD PTR __real@437f0000
        cvtss2sd xmm0, xmm0
        sub     esp, 8
        movsd   QWORD PTR [esp], xmm0
        call    _floor
        add     esp, 8
        call    __ftol2
        mov     DWORD PTR _a$[ebp], eax
        mov     eax, DWORD PTR _a$[ebp]
        shl     eax, 24                             ; 00000018H
        mov     ecx, DWORD PTR _r$[ebp]
        shl     ecx, 16                             ; 00000010H
        or      eax, ecx
        mov     edx, DWORD PTR _g$[ebp]
        shl     edx, 8
        or      eax, edx
        or      eax, DWORD PTR _b$[ebp]
        mov     esp, ebp
        pop     ebp
        ret     8
unsigned int DX8Wrapper::Convert_Color(Vector3 const &,float) ENDP     ; DX8Wrapper::Convert_Color
```

</td>
</table>

While both assemblies are very different, they reach the same output. It would be worth investigating which is more
efficient in modern architectures, or even if the efficiency difference is worth it. I am calling `floor` because of the
`fld` in assembly, it will be the way to make it work the same, even if it changes the assembly output.

```diff
@@ -1,69 +1,68 @@
 __real@437f0000 DD 0437f0000r             ; 255
 
-_this$ = -12                                            ; size = 4
-_col$ = -8                                          ; size = 4
-_scale$ = -4                                            ; size = 4
+_scale$ = -24                                     ; size = 4
+_this$ = -20                                            ; size = 4
+_b$ = -16                                         ; size = 4
+_g$ = -12                                         ; size = 4
+_r$ = -8                                                ; size = 4
+_a$ = -4                                                ; size = 4
 _color$ = 8                                   ; size = 4
 _alpha$ = 12                                            ; size = 4
 unsigned int DX8Wrapper::Convert_Color(Vector3 const &,float) PROC     ; DX8Wrapper::Convert_Color
         push    ebp
         mov     ebp, esp
-        sub     esp, 12                             ; 0000000cH
-        push    ebx
-        push    esi
-        push    edi
+        sub     esp, 24                             ; 00000018H
         mov     DWORD PTR _this$[ebp], ecx
         movss   xmm0, DWORD PTR __real@437f0000
         movss   DWORD PTR _scale$[ebp], xmm0
-        sub     esp, 20                             ; 00000014H
-        fwait
-        fstcw   TBYTE PTR [esp+16]
-        mov     eax, DWORD PTR [esp+16]
-        mov     edi, eax
-        and     eax, -3073                                ; fffff3ffH
-        or      eax, 3072                     ; 00000c00H
-        sub     edi, eax
-        je      SHORT $skip$3
-        mov     DWORD PTR [esp], eax
-        fldcw   TBYTE PTR [esp]
-$skip$3:
-        mov     esi, DWORD PTR _color$[ebp]
-        fld     DWORD PTR _scale$[ebp]
-        fld     DWORD PTR [esi]
-        fld     DWORD PTR [esi+4]
-        fld     DWORD PTR [esi+8]
-        fld     DWORD PTR _alpha$[ebp]
-        fld     ST(4)
-        fmul    ST(4), ST(0)
-        fmul    ST(3), ST(0)
-        fmul    ST(2), ST(0)
-        fmulp   ST(1), ST(0)
-        fistp   DWORD PTR [esp]
-        fistp   DWORD PTR [esp+4]
-        fistp   DWORD PTR [esp+8]
-        fistp   DWORD PTR [esp+12]
-        mov     ecx, DWORD PTR [esp]
-        mov     eax, DWORD PTR [esp+4]
-        mov     edx, DWORD PTR [esp+8]
-        mov     ebx, DWORD PTR [esp+12]
-        shl     ecx, 24                             ; 00000018H
-        shl     ebx, 16                             ; 00000010H
-        shl     edx, 8
+        mov     eax, DWORD PTR _color$[ebp]
+        movss   xmm0, DWORD PTR [eax]
+        mulss   xmm0, DWORD PTR __real@437f0000
+        cvtss2sd xmm0, xmm0
+        sub     esp, 8
+        movsd   QWORD PTR [esp], xmm0
+        call    _floor
+        add     esp, 8
+        call    __ftol2
+        mov     DWORD PTR _r$[ebp], eax
+        mov     ecx, DWORD PTR _color$[ebp]
+        movss   xmm0, DWORD PTR [ecx+4]
+        mulss   xmm0, DWORD PTR __real@437f0000
+        cvtss2sd xmm0, xmm0
+        sub     esp, 8
+        movsd   QWORD PTR [esp], xmm0
+        call    _floor
+        add     esp, 8
+        call    __ftol2
+        mov     DWORD PTR _g$[ebp], eax
+        mov     edx, DWORD PTR _color$[ebp]
+        movss   xmm0, DWORD PTR [edx+8]
+        mulss   xmm0, DWORD PTR __real@437f0000
+        cvtss2sd xmm0, xmm0
+        sub     esp, 8
+        movsd   QWORD PTR [esp], xmm0
+        call    _floor
+        add     esp, 8
+        call    __ftol2
+        mov     DWORD PTR _b$[ebp], eax
+        movss   xmm0, DWORD PTR _alpha$[ebp]
+        mulss   xmm0, DWORD PTR __real@437f0000
+        cvtss2sd xmm0, xmm0
+        sub     esp, 8
+        movsd   QWORD PTR [esp], xmm0
+        call    _floor
+        add     esp, 8
+        call    __ftol2
+        mov     DWORD PTR _a$[ebp], eax
+        mov     eax, DWORD PTR _a$[ebp]
+        shl     eax, 24                             ; 00000018H
+        mov     ecx, DWORD PTR _r$[ebp]
+        shl     ecx, 16                             ; 00000010H
         or      eax, ecx
-        or      eax, ebx
+        mov     edx, DWORD PTR _g$[ebp]
+        shl     edx, 8
         or      eax, edx
-        fstp    ST(0)
-        cmp     edi, 0
-        je      SHORT $not_changed$4
-        fwait
-        fldcw   TBYTE PTR [esp+16]
-$not_changed$4:
-        add     esp, 20                             ; 00000014H
-        mov     DWORD PTR _col$[ebp], eax
-        mov     eax, DWORD PTR _col$[ebp]
-        pop     edi
-        pop     esi
-        pop     ebx
+        or      eax, DWORD PTR _b$[ebp]
         mov     esp, ebp
         pop     ebp
         ret     8
```

</details>

<details>
<summary>DX8Wrapper::Clamp_Color</summary>

```c++
WWINLINE void DX8Wrapper::Clamp_Color(Vector4& color)
{
	if (!CPUDetectClass::Has_CMOV_Instruction()) {
		for (int i=0;i<4;++i) {
			float f=(color[i]<0.0f) ? 0.0f : color[i];
			color[i]=(f>1.0f) ? 1.0f : f;
		}
		return;
	}

	__asm
	{
		mov	esi,dword ptr color

		mov edx,0x3f800000

		mov edi,dword ptr[esi]
		mov ebx,edi
		sar edi,31
		not edi			// mask is now zero if negative value
		and edi,ebx
		cmp edi,edx		// if no less than 1.0 set to 1.0
		cmovnb edi,edx
		mov dword ptr[esi],edi

		mov edi,dword ptr[esi+4]
		mov ebx,edi
		sar edi,31
		not edi			// mask is now zero if negative value
		and edi,ebx
		cmp edi,edx		// if no less than 1.0 set to 1.0
		cmovnb edi,edx
		mov dword ptr[esi+4],edi

		mov edi,dword ptr[esi+8]
		mov ebx,edi
		sar edi,31
		not edi			// mask is now zero if negative value
		and edi,ebx
		cmp edi,edx		// if no less than 1.0 set to 1.0
		cmovnb edi,edx
		mov dword ptr[esi+8],edi

		mov edi,dword ptr[esi+12]
		mov ebx,edi
		sar edi,31
		not edi			// mask is now zero if negative value
		and edi,ebx
		cmp edi,edx		// if no less than 1.0 set to 1.0
		cmovnb edi,edx
		mov dword ptr[esi+12],edi
	}
}
```

This block of assembly is used to clamp the colors between 0F and 1F, as long as the CMOV instruction is available.
Modern C++ compilers should call this instruction automatically if it is available, which makes the code simpler.

My goto equivalent is:

```c++
WWINLINE void DX8Wrapper::Clamp_Color(Vector4& color)
{
    for (int i=0;i<4;++i) {
        float f=(color[i]<0.0f) ? 0.0f : color[i];
        color[i]=(f>1.0f) ? 1.0f : f;
    }
}
```

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
__real@3f800000 DD 03f800000r             ; 1

_this$ = -4                                   ; size = 4
_i$ = 8                                       ; size = 4
float & Vector4::operator[](int) PROC                          ; Vector4::operator[], COMDAT
        push    ebp
        mov     ebp, esp
        push    ecx
        mov     DWORD PTR _this$[ebp], ecx
        mov     eax, DWORD PTR _i$[ebp]
        mov     ecx, DWORD PTR _this$[ebp]
        lea     eax, DWORD PTR [ecx+eax*4]
        mov     esp, ebp
        pop     ebp
        ret     4
float & Vector4::operator[](int) ENDP                          ; Vector4::operator[]

_this$ = -20                                            ; size = 4
tv81 = -16                                          ; size = 4
_f$1 = -12                                          ; size = 4
tv76 = -8                                         ; size = 4
_i$2 = -4                                         ; size = 4
_color$ = 8                                   ; size = 4
void DX8Wrapper::Clamp_Color(Vector4 &) PROC        ; DX8Wrapper::Clamp_Color
        push    ebp
        mov     ebp, esp
        sub     esp, 20                             ; 00000014H
        push    ebx
        push    esi
        push    edi
        mov     DWORD PTR _this$[ebp], ecx
        call    static bool CPUDetectClass::Has_CMOV_Instruction(void) ; CPUDetectClass::Has_CMOV_Instruction
        movzx   eax, al
        test    eax, eax
        jne     $LN5@Clamp_Colo
        mov     DWORD PTR _i$2[ebp], 0
        jmp     SHORT $LN4@Clamp_Colo
$LN2@Clamp_Colo:
        mov     ecx, DWORD PTR _i$2[ebp]
        add     ecx, 1
        mov     DWORD PTR _i$2[ebp], ecx
$LN4@Clamp_Colo:
        cmp     DWORD PTR _i$2[ebp], 4
        jge     SHORT $LN3@Clamp_Colo
        mov     edx, DWORD PTR _i$2[ebp]
        push    edx
        mov     ecx, DWORD PTR _color$[ebp]
        call    float & Vector4::operator[](int)             ; Vector4::operator[]
        xorps   xmm0, xmm0
        comiss  xmm0, DWORD PTR [eax]
        jbe     SHORT $LN7@Clamp_Colo
        xorps   xmm0, xmm0
        movss   DWORD PTR tv76[ebp], xmm0
        jmp     SHORT $LN8@Clamp_Colo
$LN7@Clamp_Colo:
        mov     eax, DWORD PTR _i$2[ebp]
        push    eax
        mov     ecx, DWORD PTR _color$[ebp]
        call    float & Vector4::operator[](int)             ; Vector4::operator[]
        movss   xmm0, DWORD PTR [eax]
        movss   DWORD PTR tv76[ebp], xmm0
$LN8@Clamp_Colo:
        movss   xmm0, DWORD PTR tv76[ebp]
        movss   DWORD PTR _f$1[ebp], xmm0
        movss   xmm0, DWORD PTR _f$1[ebp]
        comiss  xmm0, DWORD PTR __real@3f800000
        jbe     SHORT $LN9@Clamp_Colo
        movss   xmm0, DWORD PTR __real@3f800000
        movss   DWORD PTR tv81[ebp], xmm0
        jmp     SHORT $LN10@Clamp_Colo
$LN9@Clamp_Colo:
        movss   xmm0, DWORD PTR _f$1[ebp]
        movss   DWORD PTR tv81[ebp], xmm0
$LN10@Clamp_Colo:
        mov     ecx, DWORD PTR _i$2[ebp]
        push    ecx
        mov     ecx, DWORD PTR _color$[ebp]
        call    float & Vector4::operator[](int)             ; Vector4::operator[]
        movss   xmm0, DWORD PTR tv81[ebp]
        movss   DWORD PTR [eax], xmm0
        jmp     $LN2@Clamp_Colo
$LN3@Clamp_Colo:
        jmp     SHORT $LN1@Clamp_Colo
$LN5@Clamp_Colo:
        mov     esi, DWORD PTR _color$[ebp]
        mov     edx, 1065353216                     ; 3f800000H
        mov     edi, DWORD PTR [esi]
        mov     ebx, edi
        sar     edi, 31                             ; 0000001fH
        not     edi
        and     edi, ebx
        cmp     edi, edx
        cmovae  edi, edx
        mov     DWORD PTR [esi], edi
        mov     edi, DWORD PTR [esi+4]
        mov     ebx, edi
        sar     edi, 31                             ; 0000001fH
        not     edi
        and     edi, ebx
        cmp     edi, edx
        cmovae  edi, edx
        mov     DWORD PTR [esi+4], edi
        mov     edi, DWORD PTR [esi+8]
        mov     ebx, edi
        sar     edi, 31                             ; 0000001fH
        not     edi
        and     edi, ebx
        cmp     edi, edx
        cmovae  edi, edx
        mov     DWORD PTR [esi+8], edi
        mov     edi, DWORD PTR [esi+12]
        mov     ebx, edi
        sar     edi, 31                             ; 0000001fH
        not     edi
        and     edi, ebx
        cmp     edi, edx
        cmovae  edi, edx
        mov     DWORD PTR [esi+12], edi
$LN1@Clamp_Colo:
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret     4
void DX8Wrapper::Clamp_Color(Vector4 &) ENDP        ; DX8Wrapper::Clamp_Color
```

</td>
<td>

```asm
__real@3f800000 DD 03f800000r             ; 1

_this$ = -4                                   ; size = 4
_i$ = 8                                       ; size = 4
float & Vector4::operator[](int) PROC                          ; Vector4::operator[], COMDAT
        push    ebp
        mov     ebp, esp
        push    ecx
        mov     DWORD PTR _this$[ebp], ecx
        mov     eax, DWORD PTR _i$[ebp]
        mov     ecx, DWORD PTR _this$[ebp]
        lea     eax, DWORD PTR [ecx+eax*4]
        mov     esp, ebp
        pop     ebp
        ret     4
float & Vector4::operator[](int) ENDP                          ; Vector4::operator[]

_this$ = -20                                            ; size = 4
tv78 = -16                                          ; size = 4
_f$1 = -12                                          ; size = 4
tv73 = -8                                         ; size = 4
_i$2 = -4                                         ; size = 4
_color$ = 8                                   ; size = 4
void DX8Wrapper::Clamp_Color(Vector4 &) PROC        ; DX8Wrapper::Clamp_Color
        push    ebp
        mov     ebp, esp
        sub     esp, 20                             ; 00000014H
        mov     DWORD PTR _this$[ebp], ecx
        mov     DWORD PTR _i$2[ebp], 0
        jmp     SHORT $LN4@Clamp_Colo
$LN2@Clamp_Colo:
        mov     eax, DWORD PTR _i$2[ebp]
        add     eax, 1
        mov     DWORD PTR _i$2[ebp], eax
$LN4@Clamp_Colo:
        cmp     DWORD PTR _i$2[ebp], 4
        jge     SHORT $LN3@Clamp_Colo
        mov     ecx, DWORD PTR _i$2[ebp]
        push    ecx
        mov     ecx, DWORD PTR _color$[ebp]
        call    float & Vector4::operator[](int)             ; Vector4::operator[]
        xorps   xmm0, xmm0
        comiss  xmm0, DWORD PTR [eax]
        jbe     SHORT $LN6@Clamp_Colo
        xorps   xmm0, xmm0
        movss   DWORD PTR tv73[ebp], xmm0
        jmp     SHORT $LN7@Clamp_Colo
$LN6@Clamp_Colo:
        mov     edx, DWORD PTR _i$2[ebp]
        push    edx
        mov     ecx, DWORD PTR _color$[ebp]
        call    float & Vector4::operator[](int)             ; Vector4::operator[]
        movss   xmm0, DWORD PTR [eax]
        movss   DWORD PTR tv73[ebp], xmm0
$LN7@Clamp_Colo:
        movss   xmm0, DWORD PTR tv73[ebp]
        movss   DWORD PTR _f$1[ebp], xmm0
        movss   xmm0, DWORD PTR _f$1[ebp]
        comiss  xmm0, DWORD PTR __real@3f800000
        jbe     SHORT $LN8@Clamp_Colo
        movss   xmm0, DWORD PTR __real@3f800000
        movss   DWORD PTR tv78[ebp], xmm0
        jmp     SHORT $LN9@Clamp_Colo
$LN8@Clamp_Colo:
        movss   xmm0, DWORD PTR _f$1[ebp]
        movss   DWORD PTR tv78[ebp], xmm0
$LN9@Clamp_Colo:
        mov     eax, DWORD PTR _i$2[ebp]
        push    eax
        mov     ecx, DWORD PTR _color$[ebp]
        call    float & Vector4::operator[](int)             ; Vector4::operator[]
        movss   xmm0, DWORD PTR tv78[ebp]
        movss   DWORD PTR [eax], xmm0
        jmp     $LN2@Clamp_Colo
$LN3@Clamp_Colo:
        mov     esp, ebp
        pop     ebp
        ret     4
void DX8Wrapper::Clamp_Color(Vector4 &) ENDP        ; DX8Wrapper::Clamp_Color
```

</td>
</table>

While the assemblies are different and the C++ solution doesn't use the `CMOV` instruction, both reach the same result.
Either use inline assembly like in the original for the `CMOV` or use optimizations in a modern compiler, which will
further help with more than just `CMOV`.

Even then, the modern compiler has generated a smaller assembly output than the original, and with `/O2` it should be
just as, if not more, performant.

```diff
@@ -16,110 +16,63 @@ float & Vector4::operator[](int) PROC                          ; Vector4::operat
 float & Vector4::operator[](int) ENDP                          ; Vector4::operator[]
 
 _this$ = -20                                            ; size = 4
-tv81 = -16                                          ; size = 4
+tv78 = -16                                          ; size = 4
 _f$1 = -12                                          ; size = 4
-tv76 = -8                                         ; size = 4
+tv73 = -8                                         ; size = 4
 _i$2 = -4                                         ; size = 4
 _color$ = 8                                   ; size = 4
 void DX8Wrapper::Clamp_Color(Vector4 &) PROC        ; DX8Wrapper::Clamp_Color
         push    ebp
         mov     ebp, esp
         sub     esp, 20                             ; 00000014H
-        push    ebx
-        push    esi
-        push    edi
         mov     DWORD PTR _this$[ebp], ecx
-        call    static bool CPUDetectClass::Has_CMOV_Instruction(void) ; CPUDetectClass::Has_CMOV_Instruction
-        movzx   eax, al
-        test    eax, eax
-        jne     $LN5@Clamp_Colo
         mov     DWORD PTR _i$2[ebp], 0
         jmp     SHORT $LN4@Clamp_Colo
 $LN2@Clamp_Colo:
-        mov     ecx, DWORD PTR _i$2[ebp]
-        add     ecx, 1
-        mov     DWORD PTR _i$2[ebp], ecx
+        mov     eax, DWORD PTR _i$2[ebp]
+        add     eax, 1
+        mov     DWORD PTR _i$2[ebp], eax
 $LN4@Clamp_Colo:
         cmp     DWORD PTR _i$2[ebp], 4
         jge     SHORT $LN3@Clamp_Colo
-        mov     edx, DWORD PTR _i$2[ebp]
-        push    edx
+        mov     ecx, DWORD PTR _i$2[ebp]
+        push    ecx
         mov     ecx, DWORD PTR _color$[ebp]
         call    float & Vector4::operator[](int)             ; Vector4::operator[]
         xorps   xmm0, xmm0
         comiss  xmm0, DWORD PTR [eax]
-        jbe     SHORT $LN7@Clamp_Colo
+        jbe     SHORT $LN6@Clamp_Colo
         xorps   xmm0, xmm0
-        movss   DWORD PTR tv76[ebp], xmm0
-        jmp     SHORT $LN8@Clamp_Colo
-$LN7@Clamp_Colo:
-        mov     eax, DWORD PTR _i$2[ebp]
-        push    eax
+        movss   DWORD PTR tv73[ebp], xmm0
+        jmp     SHORT $LN7@Clamp_Colo
+$LN6@Clamp_Colo:
+        mov     edx, DWORD PTR _i$2[ebp]
+        push    edx
         mov     ecx, DWORD PTR _color$[ebp]
         call    float & Vector4::operator[](int)             ; Vector4::operator[]
         movss   xmm0, DWORD PTR [eax]
-        movss   DWORD PTR tv76[ebp], xmm0
-$LN8@Clamp_Colo:
-        movss   xmm0, DWORD PTR tv76[ebp]
+        movss   DWORD PTR tv73[ebp], xmm0
+$LN7@Clamp_Colo:
+        movss   xmm0, DWORD PTR tv73[ebp]
         movss   DWORD PTR _f$1[ebp], xmm0
         movss   xmm0, DWORD PTR _f$1[ebp]
         comiss  xmm0, DWORD PTR __real@3f800000
-        jbe     SHORT $LN9@Clamp_Colo
+        jbe     SHORT $LN8@Clamp_Colo
         movss   xmm0, DWORD PTR __real@3f800000
-        movss   DWORD PTR tv81[ebp], xmm0
-        jmp     SHORT $LN10@Clamp_Colo
-$LN9@Clamp_Colo:
+        movss   DWORD PTR tv78[ebp], xmm0
+        jmp     SHORT $LN9@Clamp_Colo
+$LN8@Clamp_Colo:
         movss   xmm0, DWORD PTR _f$1[ebp]
-        movss   DWORD PTR tv81[ebp], xmm0
-$LN10@Clamp_Colo:
-        mov     ecx, DWORD PTR _i$2[ebp]
-        push    ecx
+        movss   DWORD PTR tv78[ebp], xmm0
+$LN9@Clamp_Colo:
+        mov     eax, DWORD PTR _i$2[ebp]
+        push    eax
         mov     ecx, DWORD PTR _color$[ebp]
         call    float & Vector4::operator[](int)             ; Vector4::operator[]
-        movss   xmm0, DWORD PTR tv81[ebp]
+        movss   xmm0, DWORD PTR tv78[ebp]
         movss   DWORD PTR [eax], xmm0
         jmp     $LN2@Clamp_Colo
 $LN3@Clamp_Colo:
-        jmp     SHORT $LN1@Clamp_Colo
-$LN5@Clamp_Colo:
-        mov     esi, DWORD PTR _color$[ebp]
-        mov     edx, 1065353216                     ; 3f800000H
-        mov     edi, DWORD PTR [esi]
-        mov     ebx, edi
-        sar     edi, 31                             ; 0000001fH
-        not     edi
-        and     edi, ebx
-        cmp     edi, edx
-        cmovae  edi, edx
-        mov     DWORD PTR [esi], edi
-        mov     edi, DWORD PTR [esi+4]
-        mov     ebx, edi
-        sar     edi, 31                             ; 0000001fH
-        not     edi
-        and     edi, ebx
-        cmp     edi, edx
-        cmovae  edi, edx
-        mov     DWORD PTR [esi+4], edi
-        mov     edi, DWORD PTR [esi+8]
-        mov     ebx, edi
-        sar     edi, 31                             ; 0000001fH
-        not     edi
-        and     edi, ebx
-        cmp     edi, edx
-        cmovae  edi, edx
-        mov     DWORD PTR [esi+8], edi
-        mov     edi, DWORD PTR [esi+12]
-        mov     ebx, edi
-        sar     edi, 31                             ; 0000001fH
-        not     edi
-        and     edi, ebx
-        cmp     edi, edx
-        cmovae  edi, edx
-        mov     DWORD PTR [esi+12], edi
-$LN1@Clamp_Colo:
-        pop     edi
-        pop     esi
-        pop     ebx
         mov     esp, ebp
         pop     ebp
         ret     4
```

</details>

</details>

---

<details>
<summary>GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/Except.cpp</summary>

This file contains the following inline assembly blocks:

<details>
<summary>_purecall</summary>

```c++
int __cdecl _purecall(void)
{
	int return_code = 0;

#ifdef WWDEBUG
	/*
	** Use int3 to cause an exception.
	*/
	WWDEBUG_SAY(("Pure Virtual Function call. Oh No!\n"));
	_asm int 0x03;
#endif	//_DEBUG_ASSERT

	return(return_code);
}
```

While I can't generate this in godbolt due to the `WWDEBUG` def, `_asm int 0x03` is simply a `__debugbreak()`:

```c++
#include <intrin.h>

int __cdecl _purecall(void)
{
	int return_code = 0;

#ifdef WWDEBUG
	/*
	** Use int3 to cause an exception.
	*/
	WWDEBUG_SAY(("Pure Virtual Function call. Oh No!\n"));
	__debugbreak();
#endif	//_DEBUG_ASSERT

	return(return_code);
}
```

</details>

<details>
<summary>Dump_Exception_Info</summary>

```c++
void Dump_Exception_Info()
{
	// ...

    void *fp_data_ptr = 0;
    double fp_value;

    /*
    ** Convert FP dump from temporary real value (10 bytes) to double (8 bytes).
    */
    _asm {
        push	eax
        mov	eax,fp_data_ptr
        fld   tbyte ptr [eax]
        fstp	qword ptr [fp_value]
        pop	eax
    }

	// ...
}
```

The assembly block here is being used to efficiently truncate a data pointer into a double value.

My goto equivalent is:

```c++
void Dump_Exception_Info()
{
	// ...

    void *fp_data_ptr = 0;
    const long double* longDoublePtr = (const long double*)fp_data_ptr;
    double fp_value = (double)(*longDoublePtr);

	// ...
}
```

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
_fp_value$ = -12                                        ; size = 8
_fp_data_ptr$ = -4                                  ; size = 4
void Dump_Exception_Info(void) PROC                        ; Dump_Exception_Info
        push    ebp
        mov     ebp, esp
        sub     esp, 12                             ; 0000000cH
        mov     DWORD PTR _fp_data_ptr$[ebp], 0
        push    eax
        mov     eax, DWORD PTR _fp_data_ptr$[ebp]
        fld     TBYTE PTR [eax]
        fstp    QWORD PTR _fp_value$[ebp]
        pop     eax
        mov     esp, ebp
        pop     ebp
        ret     0
void Dump_Exception_Info(void) ENDP                        ; Dump_Exception_Info
```

</td>
<td>

```asm
_fp_value$ = -16                                        ; size = 8
_longDoublePtr$ = -8                                    ; size = 4
_fp_data_ptr$ = -4                                  ; size = 4
void Dump_Exception_Info(void) PROC                        ; Dump_Exception_Info
        push    ebp
        mov     ebp, esp
        sub     esp, 16                             ; 00000010H
        mov     DWORD PTR _fp_data_ptr$[ebp], 0
        mov     eax, DWORD PTR _fp_data_ptr$[ebp]
        mov     DWORD PTR _longDoublePtr$[ebp], eax
        mov     ecx, DWORD PTR _longDoublePtr$[ebp]
        movsd   xmm0, QWORD PTR [ecx]
        movsd   QWORD PTR _fp_value$[ebp], xmm0
        mov     esp, ebp
        pop     ebp
        ret     0
void Dump_Exception_Info(void) ENDP                        ; Dump_Exception_Info
```

</td>
</table>

While there are minor differences in the assembly, the result is identical other than possible differences in
performance:

```diff
@@ -1,15 +1,16 @@
-_fp_value$ = -12                                        ; size = 8
+_fp_value$ = -16                                        ; size = 8
+_longDoublePtr$ = -8                                    ; size = 4
 _fp_data_ptr$ = -4                                  ; size = 4
 void Dump_Exception_Info(void) PROC                        ; Dump_Exception_Info
         push    ebp
         mov     ebp, esp
-        sub     esp, 12                             ; 0000000cH
+        sub     esp, 16                             ; 00000010H
         mov     DWORD PTR _fp_data_ptr$[ebp], 0
-        push    eax
         mov     eax, DWORD PTR _fp_data_ptr$[ebp]
-        fld     TBYTE PTR [eax]
-        fstp    QWORD PTR _fp_value$[ebp]
-        pop     eax
+        mov     DWORD PTR _longDoublePtr$[ebp], eax
+        mov     ecx, DWORD PTR _longDoublePtr$[ebp]
+        movsd   xmm0, QWORD PTR [ecx]
+        movsd   QWORD PTR _fp_value$[ebp], xmm0
         mov     esp, ebp
         pop     ebp
         ret     0
```

</details>

<details>
<summary>Stack_Walk</summary>

```c++
int Stack_Walk(unsigned long *return_addresses, int num_addresses, CONTEXT *context)
{
	// ...

STACKFRAME stack_frame;
	memset(&stack_frame, 0, sizeof(stack_frame));

	unsigned long reg_eip, reg_ebp, reg_esp;

	__asm {
here:
		lea	eax,here
		mov	reg_eip,eax
		mov	reg_ebp,ebp
		mov	reg_esp,esp
	}

	stack_frame.AddrPC.Mode = AddrModeFlat;
	stack_frame.AddrPC.Offset = reg_eip;
	stack_frame.AddrStack.Mode = AddrModeFlat;
	stack_frame.AddrStack.Offset = reg_esp;
	stack_frame.AddrFrame.Mode = AddrModeFlat;
	stack_frame.AddrFrame.Offset = reg_ebp;

    // ...
    
    // return pointer_index;
    return 1;
}
```

This assembly is trying to capture the values of the registers `EIP`, `EBP` and `ESP`.

My goto equivalent is:

```c++
#include <windows.h>
#include <dbghelp.h>
#include <string.h>

int Stack_Walk(unsigned long *return_addresses, int num_addresses, CONTEXT *context)
{
	// ...

	STACKFRAME stack_frame;
    memset(&stack_frame, 0, sizeof(stack_frame));

    CONTEXT local_context;
    if (!context) {
        // If no context is provided, capture the current thread's execution context.
        memset(&local_context, 0, sizeof(local_context));
        local_context.ContextFlags = CONTEXT_CONTROL; // Request EIP, ESP, EBP.

        RtlCaptureContext(&local_context);
        context = &local_context;
    }

    // Assign the starting register values from the provided or captured context.
    stack_frame.AddrPC.Mode = AddrModeFlat;
    stack_frame.AddrPC.Offset = context->Eip;

    stack_frame.AddrStack.Mode = AddrModeFlat;
    stack_frame.AddrStack.Offset = context->Esp;

    stack_frame.AddrFrame.Mode = AddrModeFlat;
    stack_frame.AddrFrame.Offset = context->Ebp;


here:
	// ...

    // return pointer_index;
    return 1;
}
```

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
voltbl  SEGMENT
        DDSymXIndex:    FLAT:int Stack_Walk(unsigned long *,int,_CONTEXT *)
voltbl  ENDS

_reg_ebp$ = -180                                        ; size = 4
_reg_esp$ = -176                                        ; size = 4
_reg_eip$ = -172                                        ; size = 4
_stack_frame$ = -168                                    ; size = 164
__$ArrayPad$ = -4                                 ; size = 4
_return_addresses$ = 8                              ; size = 4
_num_addresses$ = 12                                    ; size = 4
_context$ = 16                                      ; size = 4
int Stack_Walk(unsigned long *,int,_CONTEXT *) PROC                ; Stack_Walk
        push    ebp
        mov     ebp, esp
        sub     esp, 180                      ; 000000b4H
        mov     eax, DWORD PTR ___security_cookie
        xor     eax, ebp
        mov     DWORD PTR __$ArrayPad$[ebp], eax
        push    164                           ; 000000a4H
        push    0
        lea     eax, DWORD PTR _stack_frame$[ebp]
        push    eax
        call    _memset
        add     esp, 12                             ; 0000000cH
$here$3:
        lea     eax, OFFSET $here$3
        mov     DWORD PTR _reg_eip$[ebp], eax
        mov     DWORD PTR _reg_ebp$[ebp], ebp
        mov     DWORD PTR _reg_esp$[ebp], esp
        mov     DWORD PTR _stack_frame$[ebp+8], 3
        mov     ecx, DWORD PTR _reg_eip$[ebp]
        mov     DWORD PTR _stack_frame$[ebp], ecx
        mov     DWORD PTR _stack_frame$[ebp+44], 3
        mov     edx, DWORD PTR _reg_esp$[ebp]
        mov     DWORD PTR _stack_frame$[ebp+36], edx
        mov     DWORD PTR _stack_frame$[ebp+32], 3
        mov     eax, DWORD PTR _reg_ebp$[ebp]
        mov     DWORD PTR _stack_frame$[ebp+24], eax
        mov     eax, 1
        mov     ecx, DWORD PTR __$ArrayPad$[ebp]
        xor     ecx, ebp
        call    @__security_check_cookie@4
        mov     esp, ebp
        pop     ebp
        ret     0
int Stack_Walk(unsigned long *,int,_CONTEXT *) ENDP                ; Stack_Walk
```

</td>
<td>

```asm
voltbl  SEGMENT
        DDSymXIndex:    FLAT:int Stack_Walk(unsigned long *,int,_CONTEXT *)
voltbl  ENDS

_local_context$ = -884                              ; size = 716
_stack_frame$ = -168                                    ; size = 164
__$ArrayPad$ = -4                                 ; size = 4
_return_addresses$ = 8                              ; size = 4
_num_addresses$ = 12                                    ; size = 4
_context$ = 16                                      ; size = 4
int Stack_Walk(unsigned long *,int,_CONTEXT *) PROC                ; Stack_Walk
        push    ebp
        mov     ebp, esp
        sub     esp, 884                      ; 00000374H
        mov     eax, DWORD PTR ___security_cookie
        xor     eax, ebp
        mov     DWORD PTR __$ArrayPad$[ebp], eax
        push    164                           ; 000000a4H
        push    0
        lea     eax, DWORD PTR _stack_frame$[ebp]
        push    eax
        call    _memset
        add     esp, 12                             ; 0000000cH
        cmp     DWORD PTR _context$[ebp], 0
        jne     SHORT $LN2@Stack_Walk
        push    716                           ; 000002ccH
        push    0
        lea     ecx, DWORD PTR _local_context$[ebp]
        push    ecx
        call    _memset
        add     esp, 12                             ; 0000000cH
        mov     DWORD PTR _local_context$[ebp], 65537     ; 00010001H
        lea     edx, DWORD PTR _local_context$[ebp]
        push    edx
        call    DWORD PTR __imp__RtlCaptureContext@4
        lea     eax, DWORD PTR _local_context$[ebp]
        mov     DWORD PTR _context$[ebp], eax
$LN2@Stack_Walk:
        mov     DWORD PTR _stack_frame$[ebp+8], 3
        mov     ecx, DWORD PTR _context$[ebp]
        mov     edx, DWORD PTR [ecx+184]
        mov     DWORD PTR _stack_frame$[ebp], edx
        mov     DWORD PTR _stack_frame$[ebp+44], 3
        mov     eax, DWORD PTR _context$[ebp]
        mov     ecx, DWORD PTR [eax+196]
        mov     DWORD PTR _stack_frame$[ebp+36], ecx
        mov     DWORD PTR _stack_frame$[ebp+32], 3
        mov     edx, DWORD PTR _context$[ebp]
        mov     eax, DWORD PTR [edx+180]
        mov     DWORD PTR _stack_frame$[ebp+24], eax
$here$4:
        mov     eax, 1
        mov     ecx, DWORD PTR __$ArrayPad$[ebp]
        xor     ecx, ebp
        call    @__security_check_cookie@4
        mov     esp, ebp
        pop     ebp
        ret     0
int Stack_Walk(unsigned long *,int,_CONTEXT *) ENDP                ; Stack_Walk
```

</td>
</table>

I think it is clear why inline assembly was used, the WIN32 API was probably not as performant and was definitely
generating a bigger assembly. It may be the case that with modern compilers and optimizations on, this isn't as
important.

```diff
@@ -2,9 +2,7 @@ voltbl  SEGMENT
         DDSymXIndex:    FLAT:int Stack_Walk(unsigned long *,int,_CONTEXT *)
 voltbl  ENDS
 
-_reg_ebp$ = -180                                        ; size = 4
-_reg_esp$ = -176                                        ; size = 4
-_reg_eip$ = -172                                        ; size = 4
+_local_context$ = -884                              ; size = 716
 _stack_frame$ = -168                                    ; size = 164
 __$ArrayPad$ = -4                                 ; size = 4
 _return_addresses$ = 8                              ; size = 4
@@ -13,7 +11,7 @@ _context$ = 16                                      ; size = 4
 int Stack_Walk(unsigned long *,int,_CONTEXT *) PROC                ; Stack_Walk
         push    ebp
         mov     ebp, esp
-        sub     esp, 180                      ; 000000b4H
+        sub     esp, 884                      ; 00000374H
         mov     eax, DWORD PTR ___security_cookie
         xor     eax, ebp
         mov     DWORD PTR __$ArrayPad$[ebp], eax
@@ -23,20 +21,34 @@ int Stack_Walk(unsigned long *,int,_CONTEXT *) PROC                ; Stack_Walk
         push    eax
         call    _memset
         add     esp, 12                             ; 0000000cH
-$here$3:
-        lea     eax, OFFSET $here$3
-        mov     DWORD PTR _reg_eip$[ebp], eax
-        mov     DWORD PTR _reg_ebp$[ebp], ebp
-        mov     DWORD PTR _reg_esp$[ebp], esp
+        cmp     DWORD PTR _context$[ebp], 0
+        jne     SHORT $LN2@Stack_Walk
+        push    716                           ; 000002ccH
+        push    0
+        lea     ecx, DWORD PTR _local_context$[ebp]
+        push    ecx
+        call    _memset
+        add     esp, 12                             ; 0000000cH
+        mov     DWORD PTR _local_context$[ebp], 65537     ; 00010001H
+        lea     edx, DWORD PTR _local_context$[ebp]
+        push    edx
+        call    DWORD PTR __imp__RtlCaptureContext@4
+        lea     eax, DWORD PTR _local_context$[ebp]
+        mov     DWORD PTR _context$[ebp], eax
+$LN2@Stack_Walk:
         mov     DWORD PTR _stack_frame$[ebp+8], 3
-        mov     ecx, DWORD PTR _reg_eip$[ebp]
-        mov     DWORD PTR _stack_frame$[ebp], ecx
+        mov     ecx, DWORD PTR _context$[ebp]
+        mov     edx, DWORD PTR [ecx+184]
+        mov     DWORD PTR _stack_frame$[ebp], edx
         mov     DWORD PTR _stack_frame$[ebp+44], 3
-        mov     edx, DWORD PTR _reg_esp$[ebp]
-        mov     DWORD PTR _stack_frame$[ebp+36], edx
+        mov     eax, DWORD PTR _context$[ebp]
+        mov     ecx, DWORD PTR [eax+196]
+        mov     DWORD PTR _stack_frame$[ebp+36], ecx
         mov     DWORD PTR _stack_frame$[ebp+32], 3
-        mov     eax, DWORD PTR _reg_ebp$[ebp]
+        mov     edx, DWORD PTR _context$[ebp]
+        mov     eax, DWORD PTR [edx+180]
         mov     DWORD PTR _stack_frame$[ebp+24], eax
+$here$4:
         mov     eax, 1
         mov     ecx, DWORD PTR __$ArrayPad$[ebp]
         xor     ecx, ebp
```

</details>

</details>

---

<details>
<summary>Generals/Code/Libraries/Source/WWVegas/WWMath/matrix3d.cpp</summary>

This file includes the following assembly blocks:

<details>Matrix3D::Multiply</details>

This block is already documented in the source file by the authors.
</details>

---

<details>
<summary>Generals/Code/GameEngine/Include/Common/PerfTimer.h</summary>

This file includes the following assembly block:

```c++
__forceinline void GetPrecisionTimer(Int64* t)
{
#ifdef USE_QPF
	QueryPerformanceCounter((LARGE_INTEGER*)t);
#else
	// CPUID is needed to force serialization of any previous instructions. 
	__asm 
	{
		// for now, I am commenting this out. It throws the timings off a bit more (up to .001%) jkmcd
		//		CPUID
		RDTSC
		MOV ECX,[t]
		MOV [ECX], EAX
		MOV [ECX+4], EDX
	}
#endif
}
```

This is using the [RDTSC](https://www.aldeid.com/wiki/X86-assembly/Instructions/rdtsc) instruction to get a precision
timer.

My goto equivalent is:

```c++
#include <intrin.h>

__forceinline void GetPrecisionTimer(Int64* t)
{
#ifdef USE_QPF
	QueryPerformanceCounter((LARGE_INTEGER*)t);
#else
	*t = __rdtsc();
#endif
}
```

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
</table>

The output assemblies are identical.

</details>

---

<details>
<summary>GeneralsMD/Code/GameEngine/Source/Common/PerfTimer.cpp</summary>

This file includes the following assembly block:

```c++
__forceinline void ProfileGetTime(__int64 &t)
{
  _asm
  {
    mov ecx,[t]
    push eax
    push edx
    rdtsc
    mov [ecx],eax
    mov [ecx+4],edx
    pop edx
    pop eax
  };
}
```

This is using the assembly block to retrieve the value
of [RDTSC](https://www.aldeid.com/wiki/X86-assembly/Instructions/rdtsc) and move it to `t`.

My goto equivalent is:

```c++
#include <windows.h>
#include <intrin.h>

__forceinline void ProfileGetTime(__int64 &t)
{
  t = __rdtsc();
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
void ProfileGetTime(__int64 &) PROC               ; ProfileGetTime
        push    ebp
        mov     ebp, esp
        mov     ecx, DWORD PTR _t$[ebp]
        push    eax
        push    edx
        rdtsc
        mov     DWORD PTR [ecx], eax
        mov     DWORD PTR [ecx+4], edx
        pop     edx
        pop     eax
        pop     ebp
        ret     0
void ProfileGetTime(__int64 &) ENDP               ; ProfileGetTime
```

</td>
<td>

```asm
_t$ = 8                                       ; size = 4
void ProfileGetTime(__int64 &) PROC               ; ProfileGetTime
        push    ebp
        mov     ebp, esp
        rdtsc
        mov     ecx, DWORD PTR _t$[ebp]
        mov     DWORD PTR [ecx], eax
        mov     DWORD PTR [ecx+4], edx
        pop     ebp
        ret     0
void ProfileGetTime(__int64 &) ENDP               ; ProfileGetTime
```

</td>
</table>

The assemblies are different in structure but equal in result. The intrinsic version appears shorter.

```diff
@@ -2,14 +2,10 @@ _t$ = 8                                       ; size = 4
 void ProfileGetTime(__int64 &) PROC               ; ProfileGetTime
         push    ebp
         mov     ebp, esp
-        mov     ecx, DWORD PTR _t$[ebp]
-        push    eax
-        push    edx
         rdtsc
+        mov     ecx, DWORD PTR _t$[ebp]
         mov     DWORD PTR [ecx], eax
         mov     DWORD PTR [ecx+4], edx
-        pop     edx
-        pop     eax
         pop     ebp
         ret     0
 void ProfileGetTime(__int64 &) ENDP               ; ProfileGetTime
```

</details>

---

<details>
<summary>Generals/Code/Libraries/Source/WWVegas/WWMath/quat.cpp</summary>

`Fast_Slerp` is already documented within the assembly block by the authors.

</details>

---

<details>
<summary>Generals(MD)/Code/GameEngine/Source/Common/System/StackDump.cpp</summary>

This file includes the following assembly blocks:

<details>
<summary>StackDump</summary>

```c++
void StackDump(void (*callback)(const char*))
{
	if (callback == NULL) 
	{
		callback = StackDumpDefaultHandler;
	}

	InitSymbolInfo();

	DWORD myeip,myesp,myebp;

_asm
{
MYEIP1:
 mov eax, MYEIP1
 mov dword ptr [myeip] , eax
 mov eax, esp
 mov dword ptr [myesp] , eax
 mov eax, ebp
 mov dword ptr [myebp] , eax
}


	MakeStackTrace(myeip,myesp,myebp, 2, callback);
}
```

This is using the assembly to get the values of the instruction pointer, stack pointer and base pointer
(EIP, ESP and EBP registers).

My goto equivalent is:

```c++
#include <windows.h>
#include <intrin.h>

extern void InitSymbolInfo(); // impl not relevant

extern void StackDumpDefaultHandler(const char*); // impl not relevant

extern void MakeStackTrace(DWORD, DWORD, DWORD, int, void (*callback)(const char*)); // impl not relevant

void StackDump(void (*callback)(const char*))
{
if (callback == NULL)
{
callback = StackDumpDefaultHandler;
}

	InitSymbolInfo();

	DWORD myeip = (DWORD)(intptr_t)_ReturnAddress();
	DWORD myesp = (DWORD)((intptr_t)_AddressOfReturnAddress() + 4);
	DWORD myebp = (DWORD)((intptr_t)_AddressOfReturnAddress() + 4);

	MakeStackTrace(myeip,myesp,myebp, 2, callback);
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
_myeip$ = -12                                     ; size = 4
_myesp$ = -8                                            ; size = 4
_myebp$ = -4                                            ; size = 4
_callback$ = 8                                      ; size = 4
void StackDump(void (__cdecl*)(char const *)) PROC               ; StackDump
        push    ebp
        mov     ebp, esp
        sub     esp, 12                             ; 0000000cH
        cmp     DWORD PTR _callback$[ebp], 0
        jne     SHORT $LN2@StackDump
        mov     DWORD PTR _callback$[ebp], OFFSET void StackDumpDefaultHandler(char const *) ; StackDumpDefaultHandler
$LN2@StackDump:
        call    void InitSymbolInfo(void)              ; InitSymbolInfo
$MYEIP1$4:
        mov     eax, OFFSET $MYEIP1$4
        mov     DWORD PTR _myeip$[ebp], eax
        mov     eax, esp
        mov     DWORD PTR _myesp$[ebp], eax
        mov     eax, ebp
        mov     DWORD PTR _myebp$[ebp], eax
        mov     eax, DWORD PTR _callback$[ebp]
        push    eax
        push    2
        mov     ecx, DWORD PTR _myebp$[ebp]
        push    ecx
        mov     edx, DWORD PTR _myesp$[ebp]
        push    edx
        mov     eax, DWORD PTR _myeip$[ebp]
        push    eax
        call    void MakeStackTrace(unsigned long,unsigned long,unsigned long,int,void (__cdecl*)(char const *))     ; MakeStackTrace
        add     esp, 20                             ; 00000014H
        mov     esp, ebp
        pop     ebp
        ret     0
void StackDump(void (__cdecl*)(char const *)) ENDP               ; StackDump
```

</td>
<td>

```asm
_myeip$ = -12                                     ; size = 4
_myesp$ = -8                                            ; size = 4
_myebp$ = -4                                            ; size = 4
__$ReturnAddr$ = 4                                  ; size = 4
_callback$ = 8                                      ; size = 4
void StackDump(void (__cdecl*)(char const *)) PROC               ; StackDump
        push    ebp
        mov     ebp, esp
        sub     esp, 12                             ; 0000000cH
        cmp     DWORD PTR _callback$[ebp], 0
        jne     SHORT $LN2@StackDump
        mov     DWORD PTR _callback$[ebp], OFFSET void StackDumpDefaultHandler(char const *) ; StackDumpDefaultHandler
$LN2@StackDump:
        call    void InitSymbolInfo(void)              ; InitSymbolInfo
        mov     eax, DWORD PTR __$ReturnAddr$[ebp]
        mov     DWORD PTR _myeip$[ebp], eax
        lea     ecx, DWORD PTR __$ReturnAddr$[ebp]
        add     ecx, 4
        mov     DWORD PTR _myesp$[ebp], ecx
        lea     edx, DWORD PTR __$ReturnAddr$[ebp]
        sub     edx, 4
        mov     DWORD PTR _myebp$[ebp], edx
        mov     eax, DWORD PTR _callback$[ebp]
        push    eax
        push    2
        mov     ecx, DWORD PTR _myebp$[ebp]
        push    ecx
        mov     edx, DWORD PTR _myesp$[ebp]
        push    edx
        mov     eax, DWORD PTR _myeip$[ebp]
        push    eax
        call    void MakeStackTrace(unsigned long,unsigned long,unsigned long,int,void (__cdecl*)(char const *))     ; MakeStackTrace
        add     esp, 20                             ; 00000014H
        mov     esp, ebp
        pop     ebp
        ret     0
void StackDump(void (__cdecl*)(char const *)) ENDP               ; StackDump
```

</td>
</table>

While these assemblies **are** different, the functionalities are **not** different. This means the inline assembly is
a lighter implementation than the C++ alternative since it doesn't use intrinsic functions, but accesses the registers
directly.

```diff
@@ -1,6 +1,7 @@
 _myeip$ = -12                                     ; size = 4
 _myesp$ = -8                                            ; size = 4
 _myebp$ = -4                                            ; size = 4
+__$ReturnAddr$ = 4                                  ; size = 4
 _callback$ = 8                                      ; size = 4
 void StackDump(void (__cdecl*)(char const *)) PROC               ; StackDump
         push    ebp
@@ -11,13 +12,14 @@ void StackDump(void (__cdecl*)(char const *)) PROC               ; StackDump
         mov     DWORD PTR _callback$[ebp], OFFSET void StackDumpDefaultHandler(char const *) ; StackDumpDefaultHandler
 $LN2@StackDump:
         call    void InitSymbolInfo(void)              ; InitSymbolInfo
-$MYEIP1$4:
-        mov     eax, OFFSET $MYEIP1$4
+        mov     eax, DWORD PTR __$ReturnAddr$[ebp]
         mov     DWORD PTR _myeip$[ebp], eax
-        mov     eax, esp
-        mov     DWORD PTR _myesp$[ebp], eax
-        mov     eax, ebp
-        mov     DWORD PTR _myebp$[ebp], eax
+        lea     ecx, DWORD PTR __$ReturnAddr$[ebp]
+        add     ecx, 4
+        mov     DWORD PTR _myesp$[ebp], ecx
+        lea     edx, DWORD PTR __$ReturnAddr$[ebp]
+        sub     edx, 4
+        mov     DWORD PTR _myebp$[ebp], edx
         mov     eax, DWORD PTR _callback$[ebp]
         push    eax
         push    2
```

</details>

<details>
<summary>FillStackAddresses</summary>

```c++
void FillStackAddresses(void**addresses, unsigned int count, unsigned int skip)
{
	InitSymbolInfo();

	STACKFRAME	stack_frame;

	
	HANDLE thread = GetCurrentThread();
	HANDLE process = GetCurrentProcess();

    memset(&gsContext, 0, sizeof(CONTEXT));
    gsContext.ContextFlags = CONTEXT_FULL;

	DWORD myeip,myesp,myebp;
_asm
{
MYEIP2:
 mov eax, MYEIP2
 mov dword ptr [myeip] , eax
 mov eax, esp
 mov dword ptr [myesp] , eax
 mov eax, ebp
 mov dword ptr [myebp] , eax
 xor eax,eax
}
memset(&stack_frame, 0, sizeof(STACKFRAME));
stack_frame.AddrPC.Mode = AddrModeFlat;
stack_frame.AddrPC.Offset = myeip;
stack_frame.AddrStack.Mode = AddrModeFlat;
stack_frame.AddrStack.Offset = myesp;
stack_frame.AddrFrame.Mode = AddrModeFlat;
stack_frame.AddrFrame.Offset = myebp;

{
/*
    if(GetThreadContext(thread, &gsContext))
    {
        memset(&stack_frame, 0, sizeof(STACKFRAME));
        stack_frame.AddrPC.Mode = AddrModeFlat;
        stack_frame.AddrPC.Offset = gsContext.Eip;
        stack_frame.AddrStack.Mode = AddrModeFlat;
        stack_frame.AddrStack.Offset = gsContext.Esp;
        stack_frame.AddrFrame.Mode = AddrModeFlat;
        stack_frame.AddrFrame.Offset = gsContext.Ebp;
*/

		Bool stillgoing = TRUE;
//	unsigned int cd = count;

		// Skip some?
		while (stillgoing&&skip)
		{
			stillgoing = StackWalk(IMAGE_FILE_MACHINE_I386,
								process,
								thread,
								&stack_frame,
								NULL,	//&gsContext,
								NULL,
								SymFunctionTableAccess,
								SymGetModuleBase,
								NULL) != 0;
			skip--;
		}

		while(stillgoing&&count)
		{
			stillgoing = StackWalk(IMAGE_FILE_MACHINE_I386,
								process,
								thread,
								&stack_frame,
								NULL, //&gsContext,
								NULL,
								SymFunctionTableAccess,
								SymGetModuleBase,
								NULL) != 0;
			if (stillgoing)
			{
				*addresses  = (void*)stack_frame.AddrPC.Offset;				
				addresses++;
				count--;
			}
		}

		// Fill remainder
		while (count)
		{
			*addresses = NULL;
			addresses++;
			count--;
		}

	}
/*
	else
	{
		memset(addresses,NULL,count*sizeof(void*));
	}
*/	
}
```

This function is using inline assembly to retrieve the instruction pointer, the stack pointer and the base pointer
(EIP, ESP and EBP registers) and storing these values. Then, it clears EAX.

My goto equivalent is:

```c++
#include <windows.h>
#include <intrin.h>
#include <dbghelp.h>

typedef _CONTEXT CONTEXT;

static CONTEXT gsContext;

extern void InitSymbolInfo(); // impl not relevant

void FillStackAddresses(void**addresses, unsigned int count, unsigned int skip)
{
	InitSymbolInfo();

	STACKFRAME	stack_frame;

	
	HANDLE thread = GetCurrentThread();
	HANDLE process = GetCurrentProcess();

    memset(&gsContext, 0, sizeof(CONTEXT));
    gsContext.ContextFlags = CONTEXT_FULL;

    DWORD myeip = (DWORD)(intptr_t)_ReturnAddress();
	DWORD myesp = (DWORD)((intptr_t)_AddressOfReturnAddress() + 4);
	DWORD myebp = (DWORD)((intptr_t)_AddressOfReturnAddress() - 4);

    memset(&stack_frame, 0, sizeof(STACKFRAME));
    stack_frame.AddrPC.Mode = AddrModeFlat;
    stack_frame.AddrPC.Offset = myeip;
    stack_frame.AddrStack.Mode = AddrModeFlat;
    stack_frame.AddrStack.Offset = myesp;
    stack_frame.AddrFrame.Mode = AddrModeFlat;
    stack_frame.AddrFrame.Offset = myebp;

    {
        /*
        if(GetThreadContext(thread, &gsContext))
        {
            memset(&stack_frame, 0, sizeof(STACKFRAME));
            stack_frame.AddrPC.Mode = AddrModeFlat;
            stack_frame.AddrPC.Offset = gsContext.Eip;
            stack_frame.AddrStack.Mode = AddrModeFlat;
            stack_frame.AddrStack.Offset = gsContext.Esp;
            stack_frame.AddrFrame.Mode = AddrModeFlat;
            stack_frame.AddrFrame.Offset = gsContext.Ebp;
        */

		BOOL stillgoing = TRUE;
        //	unsigned int cd = count;

		// Skip some?
		while (stillgoing&&skip)
		{
			stillgoing = StackWalk(IMAGE_FILE_MACHINE_I386,
								process,
								thread,
								&stack_frame,
								NULL,	//&gsContext,
								NULL,
								SymFunctionTableAccess,
								SymGetModuleBase,
								NULL) != 0;
			skip--;
		}

		while(stillgoing&&count)
		{
			stillgoing = StackWalk(IMAGE_FILE_MACHINE_I386,
								process,
								thread,
								&stack_frame,
								NULL, //&gsContext,
								NULL,
								SymFunctionTableAccess,
								SymGetModuleBase,
								NULL) != 0;
			if (stillgoing)
			{
				*addresses  = (void*)stack_frame.AddrPC.Offset;				
				addresses++;
				count--;
			}
		}

		// Fill remainder
		while (count)
		{
			*addresses = NULL;
			addresses++;
			count--;
		}

	}
    /*
	else
	{
		memset(addresses,NULL,count*sizeof(void*));
	}
    */	
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
_CONTEXT gsContext DB 02ccH DUP (?)          ; gsContext
voltbl  SEGMENT
_volmd  DD  0ffffffffH
        DDSymXIndex:    FLAT:void FillStackAddresses(void * *,unsigned int,unsigned int)
        DD      010H
        DD      01fcH
voltbl  ENDS

_myebp$ = -200                                      ; size = 4
_myesp$ = -196                                      ; size = 4
_myeip$ = -192                                      ; size = 4
tv132 = -188                                            ; size = 4
_process$ = -184                                        ; size = 4
_thread$ = -180                               ; size = 4
tv85 = -176                                   ; size = 4
_stillgoing$1 = -172                                    ; size = 4
_stack_frame$ = -168                                    ; size = 164
__$ArrayPad$ = -4                                 ; size = 4
_addresses$ = 8                               ; size = 4
_count$ = 12                                            ; size = 4
_skip$ = 16                                   ; size = 4
void FillStackAddresses(void * *,unsigned int,unsigned int) PROC              ; FillStackAddresses
        push    ebp
        mov     ebp, esp
        sub     esp, 200                      ; 000000c8H
        mov     eax, DWORD PTR ___security_cookie
        xor     eax, ebp
        mov     DWORD PTR __$ArrayPad$[ebp], eax
        call    void InitSymbolInfo(void)              ; InitSymbolInfo
        call    DWORD PTR __imp__GetCurrentThread@0
        mov     DWORD PTR _thread$[ebp], eax
        call    DWORD PTR __imp__GetCurrentProcess@0
        mov     DWORD PTR _process$[ebp], eax
        push    716                           ; 000002ccH
        push    0
        push    OFFSET _CONTEXT gsContext
        call    _memset
        add     esp, 12                             ; 0000000cH
        mov     DWORD PTR _CONTEXT gsContext, 65543 ; 00010007H
$MYEIP2$14:
        mov     eax, OFFSET $MYEIP2$14
        mov     DWORD PTR _myeip$[ebp], eax
        mov     eax, esp
        mov     DWORD PTR _myesp$[ebp], eax
        mov     eax, ebp
        mov     DWORD PTR _myebp$[ebp], eax
        xor     eax, eax
        push    164                           ; 000000a4H
        push    0
        lea     eax, DWORD PTR _stack_frame$[ebp]
        push    eax
        call    _memset
        add     esp, 12                             ; 0000000cH
        mov     DWORD PTR _stack_frame$[ebp+8], 3
        mov     ecx, DWORD PTR _myeip$[ebp]
        mov     DWORD PTR _stack_frame$[ebp], ecx
        mov     DWORD PTR _stack_frame$[ebp+44], 3
        mov     edx, DWORD PTR _myesp$[ebp]
        mov     DWORD PTR _stack_frame$[ebp+36], edx
        mov     DWORD PTR _stack_frame$[ebp+32], 3
        mov     eax, DWORD PTR _myebp$[ebp]
        mov     DWORD PTR _stack_frame$[ebp+24], eax
        mov     DWORD PTR _stillgoing$1[ebp], 1
$LN2@FillStackA:
        cmp     DWORD PTR _stillgoing$1[ebp], 0
        je      SHORT $LN4@FillStackA
        cmp     DWORD PTR _skip$[ebp], 0
        je      SHORT $LN4@FillStackA
        push    0
        mov     ecx, DWORD PTR __imp__SymGetModuleBase@8
        push    ecx
        mov     edx, DWORD PTR __imp__SymFunctionTableAccess@8
        push    edx
        push    0
        push    0
        lea     eax, DWORD PTR _stack_frame$[ebp]
        push    eax
        mov     ecx, DWORD PTR _thread$[ebp]
        push    ecx
        mov     edx, DWORD PTR _process$[ebp]
        push    edx
        push    332                           ; 0000014cH
        call    DWORD PTR __imp__StackWalk@36
        test    eax, eax
        je      SHORT $LN10@FillStackA
        mov     DWORD PTR tv85[ebp], 1
        jmp     SHORT $LN11@FillStackA
$LN10@FillStackA:
        mov     DWORD PTR tv85[ebp], 0
$LN11@FillStackA:
        mov     eax, DWORD PTR tv85[ebp]
        mov     DWORD PTR _stillgoing$1[ebp], eax
        mov     ecx, DWORD PTR _skip$[ebp]
        sub     ecx, 1
        mov     DWORD PTR _skip$[ebp], ecx
        jmp     SHORT $LN2@FillStackA
$LN4@FillStackA:
        cmp     DWORD PTR _stillgoing$1[ebp], 0
        je      $LN6@FillStackA
        cmp     DWORD PTR _count$[ebp], 0
        je      $LN6@FillStackA
        push    0
        mov     edx, DWORD PTR __imp__SymGetModuleBase@8
        push    edx
        mov     eax, DWORD PTR __imp__SymFunctionTableAccess@8
        push    eax
        push    0
        push    0
        lea     ecx, DWORD PTR _stack_frame$[ebp]
        push    ecx
        mov     edx, DWORD PTR _thread$[ebp]
        push    edx
        mov     eax, DWORD PTR _process$[ebp]
        push    eax
        push    332                           ; 0000014cH
        call    DWORD PTR __imp__StackWalk@36
        test    eax, eax
        je      SHORT $LN12@FillStackA
        mov     DWORD PTR tv132[ebp], 1
        jmp     SHORT $LN13@FillStackA
$LN12@FillStackA:
        mov     DWORD PTR tv132[ebp], 0
$LN13@FillStackA:
        mov     ecx, DWORD PTR tv132[ebp]
        mov     DWORD PTR _stillgoing$1[ebp], ecx
        cmp     DWORD PTR _stillgoing$1[ebp], 0
        je      SHORT $LN8@FillStackA
        mov     edx, DWORD PTR _addresses$[ebp]
        mov     eax, DWORD PTR _stack_frame$[ebp]
        mov     DWORD PTR [edx], eax
        mov     ecx, DWORD PTR _addresses$[ebp]
        add     ecx, 4
        mov     DWORD PTR _addresses$[ebp], ecx
        mov     edx, DWORD PTR _count$[ebp]
        sub     edx, 1
        mov     DWORD PTR _count$[ebp], edx
$LN8@FillStackA:
        jmp     $LN4@FillStackA
$LN6@FillStackA:
        cmp     DWORD PTR _count$[ebp], 0
        je      SHORT $LN1@FillStackA
        mov     eax, DWORD PTR _addresses$[ebp]
        mov     DWORD PTR [eax], 0
        mov     ecx, DWORD PTR _addresses$[ebp]
        add     ecx, 4
        mov     DWORD PTR _addresses$[ebp], ecx
        mov     edx, DWORD PTR _count$[ebp]
        sub     edx, 1
        mov     DWORD PTR _count$[ebp], edx
        jmp     SHORT $LN6@FillStackA
$LN1@FillStackA:
        mov     ecx, DWORD PTR __$ArrayPad$[ebp]
        xor     ecx, ebp
        call    @__security_check_cookie@4
        mov     esp, ebp
        pop     ebp
        ret     0
void FillStackAddresses(void * *,unsigned int,unsigned int) ENDP              ; FillStackAddresses
```

</td>
<td>

```asm
_CONTEXT gsContext DB 02ccH DUP (?)          ; gsContext
voltbl  SEGMENT
_volmd  DD  0ffffffffH
        DDSymXIndex:    FLAT:void FillStackAddresses(void * *,unsigned int,unsigned int)
        DD      010H
        DD      0200H
voltbl  ENDS

_myebp$ = -200                                      ; size = 4
_myesp$ = -196                                      ; size = 4
_myeip$ = -192                                      ; size = 4
tv137 = -188                                            ; size = 4
_process$ = -184                                        ; size = 4
_thread$ = -180                               ; size = 4
tv90 = -176                                   ; size = 4
_stillgoing$1 = -172                                    ; size = 4
_stack_frame$ = -168                                    ; size = 164
__$ArrayPad$ = -4                                 ; size = 4
__$ReturnAddr$ = 4                                  ; size = 4
_addresses$ = 8                               ; size = 4
_count$ = 12                                            ; size = 4
_skip$ = 16                                   ; size = 4
void FillStackAddresses(void * *,unsigned int,unsigned int) PROC              ; FillStackAddresses
        push    ebp
        mov     ebp, esp
        sub     esp, 200                      ; 000000c8H
        mov     eax, DWORD PTR ___security_cookie
        xor     eax, ebp
        mov     DWORD PTR __$ArrayPad$[ebp], eax
        call    void InitSymbolInfo(void)              ; InitSymbolInfo
        call    DWORD PTR __imp__GetCurrentThread@0
        mov     DWORD PTR _thread$[ebp], eax
        call    DWORD PTR __imp__GetCurrentProcess@0
        mov     DWORD PTR _process$[ebp], eax
        push    716                           ; 000002ccH
        push    0
        push    OFFSET _CONTEXT gsContext
        call    _memset
        add     esp, 12                             ; 0000000cH
        mov     DWORD PTR _CONTEXT gsContext, 65543 ; 00010007H
        mov     eax, DWORD PTR __$ReturnAddr$[ebp]
        mov     DWORD PTR _myeip$[ebp], eax
        lea     ecx, DWORD PTR __$ReturnAddr$[ebp]
        add     ecx, 4
        mov     DWORD PTR _myesp$[ebp], ecx
        lea     edx, DWORD PTR __$ReturnAddr$[ebp]
        sub     edx, 4
        mov     DWORD PTR _myebp$[ebp], edx
        push    164                           ; 000000a4H
        push    0
        lea     eax, DWORD PTR _stack_frame$[ebp]
        push    eax
        call    _memset
        add     esp, 12                             ; 0000000cH
        mov     DWORD PTR _stack_frame$[ebp+8], 3
        mov     ecx, DWORD PTR _myeip$[ebp]
        mov     DWORD PTR _stack_frame$[ebp], ecx
        mov     DWORD PTR _stack_frame$[ebp+44], 3
        mov     edx, DWORD PTR _myesp$[ebp]
        mov     DWORD PTR _stack_frame$[ebp+36], edx
        mov     DWORD PTR _stack_frame$[ebp+32], 3
        mov     eax, DWORD PTR _myebp$[ebp]
        mov     DWORD PTR _stack_frame$[ebp+24], eax
        mov     DWORD PTR _stillgoing$1[ebp], 1
$LN2@FillStackA:
        cmp     DWORD PTR _stillgoing$1[ebp], 0
        je      SHORT $LN3@FillStackA
        cmp     DWORD PTR _skip$[ebp], 0
        je      SHORT $LN3@FillStackA
        push    0
        mov     ecx, DWORD PTR __imp__SymGetModuleBase@8
        push    ecx
        mov     edx, DWORD PTR __imp__SymFunctionTableAccess@8
        push    edx
        push    0
        push    0
        lea     eax, DWORD PTR _stack_frame$[ebp]
        push    eax
        mov     ecx, DWORD PTR _thread$[ebp]
        push    ecx
        mov     edx, DWORD PTR _process$[ebp]
        push    edx
        push    332                           ; 0000014cH
        call    DWORD PTR __imp__StackWalk@36
        test    eax, eax
        je      SHORT $LN10@FillStackA
        mov     DWORD PTR tv90[ebp], 1
        jmp     SHORT $LN11@FillStackA
$LN10@FillStackA:
        mov     DWORD PTR tv90[ebp], 0
$LN11@FillStackA:
        mov     eax, DWORD PTR tv90[ebp]
        mov     DWORD PTR _stillgoing$1[ebp], eax
        mov     ecx, DWORD PTR _skip$[ebp]
        sub     ecx, 1
        mov     DWORD PTR _skip$[ebp], ecx
        jmp     SHORT $LN2@FillStackA
$LN3@FillStackA:
        cmp     DWORD PTR _stillgoing$1[ebp], 0
        je      $LN5@FillStackA
        cmp     DWORD PTR _count$[ebp], 0
        je      $LN5@FillStackA
        push    0
        mov     edx, DWORD PTR __imp__SymGetModuleBase@8
        push    edx
        mov     eax, DWORD PTR __imp__SymFunctionTableAccess@8
        push    eax
        push    0
        push    0
        lea     ecx, DWORD PTR _stack_frame$[ebp]
        push    ecx
        mov     edx, DWORD PTR _thread$[ebp]
        push    edx
        mov     eax, DWORD PTR _process$[ebp]
        push    eax
        push    332                           ; 0000014cH
        call    DWORD PTR __imp__StackWalk@36
        test    eax, eax
        je      SHORT $LN12@FillStackA
        mov     DWORD PTR tv137[ebp], 1
        jmp     SHORT $LN13@FillStackA
$LN12@FillStackA:
        mov     DWORD PTR tv137[ebp], 0
$LN13@FillStackA:
        mov     ecx, DWORD PTR tv137[ebp]
        mov     DWORD PTR _stillgoing$1[ebp], ecx
        cmp     DWORD PTR _stillgoing$1[ebp], 0
        je      SHORT $LN8@FillStackA
        mov     edx, DWORD PTR _addresses$[ebp]
        mov     eax, DWORD PTR _stack_frame$[ebp]
        mov     DWORD PTR [edx], eax
        mov     ecx, DWORD PTR _addresses$[ebp]
        add     ecx, 4
        mov     DWORD PTR _addresses$[ebp], ecx
        mov     edx, DWORD PTR _count$[ebp]
        sub     edx, 1
        mov     DWORD PTR _count$[ebp], edx
$LN8@FillStackA:
        jmp     $LN3@FillStackA
$LN5@FillStackA:
        cmp     DWORD PTR _count$[ebp], 0
        je      SHORT $LN7@FillStackA
        mov     eax, DWORD PTR _addresses$[ebp]
        mov     DWORD PTR [eax], 0
        mov     ecx, DWORD PTR _addresses$[ebp]
        add     ecx, 4
        mov     DWORD PTR _addresses$[ebp], ecx
        mov     edx, DWORD PTR _count$[ebp]
        sub     edx, 1
        mov     DWORD PTR _count$[ebp], edx
        jmp     SHORT $LN5@FillStackA
$LN7@FillStackA:
        mov     ecx, DWORD PTR __$ArrayPad$[ebp]
        xor     ecx, ebp
        call    @__security_check_cookie@4
        mov     esp, ebp
        pop     ebp
        ret     0
void FillStackAddresses(void * *,unsigned int,unsigned int) ENDP              ; FillStackAddresses
```

</td>
</table>

While these assemblies **are** different, the functionalities are **not** different. This means the inline assembly is
a lighter implementation than the C++ alternative since it doesn't use intrinsic functions, but accesses the registers
directly.

```diff
@@ -3,19 +3,20 @@ voltbl  SEGMENT
 _volmd  DD  0ffffffffH
         DDSymXIndex:    FLAT:void FillStackAddresses(void * *,unsigned int,unsigned int)
         DD      010H
-        DD      01fcH
+        DD      0200H
 voltbl  ENDS
 
 _myebp$ = -200                                      ; size = 4
 _myesp$ = -196                                      ; size = 4
 _myeip$ = -192                                      ; size = 4
-tv132 = -188                                            ; size = 4
+tv137 = -188                                            ; size = 4
 _process$ = -184                                        ; size = 4
 _thread$ = -180                               ; size = 4
-tv85 = -176                                   ; size = 4
+tv90 = -176                                   ; size = 4
 _stillgoing$1 = -172                                    ; size = 4
 _stack_frame$ = -168                                    ; size = 164
 __$ArrayPad$ = -4                                 ; size = 4
+__$ReturnAddr$ = 4                                  ; size = 4
 _addresses$ = 8                               ; size = 4
 _count$ = 12                                            ; size = 4
 _skip$ = 16                                   ; size = 4
@@ -37,14 +38,14 @@ void FillStackAddresses(void * *,unsigned int,unsigned int) PROC              ;
         call    _memset
         add     esp, 12                             ; 0000000cH
         mov     DWORD PTR _CONTEXT gsContext, 65543 ; 00010007H
-$MYEIP2$14:
-        mov     eax, OFFSET $MYEIP2$14
+        mov     eax, DWORD PTR __$ReturnAddr$[ebp]
         mov     DWORD PTR _myeip$[ebp], eax
-        mov     eax, esp
-        mov     DWORD PTR _myesp$[ebp], eax
-        mov     eax, ebp
-        mov     DWORD PTR _myebp$[ebp], eax
-        xor     eax, eax
+        lea     ecx, DWORD PTR __$ReturnAddr$[ebp]
+        add     ecx, 4
+        mov     DWORD PTR _myesp$[ebp], ecx
+        lea     edx, DWORD PTR __$ReturnAddr$[ebp]
+        sub     edx, 4
+        mov     DWORD PTR _myebp$[ebp], edx
         push    164                           ; 000000a4H
         push    0
         lea     eax, DWORD PTR _stack_frame$[ebp]
@@ -63,9 +64,9 @@ $MYEIP2$14:
         mov     DWORD PTR _stillgoing$1[ebp], 1
 $LN2@FillStackA:
         cmp     DWORD PTR _stillgoing$1[ebp], 0
-        je      SHORT $LN4@FillStackA
+        je      SHORT $LN3@FillStackA
         cmp     DWORD PTR _skip$[ebp], 0
-        je      SHORT $LN4@FillStackA
+        je      SHORT $LN3@FillStackA
         push    0
         mov     ecx, DWORD PTR __imp__SymGetModuleBase@8
         push    ecx
@@ -83,22 +84,22 @@ $LN2@FillStackA:
         call    DWORD PTR __imp__StackWalk@36
         test    eax, eax
         je      SHORT $LN10@FillStackA
-        mov     DWORD PTR tv85[ebp], 1
+        mov     DWORD PTR tv90[ebp], 1
         jmp     SHORT $LN11@FillStackA
 $LN10@FillStackA:
-        mov     DWORD PTR tv85[ebp], 0
+        mov     DWORD PTR tv90[ebp], 0
 $LN11@FillStackA:
-        mov     eax, DWORD PTR tv85[ebp]
+        mov     eax, DWORD PTR tv90[ebp]
         mov     DWORD PTR _stillgoing$1[ebp], eax
         mov     ecx, DWORD PTR _skip$[ebp]
         sub     ecx, 1
         mov     DWORD PTR _skip$[ebp], ecx
         jmp     SHORT $LN2@FillStackA
-$LN4@FillStackA:
+$LN3@FillStackA:
         cmp     DWORD PTR _stillgoing$1[ebp], 0
-        je      $LN6@FillStackA
+        je      $LN5@FillStackA
         cmp     DWORD PTR _count$[ebp], 0
-        je      $LN6@FillStackA
+        je      $LN5@FillStackA
         push    0
         mov     edx, DWORD PTR __imp__SymGetModuleBase@8
         push    edx
@@ -116,12 +117,12 @@ $LN4@FillStackA:
         call    DWORD PTR __imp__StackWalk@36
         test    eax, eax
         je      SHORT $LN12@FillStackA
-        mov     DWORD PTR tv132[ebp], 1
+        mov     DWORD PTR tv137[ebp], 1
         jmp     SHORT $LN13@FillStackA
 $LN12@FillStackA:
-        mov     DWORD PTR tv132[ebp], 0
+        mov     DWORD PTR tv137[ebp], 0
 $LN13@FillStackA:
-        mov     ecx, DWORD PTR tv132[ebp]
+        mov     ecx, DWORD PTR tv137[ebp]
         mov     DWORD PTR _stillgoing$1[ebp], ecx
         cmp     DWORD PTR _stillgoing$1[ebp], 0
         je      SHORT $LN8@FillStackA
@@ -135,10 +136,10 @@ $LN13@FillStackA:
         sub     edx, 1
         mov     DWORD PTR _count$[ebp], edx
 $LN8@FillStackA:
-        jmp     $LN4@FillStackA
-$LN6@FillStackA:
+        jmp     $LN3@FillStackA
+$LN5@FillStackA:
         cmp     DWORD PTR _count$[ebp], 0
-        je      SHORT $LN1@FillStackA
+        je      SHORT $LN7@FillStackA
         mov     eax, DWORD PTR _addresses$[ebp]
         mov     DWORD PTR [eax], 0
         mov     ecx, DWORD PTR _addresses$[ebp]
@@ -147,8 +148,8 @@ $LN6@FillStackA:
         mov     edx, DWORD PTR _count$[ebp]
         sub     edx, 1
         mov     DWORD PTR _count$[ebp], edx
-        jmp     SHORT $LN6@FillStackA
-$LN1@FillStackA:
+        jmp     SHORT $LN5@FillStackA
+$LN7@FillStackA:
         mov     ecx, DWORD PTR __$ArrayPad$[ebp]
         xor     ecx, ebp
         call    @__security_check_cookie@4
```

</details>

</details>

---

<details>
<summary>Generals(MD)/Code/Tools/timingTest/timingTest.cpp</summary>

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

void GetPrecisionTimer(__int64* t)
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
</table>

These are **not** different.

</details>

---

<details>
<summary>Generals(MD)/Code/Libraries/Source/WWVegas/WWMath/vp.cpp</summary>

This file defines the following assembly blocks:

<details>
<summary>BROADCAST</summary>

Defined as:

```c++
#define	BROADCAST(XMM, INDEX)	__asm	shufps	XMM,XMM,(((INDEX)&3)<< 6|((INDEX)&3)<<4|((INDEX)&3)<< 2|((INDEX)&3))
```

This is equivalent to the following function:

```c++
#include <xmmintrin.h>

template<int index>
void BROADCAST(__m128& xmm)
{
    // Create the shuffle control mask
    constexpr int shuffleMask = (index & 3 << 6) | (index & 3 << 4) | (index & 3 << 2) | index & 3;


    // Perform the shuffle operation on the xmm register
    xmm = _mm_shuffle_ps(xmm, xmm, shuffleMask);
}

int main()
{
    __m128 test;
    BROADCAST<1>(test);
    return 0;
}
```

> **NOTE:** While the equality is correct, YOU CANNOT compile this in x86 MSVC for `shuffleMask` must be constant.
> An alternative would be to make it a compile time constant, but I can't generate this without a main function. I
> cannot compile the version with the `#define BROADCAST` block in modern MSVC, so I can't show a diff for the
> generated assemblies

</details>

<details>
<summary>TRANSPOSE</summary>

```c++
#define TRANSPOSE(BX, BY, BZ, BW, TV)					\
		__asm	movaps		TV,BZ						\
		__asm	unpcklps	BZ,BW						\
		__asm	unpckhps	TV,BW						\
		__asm	movaps		BW,BX						\
		__asm	unpcklps	BX,BY						\
		__asm	unpckhps	BW,BY						\
		__asm	movaps		BY,BX						\
		__asm	shufps		BX,BZ,SHUFFLE(1, 0, 1, 0)	\
		__asm	shufps		BY,BZ,SHUFFLE(3, 2, 3, 2)	\
		__asm	movaps		BZ,BW						\
		__asm	shufps		BZ,TV,SHUFFLE(1, 0, 1, 0)	\
		__asm	shufps		BW,TV,SHUFFLE(3, 2, 3, 2)
```

This is shuffling the given `__m128` values.

My goto equivalent is:

```c++
#include <xmmintrin.h>

void TRANSPOSE(__m128& BX, __m128& BY, __m128& BZ, __m128& BW) {
	   __m128 TV;

	   // Step 1: Unpack the lower and upper halves row-wise
	   TV = BZ;
	   BZ = _mm_unpacklo_ps(BZ, BW); // Interleave lower halves of BZ and BW
	   TV = _mm_unpackhi_ps(TV, BW); // Interleave upper halves of BZ and BW

	   BW = BX;
	   BX = _mm_unpacklo_ps(BX, BY); // Interleave lower halves of BX and BY
	   BW = _mm_unpackhi_ps(BW, BY); // Interleave upper halves of BX and BY

	   // Step 2: Shuffle to rearrange rows and columns
	   BY = BX;
	   BX = _mm_shuffle_ps(BX, BZ, _MM_SHUFFLE(1, 0, 1, 0)); // Shuffle: Row 0 (from BX+BZ)
	   BY = _mm_shuffle_ps(BY, BZ, _MM_SHUFFLE(3, 2, 3, 2)); // Shuffle: Row 1 (from BY+BZ)

	   BZ = BW;
	   BZ = _mm_shuffle_ps(BZ, TV, _MM_SHUFFLE(1, 0, 1, 0)); // Shuffle: Row 2 (from BW+TV)
	   BW = _mm_shuffle_ps(BW, TV, _MM_SHUFFLE(3, 2, 3, 2)); // Shuffle: Row 3 (from BW+TV)
}
```

> **NOTE:** While the equality is correct, I cannot generate the `#define TRANSPOSE` block in modern MSVC x86 and I will
> not be able to show the compared assemblies.

</details>

<details>
<summary>VectorProcessorClass::Transform</summary>

```c++
static Vector4 lastrow(0.0f,0.0f,0.0f,1.0f);
void VectorProcessorClass::Transform (Vector3* dst,const Vector3 *src, const Matrix3D& mtx, const int count)
{
	if (count<=0) return;

#if defined (__ICL)    // Detect Intel compiler
	if (CPUDetectClass::_Has_SSE_Instruction_Set()) {

		__asm	{
			mov		edx,dst
			mov		eax,src
			mov		ebx,mtx
			mov		edi,count

			movups	xmm4,[ebx+0]
			movups	xmm5,[ebx+16]
			movups	xmm6,[ebx+32]
			movups	xmm7,lastrow	//[ebx+48]

			TRANSPOSE(xmm4, xmm5, xmm6, xmm7, xmm0);

			shufps	xmm4,xmm4,SHUFFLE(2,1,0,0)
			shufps	xmm5,xmm5,SHUFFLE(2,1,0,0)
			shufps	xmm6,xmm6,SHUFFLE(2,1,0,0)
			shufps	xmm7,xmm7,SHUFFLE(2,1,0,0)

			mov		esi,edx
		_lp:
			test	edi,edi
			jz		_ulos
			test	esi,0xf
			jz		_aligned
			movss	xmm0,[eax]
			movss	xmm1,[eax+4]
			movss	xmm2,[eax+8]
			BROADCAST(xmm0,0)
			BROADCAST(xmm1,0)
			BROADCAST(xmm2,0)
			mulps	xmm0,xmm4
			mulps	xmm1,xmm5
			mulps	xmm2,xmm6
			addps	xmm0,xmm1
			addps	xmm0,xmm2
			addps	xmm0,xmm7
			movss	[edx],xmm0
			movhps	[edx+4],xmm0
			add		eax,12
			add		edx,12
			add		esi,12
			dec		edi
			jmp		_lp
		_aligned:

			mov		esi,1
		
			mov		ecx,edi
			and		edi,3
			and		ecx,~3
			jz		_lp
			
			lea		ecx,[ecx+ecx*2]
			shl		ecx,2
			add		eax,ecx
			add		edx,ecx
			neg		ecx

			cmp		dword ptr [ebx+12],0
			jne		_xlatelp
			cmp		dword ptr [ebx+28],0
			jne		_xlatelp
			cmp		dword ptr [ebx+44],0
			jne		_xlatelp
			jmp		_noxlatelp

			align	16

			_noxlatelp:
				prefetchnta	[eax+ecx+48]
				prefetchnta	[eax+ecx+48+32]

				movss	xmm0,[eax+ecx]
				BROADCAST(xmm0,0)
				movss	xmm1,[eax+ecx+4]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+8]
				BROADCAST(xmm2,0)
				mulps	xmm0,xmm4
				mulps	xmm1,xmm5
				mulps	xmm2,xmm6
				addps	xmm0,xmm1
				addps	xmm0,xmm2

				movss	xmm1,[eax+ecx+12]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+16]
				BROADCAST(xmm2,0)
				movss	xmm3,[eax+ecx+20]
				BROADCAST(xmm3,0)
				mulps	xmm1,xmm4
				mulps	xmm2,xmm5
				mulps	xmm3,xmm6
				addps	xmm1,xmm2
				addps	xmm3,xmm1

				movss	xmm0,xmm3
				shufps	xmm0,xmm0,SHUFFLE(0,3,2,1)

				movaps	[edx+ecx],xmm0

				prefetcht0	[edx+ecx+48]
				prefetcht0	[edx+ecx+48+32]

				movss	xmm0,[eax+ecx+24]
				BROADCAST(xmm0,0)
				movss	xmm1,[eax+ecx+24+4]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+24+8]
				BROADCAST(xmm2,0)
				mulps	xmm0,xmm4
				mulps	xmm1,xmm5
				mulps	xmm2,xmm6
				addps	xmm0,xmm1
				addps	xmm0,xmm2

				shufps	xmm3,xmm0,SHUFFLE(2,1,3,2)
				movaps	[edx+ecx+16],xmm3

				movss	xmm1,[eax+ecx+24+12]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+24+16]
				BROADCAST(xmm2,0)
				movss	xmm3,[eax+ecx+24+20]
				BROADCAST(xmm3,0)
				mulps	xmm1,xmm4
				mulps	xmm2,xmm5
				mulps	xmm3,xmm6
				addps	xmm1,xmm2
				addps	xmm1,xmm3

				shufps	xmm0,xmm0,SHUFFLE(2,1,0,3)
				movss	xmm1,xmm0
				movaps	[edx+ecx+32],xmm1

				add		ecx,48
				js		_noxlatelp

			jmp	_lp

			align	16

			_xlatelp:
				prefetchnta	[eax+ecx+48]
				prefetchnta	[eax+ecx+48+32]

				movss	xmm0,[eax+ecx]
				BROADCAST(xmm0,0)
				movss	xmm1,[eax+ecx+4]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+8]
				BROADCAST(xmm2,0)
				mulps	xmm0,xmm4
				mulps	xmm1,xmm5
				mulps	xmm2,xmm6
				addps	xmm0,xmm1
				addps	xmm0,xmm2
				addps	xmm0,xmm7

				movss	xmm1,[eax+ecx+12]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+16]
				BROADCAST(xmm2,0)
				movss	xmm3,[eax+ecx+20]
				BROADCAST(xmm3,0)
				mulps	xmm1,xmm4
				mulps	xmm2,xmm5
				mulps	xmm3,xmm6
				addps	xmm1,xmm2
				addps	xmm3,xmm1
				addps	xmm3,xmm7

				movss	xmm0,xmm3
				shufps	xmm0,xmm0,SHUFFLE(0,3,2,1)
				movaps	[edx+ecx],xmm0

				prefetcht0	[edx+ecx+48]
				prefetcht0	[edx+ecx+48+32]

				movss	xmm0,[eax+ecx+24]
				BROADCAST(xmm0,0)
				movss	xmm1,[eax+ecx+24+4]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+24+8]
				BROADCAST(xmm2,0)
				mulps	xmm0,xmm4
				mulps	xmm1,xmm5
				mulps	xmm2,xmm6
				addps	xmm0,xmm1
				addps	xmm0,xmm2
				addps	xmm0,xmm7

				shufps	xmm3,xmm0,SHUFFLE(2,1,3,2)
				movaps	[edx+ecx+16],xmm3

				movss	xmm1,[eax+ecx+24+12]
				BROADCAST(xmm1,0)
				movss	xmm2,[eax+ecx+24+16]
				BROADCAST(xmm2,0)
				movss	xmm3,[eax+ecx+24+20]
				BROADCAST(xmm3,0)
				mulps	xmm1,xmm4
				mulps	xmm2,xmm5
				mulps	xmm3,xmm6
				addps	xmm1,xmm2
				addps	xmm1,xmm3
				addps	xmm1,xmm7

				shufps	xmm0,xmm0,SHUFFLE(2,1,0,3)
				movss	xmm1,xmm0

				movaps	[edx+ecx+32],xmm1
				
				add		ecx,48
				js		_xlatelp

			jmp	_lp
		_ulos:
		}

	}
	else
#endif
	{
		mtx.mulVector3Array(src, dst, count);
	}
}
```

This assembly code is trying to do an efficient vector transform.

My goto equivalent is:

```c++
#include <xmmintrin.h>
#include <emmintrin.h>

static Vector4 lastrow(0.0f,0.0f,0.0f,1.0f);
void VectorProcessorClass::Transform (Vector3* dst,const Vector3 *src, const Matrix3D& mtx, const int count)
{
	if (count<=0) return;

#if defined (__ICL)    // Detect Intel compiler
	if (CPUDetectClass::_Has_SSE_Instruction_Set()) {
        // Load rows of the matrix into SSE registers.
        __m128 row1 = __mm_loadu_ps((const float*)(&mtx[0]));
        __m128 row2 = __mm_loadu_ps((const float*)(&mtx[1]));
        __m128 row3 = __mm_loadu_ps((const float*)(&mtx[2]));
        __m128 row4 = __mm_set_ps(1.F, 0.F, 0.F, 0.F);

        // Iterate over the count of vectors to process.
        for (int i = 0; i < count; ++i) {
            // Load source vector components into SSE registers
            __m128 src_x = __mm_set1_ps(src[i].X); // Broadcast src[i].X to all elements
            __m128 src_y = _mm_set1_ps(src[i].Y); // Broadcast src[i].Y to all elements
            __m128 src_z = _mm_set1_ps(src[i].Z); // Broadcast src[i].Z to all elements

            // Perform matrix multiplication for X, Y, Z (and W is assumed to be 1)
            __m128 result = _mm_mul_ps(src_x, row1);
            result = _mm_add_ps(result, _mm_mul_ps(src_y, row2));
            result = _mm_add_ps(result, _mm_mul_ps(src_z, row3));
            result = _mm_add_ps(result, row4);

            // Store the resulting vector into the destination array
            _mm_store_ss(&dst[i].X, result); // Store X
            result = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 2, 1, 1)); // Rotate for Y
            _mm_store_ss(&dst[i].Y, result); // Store Y
            result = _mm_shuffle_ps(result, result, _MM_SHUFFLE(3, 2, 1, 2)); // Rotate for Z
            _mm_store_ss(&dst[i].Z, result); // Store Z
        }
	}
	else
#endif
	{
		mtx.mulVector3Array(src, dst, count);
	}
}
```

> **NOTE**: I don't have access to the Intel compiler; so I can't show the assembly output from this.

</details>

</details>

---

<details>
<summary>GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/mutex.h</summary>

```c++
class FastCriticalSectionClass
{
	unsigned Flag;

public:
	// Name can (and usually should) be NULL. Use name only if you wish to create a globally unique mutex
	FastCriticalSectionClass() : Flag(0) {}

	class LockClass
	{
		FastCriticalSectionClass& cs;
	public:
		__forceinline LockClass(FastCriticalSectionClass& critical_section) : cs(critical_section)
		{
		  unsigned& nFlag=cs.Flag;

		  #define ts_lock _emit 0xF0
		  assert(((unsigned)&nFlag % 4) == 0);

      // I'm terribly sorry for these emits in here but
      // VC won't inline any functions that have labels in them...

      // Had to remove the emits back to normal
      // ASM statements because sometimes the jump
      // would be 1 byte off....
      
		  __asm mov ebx, [nFlag]
		  __asm ts_lock
		  __asm bts dword ptr [ebx], 0
		  __asm jnc BitSet
      //__asm _emit 0x73
      //__asm _emit 0x0f

		  The_Bit_Was_Previously_Set_So_Try_Again:
		    ThreadClass::Switch_Thread();
		  __asm mov ebx, [nFlag]
		  __asm ts_lock
		  __asm bts dword ptr [ebx], 0
		  __asm jc  The_Bit_Was_Previously_Set_So_Try_Again
      //_asm _emit 0x72
      //_asm _emit 0xf1

      BitSet:
        ;
		}

		~LockClass()
		{
      cs.Flag=0;
		}
    
	private:
		LockClass &operator=(const LockClass&);
    LockClass(const LockClass&);
	};

  friend class LockClass;
};
```

This assembly block is a pre-`std::atomic` atomic value access and modification.

My goto equivalent is:

```c++
#include <atomic>
#include <assert.h>

class ThreadClass {
public:
    static void Switch_Thread();
};

class FastCriticalSectionClass
{
    unsigned Flag;
    public:

    class LockClass {
    private:
        FastCriticalSectionClass& cs;

    public:
        LockClass(FastCriticalSectionClass& critical_section) : cs(critical_section) {
            std::atomic<unsigned>& nFlag = (std::atomic<unsigned>&)(cs.Flag);

            // Ensure the address is properly aligned to 4 bytes (as in the `assert` from the original code).
            assert((uintptr_t)(&nFlag) % 4 == 0);

            while (true) {
                unsigned expected = nFlag.load(std::memory_order_relaxed); // Load the current value of the flag.
                // Try to atomically set the 0th bit using compare-and-swap.
                if (nFlag.compare_exchange_weak(expected, expected | 1, std::memory_order_acquire)) {
                    // If `compare_exchange_weak` succeeded, the 0th bit was successfully set.
                    break;
                } else {
                    // If the 0th bit was already set, switch threads.
                    if (expected & 1) {
                        ThreadClass::Switch_Thread();
                    }
                }
            }
        }
    };
};

int main()
{
    FastCriticalSectionClass criticalSection;
    FastCriticalSectionClass::LockClass lock(criticalSection);
}
```

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
`string' DB '('
        DB      00H, '(', 00H, 'u', 00H, 'n', 00H, 's', 00H, 'i', 00H, 'g', 00H
        DB      'n', 00H, 'e', 00H, 'd', 00H, ')', 00H, '&', 00H, 'n', 00H, 'F'
        DB      00H, 'l', 00H, 'a', 00H, 'g', 00H, ' ', 00H, '%', 00H, ' ', 00H
        DB      '4', 00H, ')', 00H, ' ', 00H, '=', 00H, '=', 00H, ' ', 00H, '0'
        DB      00H, 00H, 00H                         ; `string'
`string' DB 'C'
        DB      00H, ':', 00H, '\', 00H, 'W', 00H, 'i', 00H, 'n', 00H, 'd', 00H
        DB      'o', 00H, 'w', 00H, 's', 00H, '\', 00H, 'T', 00H, 'E', 00H, 'M'
        DB      00H, 'P', 00H, '\', 00H, 'c', 00H, 'o', 00H, 'm', 00H, 'p', 00H
        DB      'i', 00H, 'l', 00H, 'e', 00H, 'r', 00H, '-', 00H, 'e', 00H, 'x'
        DB      00H, 'p', 00H, 'l', 00H, 'o', 00H, 'r', 00H, 'e', 00H, 'r', 00H
        DB      '-', 00H, 'c', 00H, 'o', 00H, 'm', 00H, 'p', 00H, 'i', 00H, 'l'
        DB      00H, 'e', 00H, 'r', 00H, '8', 00H, 'J', 00H, 'Z', 00H, 'U', 00H
        DB      '1', 00H, 's', 00H, '\', 00H, 'e', 00H, 'x', 00H, 'a', 00H, 'm'
        DB      00H, 'p', 00H, 'l', 00H, 'e', 00H, '.', 00H, 'c', 00H, 'p', 00H
        DB      'p', 00H, 00H, 00H                      ; `string'

_this$ = -8                                   ; size = 4
_nFlag$ = -4                                            ; size = 4
_critical_section$ = 8                              ; size = 4
FastCriticalSectionClass::LockClass::LockClass(FastCriticalSectionClass &) PROC ; FastCriticalSectionClass::LockClass::LockClass, COMDAT
        push    ebp
        mov     ebp, esp
        sub     esp, 8
        push    ebx
        push    esi
        push    edi
        mov     DWORD PTR _this$[ebp], ecx
        mov     eax, DWORD PTR _this$[ebp]
        mov     ecx, DWORD PTR _critical_section$[ebp]
        mov     DWORD PTR [eax], ecx
        mov     edx, DWORD PTR _this$[ebp]
        mov     eax, DWORD PTR [edx]
        mov     DWORD PTR _nFlag$[ebp], eax
        mov     eax, DWORD PTR _nFlag$[ebp]
        xor     edx, edx
        mov     ecx, 4
        div     ecx
        test    edx, edx
        je      SHORT $LN3@LockClass
        push    23                                  ; 00000017H
        push    OFFSET `string'
        push    OFFSET `string'
        call    __wassert
        add     esp, 12                             ; 0000000cH
$LN3@LockClass:
        mov     ebx, DWORD PTR _nFlag$[ebp]
        DB      -16                       ; fffffff0H
        bts     DWORD PTR [ebx], 0
        jae     SHORT $BitSet$4
$The_Bit_Was_Previously_Set_So_Try_Again$5:
        call    static void ThreadClass::Switch_Thread(void)       ; ThreadClass::Switch_Thread
        mov     ebx, DWORD PTR _nFlag$[ebp]
        DB      -16                       ; fffffff0H
        bts     DWORD PTR [ebx], 0
        jb      SHORT $The_Bit_Was_Previously_Set_So_Try_Again$5
$BitSet$4:
        mov     eax, DWORD PTR _this$[ebp]
        pop     edi
        pop     esi
        pop     ebx
        mov     esp, ebp
        pop     ebp
        ret     4
FastCriticalSectionClass::LockClass::LockClass(FastCriticalSectionClass &) ENDP ; FastCriticalSectionClass::LockClass::LockClass

_lock$ = -8                                   ; size = 4
_criticalSection$ = -4                              ; size = 4
_main   PROC
        push    ebp
        mov     ebp, esp
        sub     esp, 8
        lea     eax, DWORD PTR _criticalSection$[ebp]
        push    eax
        lea     ecx, DWORD PTR _lock$[ebp]
        call    FastCriticalSectionClass::LockClass::LockClass(FastCriticalSectionClass &) ; FastCriticalSectionClass::LockClass::LockClass
        npad    1
        xor     eax, eax
        mov     esp, ebp
        pop     ebp
        ret     0
_main   ENDP
```

</td>
<td>

```asm
`string' DB '('
        DB      00H, 'u', 00H, 'i', 00H, 'n', 00H, 't', 00H, 'p', 00H, 't', 00H
        DB      'r', 00H, '_', 00H, 't', 00H, ')', 00H, '(', 00H, '&', 00H, 'n'
        DB      00H, 'F', 00H, 'l', 00H, 'a', 00H, 'g', 00H, ')', 00H, ' ', 00H
        DB      '%', 00H, ' ', 00H, '4', 00H, ' ', 00H, '=', 00H, '=', 00H, ' '
        DB      00H, '0', 00H, 00H, 00H           ; `string'
`string' DB 'C'
        DB      00H, ':', 00H, '\', 00H, 'W', 00H, 'i', 00H, 'n', 00H, 'd', 00H
        DB      'o', 00H, 'w', 00H, 's', 00H, '\', 00H, 'T', 00H, 'E', 00H, 'M'
        DB      00H, 'P', 00H, '\', 00H, 'c', 00H, 'o', 00H, 'm', 00H, 'p', 00H
        DB      'i', 00H, 'l', 00H, 'e', 00H, 'r', 00H, '-', 00H, 'e', 00H, 'x'
        DB      00H, 'p', 00H, 'l', 00H, 'o', 00H, 'r', 00H, 'e', 00H, 'r', 00H
        DB      '-', 00H, 'c', 00H, 'o', 00H, 'm', 00H, 'p', 00H, 'i', 00H, 'l'
        DB      00H, 'e', 00H, 'r', 00H, 'E', 00H, 'b', 00H, '9', 00H, 'W', 00H
        DB      'b', 00H, 'c', 00H, '\', 00H, 'e', 00H, 'x', 00H, 'a', 00H, 'm'
        DB      00H, 'p', 00H, 'l', 00H, 'e', 00H, '.', 00H, 'c', 00H, 'p', 00H
        DB      'p', 00H, 00H, 00H                      ; `string'
voltbl  SEGMENT
_volmd  DB  01bH
        DB      03cH
voltbl  ENDS

_this$ = -12                                            ; size = 4
_expected$1 = -8                                        ; size = 4
_nFlag$ = -4                                            ; size = 4
_critical_section$ = 8                              ; size = 4
FastCriticalSectionClass::LockClass::LockClass(FastCriticalSectionClass &) PROC ; FastCriticalSectionClass::LockClass::LockClass, COMDAT
        push    ebp
        mov     ebp, esp
        sub     esp, 12                             ; 0000000cH
        mov     DWORD PTR _this$[ebp], ecx
        mov     eax, DWORD PTR _this$[ebp]
        mov     ecx, DWORD PTR _critical_section$[ebp]
        mov     DWORD PTR [eax], ecx
        mov     edx, DWORD PTR _this$[ebp]
        mov     eax, DWORD PTR [edx]
        mov     DWORD PTR _nFlag$[ebp], eax
        mov     eax, DWORD PTR _nFlag$[ebp]
        xor     edx, edx
        mov     ecx, 4
        div     ecx
        test    edx, edx
        je      SHORT $LN8@LockClass
        push    23                                  ; 00000017H
        push    OFFSET `string'
        push    OFFSET `string'
        call    __wassert
        add     esp, 12                             ; 0000000cH
$LN8@LockClass:
        mov     eax, 1
        test    eax, eax
        je      SHORT $LN3@LockClass
        push    0
        mov     ecx, DWORD PTR _nFlag$[ebp]
        call    unsigned int std::_Atomic_storage<unsigned int,4>::load(std::memory_order)const  ; std::_Atomic_storage<unsigned int,4>::load
        mov     DWORD PTR _expected$1[ebp], eax
        push    2
        mov     ecx, DWORD PTR _expected$1[ebp]
        or      ecx, 1
        push    ecx
        lea     edx, DWORD PTR _expected$1[ebp]
        push    edx
        mov     ecx, DWORD PTR _nFlag$[ebp]
        call    bool std::atomic<unsigned int>::compare_exchange_weak(unsigned int &,unsigned int,std::memory_order) ; std::atomic<unsigned int>::compare_exchange_weak
        movzx   eax, al
        test    eax, eax
        je      SHORT $LN4@LockClass
        jmp     SHORT $LN3@LockClass
        jmp     SHORT $LN6@LockClass
$LN4@LockClass:
        mov     ecx, DWORD PTR _expected$1[ebp]
        and     ecx, 1
        je      SHORT $LN6@LockClass
        call    static void ThreadClass::Switch_Thread(void)       ; ThreadClass::Switch_Thread
        npad    1
$LN6@LockClass:
        jmp     SHORT $LN8@LockClass
$LN3@LockClass:
        mov     eax, DWORD PTR _this$[ebp]
        mov     esp, ebp
        pop     ebp
        ret     4
FastCriticalSectionClass::LockClass::LockClass(FastCriticalSectionClass &) ENDP ; FastCriticalSectionClass::LockClass::LockClass

_lock$ = -8                                   ; size = 4
_criticalSection$ = -4                              ; size = 4
_main   PROC
        push    ebp
        mov     ebp, esp
        sub     esp, 8
        lea     eax, DWORD PTR _criticalSection$[ebp]
        push    eax
        lea     ecx, DWORD PTR _lock$[ebp]
        call    FastCriticalSectionClass::LockClass::LockClass(FastCriticalSectionClass &) ; FastCriticalSectionClass::LockClass::LockClass
        npad    1
        xor     eax, eax
        mov     esp, ebp
        pop     ebp
        ret     0
_main   ENDP
```

</td>
</table>

These assemblies can't be identical due to the differences in atomic access. It is probable that this method can be
further improved by creating a class specifically tailored for modern C++ atomics.

```diff
@@ -1,9 +1,9 @@
 `string' DB '('
-        DB      00H, '(', 00H, 'u', 00H, 'n', 00H, 's', 00H, 'i', 00H, 'g', 00H
-        DB      'n', 00H, 'e', 00H, 'd', 00H, ')', 00H, '&', 00H, 'n', 00H, 'F'
-        DB      00H, 'l', 00H, 'a', 00H, 'g', 00H, ' ', 00H, '%', 00H, ' ', 00H
-        DB      '4', 00H, ')', 00H, ' ', 00H, '=', 00H, '=', 00H, ' ', 00H, '0'
-        DB      00H, 00H, 00H                         ; `string'
+        DB      00H, 'u', 00H, 'i', 00H, 'n', 00H, 't', 00H, 'p', 00H, 't', 00H
+        DB      'r', 00H, '_', 00H, 't', 00H, ')', 00H, '(', 00H, '&', 00H, 'n'
+        DB      00H, 'F', 00H, 'l', 00H, 'a', 00H, 'g', 00H, ')', 00H, ' ', 00H
+        DB      '%', 00H, ' ', 00H, '4', 00H, ' ', 00H, '=', 00H, '=', 00H, ' '
+        DB      00H, '0', 00H, 00H, 00H           ; `string'
 `string' DB 'C'
         DB      00H, ':', 00H, '\', 00H, 'W', 00H, 'i', 00H, 'n', 00H, 'd', 00H
         DB      'o', 00H, 'w', 00H, 's', 00H, '\', 00H, 'T', 00H, 'E', 00H, 'M'
@@ -11,21 +11,23 @@
         DB      'i', 00H, 'l', 00H, 'e', 00H, 'r', 00H, '-', 00H, 'e', 00H, 'x'
         DB      00H, 'p', 00H, 'l', 00H, 'o', 00H, 'r', 00H, 'e', 00H, 'r', 00H
         DB      '-', 00H, 'c', 00H, 'o', 00H, 'm', 00H, 'p', 00H, 'i', 00H, 'l'
-        DB      00H, 'e', 00H, 'r', 00H, '8', 00H, 'J', 00H, 'Z', 00H, 'U', 00H
-        DB      '1', 00H, 's', 00H, '\', 00H, 'e', 00H, 'x', 00H, 'a', 00H, 'm'
+        DB      00H, 'e', 00H, 'r', 00H, 'E', 00H, 'b', 00H, '9', 00H, 'W', 00H
+        DB      'b', 00H, 'c', 00H, '\', 00H, 'e', 00H, 'x', 00H, 'a', 00H, 'm'
         DB      00H, 'p', 00H, 'l', 00H, 'e', 00H, '.', 00H, 'c', 00H, 'p', 00H
         DB      'p', 00H, 00H, 00H                      ; `string'
+voltbl  SEGMENT
+_volmd  DB  01bH
+        DB      03cH
+voltbl  ENDS
 
-_this$ = -8                                   ; size = 4
+_this$ = -12                                            ; size = 4
+_expected$1 = -8                                        ; size = 4
 _nFlag$ = -4                                            ; size = 4
 _critical_section$ = 8                              ; size = 4
 FastCriticalSectionClass::LockClass::LockClass(FastCriticalSectionClass &) PROC ; FastCriticalSectionClass::LockClass::LockClass, COMDAT
         push    ebp
         mov     ebp, esp
-        sub     esp, 8
-        push    ebx
-        push    esi
-        push    edi
+        sub     esp, 12                             ; 0000000cH
         mov     DWORD PTR _this$[ebp], ecx
         mov     eax, DWORD PTR _this$[ebp]
         mov     ecx, DWORD PTR _critical_section$[ebp]
@@ -38,28 +40,43 @@ FastCriticalSectionClass::LockClass::LockClass(FastCriticalSectionClass &) PROC
         mov     ecx, 4
         div     ecx
         test    edx, edx
-        je      SHORT $LN3@LockClass
+        je      SHORT $LN8@LockClass
         push    23                                  ; 00000017H
         push    OFFSET `string'
         push    OFFSET `string'
         call    __wassert
         add     esp, 12                             ; 0000000cH
-$LN3@LockClass:
-        mov     ebx, DWORD PTR _nFlag$[ebp]
-        DB      -16                       ; fffffff0H
-        bts     DWORD PTR [ebx], 0
-        jae     SHORT $BitSet$4
-$The_Bit_Was_Previously_Set_So_Try_Again$5:
+$LN8@LockClass:
+        mov     eax, 1
+        test    eax, eax
+        je      SHORT $LN3@LockClass
+        push    0
+        mov     ecx, DWORD PTR _nFlag$[ebp]
+        call    unsigned int std::_Atomic_storage<unsigned int,4>::load(std::memory_order)const  ; std::_Atomic_storage<unsigned int,4>::load
+        mov     DWORD PTR _expected$1[ebp], eax
+        push    2
+        mov     ecx, DWORD PTR _expected$1[ebp]
+        or      ecx, 1
+        push    ecx
+        lea     edx, DWORD PTR _expected$1[ebp]
+        push    edx
+        mov     ecx, DWORD PTR _nFlag$[ebp]
+        call    bool std::atomic<unsigned int>::compare_exchange_weak(unsigned int &,unsigned int,std::memory_order) ; std::atomic<unsigned int>::compare_exchange_weak
+        movzx   eax, al
+        test    eax, eax
+        je      SHORT $LN4@LockClass
+        jmp     SHORT $LN3@LockClass
+        jmp     SHORT $LN6@LockClass
+$LN4@LockClass:
+        mov     ecx, DWORD PTR _expected$1[ebp]
+        and     ecx, 1
+        je      SHORT $LN6@LockClass
         call    static void ThreadClass::Switch_Thread(void)       ; ThreadClass::Switch_Thread
-        mov     ebx, DWORD PTR _nFlag$[ebp]
-        DB      -16                       ; fffffff0H
-        bts     DWORD PTR [ebx], 0
-        jb      SHORT $The_Bit_Was_Previously_Set_So_Try_Again$5
-$BitSet$4:
+        npad    1
+$LN6@LockClass:
+        jmp     SHORT $LN8@LockClass
+$LN3@LockClass:
         mov     eax, DWORD PTR _this$[ebp]
-        pop     edi
-        pop     esi
-        pop     ebx
         mov     esp, ebp
         pop     ebp
         ret     4
```

</details>

---

<details>
<summary>Generals/Code/Libraries/Source/WWVegas/WWDebug/wwdebug.h</summary>

This header defines the following with assembly:

```c++
#ifdef WWDEBUG
#define WWDEBUG_BREAK							_asm int 0x03
#else
#define WWDEBUG_BREAK							_asm int 0x03
#endif
```

This is redundant as it is, but `_asm int 0x03` is equivalent to `__debugbreak();` on Win32:

```c++
#include <intrin.h>

#define WWDEBUG_BREAK __debugbreak();
```

> **NOTE**: Observe that for this program:
>
> ```c++
> #include <intrin.h>
> 
> #define WWDEBUG_BREAK							_asm int 0x03
> 
> void first() {
>     WWDEBUG_BREAK;
> }
> 
> void second() {
>     __debugbreak();
> }
> ```
>
> The resulting assembly is:
>
> ```asm
> void first(void) PROC                                  ; first
>         push    ebp
>         mov     ebp, esp
>         int     3
>         pop     ebp
>         ret     0
> void first(void) ENDP                                  ; first
> 
> void second(void) PROC                           ; second
>         push    ebp
>         mov     ebp, esp
>         int     3
>         pop     ebp
>         ret     0
> void second(void) ENDP                           ; second
> ```
>
> These `#ifdef` blocks will be useful for cross-platform compatibility:
>
> ```c++
> #ifdef _WIN32
> #include <intrin.h>
> 
> #define WWDEBUG_BREAK __debugbreak();
> #else
> #include <signal.h>
> 
> #define WWDEBUG_BREAK raise(SIGTRAP);
> #endif
> ```

</details>

---

<details>
<summary>GeneralsMD/Code/Libraries/Source/WWVegas/WWDebug/wwdebug.cpp</summary>

This file includes the following assembly block:

```c++
void WWDebug_Assert_Fail(const char * expr,const char * file, int line)
{
	if (_CurAssertHandler != NULL) {

		char buffer[4096];
		sprintf(buffer,"%s (%d) Assert: %s\n",file,line,expr);
		_CurAssertHandler(buffer);

	} else {

		/*
		// If the exception handler is try to quit the game then don't show an assert.
		*/
		if (Is_Trying_To_Exit()) {
			ExitProcess(0);
		}

      char assertbuf[4096];
		sprintf(assertbuf, "Assert failed\n\n. File %s Line %d", file, line);

      int code = MessageBoxA(NULL, assertbuf, "WWDebug_Assert_Fail", MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

      if (code == IDABORT) {
      	raise(SIGABRT);
      	_exit(3);
      }

		if (code == IDRETRY) {
			_asm int 3;
      	return;
		}
   }
}
```

This is simply triggering a debugger breakpoint if `code == IDRETRY`.

My goto equivalent is:

```c++
#include <intrin.h>

void WWDebug_Assert_Fail(const char * expr,const char * file, int line)
{
	if (_CurAssertHandler != NULL) {

		char buffer[4096];
		sprintf(buffer,"%s (%d) Assert: %s\n",file,line,expr);
		_CurAssertHandler(buffer);

	} else {

		/*
		// If the exception handler is try to quit the game then don't show an assert.
		*/
		if (Is_Trying_To_Exit()) {
			ExitProcess(0);
		}

      char assertbuf[4096];
		sprintf(assertbuf, "Assert failed\n\n. File %s Line %d", file, line);

      int code = MessageBoxA(NULL, assertbuf, "WWDebug_Assert_Fail", MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

      if (code == IDABORT) {
      	raise(SIGABRT);
      	_exit(3);
      }

		if (code == IDRETRY) {
			__debugbreak();
      	return;
		}
   }
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
$SG123482 DB    '%s (%d) Assert: %s', 0aH, 00H
$SG123484 DB    'Assert failed', 0aH, 0aH, '. File %s Line %d', 00H
        ORG $+3
$SG123485 DB    'WWDebug_Assert_Fail', 00H
unsigned __int64 `__local_stdio_printf_options'::`2'::_OptionsStorage DQ 01H DUP (?) ; `__local_stdio_printf_options'::`2'::_OptionsStorage
void (__cdecl* _CurAssertHandler)(char const *) DD 01H DUP (?)      ; _CurAssertHandler
voltbl  SEGMENT
_volmd  DD  0ffffffffH
        DDSymXIndex:    FLAT:void WWDebug_Assert_Fail(char const *,char const *,int)
        DD      014H
        DD      0c7H
voltbl  ENDS

_code$1 = -8200                               ; size = 4
_assertbuf$2 = -8196                                    ; size = 4096
_buffer$3 = -4100                                 ; size = 4096
__$ArrayPad$ = -4                                 ; size = 4
_expr$ = 8                                          ; size = 4
_file$ = 12                                   ; size = 4
_line$ = 16                                   ; size = 4
void WWDebug_Assert_Fail(char const *,char const *,int) PROC             ; WWDebug_Assert_Fail
        push    ebp
        mov     ebp, esp
        mov     eax, 8200               ; 00002008H
        call    __chkstk
        mov     eax, DWORD PTR ___security_cookie
        xor     eax, ebp
        mov     DWORD PTR __$ArrayPad$[ebp], eax
        cmp     DWORD PTR void (__cdecl* _CurAssertHandler)(char const *), 0
        je      SHORT $LN2@WWDebug_As
        mov     eax, DWORD PTR _expr$[ebp]
        push    eax
        mov     ecx, DWORD PTR _line$[ebp]
        push    ecx
        mov     edx, DWORD PTR _file$[ebp]
        push    edx
        push    OFFSET $SG123482
        lea     eax, DWORD PTR _buffer$3[ebp]
        push    eax
        call    _sprintf
        add     esp, 20                             ; 00000014H
        lea     ecx, DWORD PTR _buffer$3[ebp]
        push    ecx
        call    DWORD PTR void (__cdecl* _CurAssertHandler)(char const *)
        add     esp, 4
        jmp     SHORT $LN7@WWDebug_As
$LN2@WWDebug_As:
        call    bool Is_Trying_To_Exit(void)          ; Is_Trying_To_Exit
        movzx   edx, al
        test    edx, edx
        je      SHORT $LN4@WWDebug_As
        push    0
        call    DWORD PTR __imp__ExitProcess@4
        npad    1
$LN4@WWDebug_As:
        mov     eax, DWORD PTR _line$[ebp]
        push    eax
        mov     ecx, DWORD PTR _file$[ebp]
        push    ecx
        push    OFFSET $SG123484
        lea     edx, DWORD PTR _assertbuf$2[ebp]
        push    edx
        call    _sprintf
        add     esp, 16                             ; 00000010H
        push    73746                             ; 00012012H
        push    OFFSET $SG123485
        lea     eax, DWORD PTR _assertbuf$2[ebp]
        push    eax
        push    0
        call    DWORD PTR __imp__MessageBoxA@16
        mov     DWORD PTR _code$1[ebp], eax
        cmp     DWORD PTR _code$1[ebp], 3
        jne     SHORT $LN5@WWDebug_As
        push    22                                  ; 00000016H
        call    _raise
        add     esp, 4
        push    3
        call    __exit
        npad    1
$LN5@WWDebug_As:
        cmp     DWORD PTR _code$1[ebp], 4
        jne     SHORT $LN7@WWDebug_As
        int     3
$LN7@WWDebug_As:
        mov     ecx, DWORD PTR __$ArrayPad$[ebp]
        xor     ecx, ebp
        call    @__security_check_cookie@4
        mov     esp, ebp
        pop     ebp
        ret     0
void WWDebug_Assert_Fail(char const *,char const *,int) ENDP             ; WWDebug_Assert_Fail
```

</td>
<td>

```asm
$SG123482 DB    '%s (%d) Assert: %s', 0aH, 00H
$SG123484 DB    'Assert failed', 0aH, 0aH, '. File %s Line %d', 00H
        ORG $+3
$SG123485 DB    'WWDebug_Assert_Fail', 00H
unsigned __int64 `__local_stdio_printf_options'::`2'::_OptionsStorage DQ 01H DUP (?) ; `__local_stdio_printf_options'::`2'::_OptionsStorage
void (__cdecl* _CurAssertHandler)(char const *) DD 01H DUP (?)      ; _CurAssertHandler
voltbl  SEGMENT
_volmd  DD  0ffffffffH
        DDSymXIndex:    FLAT:void WWDebug_Assert_Fail(char const *,char const *,int)
        DD      014H
        DD      0c7H
voltbl  ENDS

_code$1 = -8200                               ; size = 4
_assertbuf$2 = -8196                                    ; size = 4096
_buffer$3 = -4100                                 ; size = 4096
__$ArrayPad$ = -4                                 ; size = 4
_expr$ = 8                                          ; size = 4
_file$ = 12                                   ; size = 4
_line$ = 16                                   ; size = 4
void WWDebug_Assert_Fail(char const *,char const *,int) PROC             ; WWDebug_Assert_Fail
        push    ebp
        mov     ebp, esp
        mov     eax, 8200               ; 00002008H
        call    __chkstk
        mov     eax, DWORD PTR ___security_cookie
        xor     eax, ebp
        mov     DWORD PTR __$ArrayPad$[ebp], eax
        cmp     DWORD PTR void (__cdecl* _CurAssertHandler)(char const *), 0
        je      SHORT $LN2@WWDebug_As
        mov     eax, DWORD PTR _expr$[ebp]
        push    eax
        mov     ecx, DWORD PTR _line$[ebp]
        push    ecx
        mov     edx, DWORD PTR _file$[ebp]
        push    edx
        push    OFFSET $SG123482
        lea     eax, DWORD PTR _buffer$3[ebp]
        push    eax
        call    _sprintf
        add     esp, 20                             ; 00000014H
        lea     ecx, DWORD PTR _buffer$3[ebp]
        push    ecx
        call    DWORD PTR void (__cdecl* _CurAssertHandler)(char const *)
        add     esp, 4
        jmp     SHORT $LN6@WWDebug_As
$LN2@WWDebug_As:
        call    bool Is_Trying_To_Exit(void)          ; Is_Trying_To_Exit
        movzx   edx, al
        test    edx, edx
        je      SHORT $LN4@WWDebug_As
        push    0
        call    DWORD PTR __imp__ExitProcess@4
        npad    1
$LN4@WWDebug_As:
        mov     eax, DWORD PTR _line$[ebp]
        push    eax
        mov     ecx, DWORD PTR _file$[ebp]
        push    ecx
        push    OFFSET $SG123484
        lea     edx, DWORD PTR _assertbuf$2[ebp]
        push    edx
        call    _sprintf
        add     esp, 16                             ; 00000010H
        push    73746                             ; 00012012H
        push    OFFSET $SG123485
        lea     eax, DWORD PTR _assertbuf$2[ebp]
        push    eax
        push    0
        call    DWORD PTR __imp__MessageBoxA@16
        mov     DWORD PTR _code$1[ebp], eax
        cmp     DWORD PTR _code$1[ebp], 3
        jne     SHORT $LN5@WWDebug_As
        push    22                                  ; 00000016H
        call    _raise
        add     esp, 4
        push    3
        call    __exit
        npad    1
$LN5@WWDebug_As:
        cmp     DWORD PTR _code$1[ebp], 4
        jne     SHORT $LN6@WWDebug_As
        int     3
$LN6@WWDebug_As:
$LN1@WWDebug_As:
        mov     ecx, DWORD PTR __$ArrayPad$[ebp]
        xor     ecx, ebp
        call    @__security_check_cookie@4
        mov     esp, ebp
        pop     ebp
        ret     0
void WWDebug_Assert_Fail(char const *,char const *,int) ENDP             ; WWDebug_Assert_Fail
```

</td>
</table>

While the produced assemblies are mildly different, the effect is identical.

```diff
@@ -43,7 +43,7 @@ void WWDebug_Assert_Fail(char const *,char const *,int) PROC             ; WWDeb
         push    ecx
         call    DWORD PTR void (__cdecl* _CurAssertHandler)(char const *)
         add     esp, 4
-        jmp     SHORT $LN7@WWDebug_As
+        jmp     SHORT $LN6@WWDebug_As
 $LN2@WWDebug_As:
         call    bool Is_Trying_To_Exit(void)          ; Is_Trying_To_Exit
         movzx   edx, al
@@ -79,9 +79,10 @@ $LN4@WWDebug_As:
         npad    1
 $LN5@WWDebug_As:
         cmp     DWORD PTR _code$1[ebp], 4
-        jne     SHORT $LN7@WWDebug_As
+        jne     SHORT $LN6@WWDebug_As
         int     3
-$LN7@WWDebug_As:
+$LN6@WWDebug_As:
+$LN1@WWDebug_As:
         mov     ecx, DWORD PTR __$ArrayPad$[ebp]
         xor     ecx, ebp
         call    @__security_check_cookie@4
```

</details>

---

<details>
<summary>Generals/Code/Libraries/Source/WWVegas/WWMath/wwmath.h</summary>

```c++
#if defined(_MSC_VER) && defined(_M_IX86)
WWINLINE __declspec(naked) float __fastcall WWMath::Inv_Sqrt(float a)
{
	__asm {
		mov		eax, 0be6eb508h
		mov		DWORD PTR [esp-12],03fc00000h ;  1.5 on the stack
		sub		eax, DWORD PTR [esp+4]; a
		sub		DWORD PTR [esp+4], 800000h ; a/2 a=Y0
		shr		eax, 1     ; firs approx in eax=R0
		mov		DWORD PTR [esp-8], eax

		fld		DWORD PTR [esp-8] ;r
		fmul	st, st            ;r*r
		fld		DWORD PTR [esp-8] ;r
		fxch	st(1)
		fmul	DWORD PTR [esp+4];a ;r*r*y0
		fld		DWORD PTR [esp-12];load 1.5
		fld		st(0)
		fsub	st,st(2)			   ;r1 = 1.5 - y1
		;x1 = st(3)
		;y1 = st(2)
		;1.5 = st(1)
		;r1 = st(0)

		fld		st(1)
		fxch	st(1)
		fmul	st(3),st			; y2=y1*r1*...
		fmul	st(3),st			; y2=y1*r1*r1
		fmulp	st(4),st            ; x2=x1*r1
		fsub	st,st(2)               ; r2=1.5-y2
		;x2=st(3)
		;y2=st(2)
		;1.5=st(1)
		;r2 = st(0)

		fmul	st(2),st			;y3=y2*r2*...
		fmul	st(3),st			;x3=x2*r2
		fmulp	st(2),st			;y3=y2*r2*r2
		fxch	st(1)
		fsubp	st(1),st			;r3= 1.5 - y3
		;x3 = st(1)
		;r3 = st(0)
		fmulp	st(1), st
		ret 4
	}
}
#else
WWINLINE float WWMath::Inv_Sqrt(float val)
{
	return 1.0f / (float)sqrt(val);
}
#endif
```

This is the [fast inverse square root from Id Software](https://en.wikipedia.org/wiki/Fast_inverse_square_root).

My goto equivalent is:

```c++
class WWMath {
    public:
    inline float __fastcall Inv_Sqrt(float a);
};

#if defined(_MSC_VER) && defined(_M_IX86)
inline float __fastcall WWMath::Inv_Sqrt(float a)
{
	if (a == 0.0f) return 0.0f;

   long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = a * 0.5F;
	y  = a;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration

	return y;
}
#else
inline float WWMath::Inv_Sqrt(float val)
{
	return 1.0f / (float)sqrt(val);
}
#endif
```

<table>
<tr>
<th>With Inline Assembly</th>
<th>Without Inline Assembly</th>
</tr>
<td>

```asm
_a$ = 8                                       ; size = 4
float WWMath::Inv_Sqrt(float) PROC                      ; WWMath::Inv_Sqrt
        mov     eax, -1100040952              ; be6eb508H
        mov     DWORD PTR [esp-12], 1069547520            ; 3fc00000H
        sub     eax, DWORD PTR [esp+4]
        sub     DWORD PTR [esp+4], 8388608                ; 00800000H
        shr     eax, 1
        mov     DWORD PTR [esp-8], eax
        fld     DWORD PTR [esp-8]
        fmul    ST(0), ST(0)
        fld     DWORD PTR [esp-8]
        fxch    ST(1)
        fmul    DWORD PTR [esp+4]
        fld     DWORD PTR [esp-12]
        fld     ST(0)
        fsub    ST(0), ST(2)
        fld     ST(1)
        fxch    ST(1)
        fmul    ST(3), ST(0)
        fmul    ST(3), ST(0)
        fmulp   ST(4), ST(0)
        fsub    ST(0), ST(2)
        fmul    ST(2), ST(0)
        fmul    ST(3), ST(0)
        fmulp   ST(2), ST(0)
        fxch    ST(1)
        fsubp   ST(1), ST(0)
        fmulp   ST(1), ST(0)
        ret     4
float WWMath::Inv_Sqrt(float) ENDP                      ; WWMath::Inv_Sqrt
```

</td>
<td>

```asm
__real@3fc00000 DD 03fc00000r             ; 1.5
__real@3f000000 DD 03f000000r             ; 0.5
__real@00000000 DD 000000000r             ; 0

_threehalfs$ = -20                                  ; size = 4
_this$ = -16                                            ; size = 4
_x2$ = -12                                          ; size = 4
_i$ = -8                                                ; size = 4
_y$ = -4                                                ; size = 4
_a$ = 8                                       ; size = 4
float WWMath::Inv_Sqrt(float) PROC                      ; WWMath::Inv_Sqrt
        push    ebp
        mov     ebp, esp
        sub     esp, 20                             ; 00000014H
        mov     DWORD PTR _this$[ebp], ecx
        movss   xmm0, DWORD PTR _a$[ebp]
        ucomiss xmm0, DWORD PTR __real@00000000
        lahf
        test    ah, 68                              ; 00000044H
        jp      SHORT $LN2@Inv_Sqrt
        fldz
        jmp     SHORT $LN1@Inv_Sqrt
$LN2@Inv_Sqrt:
        movss   xmm0, DWORD PTR __real@3fc00000
        movss   DWORD PTR _threehalfs$[ebp], xmm0
        movss   xmm0, DWORD PTR _a$[ebp]
        mulss   xmm0, DWORD PTR __real@3f000000
        movss   DWORD PTR _x2$[ebp], xmm0
        movss   xmm0, DWORD PTR _a$[ebp]
        movss   DWORD PTR _y$[ebp], xmm0
        mov     eax, DWORD PTR _y$[ebp]
        mov     DWORD PTR _i$[ebp], eax
        mov     ecx, DWORD PTR _i$[ebp]
        sar     ecx, 1
        mov     edx, 1597463007                     ; 5f3759dfH
        sub     edx, ecx
        mov     DWORD PTR _i$[ebp], edx
        movss   xmm0, DWORD PTR _i$[ebp]
        movss   DWORD PTR _y$[ebp], xmm0
        movss   xmm0, DWORD PTR _x2$[ebp]
        mulss   xmm0, DWORD PTR _y$[ebp]
        mulss   xmm0, DWORD PTR _y$[ebp]
        movss   xmm1, DWORD PTR __real@3fc00000
        subss   xmm1, xmm0
        mulss   xmm1, DWORD PTR _y$[ebp]
        movss   DWORD PTR _y$[ebp], xmm1
        fld     DWORD PTR _y$[ebp]
$LN1@Inv_Sqrt:
        mov     esp, ebp
        pop     ebp
        ret     4
float WWMath::Inv_Sqrt(float) ENDP                      ; WWMath::Inv_Sqrt
```

</td>
</table>

This will definitely be slower/bigger unless you enable `/O2` and equivalents.

```diff
@@ -1,30 +1,52 @@
+__real@3fc00000 DD 03fc00000r             ; 1.5
+__real@3f000000 DD 03f000000r             ; 0.5
+__real@00000000 DD 000000000r             ; 0
+
+_threehalfs$ = -20                                  ; size = 4
+_this$ = -16                                            ; size = 4
+_x2$ = -12                                          ; size = 4
+_i$ = -8                                                ; size = 4
+_y$ = -4                                                ; size = 4
 _a$ = 8                                       ; size = 4
 float WWMath::Inv_Sqrt(float) PROC                      ; WWMath::Inv_Sqrt
-        mov     eax, -1100040952              ; be6eb508H
-        mov     DWORD PTR [esp-12], 1069547520            ; 3fc00000H
-        sub     eax, DWORD PTR [esp+4]
-        sub     DWORD PTR [esp+4], 8388608                ; 00800000H
-        shr     eax, 1
-        mov     DWORD PTR [esp-8], eax
-        fld     DWORD PTR [esp-8]
-        fmul    ST(0), ST(0)
-        fld     DWORD PTR [esp-8]
-        fxch    ST(1)
-        fmul    DWORD PTR [esp+4]
-        fld     DWORD PTR [esp-12]
-        fld     ST(0)
-        fsub    ST(0), ST(2)
-        fld     ST(1)
-        fxch    ST(1)
-        fmul    ST(3), ST(0)
-        fmul    ST(3), ST(0)
-        fmulp   ST(4), ST(0)
-        fsub    ST(0), ST(2)
-        fmul    ST(2), ST(0)
-        fmul    ST(3), ST(0)
-        fmulp   ST(2), ST(0)
-        fxch    ST(1)
-        fsubp   ST(1), ST(0)
-        fmulp   ST(1), ST(0)
+        push    ebp
+        mov     ebp, esp
+        sub     esp, 20                             ; 00000014H
+        mov     DWORD PTR _this$[ebp], ecx
+        movss   xmm0, DWORD PTR _a$[ebp]
+        ucomiss xmm0, DWORD PTR __real@00000000
+        lahf
+        test    ah, 68                              ; 00000044H
+        jp      SHORT $LN2@Inv_Sqrt
+        fldz
+        jmp     SHORT $LN1@Inv_Sqrt
+$LN2@Inv_Sqrt:
+        movss   xmm0, DWORD PTR __real@3fc00000
+        movss   DWORD PTR _threehalfs$[ebp], xmm0
+        movss   xmm0, DWORD PTR _a$[ebp]
+        mulss   xmm0, DWORD PTR __real@3f000000
+        movss   DWORD PTR _x2$[ebp], xmm0
+        movss   xmm0, DWORD PTR _a$[ebp]
+        movss   DWORD PTR _y$[ebp], xmm0
+        mov     eax, DWORD PTR _y$[ebp]
+        mov     DWORD PTR _i$[ebp], eax
+        mov     ecx, DWORD PTR _i$[ebp]
+        sar     ecx, 1
+        mov     edx, 1597463007                     ; 5f3759dfH
+        sub     edx, ecx
+        mov     DWORD PTR _i$[ebp], edx
+        movss   xmm0, DWORD PTR _i$[ebp]
+        movss   DWORD PTR _y$[ebp], xmm0
+        movss   xmm0, DWORD PTR _x2$[ebp]
+        mulss   xmm0, DWORD PTR _y$[ebp]
+        mulss   xmm0, DWORD PTR _y$[ebp]
+        movss   xmm1, DWORD PTR __real@3fc00000
+        subss   xmm1, xmm0
+        mulss   xmm1, DWORD PTR _y$[ebp]
+        movss   DWORD PTR _y$[ebp], xmm1
+        fld     DWORD PTR _y$[ebp]
+$LN1@Inv_Sqrt:
+        mov     esp, ebp
+        pop     ebp
         ret     4
 float WWMath::Inv_Sqrt(float) ENDP                      ; WWMath::Inv_Sqrt
```

</details>

---

<details>
<summary>Generals/Code/Libraries/Source/WWVegas/WWDebug/wwprofile.cpp</summary>

This file includes the following assembly block:

```c++
inline void WWProfile_Get_Ticks(_int64 * ticks)
{
#ifdef _UNIX
	*ticks = 0;
#else 
	__asm
	{
		push edx;
		push ecx;
		mov ecx,ticks;
		_emit 0Fh
		_emit 31h
		mov [ecx],eax;
		mov [ecx+4],edx;
		pop ecx;
		pop edx;
	}
#endif
}
```

This assembly block is trying to get the current ticks using
the [RDTSC](https://www.aldeid.com/wiki/X86-assembly/Instructions/rdtsc)
instruction.

My goto equivalent is:

```c++
#include <intrin.h>

inline void WWProfile_Get_Ticks(_int64 * ticks)
{
#ifdef _UNIX
	*ticks = 0;
#else 
	*ticks = __rdtsc();
#endif
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
_ticks$ = 8                                   ; size = 4
void WWProfile_Get_Ticks(__int64 *) PROC                    ; WWProfile_Get_Ticks
        push    ebp
        mov     ebp, esp
        push    ebx
        push    esi
        push    edi
        push    edx
        push    ecx
        mov     ecx, DWORD PTR _ticks$[ebp]
        DB      15                              ; 0000000fH
        DB      49                              ; 00000031H
        mov     DWORD PTR [ecx], eax
        mov     DWORD PTR [ecx+4], edx
        pop     ecx
        pop     edx
        pop     edi
        pop     esi
        pop     ebx
        pop     ebp
        ret     0
void WWProfile_Get_Ticks(__int64 *) ENDP                    ; WWProfile_Get_Ticks
```

</td>
<td>

```asm
_ticks$ = 8                                   ; size = 4
void WWProfile_Get_Ticks(__int64 *) PROC                    ; WWProfile_Get_Ticks
        push    ebp
        mov     ebp, esp
        rdtsc
        mov     ecx, DWORD PTR _ticks$[ebp]
        mov     DWORD PTR [ecx], eax
        mov     DWORD PTR [ecx+4], edx
        pop     ebp
        ret     0
void WWProfile_Get_Ticks(__int64 *) ENDP                    ; WWProfile_Get_Ticks
```

</td>
</table>

Using the modern intrinsics actually allows for a smaller assembly in this case.

```diff
@@ -2,21 +2,10 @@ _ticks$ = 8                                   ; size = 4
 void WWProfile_Get_Ticks(__int64 *) PROC                    ; WWProfile_Get_Ticks
         push    ebp
         mov     ebp, esp
-        push    ebx
-        push    esi
-        push    edi
-        push    edx
-        push    ecx
+        rdtsc
         mov     ecx, DWORD PTR _ticks$[ebp]
-        DB      15                              ; 0000000fH
-        DB      49                              ; 00000031H
         mov     DWORD PTR [ecx], eax
         mov     DWORD PTR [ecx+4], edx
-        pop     ecx
-        pop     edx
-        pop     edi
-        pop     esi
-        pop     ebx
         pop     ebp
         ret     0
 void WWProfile_Get_Ticks(__int64 *) ENDP                    ; WWProfile_Get_Ticks
```

</details>

---
