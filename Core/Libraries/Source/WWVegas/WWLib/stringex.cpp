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

#include "stringex.h"
#include <ctype.h>
#include <string.h>

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t dstsize)
{
	size_t srclen = strlen(src);

	if (dstsize != 0)
	{
		size_t copylen = (srclen >= dstsize) ? dstsize - 1 : srclen;
		memcpy(dst, src, copylen);
		dst[copylen] = '\0';
	}

	return srclen; // length tried to create
}
#endif // HAVE_STRLCPY

#ifndef HAVE_STRLCAT
size_t strlcat(char *dst, const char *src, size_t dstsize)
{
	size_t dstlen = strnlen(dst, dstsize); // safer than walking blindly
	size_t srclen = strlen(src);

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
		memcpy(dst + dstlen, src, copylen);
		dst[dstlen + copylen] = '\0';
	}

	return dstlen + srclen; // length tried to create
}
#endif // HAVE_STRLCAT

size_t wcslcpy(wchar_t *dst, const wchar_t *src, size_t dstsize)
{
	size_t srclen = wcslen(src);

	if (dstsize != 0)
	{
		size_t copylen = (srclen >= dstsize) ? dstsize - 1 : srclen;
		memcpy(dst, src, copylen * sizeof(wchar_t));
		dst[copylen] = L'\0';
	}

	return srclen; // length tried to create
}

size_t wcslcat(wchar_t *dst, const wchar_t *src, size_t dstsize)
{
	size_t dstlen = wcsnlen(dst, dstsize); // safer than walking blindly
	size_t srclen = wcslen(src);

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
		memcpy(dst + dstlen, src, copylen * sizeof(wchar_t));
		dst[dstlen + copylen] = L'\0';
	}

	return dstlen + srclen; // length tried to create
}

size_t strlcpy_overlap(char *dst, const char *src, size_t dstsize)
{
	size_t srclen = strlen(src);

	if (dstsize > 0)
	{
		size_t copylen = (srclen >= dstsize) ? dstsize - 1 : srclen;
		memmove(dst, src, copylen);
		dst[copylen] = '\0';
	}

	return srclen; // length tried to create
}

size_t strlcat_overlap(char *dst, const char *src, size_t dstsize)
{
	size_t dstlen = strnlen(dst, dstsize);
	size_t srclen = strlen(src);

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
		memmove(dst + dstlen, src, copylen);
		dst[dstlen + copylen] = '\0';
	}

	return dstlen + srclen; // length tried to create
}

size_t wcslcpy_overlap(wchar_t *dst, const wchar_t *src, size_t dstsize)
{
	size_t srclen = wcslen(src);

	if (dstsize > 0)
	{
		size_t copylen = (srclen >= dstsize) ? dstsize - 1 : srclen;
		memmove(dst, src, copylen);
		dst[copylen] = L'\0';
	}

	return srclen; // length tried to create
}

size_t wcslcat_overlap(wchar_t *dst, const wchar_t *src, size_t dstsize)
{
	size_t dstlen = wcsnlen(dst, dstsize);
	size_t srclen = wcslen(src);

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
		memmove(dst + dstlen, src, copylen * sizeof(wchar_t));
		dst[dstlen + copylen] = L'\0';
	}

	return dstlen + srclen; // length tried to create
}
