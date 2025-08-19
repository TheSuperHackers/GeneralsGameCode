/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#include "bittype.h"
#include <string.h>


// Declaration

template<typename T> size_t strlen_t(const T *str);
template<typename T> size_t strnlen_t(const T *str, size_t maxlen);

#if defined(_MSC_VER) && _MSC_VER < 1300
size_t strnlen(const char *str, size_t maxlen);
size_t wcsnlen(const wchar_t *str, size_t maxlen);
#endif

template<typename T> size_t strlcpy_t(T *dst, const T *src, size_t dstsize);
template<typename T> size_t strlcat_t(T *dst, const T *src, size_t dstsize);

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t dstsize);
#endif
#ifndef HAVE_STRLCAT
size_t strlcat(char *dst, const char *src, size_t dstsize);
#endif
size_t wcslcpy(wchar_t *dst, const wchar_t *src, size_t dstsize);
size_t wcslcat(wchar_t *dst, const wchar_t *src, size_t dstsize);

template<typename T> size_t strlcpy_overlap_t(T *dst, const T *src, size_t dstsize);
template<typename T> size_t strlcat_overlap_t(T *dst, const T *src, size_t dstsize);

size_t strlcpy_overlap(char *dst, const char *src, size_t dstsize);
size_t strlcat_overlap(char *dst, const char *src, size_t dstsize);
size_t wcslcpy_overlap(wchar_t *dst, const wchar_t *src, size_t dstsize);
size_t wcslcat_overlap(wchar_t *dst, const wchar_t *src, size_t dstsize);

#if !(defined(_MSC_VER) && _MSC_VER < 1300)
template<size_t Size> size_t strlcpy_t(char (&dst)[Size], const char *src);
template<size_t Size> size_t strlcat_t(char (&dst)[Size], const char *src);
template<size_t Size> size_t strlcpy_overlap_t(char (&dst)[Size], const char *src);
template<size_t Size> size_t strlcat_overlap_t(char (&dst)[Size], const char *src);
#endif


// Implementation

template<typename T> size_t strlen_t(const T *str)
{
	const T* begin = str;
	while (*str)
		++str;
	return static_cast<size_t>(str - begin);
}

template<typename T> size_t strnlen_t(const T *str, size_t maxlen)
{
	const T* begin = str;
	const T* end = str + maxlen;
	while (str < end && *str)
		++str;
	return static_cast<size_t>(str - begin);
}

#if defined(_MSC_VER) && _MSC_VER < 1300
inline size_t strnlen(const char *str, size_t maxlen) { return strnlen_t(str, maxlen); }
inline size_t wcsnlen(const wchar_t *str, size_t maxlen) { return strnlen_t(str, maxlen); }
#endif

template<typename T> size_t strlcpy_t(T *dst, const T *src, size_t dstsize)
{
	size_t srclen = strlen_t(src);
	if (dstsize != 0)
	{
		size_t copylen = (srclen >= dstsize) ? dstsize - 1 : srclen;
		memcpy(dst, src, copylen * sizeof(T));
		dst[copylen] = 0;
	}
	return srclen; // length tried to create
}

template<typename T> size_t strlcat_t(T *dst, const T *src, size_t dstsize)
{
	size_t dstlen = strnlen_t(dst, dstsize); // safer than walking blindly
	size_t srclen = strlen_t(src);
	if (dstlen == dstsize)
	{
		return dstsize + srclen; // dst not NUL terminated
	}
	size_t copylen = dstsize - dstlen - 1; // space left (excluding NUL)
	if (copylen > srclen)
	{
		copylen = srclen;
	}
	if (copylen > 0)
	{
		memcpy(dst + dstlen, src, copylen * sizeof(T));
		dst[dstlen + copylen] = 0;
	}
	return dstlen + srclen; // length tried to create
}

#ifndef HAVE_STRLCPY
inline size_t strlcpy(char *dst, const char *src, size_t dstsize) { return strlcpy_t(dst, src, dstsize); }
#endif
#ifndef HAVE_STRLCAT
inline size_t strlcat(char *dst, const char *src, size_t dstsize) { return strlcat_t(dst, src, dstsize); }
#endif
inline size_t wcslcpy(wchar_t *dst, const wchar_t *src, size_t dstsize) { return strlcpy_t(dst, src, dstsize); }
inline size_t wcslcat(wchar_t *dst, const wchar_t *src, size_t dstsize) { return strlcat_t(dst, src, dstsize); }

template<typename T> size_t strlcpy_overlap_t(T *dst, const T *src, size_t dstsize)
{
	size_t srclen = strlen_t(src);
	if (dstsize > 0)
	{
		size_t copylen = (srclen >= dstsize) ? dstsize - 1 : srclen;
		memmove(dst, src, copylen * sizeof(T));
		dst[copylen] = 0;
	}
	return srclen; // length tried to create
}

template<typename T> size_t strlcat_overlap_t(T *dst, const T *src, size_t dstsize)
{
	size_t dstlen = strnlen_t(dst, dstsize);
	size_t srclen = strlen_t(src);
	if (dstlen == dstsize)
	{
		return dstsize + srclen; // no space to append
	}
	size_t copylen = dstsize - dstlen - 1; // space left for src
	if (copylen > srclen)
	{
		copylen = srclen;
	}
	if (copylen > 0)
	{
		memmove(dst + dstlen, src, copylen * sizeof(T));
		dst[dstlen + copylen] = '\0';
	}
	return dstlen + srclen; // length tried to create
}

inline size_t strlcpy_overlap(char *dst, const char *src, size_t dstsize) { return strlcpy_overlap_t(dst, src, dstsize); }
inline size_t strlcat_overlap(char *dst, const char *src, size_t dstsize) { return strlcat_overlap_t(dst, src, dstsize); }
inline size_t wcslcpy_overlap(wchar_t *dst, const wchar_t *src, size_t dstsize) { return strlcpy_overlap_t(dst, src, dstsize); }
inline size_t wcslcat_overlap(wchar_t *dst, const wchar_t *src, size_t dstsize) { return strlcat_overlap_t(dst, src, dstsize); }

#if !(defined(_MSC_VER) && _MSC_VER < 1300)
template<size_t Size> size_t strlcpy_t(char (&dst)[Size], const char *src) { return strlcpy_t(dst, src, Size); }
template<size_t Size> size_t strlcat_t(char (&dst)[Size], const char *src) { return strlcat_t(dst, src, Size); }
template<size_t Size> size_t strlcpy_overlap_t(char (&dst)[Size], const char *src) { return strlcpy_overlap_t(dst, src, Size); }
template<size_t Size> size_t strlcat_overlap_t(char (&dst)[Size], const char *src) { return strlcat_overlap_t(dst, src, Size); }
#endif
