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


#include "ICU/utf8.h"
#include "ICU/IcuLoader.h"

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <vector>

#if defined(RTS_HAS_ICU)
#include <unicode/ustring.h>
#elif defined(RTS_ICU_DYNAMIC)
#include <windows.h>
#endif

// unichar is a 16-bit UTF-16 code unit on Windows and a 32-bit UTF-32 codepoint on most other
// platforms. ICU converts UTF-8 to and from UTF-16, so UTF-32 takes an extra step through UTF-16.
#if defined(WCHAR_MAX) && (WCHAR_MAX <= 0xFFFF)
#define UTF8_WCHAR_IS_UTF16 1
#else
#define UTF8_WCHAR_IS_UTF16 0
#endif

#if !UTF8_WCHAR_IS_UTF16 && !defined(RTS_HAS_ICU)
#error "A 32-bit unichar requires linked ICU"
#endif

static const int UTF8_REPLACEMENT_CHAR = 0xFFFD;

static bool Fits_Int(size_t length)
{
	return length <= static_cast<size_t>(INT_MAX);
}

// The converters below return the length the whole conversion needs, not counting a null
// terminator. They do not null-terminate, and leave dest unspecified when it is too small.
// Wide_To_Utf8_Convert returns 0 on failure. Utf8_To_Wide_Convert returns UTF8_INVALID for
// malformed UTF-8.

#if defined(RTS_HAS_ICU)

// ICU still reports the needed length when the destination is too small.
static bool Icu_Succeeded(UErrorCode error)
{
	return U_SUCCESS(error) || error == U_BUFFER_OVERFLOW_ERROR;
}

#if UTF8_WCHAR_IS_UTF16

static size_t Wide_To_Utf8_Convert(char* dest, size_t destLen, const unichar* src, size_t srcLen)
{
	UErrorCode error = U_ZERO_ERROR;
	int32_t length = 0;
	u_strToUTF8WithSub(dest, static_cast<int32_t>(destLen), &length, reinterpret_cast<const UChar*>(src),
		static_cast<int32_t>(srcLen), UTF8_REPLACEMENT_CHAR, nullptr, &error);
	if (!Icu_Succeeded(error))
	{
		assert(false);
		return 0;
	}
	return static_cast<size_t>(length);
}

static size_t Utf8_To_Wide_Convert(unichar* dest, size_t destLen, const char* src, size_t srcLen)
{
	UErrorCode error = U_ZERO_ERROR;
	int32_t length = 0;
	u_strFromUTF8(reinterpret_cast<UChar*>(dest), static_cast<int32_t>(destLen), &length, src,
		static_cast<int32_t>(srcLen), &error);
	return Icu_Succeeded(error) ? static_cast<size_t>(length) : UTF8_INVALID;
}

#else

static size_t Wide_To_Utf8_Convert(char* dest, size_t destLen, const unichar* src, size_t srcLen)
{
	const UChar32* utf32 = reinterpret_cast<const UChar32*>(src);
	UErrorCode error = U_ZERO_ERROR;
	int32_t utf16Len = 0;
	u_strFromUTF32WithSub(nullptr, 0, &utf16Len, utf32, static_cast<int32_t>(srcLen),
		UTF8_REPLACEMENT_CHAR, nullptr, &error);
	if (!Icu_Succeeded(error))
	{
		assert(false);
		return 0;
	}

	std::vector<UChar> utf16(static_cast<size_t>(utf16Len) + 1);
	error = U_ZERO_ERROR;
	u_strFromUTF32WithSub(&utf16[0], utf16Len + 1, &utf16Len, utf32, static_cast<int32_t>(srcLen),
		UTF8_REPLACEMENT_CHAR, nullptr, &error);
	int32_t length = 0;
	if (U_SUCCESS(error))
	{
		u_strToUTF8WithSub(dest, static_cast<int32_t>(destLen), &length, &utf16[0], utf16Len,
			UTF8_REPLACEMENT_CHAR, nullptr, &error);
	}
	if (!Icu_Succeeded(error))
	{
		assert(false);
		return 0;
	}
	return static_cast<size_t>(length);
}

static size_t Utf8_To_Wide_Convert(unichar* dest, size_t destLen, const char* src, size_t srcLen)
{
	UErrorCode error = U_ZERO_ERROR;
	int32_t utf16Len = 0;
	u_strFromUTF8(nullptr, 0, &utf16Len, src, static_cast<int32_t>(srcLen), &error);
	if (!Icu_Succeeded(error))
	{
		return UTF8_INVALID;
	}

	std::vector<UChar> utf16(static_cast<size_t>(utf16Len) + 1);
	error = U_ZERO_ERROR;
	u_strFromUTF8(&utf16[0], utf16Len + 1, &utf16Len, src, static_cast<int32_t>(srcLen), &error);
	int32_t length = 0;
	if (U_SUCCESS(error))
	{
		u_strToUTF32(reinterpret_cast<UChar32*>(dest), static_cast<int32_t>(destLen), &length,
			&utf16[0], utf16Len, &error);
	}
	return Icu_Succeeded(error) ? static_cast<size_t>(length) : UTF8_INVALID;
}

#endif

#elif defined(RTS_ICU_DYNAMIC)

