# Documenting the Inline Assembly

Thanks to [xezon](https://github.com/TheSuperHackers/GeneralsGameCode/pull/405#issuecomment-2709737713) for suggesting
using [godbolt](https://godbolt.org/) to compare assembly outputs.

All generated code uses the latest version of MSVC 19 x86, as available in godbolt.

As recommended by [tomson26](https://github.com/TheSuperHackers/GeneralsGameCode/pull/405#issuecomment-2710604104), I am
not including `blitblit`, `rlerle`, `lcw` or `mpu`.

The `crc` assembly block is already extensively documented in source.

## List of Documented Inline ASM

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

This is using the assembly block to retrieve the value of [RDTSC](https://www.aldeid.com/wiki/X86-assembly/Instructions/rdtsc)
and move it to `t`.

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

</details>

The assemblies are different in structure but equal in result. The intrinsic version appears shorter.

```diff
--- a/a.asm
+++ b/b.asm
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

---

<details>
<summary>Generals/Code/GameEngine/Source/Common/System/StackDump.cpp</summary>

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
--- a/a.asm
+++ b/b.asm
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
--- a/a.asm
+++ b/b.asm
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
