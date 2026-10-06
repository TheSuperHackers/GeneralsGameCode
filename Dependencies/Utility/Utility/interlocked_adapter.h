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

#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1300

inline long InterlockedCompareExchange(long volatile *Destination, long Exchange, long Comparand)
{
	return (long)InterlockedCompareExchange((PVOID*)Destination, (PVOID)Exchange, (PVOID)Comparand);
}

inline long InterlockedExchange(long volatile *Target, long Value)
{
	return (long)InterlockedExchange((LPLONG)Target, (LONG)Value);
}

inline long InterlockedExchangeAdd(long volatile *Target, long Value)
{
	return (long)InterlockedExchangeAdd((LPLONG)Target, (LONG)Value);
}

// VC6 has no interlocked functions for 8 and 16 bit values. These use the locked
// instructions of the processor. They are naked, because VC6 fails with an internal
// compiler error when it expands inline assembler into a function of a class template.
// With __fastcall the first argument is in ecx, the second in edx and the third on the
// stack. The result is returned in al or ax.
__declspec(naked) inline char __fastcall InterlockedCompareExchange8(char volatile *Destination, char Exchange, char Comparand)
{
	__asm
	{
		mov al, byte ptr [esp + 4]
		lock cmpxchg byte ptr [ecx], dl
		ret 4
	}
}

__declspec(naked) inline char __fastcall InterlockedExchange8(char volatile *Target, char Value)
{
	__asm
	{
		mov al, dl
		xchg byte ptr [ecx], al
		ret
	}
}

__declspec(naked) inline char __fastcall InterlockedExchangeAdd8(char volatile *Target, char Value)
{
	__asm
	{
		mov al, dl
		lock xadd byte ptr [ecx], al
		ret
	}
}

__declspec(naked) inline short __fastcall InterlockedCompareExchange16(short volatile *Destination, short Exchange, short Comparand)
{
	__asm
	{
		mov ax, word ptr [esp + 4]
		lock cmpxchg word ptr [ecx], dx
		ret 4
	}
}

__declspec(naked) inline short __fastcall InterlockedExchange16(short volatile *Target, short Value)
{
	__asm
	{
		mov ax, dx
		xchg word ptr [ecx], ax
		ret
	}
}

__declspec(naked) inline short __fastcall InterlockedExchangeAdd16(short volatile *Target, short Value)
{
	__asm
	{
		mov ax, dx
		lock xadd word ptr [ecx], ax
		ret
	}
}

// The VC6 SDK signatures take non-volatile pointers, so the volatile qualifier
// must be removed with const_cast before reinterpret_cast can change the type.
inline PVOID InterlockedExchangePointer(PVOID volatile *Target, PVOID Value)
{
	return reinterpret_cast<PVOID>(InterlockedExchange(reinterpret_cast<LPLONG>(const_cast<PVOID*>(Target)), reinterpret_cast<LONG>(Value)));
}

inline PVOID InterlockedCompareExchangePointer(PVOID volatile *Destination, PVOID Exchange, PVOID Comparand)
{
	return InterlockedCompareExchange(const_cast<PVOID*>(Destination), Exchange, Comparand);
}

#endif
