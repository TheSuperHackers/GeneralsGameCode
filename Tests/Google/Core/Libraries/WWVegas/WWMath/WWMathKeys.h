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

#include <cstddef>

// One list of keys gives both the WWMathKey enum and the test name of each key.
#define WWMATH_KEYS(KEY) \
	KEY(Sin_0_5f)                \
	KEY(Sin_2_7f)                \
	KEY(Sin_187_66f)             \
	KEY(Sin_0_5)                 \
	KEY(Sin_2_7)                 \
	KEY(Sin_187_66)              \
	KEY(Sinf_0_5f)               \
	KEY(Sinf_2_7f)               \
	KEY(Sinf_187_66f)            \
	KEY(Sinf_Legacy_0_5f)        \
	KEY(Sinf_Legacy_2_7f)        \
	KEY(Sinf_Legacy_187_66f)     \
	KEY(Cos_0_5f)                \
	KEY(Cos_2_7f)                \
	KEY(Cos_187_66f)             \
	KEY(Cos_0_5)                 \
	KEY(Cos_2_7)                 \
	KEY(Cos_187_66)              \
	KEY(Cosf_0_5f)               \
	KEY(Cosf_2_7f)               \
	KEY(Cosf_187_66f)            \
	KEY(Cosf_Legacy_0_5f)        \
	KEY(Cosf_Legacy_2_7f)        \
	KEY(Cosf_Legacy_187_66f)     \
	KEY(Tan_0_5f)                \
	KEY(Tan_2_7f)                \
	KEY(Tan_187_66f)             \
	KEY(Tan_0_5)                 \
	KEY(Tan_2_7)                 \
	KEY(Tan_187_66)              \
	KEY(Tanf_0_5f)               \
	KEY(Tanf_2_7f)               \
	KEY(Tanf_187_66f)            \
	KEY(Asin_Minus0_5f)          \
	KEY(Asin_0_25f)              \
	KEY(Asin_0_9f)               \
	KEY(Asin_Minus0_5)           \
	KEY(Asin_0_25)               \
	KEY(Asin_0_9)                \
	KEY(Asinf_Minus0_5f)         \
	KEY(Asinf_0_25f)             \
	KEY(Asinf_0_9f)              \
	KEY(Acos_Minus0_5f)          \
	KEY(Acos_0_25f)              \
	KEY(Acos_0_9f)               \
	KEY(Acos_Minus0_5)           \
	KEY(Acos_0_25)               \
	KEY(Acos_0_9)                \
	KEY(Acosf_Minus0_5f)         \
	KEY(Acosf_0_25f)             \
	KEY(Acosf_0_9f)              \
	KEY(Atan_0_5f)               \
	KEY(Atan_2_7f)               \
	KEY(Atan_187_66f)            \
	KEY(Atan_0_5)                \
	KEY(Atan_2_7)                \
	KEY(Atan_187_66)             \
	KEY(Atanf_0_5f)              \
	KEY(Atanf_2_7f)              \
	KEY(Atanf_187_66f)           \
	KEY(Atan2_0_4f_1_3f)         \
	KEY(Atan2_2_7f_11_9f)        \
	KEY(Atan2_Minus3_5f_2_0f)    \
	KEY(Atan2_0_4_1_3)           \
	KEY(Atan2_2_7_11_9)          \
	KEY(Atan2_Minus3_5_2_0)      \
	KEY(Atan2f_0_4f_1_3f)        \
	KEY(Atan2f_2_7f_11_9f)       \
	KEY(Atan2f_Minus3_5f_2_0f)   \
	KEY(Sinh_0_5)                \
	KEY(Sinh_Minus0_5)           \
	KEY(Sinh_2_7)                \
	KEY(Sinhf_0_5f)              \
	KEY(Sinhf_Minus0_5f)         \
	KEY(Sinhf_2_7f)              \
	KEY(Cosh_0_5)                \
	KEY(Cosh_Minus0_5)           \
	KEY(Cosh_2_7)                \
	KEY(Coshf_0_5f)              \
	KEY(Coshf_Minus0_5f)         \
	KEY(Coshf_2_7f)              \
	KEY(Tanh_0_5)                \
	KEY(Tanh_Minus0_5)           \
	KEY(Tanh_2_7)                \
	KEY(Tanhf_0_5f)              \
	KEY(Tanhf_Minus0_5f)         \
	KEY(Tanhf_2_7f)              \
	KEY(Exp_0_5)                 \
	KEY(Exp_Minus0_5)            \
	KEY(Exp_2_7)                 \
	KEY(Expf_0_5f)               \
	KEY(Expf_Minus0_5f)          \
	KEY(Expf_2_7f)               \
	KEY(Log_0_5)                 \
	KEY(Log_2_7)                 \
	KEY(Log_187_66)              \
	KEY(Logf_0_5f)               \
	KEY(Logf_2_7f)               \
	KEY(Logf_187_66f)            \
	KEY(Log10_0_5)               \
	KEY(Log10_2_7)               \
	KEY(Log10_187_66)            \
	KEY(Log10f_0_5f)             \
	KEY(Log10f_2_7f)             \
	KEY(Log10f_187_66f)          \
	KEY(Pow_0_4_1_3)             \
	KEY(Pow_2_7_11_9)            \
	KEY(Pow_Minus3_5_2_0)        \
	KEY(Powf_0_4f_1_3f)          \
	KEY(Powf_2_7f_11_9f)         \
	KEY(Powf_Minus3_5f_2_0f)     \
	KEY(Sqrt_0_5f)               \
	KEY(Sqrt_2_7f)               \
	KEY(Sqrt_187_66f)            \
	KEY(Sqrt_0_5)                \
	KEY(Sqrt_2_7)                \
	KEY(Sqrt_187_66)             \
	KEY(Sqrtf_0_5f)              \
	KEY(Sqrtf_2_7f)              \
	KEY(Sqrtf_187_66f)           \
	KEY(Inv_Sqrt_Legacy_0_5f)    \
	KEY(Inv_Sqrt_Legacy_2_7f)    \
	KEY(Inv_Sqrt_Legacy_187_66f) \
	KEY(Fabs_Minus0_5f)          \
	KEY(Fabs_2_7f)               \
	KEY(Fabs_100_5f)             \
	KEY(Fabs_Minus0_5)           \
	KEY(Fabs_2_7)                \
	KEY(Fabs_100_5)              \
	KEY(Fabsf_Minus0_5f)         \
	KEY(Fabsf_2_7f)              \
	KEY(Fabsf_100_5f)            \
	KEY(Fabsf_Legacy_Minus0_5f)  \
	KEY(Fabsf_Legacy_2_7f)       \
	KEY(Fabsf_Legacy_100_5f)     \
	KEY(Ceil_Minus0_5)           \
	KEY(Ceil_2_7)                \
	KEY(Ceil_100_5)              \
	KEY(Ceilf_Minus0_5f)         \
	KEY(Ceilf_2_7f)              \
	KEY(Ceilf_100_5f)            \
	KEY(Floor_Minus0_5)          \
	KEY(Floor_2_7)               \
	KEY(Floor_100_5)             \
	KEY(Floorf_Minus0_5f)        \
	KEY(Floorf_2_7f)             \
	KEY(Floorf_100_5f)           \
	KEY(Float_To_Long_Minus0_5f) \
	KEY(Float_To_Long_2_7f)      \
	KEY(Float_To_Long_100_5f)    \
	KEY(Float_To_Long_Minus0_5)  \
	KEY(Float_To_Long_2_7)       \
	KEY(Float_To_Long_100_5)     \
	KEY(Div_Safe_1_0f_3_0f)      \
	KEY(Div_Safe_1_0f_0_0f)      \
	KEY(Div_Safe_1_0_3_0)        \
	KEY(Div_Safe_1_0_0_0)

#define WWMATH_KEY_ENUM_VALUE(key) key,
#define WWMATH_KEY_ENUM_ENTRY(key) WWMathKey::key,
#define WWMATH_KEY_NAME(key) #key,

enum class WWMathKey
{
	WWMATH_KEYS(WWMATH_KEY_ENUM_VALUE)
	Count
};

static constexpr WWMathKey wwmathKeys[] = { WWMATH_KEYS(WWMATH_KEY_ENUM_ENTRY) };
static const char *const wwmathKeyNames[] = { WWMATH_KEYS(WWMATH_KEY_NAME) };

#undef WWMATH_KEY_ENUM_VALUE
#undef WWMATH_KEY_ENUM_ENTRY
#undef WWMATH_KEY_NAME

constexpr size_t toIndex(WWMathKey key)
{
	return static_cast<size_t>(key);
}

constexpr size_t WWMATH_KEY_COUNT = toIndex(WWMathKey::Count);