enum
{
	ICU_ZERO_ERROR = 0,
	ICU_BUFFER_OVERFLOW_ERROR = 15
};

// ICU warnings are negative. ICU still reports the needed length when the destination is too small.
static bool Icu_Succeeded(IcuLoader::ErrorCode error)
{
	return error <= ICU_ZERO_ERROR || error == ICU_BUFFER_OVERFLOW_ERROR;
}

// Win32 reports no length when the destination is too small, so measure again then.
static size_t Win32_Wide_To_Utf8(char* dest, size_t destLen, const unichar* src, size_t srcLen)
{
	int length = WideCharToMultiByte(CP_UTF8, 0, src, static_cast<int>(srcLen), dest,
		static_cast<int>(destLen), nullptr, nullptr);
	if (length == 0 && destLen != 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		length = WideCharToMultiByte(CP_UTF8, 0, src, static_cast<int>(srcLen), nullptr, 0, nullptr, nullptr);
	}
	if (length == 0 && srcLen != 0)
	{
		assert(false);
		return 0;
	}
	return static_cast<size_t>(length);
}

static size_t Win32_Utf8_To_Wide(unichar* dest, size_t destLen, const char* src, size_t srcLen)
{
	int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, static_cast<int>(srcLen), dest,
		static_cast<int>(destLen));
	if (length == 0 && destLen != 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, static_cast<int>(srcLen), nullptr, 0);
	}
	if (length == 0 && srcLen != 0)
	{
		return UTF8_INVALID;
	}
	return static_cast<size_t>(length);
}

// Fall back to Win32 when icu.dll or its exports are unavailable.
static size_t Wide_To_Utf8_Convert(char* dest, size_t destLen, const unichar* src, size_t srcLen)
{
	IcuLoader::ErrorCode error = ICU_ZERO_ERROR;
	int length = 0;
	if (!IcuLoader::toUtf8WithSub(dest, static_cast<int>(destLen), &length,
		reinterpret_cast<const IcuLoader::Char*>(src), static_cast<int>(srcLen), UTF8_REPLACEMENT_CHAR, nullptr, &error))
	{
		return Win32_Wide_To_Utf8(dest, destLen, src, srcLen);
	}
	if (!Icu_Succeeded(error))
	{
		assert(false);
		return 0;
	}
	return static_cast<size_t>(length);
}

static size_t Utf8_To_Wide_Convert(unichar* dest, size_t destLen, const char* src, size_t srcLen)
{
	IcuLoader::ErrorCode error = ICU_ZERO_ERROR;
	int length = 0;
	if (!IcuLoader::fromUtf8(reinterpret_cast<IcuLoader::Char*>(dest), static_cast<int>(destLen), &length,
		src, static_cast<int>(srcLen), &error))
	{
		return Win32_Utf8_To_Wide(dest, destLen, src, srcLen);
	}
	return Icu_Succeeded(error) ? static_cast<size_t>(length) : UTF8_INVALID;
}

#endif

size_t Wide_To_Utf8_Len(const unichar* src, size_t srcLen)
{
	return Wide_To_Utf8(nullptr, 0, src, srcLen);
}

size_t Utf8_To_Wide_Len(const char* src, size_t srcLen)
{
	return Utf8_To_Wide(nullptr, 0, src, srcLen);
}

size_t Wide_To_Utf8(char* dest, size_t destLen, const unichar* src, size_t srcLen)
{
	if (!Fits_Int(destLen) || !Fits_Int(srcLen))
	{
		assert(false);
		return 0;
	}

	const size_t needed = Wide_To_Utf8_Convert(dest, destLen, src, srcLen);
	size_t out = needed;
	if (needed > destLen)
	{
		// The converters may split or drop the last codepoint that fits, so cut a full conversion instead.
		out = 0;
		if (destLen > 0)
		{
			std::vector<char> full(needed);
			Wide_To_Utf8_Convert(&full[0], needed, src, srcLen);
			out = Utf8_Truncate_Len(&full[0], needed, destLen);
			memcpy(dest, &full[0], out);
		}
	}
	if (out < destLen)
	{
		dest[out] = '\0';
	}
	return needed;
}

size_t Utf8_To_Wide(unichar* dest, size_t destLen, const char* src, size_t srcLen)
{
	size_t needed = UTF8_INVALID;
	if (Fits_Int(destLen) && Fits_Int(srcLen))
	{
		needed = Utf8_To_Wide_Convert(dest, destLen, src, srcLen);
	}
	if (needed == UTF8_INVALID)
	{
		if (destLen > 0)
		{
			dest[0] = L'\0';
		}
		return UTF8_INVALID;
	}

	size_t out = needed;
	if (needed > destLen)
	{
		// The converters may split or drop the last codepoint that fits, so cut a full conversion instead.
		out = 0;
		if (destLen > 0)
		{
			std::vector<unichar> full(needed);
			Utf8_To_Wide_Convert(&full[0], needed, src, srcLen);
			out = destLen;
#if UTF8_WCHAR_IS_UTF16
			if (full[out - 1] >= 0xD800 && full[out - 1] <= 0xDBFF)
			{
				--out; // the low surrogate of this pair does not fit
			}
#endif
			memcpy(dest, &full[0], out * sizeof(unichar));
		}
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
