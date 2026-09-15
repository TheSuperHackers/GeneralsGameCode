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

#include "always.h"
#include "utf8.h"

// wchar_t is a 16-bit UTF-16 code unit on Windows and a 32-bit UTF-32 codepoint on most other
// platforms. WCHAR_MAX lets us distinguish the two at compile time so the surrogate-pair paths
// are excluded entirely (not just constant-folded) where wchar_t is wide enough to hold a codepoint.
#if defined(WCHAR_MAX) && (WCHAR_MAX <= 0xFFFF)
#define UTF8_WCHAR_IS_UTF16 1
#else
#define UTF8_WCHAR_IS_UTF16 0
#endif

static const unsigned int UTF8_CODEPOINT_MAX = 0x10FFFF;
static const unsigned int UTF8_SURROGATE_MIN = 0xD800;
static const unsigned int UTF8_SURROGATE_MAX = 0xDFFF;
static const unsigned int UTF8_REPLACEMENT_CHAR = 0xFFFD;

// Number of UTF-8 bytes required to encode a codepoint.
static size_t Utf8_Encoded_Length(unsigned int cp)
{
	if (cp < 0x80)
	{
		return 1;
	}
	if (cp < 0x800)
	{
		return 2;
	}
	if (cp < 0x10000)
	{
		return 3;
	}
	return 4;
}

