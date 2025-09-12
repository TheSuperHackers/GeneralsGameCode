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

// This file contains macros to help with endian conversions between different endian systems.
#pragma once

// VC6 does not support pragma once
#ifndef ENDIAN_COMPAT_H
#define ENDIAN_COMPAT_H

#include <Utility/CppMacros.h>
#include <Utility/stdint_adapter.h>


#if defined(__linux__) || defined(__CYGWIN__)
#include <endian.h>

#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#elif defined(__OpenBSD__)
#include <sys/endian.h>

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include <sys/endian.h>

#define be16toh(x) betoh16(x)
#define le16toh(x) letoh16(x)

#define be32toh(x) betoh32(x)
#define le32toh(x) letoh32(x)

#define be64toh(x) betoh64(x)
#define le64toh(x) letoh64(x)

#elif defined(_WIN32) || defined(_WIN64)
#if !(defined(_MSC_VER) && _MSC_VER < 1300)
#include <intrin.h>
#define htobe16(x) _byteswap_ushort(x)
#define htole16(x) (x)
#define be16toh(x) _byteswap_ushort(x)
#define le16toh(x) (x)

#define htobe32(x) _byteswap_ulong(x)
#define htole32(x) (x)
#define be32toh(x) _byteswap_ulong(x)
#define le32toh(x) (x)

#define htobe64(x) _byteswap_uint64(x)
#define htole64(x) (x)
#define be64toh(x) _byteswap_uint64(x)
#define le64toh(x) (x)
#else
#define bswap16(x) ( (uint16_t)( ((x & 0xFF00) >>  8) | ((x & 0x00FF) <<  8) ) )
#define bswap32(x) ( ((x & 0xFF000000) >> 24) | \
					 ((x & 0x00FF0000) >>  8) | \
					 ((x & 0x0000FF00) <<  8) | \
					 ((x & 0x000000FF) << 24) )
#define bswap64(x) ( (((uint64_t)(x) & 0xFF00000000000000) >> 56) | \
                     (((uint64_t)(x) & 0x00FF000000000000) >> 40) | \
                     (((uint64_t)(x) & 0x0000FF0000000000) >> 24) | \
                     (((uint64_t)(x) & 0x000000FF00000000) >>  8) | \
                     (((uint64_t)(x) & 0x00000000FF000000) <<  8) | \
                     (((uint64_t)(x) & 0x0000000000FF0000) << 24) | \
                     (((uint64_t)(x) & 0x000000000000FF00) << 40) | \
                     (((uint64_t)(x) & 0x00000000000000FF) << 56) )

#define htobe16(x) bswap16(x)
#define htole16(x) (x)
#define be16toh(x) bswap16(x)
#define le16toh(x) (x)

#define htobe32(x) bswap32(x)
#define htole32(x) (x)
#define be32toh(x) bswap32(x)
#define le32toh(x) (x)

#define htobe64(x) bswap64(x)
#define htole64(x) (x)
#define be64toh(x) bswap64(x)
#define le64toh(x) (x)

#endif // _MSC_VER

#else
#error platform not supported
#endif


// Endian helper function data types
#if defined(__linux__) || defined(__CYGWIN__)
typedef uint16_t SwapType16;
typedef uint32_t SwapType32;
typedef uint64_t SwapType64;

#elif defined(__APPLE__)
typedef UInt16 SwapType16;
typedef UInt32 SwapType32;
typedef UInt64 SwapType64;

#elif defined(__OpenBSD__)
typedef uint16_t SwapType16;
typedef uint32_t SwapType32;
typedef uint64_t SwapType64;

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
typedef uint16_t SwapType16;
typedef uint32_t SwapType32;
typedef uint64_t SwapType64;

#elif defined(_WIN32) || defined(_WIN64)
typedef uint16_t SwapType16;
typedef uint32_t SwapType32;
typedef uint64_t SwapType64;

#else
#error platform not supported
#endif


// Endian helper functions
static_assert(sizeof(SwapType16) == 2, "expected size does not match");
static_assert(sizeof(SwapType32) == 4, "expected size does not match");
static_assert(sizeof(SwapType64) == 8, "expected size does not match");

// VC6 compatible overloaded endian functions
#if defined(_MSC_VER) && _MSC_VER < 1300

// Big endian to host
inline int16_t  betoh(int16_t value)  { return be16toh(value); }
inline uint16_t	betoh(uint16_t value) { return be16toh(value); }
inline int32_t	betoh(int32_t value)  { return be32toh(value); }
inline uint32_t betoh(uint32_t value) { return be32toh(value); }
inline int64_t  betoh(int64_t value)  { return be64toh(value); }
inline uint64_t betoh(uint64_t value) { return be64toh(value); }
// Host to big endian
inline int16_t  htobe(int16_t value)  { return htobe16(value); }
inline uint16_t	htobe(uint16_t value) { return htobe16(value); }
inline int32_t	htobe(int32_t value)  { return htobe32(value); }
inline uint32_t htobe(uint32_t value) { return htobe32(value); }
inline int64_t  htobe(int64_t value)  { return htobe64(value); }
inline uint64_t htobe(uint64_t value) { return htobe64(value); }
// Little endian to host
inline int16_t  letoh(int16_t value)  { return le16toh(value); }
inline uint16_t	letoh(uint16_t value) { return le16toh(value); }
inline int32_t	letoh(int32_t value)  { return le32toh(value); }
inline uint32_t letoh(uint32_t value) { return le32toh(value); }
inline int64_t  letoh(int64_t value)  { return le64toh(value); }
inline uint64_t letoh(uint64_t value) { return le64toh(value); }
// Host to little endian
inline int16_t  htole(int16_t value)  { return htole16(value); }
inline uint16_t	htole(uint16_t value) { return htole16(value); }
inline int32_t	htole(int32_t value)  { return htole32(value); }
inline uint32_t htole(uint32_t value) { return htole32(value); }
inline int64_t  htole(int64_t value)  { return htole64(value); }
inline uint64_t htole(uint64_t value) { return htole64(value); }

