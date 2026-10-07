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

// Included by stringex.h

#pragma once

// VC6-compatible x86 cdecl implementation of memcchr.
// An SSE2-capable CPU is assumed.
// SSE2 uses _emit because VC6 does not recognize these instruction mnemonics.
inline __declspec(naked) const void* __cdecl memcchr(const void* data, int c, size_t n)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		mov esi, [ebp+8]
		mov ecx, [ebp+16]
		movzx eax, byte ptr [ebp+12]
		cmp ecx, 4
		jb scalar
		cmp ecx, 8
		jb four_bytes
		// movd xmm5, eax
		_emit 0x66
		_emit 0x0f
		_emit 0x6e
		_emit 0xe8
		// punpcklbw xmm5, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x60
		_emit 0xed
		// punpcklwd xmm5, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x61
		_emit 0xed
		// pshufd xmm5, xmm5, 0
		_emit 0x66
		_emit 0x0f
		_emit 0x70
		_emit 0xed
		_emit 0x00
		cmp ecx, 16
		jb eight_bytes
		cmp ecx, 32
		jbe sixteen_bytes

		// First block on its own, so an early mismatch exits immediately.
		// movdqu xmm0, [esi]
		_emit 0xf3
		_emit 0x0f
		_emit 0x6f
		_emit 0x06
		// pcmpeqb xmm0, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x74
		_emit 0xc5
		// pmovmskb eax, xmm0
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xc0
		xor eax, 0ffffh
		jnz found_vector
		lea edi, [esi+ecx-16]
		add esi, 16
		mov ebx, ecx
		sub ebx, 16
		shr ebx, 6
		jz remainder_blocks

	unrolled:
		// movdqu xmm0, [esi]
		_emit 0xf3
		_emit 0x0f
		_emit 0x6f
		_emit 0x06
		// movdqu xmm1, [esi+16]
		_emit 0xf3
		_emit 0x0f
		_emit 0x6f
		_emit 0x4e
		_emit 0x10
		// movdqu xmm2, [esi+32]
		_emit 0xf3
		_emit 0x0f
		_emit 0x6f
		_emit 0x56
		_emit 0x20
		// movdqu xmm3, [esi+48]
		_emit 0xf3
		_emit 0x0f
		_emit 0x6f
		_emit 0x5e
		_emit 0x30
		// pcmpeqb xmm0, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x74
		_emit 0xc5
		// pcmpeqb xmm1, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x74
		_emit 0xcd
		// pcmpeqb xmm2, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x74
		_emit 0xd5
		// pcmpeqb xmm3, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x74
		_emit 0xdd
		// movdqa xmm4, xmm0
		_emit 0x66
		_emit 0x0f
		_emit 0x6f
		_emit 0xe0
		// pand xmm4, xmm1
		_emit 0x66
		_emit 0x0f
		_emit 0xdb
		_emit 0xe1
		// pand xmm4, xmm2
		_emit 0x66
		_emit 0x0f
		_emit 0xdb
		_emit 0xe2
		// pand xmm4, xmm3
		_emit 0x66
		_emit 0x0f
		_emit 0xdb
		_emit 0xe3
		// pmovmskb eax, xmm4
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xc4
		cmp eax, 0ffffh
		jne unrolled_mismatch
		add esi, 64
		dec ebx
		jnz unrolled

	remainder_blocks:
		cmp esi, edi
		jae last_block
		// movdqu xmm0, [esi]
		_emit 0xf3
		_emit 0x0f
		_emit 0x6f
		_emit 0x06
		// pcmpeqb xmm0, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x74
		_emit 0xc5
		// pmovmskb eax, xmm0
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xc0
		xor eax, 0ffffh
		jnz found_vector
		add esi, 16
		jmp remainder_blocks

	last_block:
		// movdqu xmm0, [edi]
		_emit 0xf3
		_emit 0x0f
		_emit 0x6f
		_emit 0x07
		// pcmpeqb xmm0, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x74
		_emit 0xc5
		// pmovmskb eax, xmm0
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xc0
		xor eax, 0ffffh
		mov esi, edi
		jnz found_vector
		jmp equal

	unrolled_mismatch:
		// pmovmskb eax, xmm0
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xc0
		// pmovmskb edx, xmm1
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xd1
		xor eax, 0ffffh
		xor edx, 0ffffh
		shl edx, 16
		or eax, edx
		jnz found_vector
		// pmovmskb eax, xmm2
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xc2
		// pmovmskb edx, xmm3
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xd3
		xor eax, 0ffffh
		xor edx, 0ffffh
		shl edx, 16
		or eax, edx
		add esi, 32
		jmp found_vector

	sixteen_bytes:
		lea edi, [esi+ecx-16]
		// movdqu xmm0, [esi]
		_emit 0xf3
		_emit 0x0f
		_emit 0x6f
		_emit 0x06
		// movdqu xmm1, [edi]
		_emit 0xf3
		_emit 0x0f
		_emit 0x6f
		_emit 0x0f
		// pcmpeqb xmm0, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x74
		_emit 0xc5
		// pcmpeqb xmm1, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x74
		_emit 0xcd
		// pmovmskb eax, xmm0
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xc0
		// pmovmskb edx, xmm1
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xd1
		xor eax, 0ffffh
		xor edx, 0ffffh
		shl edx, 16
		or eax, edx
		jz equal
		bsf edx, eax
		cmp edx, 16
		jb head_result
		lea eax, [edi+edx-16]
		jmp done

	eight_bytes:
		lea edi, [esi+ecx-8]
		// movq xmm0, [esi]
		_emit 0xf3
		_emit 0x0f
		_emit 0x7e
		_emit 0x06
		// movq xmm1, [edi]
		_emit 0xf3
		_emit 0x0f
		_emit 0x7e
		_emit 0x0f
		// punpcklqdq xmm0, xmm1
		_emit 0x66
		_emit 0x0f
		_emit 0x6c
		_emit 0xc1
		// pcmpeqb xmm0, xmm5
		_emit 0x66
		_emit 0x0f
		_emit 0x74
		_emit 0xc5
		// pmovmskb eax, xmm0
		_emit 0x66
		_emit 0x0f
		_emit 0xd7
		_emit 0xc0
		xor eax, 0ffffh
		jz equal
		bsf edx, eax
		cmp edx, 8
		jb head_result
		lea eax, [edi+edx-8]
		jmp done

	four_bytes:
		imul eax, eax, 01010101h
		mov ebx, [esi]
		lea edi, [esi+ecx-4]
		mov edx, [edi]
		xor ebx, eax
		xor edx, eax
		test ebx, ebx
		jnz four_head
		test edx, edx
		jz equal
		bsf edx, edx
		shr edx, 3
		lea eax, [edi+edx]
		jmp done
	four_head:
		bsf edx, ebx
		shr edx, 3
		jmp head_result

	scalar:
		test ecx, ecx
		jz equal
	scalar_loop:
		cmp byte ptr [esi], al
		jne scalar_result
		inc esi
		dec ecx
		jnz scalar_loop
	equal:
		xor eax, eax
		jmp done
	scalar_result:
		mov eax, esi
		jmp done
	found_vector:
		bsf edx, eax
	head_result:
		lea eax, [esi+edx]
	done:
		pop edi
		pop esi
		pop ebx
		pop ebp
		ret
	}
}
