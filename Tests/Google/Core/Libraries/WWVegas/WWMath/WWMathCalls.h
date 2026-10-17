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

#include <string>

#include "WWMathKeys.h"
#include "WWMathTestCase.h"

typedef std::string (*WWMathCall)();

struct WWMathCallEntry
{
	WWMathKey key;
	WWMathCall call;
};

static constexpr WWMathCallEntry wwmathCalls[] = {
	{ WWMathKey::Sin_0_5f,                [] { return toBitsString(WWMath::Sin(runtimeValue(0.5f))); } },
	{ WWMathKey::Sin_2_7f,                [] { return toBitsString(WWMath::Sin(runtimeValue(2.7f))); } },
	{ WWMathKey::Sin_187_66f,             [] { return toBitsString(WWMath::Sin(runtimeValue(187.66f))); } },
	{ WWMathKey::Sin_0_5,                 [] { return toBitsString(WWMath::Sin(runtimeValue(0.5))); } },
	{ WWMathKey::Sin_2_7,                 [] { return toBitsString(WWMath::Sin(runtimeValue(2.7))); } },
	{ WWMathKey::Sin_187_66,              [] { return toBitsString(WWMath::Sin(runtimeValue(187.66))); } },
	{ WWMathKey::Sinf_0_5f,               [] { return toBitsString(WWMath::Sinf(runtimeValue(0.5f))); } },
	{ WWMathKey::Sinf_2_7f,               [] { return toBitsString(WWMath::Sinf(runtimeValue(2.7f))); } },
	{ WWMathKey::Sinf_187_66f,            [] { return toBitsString(WWMath::Sinf(runtimeValue(187.66f))); } },
	{ WWMathKey::Sinf_Legacy_0_5f,        [] { return toBitsString(WWMath::Sinf_Legacy(runtimeValue(0.5f))); } },
	{ WWMathKey::Sinf_Legacy_2_7f,        [] { return toBitsString(WWMath::Sinf_Legacy(runtimeValue(2.7f))); } },
	{ WWMathKey::Sinf_Legacy_187_66f,     [] { return toBitsString(WWMath::Sinf_Legacy(runtimeValue(187.66f))); } },

	{ WWMathKey::Cos_0_5f,                [] { return toBitsString(WWMath::Cos(runtimeValue(0.5f))); } },
	{ WWMathKey::Cos_2_7f,                [] { return toBitsString(WWMath::Cos(runtimeValue(2.7f))); } },
	{ WWMathKey::Cos_187_66f,             [] { return toBitsString(WWMath::Cos(runtimeValue(187.66f))); } },
	{ WWMathKey::Cos_0_5,                 [] { return toBitsString(WWMath::Cos(runtimeValue(0.5))); } },
	{ WWMathKey::Cos_2_7,                 [] { return toBitsString(WWMath::Cos(runtimeValue(2.7))); } },
	{ WWMathKey::Cos_187_66,              [] { return toBitsString(WWMath::Cos(runtimeValue(187.66))); } },
	{ WWMathKey::Cosf_0_5f,               [] { return toBitsString(WWMath::Cosf(runtimeValue(0.5f))); } },
	{ WWMathKey::Cosf_2_7f,               [] { return toBitsString(WWMath::Cosf(runtimeValue(2.7f))); } },
	{ WWMathKey::Cosf_187_66f,            [] { return toBitsString(WWMath::Cosf(runtimeValue(187.66f))); } },
	{ WWMathKey::Cosf_Legacy_0_5f,        [] { return toBitsString(WWMath::Cosf_Legacy(runtimeValue(0.5f))); } },
	{ WWMathKey::Cosf_Legacy_2_7f,        [] { return toBitsString(WWMath::Cosf_Legacy(runtimeValue(2.7f))); } },
	{ WWMathKey::Cosf_Legacy_187_66f,     [] { return toBitsString(WWMath::Cosf_Legacy(runtimeValue(187.66f))); } },

	{ WWMathKey::Tan_0_5f,                [] { return toBitsString(WWMath::Tan(runtimeValue(0.5f))); } },
	{ WWMathKey::Tan_2_7f,                [] { return toBitsString(WWMath::Tan(runtimeValue(2.7f))); } },
	{ WWMathKey::Tan_187_66f,             [] { return toBitsString(WWMath::Tan(runtimeValue(187.66f))); } },
	{ WWMathKey::Tan_0_5,                 [] { return toBitsString(WWMath::Tan(runtimeValue(0.5))); } },
	{ WWMathKey::Tan_2_7,                 [] { return toBitsString(WWMath::Tan(runtimeValue(2.7))); } },
	{ WWMathKey::Tan_187_66,              [] { return toBitsString(WWMath::Tan(runtimeValue(187.66))); } },
	{ WWMathKey::Tanf_0_5f,               [] { return toBitsString(WWMath::Tanf(runtimeValue(0.5f))); } },
	{ WWMathKey::Tanf_2_7f,               [] { return toBitsString(WWMath::Tanf(runtimeValue(2.7f))); } },
	{ WWMathKey::Tanf_187_66f,            [] { return toBitsString(WWMath::Tanf(runtimeValue(187.66f))); } },

	{ WWMathKey::Asin_Minus0_5f,          [] { return toBitsString(WWMath::Asin(runtimeValue(-0.5f))); } },
	{ WWMathKey::Asin_0_25f,              [] { return toBitsString(WWMath::Asin(runtimeValue(0.25f))); } },
	{ WWMathKey::Asin_0_9f,               [] { return toBitsString(WWMath::Asin(runtimeValue(0.9f))); } },
	{ WWMathKey::Asin_Minus0_5,           [] { return toBitsString(WWMath::Asin(runtimeValue(-0.5))); } },
	{ WWMathKey::Asin_0_25,               [] { return toBitsString(WWMath::Asin(runtimeValue(0.25))); } },
	{ WWMathKey::Asin_0_9,                [] { return toBitsString(WWMath::Asin(runtimeValue(0.9))); } },
	{ WWMathKey::Asinf_Minus0_5f,         [] { return toBitsString(WWMath::Asinf(runtimeValue(-0.5f))); } },
	{ WWMathKey::Asinf_0_25f,             [] { return toBitsString(WWMath::Asinf(runtimeValue(0.25f))); } },
	{ WWMathKey::Asinf_0_9f,              [] { return toBitsString(WWMath::Asinf(runtimeValue(0.9f))); } },

	{ WWMathKey::Acos_Minus0_5f,          [] { return toBitsString(WWMath::Acos(runtimeValue(-0.5f))); } },
	{ WWMathKey::Acos_0_25f,              [] { return toBitsString(WWMath::Acos(runtimeValue(0.25f))); } },
	{ WWMathKey::Acos_0_9f,               [] { return toBitsString(WWMath::Acos(runtimeValue(0.9f))); } },
	{ WWMathKey::Acos_Minus0_5,           [] { return toBitsString(WWMath::Acos(runtimeValue(-0.5))); } },
	{ WWMathKey::Acos_0_25,               [] { return toBitsString(WWMath::Acos(runtimeValue(0.25))); } },
	{ WWMathKey::Acos_0_9,                [] { return toBitsString(WWMath::Acos(runtimeValue(0.9))); } },
	{ WWMathKey::Acosf_Minus0_5f,         [] { return toBitsString(WWMath::Acosf(runtimeValue(-0.5f))); } },
	{ WWMathKey::Acosf_0_25f,             [] { return toBitsString(WWMath::Acosf(runtimeValue(0.25f))); } },
	{ WWMathKey::Acosf_0_9f,              [] { return toBitsString(WWMath::Acosf(runtimeValue(0.9f))); } },

	{ WWMathKey::Atan_0_5f,               [] { return toBitsString(WWMath::Atan(runtimeValue(0.5f))); } },
	{ WWMathKey::Atan_2_7f,               [] { return toBitsString(WWMath::Atan(runtimeValue(2.7f))); } },
	{ WWMathKey::Atan_187_66f,            [] { return toBitsString(WWMath::Atan(runtimeValue(187.66f))); } },
	{ WWMathKey::Atan_0_5,                [] { return toBitsString(WWMath::Atan(runtimeValue(0.5))); } },
	{ WWMathKey::Atan_2_7,                [] { return toBitsString(WWMath::Atan(runtimeValue(2.7))); } },
	{ WWMathKey::Atan_187_66,             [] { return toBitsString(WWMath::Atan(runtimeValue(187.66))); } },
	{ WWMathKey::Atanf_0_5f,              [] { return toBitsString(WWMath::Atanf(runtimeValue(0.5f))); } },
	{ WWMathKey::Atanf_2_7f,              [] { return toBitsString(WWMath::Atanf(runtimeValue(2.7f))); } },
	{ WWMathKey::Atanf_187_66f,           [] { return toBitsString(WWMath::Atanf(runtimeValue(187.66f))); } },

	{ WWMathKey::Atan2_0_4f_1_3f,         [] { return toBitsString(WWMath::Atan2(runtimeValue(0.4f), runtimeValue(1.3f))); } },
	{ WWMathKey::Atan2_2_7f_11_9f,        [] { return toBitsString(WWMath::Atan2(runtimeValue(2.7f), runtimeValue(11.9f))); } },
	{ WWMathKey::Atan2_Minus3_5f_2_0f,    [] { return toBitsString(WWMath::Atan2(runtimeValue(-3.5f), runtimeValue(2.0f))); } },
	{ WWMathKey::Atan2_0_4_1_3,           [] { return toBitsString(WWMath::Atan2(runtimeValue(0.4), runtimeValue(1.3))); } },
	{ WWMathKey::Atan2_2_7_11_9,          [] { return toBitsString(WWMath::Atan2(runtimeValue(2.7), runtimeValue(11.9))); } },
	{ WWMathKey::Atan2_Minus3_5_2_0,      [] { return toBitsString(WWMath::Atan2(runtimeValue(-3.5), runtimeValue(2.0))); } },
	{ WWMathKey::Atan2f_0_4f_1_3f,        [] { return toBitsString(WWMath::Atan2f(runtimeValue(0.4f), runtimeValue(1.3f))); } },
	{ WWMathKey::Atan2f_2_7f_11_9f,       [] { return toBitsString(WWMath::Atan2f(runtimeValue(2.7f), runtimeValue(11.9f))); } },
	{ WWMathKey::Atan2f_Minus3_5f_2_0f,   [] { return toBitsString(WWMath::Atan2f(runtimeValue(-3.5f), runtimeValue(2.0f))); } },

	{ WWMathKey::Sinh_0_5,                [] { return toBitsString(WWMath::Sinh(runtimeValue(0.5))); } },
	{ WWMathKey::Sinh_Minus0_5,           [] { return toBitsString(WWMath::Sinh(runtimeValue(-0.5))); } },
	{ WWMathKey::Sinh_2_7,                [] { return toBitsString(WWMath::Sinh(runtimeValue(2.7))); } },
	{ WWMathKey::Sinhf_0_5f,              [] { return toBitsString(WWMath::Sinhf(runtimeValue(0.5f))); } },
	{ WWMathKey::Sinhf_Minus0_5f,         [] { return toBitsString(WWMath::Sinhf(runtimeValue(-0.5f))); } },
	{ WWMathKey::Sinhf_2_7f,              [] { return toBitsString(WWMath::Sinhf(runtimeValue(2.7f))); } },

	{ WWMathKey::Cosh_0_5,                [] { return toBitsString(WWMath::Cosh(runtimeValue(0.5))); } },
	{ WWMathKey::Cosh_Minus0_5,           [] { return toBitsString(WWMath::Cosh(runtimeValue(-0.5))); } },
	{ WWMathKey::Cosh_2_7,                [] { return toBitsString(WWMath::Cosh(runtimeValue(2.7))); } },
	{ WWMathKey::Coshf_0_5f,              [] { return toBitsString(WWMath::Coshf(runtimeValue(0.5f))); } },
	{ WWMathKey::Coshf_Minus0_5f,         [] { return toBitsString(WWMath::Coshf(runtimeValue(-0.5f))); } },
	{ WWMathKey::Coshf_2_7f,              [] { return toBitsString(WWMath::Coshf(runtimeValue(2.7f))); } },

	{ WWMathKey::Tanh_0_5,                [] { return toBitsString(WWMath::Tanh(runtimeValue(0.5))); } },
	{ WWMathKey::Tanh_Minus0_5,           [] { return toBitsString(WWMath::Tanh(runtimeValue(-0.5))); } },
	{ WWMathKey::Tanh_2_7,                [] { return toBitsString(WWMath::Tanh(runtimeValue(2.7))); } },
	{ WWMathKey::Tanhf_0_5f,              [] { return toBitsString(WWMath::Tanhf(runtimeValue(0.5f))); } },
	{ WWMathKey::Tanhf_Minus0_5f,         [] { return toBitsString(WWMath::Tanhf(runtimeValue(-0.5f))); } },
	{ WWMathKey::Tanhf_2_7f,              [] { return toBitsString(WWMath::Tanhf(runtimeValue(2.7f))); } },

	{ WWMathKey::Exp_0_5,                 [] { return toBitsString(WWMath::Exp(runtimeValue(0.5))); } },
	{ WWMathKey::Exp_Minus0_5,            [] { return toBitsString(WWMath::Exp(runtimeValue(-0.5))); } },
	{ WWMathKey::Exp_2_7,                 [] { return toBitsString(WWMath::Exp(runtimeValue(2.7))); } },
	{ WWMathKey::Expf_0_5f,               [] { return toBitsString(WWMath::Expf(runtimeValue(0.5f))); } },
	{ WWMathKey::Expf_Minus0_5f,          [] { return toBitsString(WWMath::Expf(runtimeValue(-0.5f))); } },
	{ WWMathKey::Expf_2_7f,               [] { return toBitsString(WWMath::Expf(runtimeValue(2.7f))); } },

	{ WWMathKey::Log_0_5,                 [] { return toBitsString(WWMath::Log(runtimeValue(0.5))); } },
	{ WWMathKey::Log_2_7,                 [] { return toBitsString(WWMath::Log(runtimeValue(2.7))); } },
	{ WWMathKey::Log_187_66,              [] { return toBitsString(WWMath::Log(runtimeValue(187.66))); } },
	{ WWMathKey::Logf_0_5f,               [] { return toBitsString(WWMath::Logf(runtimeValue(0.5f))); } },
	{ WWMathKey::Logf_2_7f,               [] { return toBitsString(WWMath::Logf(runtimeValue(2.7f))); } },
	{ WWMathKey::Logf_187_66f,            [] { return toBitsString(WWMath::Logf(runtimeValue(187.66f))); } },

	{ WWMathKey::Log10_0_5,               [] { return toBitsString(WWMath::Log10(runtimeValue(0.5))); } },
	{ WWMathKey::Log10_2_7,               [] { return toBitsString(WWMath::Log10(runtimeValue(2.7))); } },
	{ WWMathKey::Log10_187_66,            [] { return toBitsString(WWMath::Log10(runtimeValue(187.66))); } },
	{ WWMathKey::Log10f_0_5f,             [] { return toBitsString(WWMath::Log10f(runtimeValue(0.5f))); } },
	{ WWMathKey::Log10f_2_7f,             [] { return toBitsString(WWMath::Log10f(runtimeValue(2.7f))); } },
	{ WWMathKey::Log10f_187_66f,          [] { return toBitsString(WWMath::Log10f(runtimeValue(187.66f))); } },

	{ WWMathKey::Pow_0_4_1_3,             [] { return toBitsString(WWMath::Pow(runtimeValue(0.4), runtimeValue(1.3))); } },
	{ WWMathKey::Pow_2_7_11_9,            [] { return toBitsString(WWMath::Pow(runtimeValue(2.7), runtimeValue(11.9))); } },
	{ WWMathKey::Pow_Minus3_5_2_0,        [] { return toBitsString(WWMath::Pow(runtimeValue(-3.5), runtimeValue(2.0))); } },
	{ WWMathKey::Powf_0_4f_1_3f,          [] { return toBitsString(WWMath::Powf(runtimeValue(0.4f), runtimeValue(1.3f))); } },
	{ WWMathKey::Powf_2_7f_11_9f,         [] { return toBitsString(WWMath::Powf(runtimeValue(2.7f), runtimeValue(11.9f))); } },
	{ WWMathKey::Powf_Minus3_5f_2_0f,     [] { return toBitsString(WWMath::Powf(runtimeValue(-3.5f), runtimeValue(2.0f))); } },

	{ WWMathKey::Sqrt_0_5f,               [] { return toBitsString(WWMath::Sqrt(runtimeValue(0.5f))); } },
	{ WWMathKey::Sqrt_2_7f,               [] { return toBitsString(WWMath::Sqrt(runtimeValue(2.7f))); } },
	{ WWMathKey::Sqrt_187_66f,            [] { return toBitsString(WWMath::Sqrt(runtimeValue(187.66f))); } },
	{ WWMathKey::Sqrt_0_5,                [] { return toBitsString(WWMath::Sqrt(runtimeValue(0.5))); } },
	{ WWMathKey::Sqrt_2_7,                [] { return toBitsString(WWMath::Sqrt(runtimeValue(2.7))); } },
	{ WWMathKey::Sqrt_187_66,             [] { return toBitsString(WWMath::Sqrt(runtimeValue(187.66))); } },
	{ WWMathKey::Sqrtf_0_5f,              [] { return toBitsString(WWMath::Sqrtf(runtimeValue(0.5f))); } },
	{ WWMathKey::Sqrtf_2_7f,              [] { return toBitsString(WWMath::Sqrtf(runtimeValue(2.7f))); } },
	{ WWMathKey::Sqrtf_187_66f,           [] { return toBitsString(WWMath::Sqrtf(runtimeValue(187.66f))); } },
	{ WWMathKey::Inv_Sqrt_Legacy_0_5f,    [] { return toBitsString(WWMath::Inv_Sqrt_Legacy(runtimeValue(0.5f))); } },
	{ WWMathKey::Inv_Sqrt_Legacy_2_7f,    [] { return toBitsString(WWMath::Inv_Sqrt_Legacy(runtimeValue(2.7f))); } },
	{ WWMathKey::Inv_Sqrt_Legacy_187_66f, [] { return toBitsString(WWMath::Inv_Sqrt_Legacy(runtimeValue(187.66f))); } },

	{ WWMathKey::Fabs_Minus0_5f,          [] { return toBitsString(WWMath::Fabs(runtimeValue(-0.5f))); } },
	{ WWMathKey::Fabs_2_7f,               [] { return toBitsString(WWMath::Fabs(runtimeValue(2.7f))); } },
	{ WWMathKey::Fabs_100_5f,             [] { return toBitsString(WWMath::Fabs(runtimeValue(100.5f))); } },
	{ WWMathKey::Fabs_Minus0_5,           [] { return toBitsString(WWMath::Fabs(runtimeValue(-0.5))); } },
	{ WWMathKey::Fabs_2_7,                [] { return toBitsString(WWMath::Fabs(runtimeValue(2.7))); } },
	{ WWMathKey::Fabs_100_5,              [] { return toBitsString(WWMath::Fabs(runtimeValue(100.5))); } },
	{ WWMathKey::Fabsf_Minus0_5f,         [] { return toBitsString(WWMath::Fabsf(runtimeValue(-0.5f))); } },
	{ WWMathKey::Fabsf_2_7f,              [] { return toBitsString(WWMath::Fabsf(runtimeValue(2.7f))); } },
	{ WWMathKey::Fabsf_100_5f,            [] { return toBitsString(WWMath::Fabsf(runtimeValue(100.5f))); } },
	{ WWMathKey::Fabsf_Legacy_Minus0_5f,  [] { return toBitsString(WWMath::Fabsf_Legacy(runtimeValue(-0.5f))); } },
	{ WWMathKey::Fabsf_Legacy_2_7f,       [] { return toBitsString(WWMath::Fabsf_Legacy(runtimeValue(2.7f))); } },
	{ WWMathKey::Fabsf_Legacy_100_5f,     [] { return toBitsString(WWMath::Fabsf_Legacy(runtimeValue(100.5f))); } },

	{ WWMathKey::Ceil_Minus0_5,           [] { return toBitsString(WWMath::Ceil(runtimeValue(-0.5))); } },
	{ WWMathKey::Ceil_2_7,                [] { return toBitsString(WWMath::Ceil(runtimeValue(2.7))); } },
	{ WWMathKey::Ceil_100_5,              [] { return toBitsString(WWMath::Ceil(runtimeValue(100.5))); } },
	{ WWMathKey::Ceilf_Minus0_5f,         [] { return toBitsString(WWMath::Ceilf(runtimeValue(-0.5f))); } },
	{ WWMathKey::Ceilf_2_7f,              [] { return toBitsString(WWMath::Ceilf(runtimeValue(2.7f))); } },
	{ WWMathKey::Ceilf_100_5f,            [] { return toBitsString(WWMath::Ceilf(runtimeValue(100.5f))); } },

	{ WWMathKey::Floor_Minus0_5,          [] { return toBitsString(WWMath::Floor(runtimeValue(-0.5))); } },
	{ WWMathKey::Floor_2_7,               [] { return toBitsString(WWMath::Floor(runtimeValue(2.7))); } },
	{ WWMathKey::Floor_100_5,             [] { return toBitsString(WWMath::Floor(runtimeValue(100.5))); } },
	{ WWMathKey::Floorf_Minus0_5f,        [] { return toBitsString(WWMath::Floorf(runtimeValue(-0.5f))); } },
	{ WWMathKey::Floorf_2_7f,             [] { return toBitsString(WWMath::Floorf(runtimeValue(2.7f))); } },
	{ WWMathKey::Floorf_100_5f,           [] { return toBitsString(WWMath::Floorf(runtimeValue(100.5f))); } },

	{ WWMathKey::Float_To_Long_Minus0_5f, [] { return toBitsString(WWMath::Float_To_Long(runtimeValue(-0.5f))); } },
	{ WWMathKey::Float_To_Long_2_7f,      [] { return toBitsString(WWMath::Float_To_Long(runtimeValue(2.7f))); } },
	{ WWMathKey::Float_To_Long_100_5f,    [] { return toBitsString(WWMath::Float_To_Long(runtimeValue(100.5f))); } },
	{ WWMathKey::Float_To_Long_Minus0_5,  [] { return toBitsString(WWMath::Float_To_Long(runtimeValue(-0.5))); } },
	{ WWMathKey::Float_To_Long_2_7,       [] { return toBitsString(WWMath::Float_To_Long(runtimeValue(2.7))); } },
	{ WWMathKey::Float_To_Long_100_5,     [] { return toBitsString(WWMath::Float_To_Long(runtimeValue(100.5))); } },

	{ WWMathKey::Div_Safe_1_0f_3_0f,      [] { return toBitsString(WWMath::Div_Safe(runtimeValue(1.0f), runtimeValue(3.0f))); } },
	{ WWMathKey::Div_Safe_1_0f_0_0f,      [] { return toBitsString(WWMath::Div_Safe(runtimeValue(1.0f), runtimeValue(0.0f))); } },
	{ WWMathKey::Div_Safe_1_0_3_0,        [] { return toBitsString(WWMath::Div_Safe(runtimeValue(1.0), runtimeValue(3.0))); } },
	{ WWMathKey::Div_Safe_1_0_0_0,        [] { return toBitsString(WWMath::Div_Safe(runtimeValue(1.0), runtimeValue(0.0))); } },
};

static_assert(hasEveryKeyInOrder(wwmathCalls), "wwmathCalls must have one row per WWMathKey, in WWMathKey order");
