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
#include "ICU/IcuSupport.h"
#include "ICU/IcuLoader.h"

#include <assert.h>

#include <limits.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(RTS_HAS_ICU) && !defined(RTS_HAS_ICU_WINSDK)
#include <vector>
#endif

namespace
{

bool FitsInt(size_t length)
{
    return length <= static_cast<size_t>(INT_MAX);
}

#ifdef _WIN32

size_t WindowsWideToUtf8Len(const wchar_t* src, size_t srcLen)
{
    if (!FitsInt(srcLen))
    {
        assert(false);
        return 0;
    }

    const int outputLength = WideCharToMultiByte(CP_UTF8, 0, src, static_cast<int>(srcLen), nullptr, 0, nullptr, nullptr);
    if (outputLength == 0 && srcLen != 0)
    {
        assert(false);
        return 0;
    }

    return static_cast<size_t>(outputLength);
}

size_t WindowsWideToUtf8(char* dest, size_t destLen, const wchar_t* src, size_t srcLen)
{
    if (!FitsInt(destLen) || !FitsInt(srcLen))
    {
        assert(false);
        return 0;
    }

    const int outputLength = WideCharToMultiByte(CP_UTF8, 0, src, static_cast<int>(srcLen),
        dest, static_cast<int>(destLen), nullptr, nullptr);
    if (outputLength == 0 && srcLen != 0)
    {
        assert(false);
        return 0;
    }

    if (static_cast<size_t>(outputLength) < destLen)
    {
        dest[outputLength] = '\0';
    }

    return static_cast<size_t>(outputLength);
}

size_t WindowsUtf8ToWideLen(const char* src, size_t srcLen)
{
    if (!FitsInt(srcLen))
    {
        return UTF8_INVALID;
    }

    const int outputLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src,
        static_cast<int>(srcLen), nullptr, 0);
    if (outputLength == 0 && srcLen != 0)
    {
        return UTF8_INVALID;
    }

    return static_cast<size_t>(outputLength);
}

size_t WindowsUtf8ToWide(wchar_t* dest, size_t destLen, const char* src, size_t srcLen)
{
    if (!FitsInt(destLen) || !FitsInt(srcLen))
    {
        if (destLen > 0)
        {
            dest[0] = L'\0';
        }

        return UTF8_INVALID;
    }

    const int outputLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src,
        static_cast<int>(srcLen), dest, static_cast<int>(destLen));
    if (outputLength == 0 && srcLen != 0)
    {
        if (destLen > 0)
        {
            dest[0] = L'\0';
        }

        return UTF8_INVALID;
    }

    if (static_cast<size_t>(outputLength) < destLen)
    {
        dest[outputLength] = L'\0';
    }

    return static_cast<size_t>(outputLength);
}

#endif

#if defined(RTS_HAS_ICU) && !defined(RTS_HAS_ICU_WINSDK)

bool IcuPreflightSucceeded(UErrorCode error)
{
    return U_SUCCESS(error) || error == U_BUFFER_OVERFLOW_ERROR;
}

bool IcuConversionSucceeded(UErrorCode error)
{
    return U_SUCCESS(error);
}

#if defined(WCHAR_MAX) && (WCHAR_MAX <= 0xFFFF)

size_t IcuWideToUtf8(char* dest, size_t destLen, const wchar_t* src, size_t srcLen)
{
    if (!FitsInt(destLen) || !FitsInt(srcLen))
    {
        assert(false);
        return 0;
    }

    UErrorCode error = U_ZERO_ERROR;
    int32_t outputLength = 0;
    u_strToUTF8WithSub(dest, static_cast<int32_t>(destLen), &outputLength,
        reinterpret_cast<const UChar*>(src), static_cast<int32_t>(srcLen), 0xFFFD, nullptr, &error);
    if (!IcuConversionSucceeded(error))
    {
        assert(false);
        return 0;
    }

    return static_cast<size_t>(outputLength);
}

