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