#else

namespace Endian
{
template <typename Type, size_t Size = sizeof(Type)> struct htobeHelper;
template <typename Type, size_t Size = sizeof(Type)> struct htoleHelper;
template <typename Type, size_t Size = sizeof(Type)> struct betohHelper;
template <typename Type, size_t Size = sizeof(Type)> struct letohHelper;

// 2 byte integer, enum
template <typename Type> struct htobeHelper<Type, 2> { static inline Type swap(Type value) { return static_cast<Type>(htobe16(static_cast<SwapType16>(value))); } };
template <typename Type> struct htoleHelper<Type, 2> { static inline Type swap(Type value) { return static_cast<Type>(htole16(static_cast<SwapType16>(value))); } };
template <typename Type> struct betohHelper<Type, 2> { static inline Type swap(Type value) { return static_cast<Type>(be16toh(static_cast<SwapType16>(value))); } };
template <typename Type> struct letohHelper<Type, 2> { static inline Type swap(Type value) { return static_cast<Type>(le16toh(static_cast<SwapType16>(value))); } };
// 4 byte integer, enum
template <typename Type> struct htobeHelper<Type, 4> { static inline Type swap(Type value) { return static_cast<Type>(htobe32(static_cast<SwapType32>(value))); } };
template <typename Type> struct htoleHelper<Type, 4> { static inline Type swap(Type value) { return static_cast<Type>(htole32(static_cast<SwapType32>(value))); } };
template <typename Type> struct betohHelper<Type, 4> { static inline Type swap(Type value) { return static_cast<Type>(be32toh(static_cast<SwapType32>(value))); } };
template <typename Type> struct letohHelper<Type, 4> { static inline Type swap(Type value) { return static_cast<Type>(le16toh(static_cast<SwapType32>(value))); } };
// 8 byte integer, enum
template <typename Type> struct htobeHelper<Type, 8> { static inline Type swap(Type value) { return static_cast<Type>(htobe64(static_cast<SwapType64>(value))); } };
template <typename Type> struct htoleHelper<Type, 8> { static inline Type swap(Type value) { return static_cast<Type>(htole64(static_cast<SwapType64>(value))); } };
template <typename Type> struct betohHelper<Type, 8> { static inline Type swap(Type value) { return static_cast<Type>(be64toh(static_cast<SwapType64>(value))); } };
template <typename Type> struct letohHelper<Type, 8> { static inline Type swap(Type value) { return static_cast<Type>(le16toh(static_cast<SwapType64>(value))); } };
// float
template <> struct htobeHelper<float, 4> { static inline float swap(float value) { SwapType32 v = htobe32(*reinterpret_cast<SwapType32*>(&value)); return *reinterpret_cast<float*>(&v); } };
template <> struct htoleHelper<float, 4> { static inline float swap(float value) { SwapType32 v = htole32(*reinterpret_cast<SwapType32*>(&value)); return *reinterpret_cast<float*>(&v); } };
template <> struct betohHelper<float, 4> { static inline float swap(float value) { SwapType32 v = be32toh(*reinterpret_cast<SwapType32*>(&value)); return *reinterpret_cast<float*>(&v); } };
template <> struct letohHelper<float, 4> { static inline float swap(float value) { SwapType32 v = le16toh(*reinterpret_cast<SwapType32*>(&value)); return *reinterpret_cast<float*>(&v); } };
// double
template <> struct htobeHelper<double, 8> { static inline double swap(double value) { SwapType64 v = htobe64(*reinterpret_cast<SwapType64*>(&value)); return *reinterpret_cast<double*>(&v); } };
template <> struct htoleHelper<double, 8> { static inline double swap(double value) { SwapType64 v = htole64(*reinterpret_cast<SwapType64*>(&value)); return *reinterpret_cast<double*>(&v); } };
template <> struct betohHelper<double, 8> { static inline double swap(double value) { SwapType64 v = be64toh(*reinterpret_cast<SwapType64*>(&value)); return *reinterpret_cast<double*>(&v); } };
template <> struct letohHelper<double, 8> { static inline double swap(double value) { SwapType64 v = le16toh(*reinterpret_cast<SwapType64*>(&value)); return *reinterpret_cast<double*>(&v); } };
} // namespace Endian

// c++ template functions, takes any 2, 4, 8 bytes, including float, double, enum

// Host to big endian
template<typename Type> inline Type htobe(Type value) { return Endian::htobeHelper<Type>::swap(value); }
// Host to little endian
template<typename Type> inline Type htole(Type value) { return Endian::htoleHelper<Type>::swap(value); }
// Big endian to host
template<typename Type> inline Type betoh(Type value) { return Endian::betohHelper<Type>::swap(value); }
// Little endian to host
template<typename Type> inline Type letoh(Type value) { return Endian::letohHelper<Type>::swap(value); }

// Host to big endian
template<typename Type> inline void htobe_ref(Type &value) { value = Endian::htobeHelper<Type>::swap(value); }
// Host to little endian
template<typename Type> inline void htole_ref(Type &value) { value = Endian::htoleHelper<Type>::swap(value); }
// Big endian to host
template<typename Type> inline void betoh_ref(Type &value) { value = Endian::betohHelper<Type>::swap(value); }
// Little endian to host
template<typename Type> inline void letoh_ref(Type &value) { value = Endian::letohHelper<Type>::swap(value); }

#endif // _MSC_VER < 1300

#endif // ENDIAN_COMPAT_H