size_t IcuWideToUtf8Len(const wchar_t* src, size_t srcLen)
{
    if (!FitsInt(srcLen))
    {
        assert(false);
        return 0;
    }

    UErrorCode error = U_ZERO_ERROR;
    int32_t outputLength = 0;
    u_strToUTF8WithSub(nullptr, 0, &outputLength, reinterpret_cast<const UChar*>(src),
        static_cast<int32_t>(srcLen), 0xFFFD, nullptr, &error);
    if (!IcuPreflightSucceeded(error))
    {
        assert(false);
        return 0;
    }

    return static_cast<size_t>(outputLength);
}

size_t IcuUtf8ToWide(wchar_t* dest, size_t destLen, const char* src, size_t srcLen)
{
    if (!FitsInt(destLen) || !FitsInt(srcLen))
    {
        if (destLen > 0)
        {
            dest[0] = L'\0';
        }

        return UTF8_INVALID;
    }

    UErrorCode error = U_ZERO_ERROR;
    int32_t outputLength = 0;
    u_strFromUTF8(reinterpret_cast<UChar*>(dest), static_cast<int32_t>(destLen), &outputLength,
        src, static_cast<int32_t>(srcLen), &error);
    if (!IcuConversionSucceeded(error))
    {
        if (destLen > 0)
        {
            dest[0] = L'\0';
        }

        return UTF8_INVALID;
    }

    return static_cast<size_t>(outputLength);
}

size_t IcuUtf8ToWideLen(const char* src, size_t srcLen)
{
    if (!FitsInt(srcLen))
    {
        return UTF8_INVALID;
    }

    UErrorCode error = U_ZERO_ERROR;
    int32_t outputLength = 0;
    u_strFromUTF8(nullptr, 0, &outputLength, src, static_cast<int32_t>(srcLen), &error);
    if (!IcuPreflightSucceeded(error))
    {
        return UTF8_INVALID;
    }

    return static_cast<size_t>(outputLength);
}

#else

bool WideToUtf16(std::vector<UChar>& utf16, const wchar_t* src, size_t srcLen)
{
    if (!FitsInt(srcLen))
    {
        return false;
    }

    UErrorCode error = U_ZERO_ERROR;
    int32_t outputLength = 0;
    u_strFromUTF32WithSub(nullptr, 0, &outputLength, reinterpret_cast<const UChar32*>(src),
        static_cast<int32_t>(srcLen), 0xFFFD, nullptr, &error);
    if (!IcuPreflightSucceeded(error))
    {
        return false;
    }

    utf16.resize(static_cast<size_t>(outputLength) + 1);
    error = U_ZERO_ERROR;
    u_strFromUTF32WithSub(&utf16[0], static_cast<int32_t>(utf16.size()), &outputLength,
        reinterpret_cast<const UChar32*>(src), static_cast<int32_t>(srcLen), 0xFFFD, nullptr, &error);
    return U_SUCCESS(error);
}

bool Utf8ToUtf16(std::vector<UChar>& utf16, const char* src, size_t srcLen)
{
    if (!FitsInt(srcLen))
    {
        return false;
    }

    UErrorCode error = U_ZERO_ERROR;
    int32_t outputLength = 0;
    u_strFromUTF8(nullptr, 0, &outputLength, src, static_cast<int32_t>(srcLen), &error);
    if (!IcuPreflightSucceeded(error))
    {
        return false;
    }

    utf16.resize(static_cast<size_t>(outputLength) + 1);
    error = U_ZERO_ERROR;
    u_strFromUTF8(&utf16[0], static_cast<int32_t>(utf16.size()), &outputLength,
        src, static_cast<int32_t>(srcLen), &error);
    return U_SUCCESS(error);
}

