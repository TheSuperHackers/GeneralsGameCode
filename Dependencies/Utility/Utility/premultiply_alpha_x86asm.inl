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

// VC6-compatible x86 implementation of premultiply_alpha.
// An SSE2-capable CPU is assumed.
inline void premultiply_alpha(unsigned char* pixels, int pitch, int width, int height)
{
	for (int y = 0; y < height; ++y, pixels += pitch)
	{
		unsigned int* line = reinterpret_cast<unsigned int*>(pixels);
		const int groups = width / 4;
		if (groups != 0)
		{
			__asm {
				mov esi, line
				mov ecx, groups
				mov eax, 0xFF000000
				// movd xmm7, eax
				_emit 0x66
				_emit 0x0f
				_emit 0x6e
				_emit 0xf8
				// pshufd xmm7, xmm7, 0 (alpha byte mask)
				_emit 0x66
				_emit 0x0f
				_emit 0x70
				_emit 0xff
				_emit 0x00
				mov eax, 0xFF00FF00
				// movd xmm6, eax
				_emit 0x66
				_emit 0x0f
				_emit 0x6e
				_emit 0xf0
				// pshufd xmm6, xmm6, 0 (odd byte mask)
				_emit 0x66
				_emit 0x0f
				_emit 0x70
				_emit 0xf6
				_emit 0x00
				mov eax, 0x007F007F
				// movd xmm5, eax
				_emit 0x66
				_emit 0x0f
				_emit 0x6e
				_emit 0xe8
				// pshufd xmm5, xmm5, 0 (127 in each 16-bit lane)
				_emit 0x66
				_emit 0x0f
				_emit 0x70
				_emit 0xed
				_emit 0x00
				mov eax, 0x80818081
				// movd xmm4, eax
				_emit 0x66
				_emit 0x0f
				_emit 0x6e
				_emit 0xe0
				// pshufd xmm4, xmm4, 0 (0x8081 in each 16-bit lane)
				_emit 0x66
				_emit 0x0f
				_emit 0x70
				_emit 0xe4
				_emit 0x00
			next_group:
				// movdqu xmm0, [esi] (color)
				_emit 0xf3
				_emit 0x0f
				_emit 0x6f
				_emit 0x06
				// movdqa xmm1, xmm0
				_emit 0x66
				_emit 0x0f
				_emit 0x6f
				_emit 0xc8
				// pand xmm1, xmm7
				_emit 0x66
				_emit 0x0f
				_emit 0xdb
				_emit 0xcf
				// movdqa xmm2, xmm1
				_emit 0x66
				_emit 0x0f
				_emit 0x6f
				_emit 0xd1
				// psrld xmm2, 16 (alpha times 256 in both 16-bit halves)
				_emit 0x66
				_emit 0x0f
				_emit 0x72
				_emit 0xd2
				_emit 0x10
				// por xmm1, xmm2
				_emit 0x66
				_emit 0x0f
				_emit 0xeb
				_emit 0xca
				// movdqa xmm2, xmm0 (c * a for blue and red)
				_emit 0x66
				_emit 0x0f
				_emit 0x6f
				_emit 0xd0
				// psllw xmm2, 8
				_emit 0x66
				_emit 0x0f
				_emit 0x71
				_emit 0xf2
				_emit 0x08
				// pmulhuw xmm2, xmm1
				_emit 0x66
				_emit 0x0f
				_emit 0xe4
				_emit 0xd1
				// movdqa xmm3, xmm0 (c * a for green and alpha)
				_emit 0x66
				_emit 0x0f
				_emit 0x6f
				_emit 0xd8
				// pand xmm3, xmm6
				_emit 0x66
				_emit 0x0f
				_emit 0xdb
				_emit 0xde
				// pmulhuw xmm3, xmm1
				_emit 0x66
				_emit 0x0f
				_emit 0xe4
				_emit 0xd9
				// paddw xmm2, xmm5 ((v + 127) / 255)
				_emit 0x66
				_emit 0x0f
				_emit 0xfd
				_emit 0xd5
				// pmulhuw xmm2, xmm4
				_emit 0x66
				_emit 0x0f
				_emit 0xe4
				_emit 0xd4
				// psrlw xmm2, 7
				_emit 0x66
				_emit 0x0f
				_emit 0x71
				_emit 0xd2
				_emit 0x07
				// paddw xmm3, xmm5 ((v + 127) / 255)
				_emit 0x66
				_emit 0x0f
				_emit 0xfd
				_emit 0xdd
				// pmulhuw xmm3, xmm4
				_emit 0x66
				_emit 0x0f
				_emit 0xe4
				_emit 0xdc
				// psrlw xmm3, 7 (result)
				_emit 0x66
				_emit 0x0f
				_emit 0x71
				_emit 0xd3
				_emit 0x07
				// psllw xmm3, 8
				_emit 0x66
				_emit 0x0f
				_emit 0x71
				_emit 0xf3
				_emit 0x08
				// por xmm2, xmm3
				_emit 0x66
				_emit 0x0f
				_emit 0xeb
				_emit 0xd3
				// movdqa xmm1, xmm7 (keep the original alpha)
				_emit 0x66
				_emit 0x0f
				_emit 0x6f
				_emit 0xcf
				// pandn xmm1, xmm2
				_emit 0x66
				_emit 0x0f
				_emit 0xdf
				_emit 0xca
				// pand xmm0, xmm7
				_emit 0x66
				_emit 0x0f
				_emit 0xdb
				_emit 0xc7
				// por xmm0, xmm1
				_emit 0x66
				_emit 0x0f
				_emit 0xeb
				_emit 0xc1
				// movdqu [esi], xmm0
				_emit 0xf3
				_emit 0x0f
				_emit 0x7f
				_emit 0x06
				add esi, 16
				dec ecx
				jnz next_group
			}
		}

		// The last 0 to 3 pixels of the line.
		for (int x = groups * 4; x < width; ++x)
		{
			const unsigned int alpha = line[x] >> 24;
			unsigned int result = line[x] & 0xFF000000u;
			result |= ((line[x] & 255) * alpha + 127) / 255;
			result |= ((((line[x] >> 8) & 255) * alpha + 127) / 255) << 8;
			result |= ((((line[x] >> 16) & 255) * alpha + 127) / 255) << 16;
			line[x] = result;
		}
	}
}