// Encode a codepoint to dest, which is assumed to have room. Returns the number of bytes written.
static size_t Utf8_Encode(char* dest, unsigned int cp)
{
	if (cp < 0x80)
	{
		dest[0] = (char)cp;
		return 1;
	}
	if (cp < 0x800)
	{
		dest[0] = (char)(0xC0 | (cp >> 6));
		dest[1] = (char)(0x80 | (cp & 0x3F));
		return 2;
	}
	if (cp < 0x10000)
	{
		dest[0] = (char)(0xE0 | (cp >> 12));
		dest[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
		dest[2] = (char)(0x80 | (cp & 0x3F));
		return 3;
	}
	dest[0] = (char)(0xF0 | (cp >> 18));
	dest[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
	dest[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
	dest[3] = (char)(0x80 | (cp & 0x3F));
	return 4;
}

// Decode one UTF-8 sequence at src, with srcLen bytes remaining. On success returns the number of
// bytes consumed (1-4) and sets cp. Returns 0 on any malformed, overlong, out-of-range or surrogate
// encoding.
static size_t Utf8_Decode(const char* src, size_t srcLen, unsigned int& cp)
{
	const unsigned char lead = (unsigned char)src[0];
	if (lead < 0x80)
	{
		cp = lead;
		return 1;
	}

	size_t count;
	unsigned int lowerBound;
	if ((lead & 0xE0) == 0xC0)
	{
		count = 2;
		cp = lead & 0x1F;
		lowerBound = 0x80;
	}
	else if ((lead & 0xF0) == 0xE0)
	{
		count = 3;
		cp = lead & 0x0F;
		lowerBound = 0x800;
	}
	else if ((lead & 0xF8) == 0xF0)
	{
		count = 4;
		cp = lead & 0x07;
		lowerBound = 0x10000;
	}
	else
	{
		return 0; // a continuation byte or a 5/6-byte form cannot start a sequence
	}

	if (srcLen < count)
	{
		return 0; // truncated sequence
	}
	for (size_t i = 1; i < count; ++i)
	{
		const unsigned char trail = (unsigned char)src[i];
		if ((trail & 0xC0) != 0x80)
		{
			return 0; // not a continuation byte
		}
		cp = (cp << 6) | (trail & 0x3F);
	}

	if (cp < lowerBound || cp > UTF8_CODEPOINT_MAX || (cp >= UTF8_SURROGATE_MIN && cp <= UTF8_SURROGATE_MAX))
	{
		return 0; // overlong, out of range, or a surrogate codepoint
	}
	return count;
}

// Read one codepoint at src, with srcLen wide characters remaining. Returns the number of wide
// characters consumed (1-2) and sets cp. Combines UTF-16 surrogate pairs where wchar_t is 16-bit;
// treats each element as a whole codepoint where wchar_t is 32-bit. Wide data that has no UTF-8
// representation is reported as U+FFFD, so the encoder never emits a sequence that the decoder
// would reject.
static size_t Wide_Read(const wchar_t* src, size_t srcLen, unsigned int& cp)
{
	size_t consumed = 1;
#if UTF8_WCHAR_IS_UTF16
	cp = (unsigned int)src[0] & 0xFFFF;
	if (cp >= UTF8_SURROGATE_MIN && cp <= 0xDBFF && srcLen > 1)
	{
		const unsigned int low = (unsigned int)src[1] & 0xFFFF;
		if (low >= 0xDC00 && low <= UTF8_SURROGATE_MAX)
		{
			cp = 0x10000 + ((cp - UTF8_SURROGATE_MIN) << 10) + (low - 0xDC00);
			consumed = 2;
		}
	}
#else
	(void)srcLen;
	cp = (unsigned int)src[0];
#endif
	if (cp > UTF8_CODEPOINT_MAX || (cp >= UTF8_SURROGATE_MIN && cp <= UTF8_SURROGATE_MAX))
	{
		cp = UTF8_REPLACEMENT_CHAR;
	}
	return consumed;
}

// Number of wide characters required to store a codepoint.
static size_t Wide_Encoded_Length(unsigned int cp)
{
#if UTF8_WCHAR_IS_UTF16
	return (cp >= 0x10000) ? 2 : 1;
#else
	(void)cp;
	return 1;
#endif
}

// Write one codepoint to a wide buffer, which is assumed to have room. Returns wide characters written.
static size_t Wide_Write(wchar_t* dest, unsigned int cp)
{
#if UTF8_WCHAR_IS_UTF16
	if (cp >= 0x10000)
	{
		cp -= 0x10000;
		dest[0] = (wchar_t)(0xD800 + (cp >> 10));
		dest[1] = (wchar_t)(0xDC00 + (cp & 0x3FF));
		return 2;
	}
#endif
	dest[0] = (wchar_t)cp;
	return 1;
}

size_t Wide_To_Utf8_Len(const wchar_t* src, size_t srcLen)
{
	size_t needed = 0;
	size_t i = 0;
	while (i < srcLen)
	{
		unsigned int cp;
		i += Wide_Read(src + i, srcLen - i, cp);
		needed += Utf8_Encoded_Length(cp);
	}
	return needed;
}

size_t Utf8_To_Wide_Len(const char* src, size_t srcLen)
{
	size_t needed = 0;
	size_t i = 0;
	while (i < srcLen)
	{
		unsigned int cp;
		const size_t consumed = Utf8_Decode(src + i, srcLen - i, cp);
		if (consumed == 0)
		{
			return UTF8_INVALID;
		}
		i += consumed;
		needed += Wide_Encoded_Length(cp);
	}
	return needed;
}

size_t Wide_To_Utf8(char* dest, size_t destLen, const wchar_t* src, size_t srcLen)
{
	size_t needed = 0;
	size_t out = 0;
	size_t i = 0;
	while (i < srcLen)
	{
		unsigned int cp;
		i += Wide_Read(src + i, srcLen - i, cp);
		const size_t need = Utf8_Encoded_Length(cp);
		// Stop writing at the first codepoint that does not fit, but keep counting for the caller.
		if (needed == out && out + need <= destLen)
		{
			out += Utf8_Encode(dest + out, cp);
		}
		needed += need;
	}
	if (out < destLen)
	{
		dest[out] = '\0';
	}
	return needed;
}

size_t Utf8_To_Wide(wchar_t* dest, size_t destLen, const char* src, size_t srcLen)
{
	size_t needed = 0;
	size_t out = 0;
	size_t i = 0;
	while (i < srcLen)
	{
		unsigned int cp;
		const size_t consumed = Utf8_Decode(src + i, srcLen - i, cp);
		if (consumed == 0)
		{
			if (destLen > 0)
			{
				dest[0] = L'\0';
			}
			return UTF8_INVALID;
		}
		i += consumed;
		const size_t need = Wide_Encoded_Length(cp);
		// Stop writing at the first codepoint that does not fit, but keep counting for the caller.
		if (needed == out && out + need <= destLen)
		{
			out += Wide_Write(dest + out, cp);
		}
		needed += need;
	}
	if (out < destLen)
	{
		dest[out] = L'\0';
	}
	return needed;
}

// A UTF-8 continuation byte matches 10xxxxxx, so it can never start a sequence.
static bool Utf8_Is_Continuation_Byte(char c)
{
	return ((unsigned char)c & 0xC0) == 0x80;
}

size_t Utf8_Truncate_Len(const char* src, size_t srcLen, size_t maxLen)
{
	if (srcLen <= maxLen)
	{
		return srcLen;
	}

	size_t len = maxLen;
	while (len > 0 && Utf8_Is_Continuation_Byte(src[len]))
	{
		--len;
	}
	return len;
}
