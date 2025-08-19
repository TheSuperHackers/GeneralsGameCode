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

#if defined(_MSC_VER) && _MSC_VER < 1300
size_t strnlen(const char *str, size_t maxlen);
size_t wcsnlen(const wchar_t *str, size_t maxlen);
#endif

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t dstsize);
#endif
#ifndef HAVE_STRLCAT
size_t strlcat(char *dst, const char *src, size_t dstsize);
#endif

size_t wcslcpy(wchar_t *dst, const wchar_t *src, size_t dstsize);
size_t wcslcat(wchar_t *dst, const wchar_t *src, size_t dstsize);

size_t strlcpy_overlap(char *dst, const char *src, size_t dstsize);
size_t strlcat_overlap(char *dst, const char *src, size_t dstsize);

size_t wcslcpy_overlap(wchar_t *dst, const wchar_t *src, size_t dstsize);
size_t wcslcat_overlap(wchar_t *dst, const wchar_t *src, size_t dstsize);


#if !(defined(_MSC_VER) && _MSC_VER < 1300)
template<size_t Size> size_t strlcpy_tpl(char (&dst)[Size], const char *src)
{
	return strlcpy(dst, src, Size);
}

template<size_t Size> size_t strlcat_tpl(char (&dst)[Size], const char *src)
{
	return strlcat(dst, src, Size);
}

template<size_t Size> size_t wcslcat_tpl(wchar_t (&dst)[Size], const wchar_t *src)
{
	return wcslcat(dst, src, Size);
}

template<size_t Size> size_t wcslcpy_tpl(wchar_t (&dst)[Size], const wchar_t *src)
{
	return wcslcpy(dst, src, Size);
}

template<size_t Size> size_t strlcpy_overlap_tpl(char (&dst)[Size], const char *src)
{
	return strlcpy_overlap(dst, src, Size);
}

template<size_t Size> size_t strlcat_overlap_tpl(char (&dst)[Size], const char *src)
{
	return strlcat_overlap(dst, src, Size);
}

template<size_t Size> size_t wcslcat_overlap_tpl(wchar_t (&dst)[Size], const wchar_t *src)
{
	return wcslcat_overlap(dst, src, Size);
}

template<size_t Size> size_t wcslcpy_overlap_tpl(wchar_t (&dst)[Size], const wchar_t *src)
{
	return wcslcpy_overlap(dst, src, Size);
}
#endif
