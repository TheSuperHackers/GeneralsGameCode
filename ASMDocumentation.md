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