size_t IcuWideToUtf8Len(const wchar_t* src, size_t srcLen)
{
    std::vector<UChar> utf16;
    if (!WideToUtf16(utf16, src, srcLen))
    {
        assert(false);
        return 0;
    }

    UErrorCode error = U_ZERO_ERROR;
    int32_t outputLength = 0;
    u_strToUTF8WithSub(nullptr, 0, &outputLength, utf16.empty() ? nullptr : &utf16[0],
        static_cast<int32_t>(utf16.empty() ? 0 : utf16.size() - 1), 0xFFFD, nullptr, &error);
    if (!IcuPreflightSucceeded(error))
    {
        assert(false);
        return 0;
    }

    return static_cast<size_t>(outputLength);
}

size_t IcuUtf8ToWideLen(const char* src, size_t srcLen)
{
    std::vector<UChar> utf16;
    if (!Utf8ToUtf16(utf16, src, srcLen))
    {
        return UTF8_INVALID;
    }

    UErrorCode error = U_ZERO_ERROR;
    int32_t outputLength = 0;
    u_strToUTF32(nullptr, 0, &outputLength, utf16.empty() ? nullptr : &utf16[0],
        static_cast<int32_t>(utf16.empty() ? 0 : utf16.size() - 1), &error);
    if (!IcuPreflightSucceeded(error))
    {
        return UTF8_INVALID;
    }

    return static_cast<size_t>(outputLength);
}

size_t IcuWideToUtf8(char* dest, size_t destLen, const wchar_t* src, size_t srcLen)
{
    if (!FitsInt(destLen))
    {
        assert(false);
        return 0;
    }

    std::vector<UChar> utf16;
    if (!WideToUtf16(utf16, src, srcLen))
    {
        assert(false);
        return 0;
    }

    UErrorCode error = U_ZERO_ERROR;
    int32_t outputLength = 0;
    u_strToUTF8WithSub(dest, static_cast<int32_t>(destLen), &outputLength,
        utf16.empty() ? nullptr : &utf16[0], static_cast<int32_t>(utf16.empty() ? 0 : utf16.size() - 1),
        0xFFFD, nullptr, &error);
    if (U_FAILURE(error))
    {
        assert(false);
        return 0;
    }

    return static_cast<size_t>(outputLength);
}

size_t IcuUtf8ToWide(wchar_t* dest, size_t destLen, const char* src, size_t srcLen)
{
    if (!FitsInt(destLen))
    {
        if (destLen > 0)
        {
            dest[0] = L'\0';
        }

        return UTF8_INVALID;
    }

    std::vector<UChar> utf16;
    if (!Utf8ToUtf16(utf16, src, srcLen))
    {
        if (destLen > 0)
        {
            dest[0] = L'\0';
        }

        return UTF8_INVALID;
    }

    UErrorCode error = U_ZERO_ERROR;
    int32_t outputLength = 0;
    u_strToUTF32(reinterpret_cast<UChar32*>(dest), static_cast<int32_t>(destLen), &outputLength,
        utf16.empty() ? nullptr : &utf16[0], static_cast<int32_t>(utf16.empty() ? 0 : utf16.size() - 1), &error);
    if (U_FAILURE(error))
    {
        if (destLen > 0)
        {
            dest[0] = L'\0';
        }

        return UTF8_INVALID;
    }

    return static_cast<size_t>(outputLength);
}

#endif

#elif defined(RTS_ICU_DYNAMIC) || defined(RTS_HAS_ICU_WINSDK)

typedef IcuLoader::Char IcuChar;
typedef IcuLoader::ErrorCode IcuErrorCode;

enum
{
    ICU_ZERO_ERROR = 0,
    ICU_BUFFER_OVERFLOW_ERROR = 15,
    ICU_REPLACEMENT_CHARACTER = 0xFFFD
};

bool IcuPreflightSucceeded(IcuErrorCode error)
{
    return error <= ICU_ZERO_ERROR || error == ICU_BUFFER_OVERFLOW_ERROR;
}

bool IcuConversionSucceeded(IcuErrorCode error)
{
    return error <= ICU_ZERO_ERROR;
}

