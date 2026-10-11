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

// VC6-compatible x86 implementations of Interlocked functions of the Windows SDK.
// The VC6 headers have them neither for volatile pointers nor for 8, 16 and 64 bit values.
//
// The functions are naked, because VC6 fails with an internal compiler error when it expands inline
// assembler into a function of a class template. With __fastcall the first argument is in ecx, the
// second in edx and the third on the stack. The result is returned in al, ax or eax.

inline __declspec(naked) long __fastcall InterlockedCompareExchange(long volatile *Destination, long Exchange, long Comparand)
{
	__asm {
		mov eax, dword ptr [esp+4]
		lock cmpxchg dword ptr [ecx], edx
		ret 4
	}
}

inline __declspec(naked) long __fastcall InterlockedExchange(long volatile *Target, long Value)
{
	__asm {
		mov eax, edx
		xchg dword ptr [ecx], eax
		ret
	}
}

inline __declspec(naked) long __fastcall InterlockedExchangeAdd(long volatile *Addend, long Value)
{
	__asm {
		mov eax, edx
		lock xadd dword ptr [ecx], eax
		ret
	}
}

inline __declspec(naked) short __fastcall InterlockedCompareExchange16(short volatile *Destination, short Exchange, short Comparand)
{
	__asm {
		mov ax, word ptr [esp+4]
		lock cmpxchg word ptr [ecx], dx
		ret 4
	}
}

inline __declspec(naked) short __fastcall InterlockedExchange16(short volatile *Target, short Value)
{
	__asm {
		mov ax, dx
		xchg word ptr [ecx], ax
		ret
	}
}

inline __declspec(naked) short __fastcall InterlockedExchangeAdd16(short volatile *Addend, short Value)
{
	__asm {
		mov ax, dx
		lock xadd word ptr [ecx], ax
		ret
	}
}

inline __declspec(naked) char __fastcall InterlockedCompareExchange8(char volatile *Destination, char Exchange, char Comparand)
{
	__asm {
		mov al, byte ptr [esp+4]
		lock cmpxchg byte ptr [ecx], dl
		ret 4
	}
}

inline __declspec(naked) char __fastcall InterlockedExchange8(char volatile *Target, char Value)
{
	__asm {
		mov al, dl
		xchg byte ptr [ecx], al
		ret
	}
}

inline __declspec(naked) char __fastcall InterlockedExchangeAdd8(char volatile *Addend, char Value)
{
	__asm {
		mov al, dl
		lock xadd byte ptr [ecx], al
		ret
	}
}

inline __declspec(naked) void* __fastcall InterlockedCompareExchangePointer(void* volatile *Destination, void* Exchange, void* Comparand)
{
	__asm {
		mov eax, dword ptr [esp+4]
		lock cmpxchg dword ptr [ecx], edx
		ret 4
	}
}

inline __declspec(naked) void* __fastcall InterlockedExchangePointer(void* volatile *Target, void* Value)
{
	__asm {
		mov eax, edx
		xchg dword ptr [ecx], eax
		ret
	}
}

// A plain read is sufficient next to the functions above, because they all store with a locked instruction.
// The reads are functions, because the VC6 optimizer does not reliably treat a data member as volatile and
// would otherwise move the read out of a loop that waits for another thread to store a value.
inline __declspec(naked) long __fastcall ReadAcquire(long const volatile *Source)
{
	__asm {
		mov eax, dword ptr [ecx]
		ret
	}
}

inline __declspec(naked) short __fastcall ReadAcquire16(short const volatile *Source)
{
	__asm {
		mov ax, word ptr [ecx]
		ret
	}
}

inline __declspec(naked) char __fastcall ReadAcquire8(char const volatile *Source)
{
	__asm {
		mov al, byte ptr [ecx]
		ret
	}
}

// The 64 bit functions build on cmpxchg8b, the one instruction that locks 8 bytes. It compares edx:eax with the
// destination and stores ecx:ebx there if they are equal, else it loads the destination into edx:eax.
// A 64 bit argument is on the stack even with __fastcall. The result is returned in edx:eax.
inline __declspec(naked) __int64 __fastcall InterlockedCompareExchange64(__int64 volatile *Destination, __int64 Exchange, __int64 Comparand)
{
	__asm {
		push ebx
		push esi
		mov esi, ecx
		mov ebx, dword ptr [esp+12]
		mov ecx, dword ptr [esp+16]
		mov eax, dword ptr [esp+20]
		mov edx, dword ptr [esp+24]
		lock cmpxchg8b qword ptr [esi]
		pop esi
		pop ebx
		ret 16
	}
}

// The processor cannot exchange or add 8 bytes in one instruction. These two repeat a compare exchange until no
// other thread got in between. The first comparand is read plainly and can be half of a value that another thread
// is storing, which only makes the first attempt fail.
inline __declspec(naked) __int64 __fastcall InterlockedExchange64(__int64 volatile *Target, __int64 Value)
{
	__asm {
		push ebx
		push esi
		mov esi, ecx
		mov ebx, dword ptr [esp+12]
		mov ecx, dword ptr [esp+16]
		mov eax, dword ptr [esi]
		mov edx, dword ptr [esi+4]
	retry:
		lock cmpxchg8b qword ptr [esi]
		jnz retry
		pop esi
		pop ebx
		ret 8
	}
}

inline __declspec(naked) __int64 __fastcall InterlockedExchangeAdd64(__int64 volatile *Addend, __int64 Value)
{
	__asm {
		push ebx
		push esi
		mov esi, ecx
		mov eax, dword ptr [esi]
		mov edx, dword ptr [esi+4]
	retry:
		mov ebx, eax
		mov ecx, edx
		add ebx, dword ptr [esp+12]
		adc ecx, dword ptr [esp+16]
		lock cmpxchg8b qword ptr [esi]
		jnz retry
		pop esi
		pop ebx
		ret 8
	}
}

// Reads the two halves one after the other, as the function of the Windows SDK does on 32 bit x86. Another thread
// can store in between, so that the halves come from two different values. The result is therefore only good as
// the first comparand of a compare exchange, which does not match such a value. A compare exchange of 0 for 0
// reads all 8 bytes in one step: it leaves every value as it is and returns it.
inline __declspec(naked) __int64 __fastcall ReadAcquire64(__int64 const volatile *Source)
{
	__asm {
		mov eax, dword ptr [ecx]
		mov edx, dword ptr [ecx+4]
		ret
	}
}