size_t IcuWideToUtf8(char* dest, size_t destLen, const wchar_t* src, size_t srcLen)
{
    if (!FitsInt(destLen) || !FitsInt(srcLen))
    {
        assert(false);
        return 0;
    }

    IcuErrorCode error = ICU_ZERO_ERROR;
    int outputLength = 0;
    if (!IcuLoader::toUtf8WithSub(dest, static_cast<int>(destLen), &outputLength,
        reinterpret_cast<const IcuChar*>(src), static_cast<int>(srcLen), ICU_REPLACEMENT_CHARACTER, nullptr, &error))
    {
        return WindowsWideToUtf8(dest, destLen, src, srcLen);
    }

    if (!IcuConversionSucceeded(error))
    {
        assert(false);
        return 0;
    }

    return static_cast<size_t>(outputLength);
}

size_t IcuWideToUtf8Len(const wchar_t* src, size_t srcLen)
{
    if (!FitsInt(srcLen))
    {
        assert(false);
        return 0;
    }

    IcuErrorCode error = ICU_ZERO_ERROR;
    int outputLength = 0;
    if (!IcuLoader::toUtf8WithSub(nullptr, 0, &outputLength, reinterpret_cast<const IcuChar*>(src),
        static_cast<int>(srcLen), ICU_REPLACEMENT_CHARACTER, nullptr, &error))
    {
        return WindowsWideToUtf8Len(src, srcLen);
    }

    if (!IcuPreflightSucceeded(error))
    {
        assert(false);
        return 0;
    }

    return static_cast<size_t>(outputLength);
}

size_t IcuUtf8ToWide(wchar_t* dest, size_t destLen, const char* src, size_t srcLen)
{
    if (!FitsInt(destLen) || !FitsInt(srcLen))
    {
        if (destLen > 0)
        {
            dest[0] = L'\0';
        }

        return UTF8_INVALID;
    }

    IcuErrorCode error = ICU_ZERO_ERROR;
    int outputLength = 0;
    if (!IcuLoader::fromUtf8(reinterpret_cast<IcuChar*>(dest), static_cast<int>(destLen), &outputLength,
        src, static_cast<int>(srcLen), &error))
    {
        return WindowsUtf8ToWide(dest, destLen, src, srcLen);
    }

    if (!IcuConversionSucceeded(error))
    {
        if (destLen > 0)
        {
            dest[0] = L'\0';
        }

        return UTF8_INVALID;
    }

    return static_cast<size_t>(outputLength);
}

size_t IcuUtf8ToWideLen(const char* src, size_t srcLen)
{
    if (!FitsInt(srcLen))
    {
        return UTF8_INVALID;
    }

    IcuErrorCode error = ICU_ZERO_ERROR;
    int outputLength = 0;
    if (!IcuLoader::fromUtf8(nullptr, 0, &outputLength, src, static_cast<int>(srcLen), &error))
    {
        return WindowsUtf8ToWideLen(src, srcLen);
    }

    if (!IcuPreflightSucceeded(error))
    {
        return UTF8_INVALID;
    }

    return static_cast<size_t>(outputLength);
}

#endif

} // namespace

size_t Wide_To_Utf8_Len(const wchar_t* src, size_t srcLen)
{
    return IcuWideToUtf8Len(src, srcLen);
}

size_t Utf8_To_Wide_Len(const char* src, size_t srcLen)
{
    return IcuUtf8ToWideLen(src, srcLen);
}

size_t Wide_To_Utf8(char* dest, size_t destLen, const wchar_t* src, size_t srcLen)
{
    return IcuWideToUtf8(dest, destLen, src, srcLen);
}

size_t Utf8_To_Wide(wchar_t* dest, size_t destLen, const char* src, size_t srcLen)
{
    return IcuUtf8ToWide(dest, destLen, src, srcLen);
}

// A UTF-8 continuation byte matches 10xxxxxx, so it can never start a sequence.
static bool Utf8_Is_Continuation_Byte(char c)
{
	return (static_cast<unsigned char>(c) & 0xC0) == 0x80;
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
