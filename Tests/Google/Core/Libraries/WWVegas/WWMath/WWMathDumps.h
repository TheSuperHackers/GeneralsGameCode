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

#include <array>

#include "WWMathKeys.h"
#include "WWMathTestCase.h"

typedef std::array<const char *, WWMATH_COLUMN_COUNT> WWMathDump;

struct WWMathDumpEntry
{
	WWMathKey key;
	WWMathDump dump;
};

static constexpr WWMathDumpEntry wwmathDumps[] = {
	//                                      DETERMINISTIC       RETAIL_COMPATIBLE
	{ WWMathKey::Sin_0_5f,                { "3EF57744",         "3EF57744"         } },
	{ WWMathKey::Sin_2_7f,                { "3EDAD188",         "3EDAD188"         } },
	{ WWMathKey::Sin_187_66f,             { "BF3DDE0A",         "BF3DDE0A"         } },
	{ WWMathKey::Sin_0_5,                 { "3FDEAEE8744B05F0", "3FDEAEE8744B05F0" } },
	{ WWMathKey::Sin_2_7,                 { "3FDB5A312424A70C", "3FDB5A312424A70C" } },
	{ WWMathKey::Sin_187_66,              { "BFE7BBC6596262AE", "BFE7BBC6596262AE" } },
	{ WWMathKey::Sinf_0_5f,               { "3EF57744",         "3EF57744"         } },
	{ WWMathKey::Sinf_2_7f,               { "3EDAD188",         "3EDAD188"         } },
	{ WWMathKey::Sinf_187_66f,            { "BF3DDE0A",         "BF3DDE0A"         } },
	{ WWMathKey::Sinf_Legacy_0_5f,        { "3EF57744",         "3EF57744"         } },
	{ WWMathKey::Sinf_Legacy_2_7f,        { "3EDAD188",         "3EDAD188"         } },
	{ WWMathKey::Sinf_Legacy_187_66f,     { "BF3DDE0A",         "BF3DDE0A"         } },

	{ WWMathKey::Cos_0_5f,                { "3F60A940",         "3F60A940"         } },
	{ WWMathKey::Cos_2_7f,                { "BF677146",         "BF677146"         } },
	{ WWMathKey::Cos_187_66f,             { "3F2BB74E",         "3F2BB74E"         } },
	{ WWMathKey::Cos_0_5,                 { "3FEC1528065B7D50", "3FEC1528065B7D50" } },
	{ WWMathKey::Cos_2_7,                 { "BFECEE28B36603A6", "BFECEE28B36603A6" } },
	{ WWMathKey::Cos_187_66,              { "3FE576E4096F1DE4", "3FE576E4096F1DE4" } },
	{ WWMathKey::Cosf_0_5f,               { "3F60A940",         "3F60A940"         } },
	{ WWMathKey::Cosf_2_7f,               { "BF677146",         "BF677146"         } },
	{ WWMathKey::Cosf_187_66f,            { "3F2BB74E",         "3F2BB74E"         } },
	{ WWMathKey::Cosf_Legacy_0_5f,        { "3F60A940",         "3F60A940"         } },
	{ WWMathKey::Cosf_Legacy_2_7f,        { "BF677146",         "BF677146"         } },
	{ WWMathKey::Cosf_Legacy_187_66f,     { "3F2BB74E",         "3F2BB74E"         } },

	{ WWMathKey::Tan_0_5f,                { "3F0BDA7A",         "3F0BDA7B"         } },
	{ WWMathKey::Tan_2_7f,                { "BEF20959",         "BEF20959"         } },
	{ WWMathKey::Tan_187_66f,             { "BF8D87C1",         "BF8D87C1"         } },
	{ WWMathKey::Tan_0_5,                 { "3FE17B4F5BF3474A", "3FE17B4F5BF3474A" } },
	{ WWMathKey::Tan_2_7,                 { "BFDE412B62BA7143", "BFDE412B62BA7143" } },
	{ WWMathKey::Tan_187_66,              { "BFF1B1009E8D90B0", "BFF1B1009E8D90B0" } },
	{ WWMathKey::Tanf_0_5f,               { "3F0BDA7A",         "3F0BDA7B"         } },
	{ WWMathKey::Tanf_2_7f,               { "BEF20959",         "BEF20959"         } },
	{ WWMathKey::Tanf_187_66f,            { "BF8D87C1",         "BF8D87C1"         } },

	{ WWMathKey::Asin_Minus0_5f,          { "BF060A94",         "BF060A92"         } },
	{ WWMathKey::Asin_0_25f,              { "3E815F4E",         "3E815F4E"         } },
	{ WWMathKey::Asin_0_9f,               { "3F8F549C",         "3F8F549B"         } },
	{ WWMathKey::Asin_Minus0_5,           { "BFE0C152382D7366", "BFE0C152382D7366" } },
	{ WWMathKey::Asin_0_25,               { "3FD02BE9CE0B87CD", "3FD02BE9CE0B87CD" } },
	{ WWMathKey::Asin_0_9,                { "3FF1EA93705FA172", "3FF1EA93705FA172" } },
	{ WWMathKey::Asinf_Minus0_5f,         { "BF060A94",         "BF060A92"         } },
	{ WWMathKey::Asinf_0_25f,             { "3E815F4E",         "3E815F4E"         } },
	{ WWMathKey::Asinf_0_9f,              { "3F8F549C",         "3F8F549B"         } },

	{ WWMathKey::Acos_Minus0_5f,          { "40060A92",         "40060A92"         } },
	{ WWMathKey::Acos_0_25f,              { "3FA8B807",         "3FA8B807"         } },
	{ WWMathKey::Acos_0_9f,               { "3EE6ECFE",         "3EE6ECFE"         } },
	{ WWMathKey::Acos_Minus0_5,           { "4000C152382D7366", "4000C152382D7366" } },
	{ WWMathKey::Acos_0_25,               { "3FF51700E0C14B25", "3FF51700E0C14B25" } },
	{ WWMathKey::Acos_0_9,                { "3FDCDD9F8F922E98", "3FDCDD9F8F922E98" } },
	{ WWMathKey::Acosf_Minus0_5f,         { "40060A92",         "40060A92"         } },
	{ WWMathKey::Acosf_0_25f,             { "3FA8B807",         "3FA8B807"         } },
	{ WWMathKey::Acosf_0_9f,              { "3EE6ECFE",         "3EE6ECFE"         } },

	{ WWMathKey::Atan_0_5f,               { "3EED6338",         "3EED6338"         } },
	{ WWMathKey::Atan_2_7f,               { "3F9BA8DC",         "3F9BA8DC"         } },
	{ WWMathKey::Atan_187_66f,            { "3FC8613E",         "3FC8613E"         } },
	{ WWMathKey::Atan_0_5,                { "3FDDAC6700000000", "3FDDAC670561BB4F" } },
	{ WWMathKey::Atan_2_7,                { "3FF3751B80000000", "3FF3751B7ED2AEC1" } },
	{ WWMathKey::Atan_187_66,             { "3FF90C27C0000000", "3FF90C27BEADA1E0" } },
	{ WWMathKey::Atanf_0_5f,              { "3EED6338",         "3EED6338"         } },
	{ WWMathKey::Atanf_2_7f,              { "3F9BA8DC",         "3F9BA8DC"         } },
	{ WWMathKey::Atanf_187_66f,           { "3FC8613E",         "3FC8613E"         } },

	{ WWMathKey::Atan2_0_4f_1_3f,         { "3E98D4DA",         "3E98D4DB"         } },
	{ WWMathKey::Atan2_2_7f_11_9f,        { "3E6477D5",         "3E6477D5"         } },
	{ WWMathKey::Atan2_Minus3_5f_2_0f,    { "BF869C79",         "BF869C79"         } },
	{ WWMathKey::Atan2_0_4_1_3,           { "3FD31A9B40000000", "3FD31A9B43436DE1" } },
	{ WWMathKey::Atan2_2_7_11_9,          { "3FCC8EFAA0000000", "3FCC8EFA8684064B" } },
	{ WWMathKey::Atan2_Minus3_5_2_0,      { "BFF0D38F20000000", "BFF0D38F2C5BA09F" } },
	{ WWMathKey::Atan2f_0_4f_1_3f,        { "3E98D4DA",         "3E98D4DB"         } },
	{ WWMathKey::Atan2f_2_7f_11_9f,       { "3E6477D5",         "3E6477D5"         } },
	{ WWMathKey::Atan2f_Minus3_5f_2_0f,   { "BF869C79",         "BF869C79"         } },

	{ WWMathKey::Sinh_0_5,                { "3FE0ACD00FE63B97", "3FE0ACD00FE63B97" } },
	{ WWMathKey::Sinh_Minus0_5,           { "BFE0ACD00FE63B97", "BFE0ACD00FE63B97" } },
	{ WWMathKey::Sinh_2_7,                { "401DA0036F88823C", "401DA0036F88823C" } },
	{ WWMathKey::Sinhf_0_5f,              { "3F056680",         "3F056680"         } },
	{ WWMathKey::Sinhf_Minus0_5f,         { "BF056680",         "BF056680"         } },
	{ WWMathKey::Sinhf_2_7f,              { "40ED001C",         "40ED001C"         } },

	{ WWMathKey::Cosh_0_5,                { "3FF20AC1862AE8D0", "3FF20AC1862AE8D0" } },
	{ WWMathKey::Cosh_Minus0_5,           { "3FF20AC1862AE8D0", "3FF20AC1862AE8D0" } },
	{ WWMathKey::Cosh_2_7,                { "401DE4D4F525D240", "401DE4D4F525D23F" } },
	{ WWMathKey::Coshf_0_5f,              { "3F90560C",         "3F90560C"         } },
	{ WWMathKey::Coshf_Minus0_5f,         { "3F90560C",         "3F90560C"         } },
	{ WWMathKey::Coshf_2_7f,              { "40EF26A8",         "40EF26A8"         } },

	{ WWMathKey::Tanh_0_5,                { "3FDD9353D7568AF3", "3FDD9353D7568AF3" } },
	{ WWMathKey::Tanh_Minus0_5,           { "BFDD9353D7568AF3", "BFDD9353D7568AF3" } },
	{ WWMathKey::Tanh_2_7,                { "3FEFB655437478D5", "3FEFB655437478D5" } },
	{ WWMathKey::Tanhf_0_5f,              { "3EEC9A9F",         "3EEC9A9F"         } },
	{ WWMathKey::Tanhf_Minus0_5f,         { "BEEC9A9F",         "BEEC9A9F"         } },
	{ WWMathKey::Tanhf_2_7f,              { "3F7DB2AA",         "3F7DB2AA"         } },

	{ WWMathKey::Exp_0_5,                 { "3FFA61298E1E069C", "3FFA61298E1E069C" } },
	{ WWMathKey::Exp_Minus0_5,            { "3FE368B2FC6F960A", "3FE368B2FC6F960A" } },
	{ WWMathKey::Exp_2_7,                 { "402DC26C32572A3E", "402DC26C32572A3E" } },
	{ WWMathKey::Expf_0_5f,               { "3FD3094C",         "3FD3094C"         } },
	{ WWMathKey::Expf_Minus0_5f,          { "3F1B4598",         "3F1B4598"         } },
	{ WWMathKey::Expf_2_7f,               { "416E1362",         "416E1362"         } },

	{ WWMathKey::Log_0_5,                 { "BFE62E42FEFA39EF", "BFE62E42FEFA39EF" } },
	{ WWMathKey::Log_2_7,                 { "3FEFC8B7F138BDEA", "3FEFC8B7F138BDEA" } },
	{ WWMathKey::Log_187_66,              { "4014F043528DBB65", "4014F043528DBB65" } },
	{ WWMathKey::Logf_0_5f,               { "BF317218",         "BF317218"         } },
	{ WWMathKey::Logf_2_7f,               { "3F7E45C0",         "3F7E45C0"         } },
	{ WWMathKey::Logf_187_66f,            { "40A7821B",         "40A7821B"         } },

	{ WWMathKey::Log10_0_5,               { "BFD34413509F79FF", "BFD34413509F79FF" } },
	{ WWMathKey::Log10_2_7,               { "3FDB9B76C2EF81F8", "3FDB9B76C2EF81F8" } },
	{ WWMathKey::Log10_187_66,            { "40022FDD82152DEA", "40022FDD82152DEA" } },
	{ WWMathKey::Log10f_0_5f,             { "BE9A209B",         "BE9A209B"         } },
	{ WWMathKey::Log10f_2_7f,             { "3EDCDBB6",         "3EDCDBB6"         } },
	{ WWMathKey::Log10f_187_66f,          { "40117EEC",         "40117EEC"         } },

	{ WWMathKey::Pow_0_4_1_3,             { "3FD3727E60000000", "3FD3727E49ADFAD4" } },
	{ WWMathKey::Pow_2_7_11_9,            { "410096F740000000", "410096F75FA58808" } },
	{ WWMathKey::Pow_Minus3_5_2_0,        { "4028800000000000", "4028800000000000" } },
	{ WWMathKey::Powf_0_4f_1_3f,          { "3E9B93F3",         "3E9B93F3"         } },
	{ WWMathKey::Powf_2_7f_11_9f,         { "4804B7BA",         "4804B7BA"         } },
	{ WWMathKey::Powf_Minus3_5f_2_0f,     { "41440000",         "41440000"         } },

	{ WWMathKey::Sqrt_0_5f,               { "3F3504F3",         "3F3504F3"         } },
	{ WWMathKey::Sqrt_2_7f,               { "3FD25352",         "3FD25352"         } },
	{ WWMathKey::Sqrt_187_66f,            { "415B2EB7",         "415B2EB7"         } },
	{ WWMathKey::Sqrt_0_5,                { "3FE6A09E667F3BCD", "3FE6A09E667F3BCD" } },
	{ WWMathKey::Sqrt_2_7,                { "3FFA4A6A2F74C6AC", "3FFA4A6A2F74C6AC" } },
	{ WWMathKey::Sqrt_187_66,             { "402B65D6E280ACC1", "402B65D6E280ACC1" } },
	{ WWMathKey::Sqrtf_0_5f,              { "3F3504F3",         "3F3504F3"         } },
	{ WWMathKey::Sqrtf_2_7f,              { "3FD25352",         "3FD25352"         } },
	{ WWMathKey::Sqrtf_187_66f,           { "415B2EB7",         "415B2EB7"         } },
	{ WWMathKey::Inv_Sqrt_Legacy_0_5f,    { "3FB504F3",         "3FB504F3"         } },
	{ WWMathKey::Inv_Sqrt_Legacy_2_7f,    { "3F1BCBF0",         "3F1BCBF0"         } },
	{ WWMathKey::Inv_Sqrt_Legacy_187_66f, { "3D958042",         "3D958041"         } },

	{ WWMathKey::Fabs_Minus0_5f,          { "3F000000",         "3F000000"         } },
	{ WWMathKey::Fabs_2_7f,               { "402CCCCD",         "402CCCCD"         } },
	{ WWMathKey::Fabs_100_5f,             { "42C90000",         "42C90000"         } },
	{ WWMathKey::Fabs_Minus0_5,           { "3FE0000000000000", "3FE0000000000000" } },
	{ WWMathKey::Fabs_2_7,                { "400599999999999A", "400599999999999A" } },
	{ WWMathKey::Fabs_100_5,              { "4059200000000000", "4059200000000000" } },
	{ WWMathKey::Fabsf_Minus0_5f,         { "3F000000",         "3F000000"         } },
	{ WWMathKey::Fabsf_2_7f,              { "402CCCCD",         "402CCCCD"         } },
	{ WWMathKey::Fabsf_100_5f,            { "42C90000",         "42C90000"         } },
	{ WWMathKey::Fabsf_Legacy_Minus0_5f,  { "3F000000",         "3F000000"         } },
	{ WWMathKey::Fabsf_Legacy_2_7f,       { "402CCCCD",         "402CCCCD"         } },
	{ WWMathKey::Fabsf_Legacy_100_5f,     { "42C90000",         "42C90000"         } },

	{ WWMathKey::Ceil_Minus0_5,           { "8000000000000000", "8000000000000000" } },
	{ WWMathKey::Ceil_2_7,                { "4008000000000000", "4008000000000000" } },
	{ WWMathKey::Ceil_100_5,              { "4059400000000000", "4059400000000000" } },
	{ WWMathKey::Ceilf_Minus0_5f,         { "80000000",         "80000000"         } },
	{ WWMathKey::Ceilf_2_7f,              { "40400000",         "40400000"         } },
	{ WWMathKey::Ceilf_100_5f,            { "42CA0000",         "42CA0000"         } },

	{ WWMathKey::Floor_Minus0_5,          { "BFF0000000000000", "BFF0000000000000" } },
	{ WWMathKey::Floor_2_7,               { "4000000000000000", "4000000000000000" } },
	{ WWMathKey::Floor_100_5,             { "4059000000000000", "4059000000000000" } },
	{ WWMathKey::Floorf_Minus0_5f,        { "BF800000",         "BF800000"         } },
	{ WWMathKey::Floorf_2_7f,             { "40000000",         "40000000"         } },
	{ WWMathKey::Floorf_100_5f,           { "42C80000",         "42C80000"         } },

	{ WWMathKey::Float_To_Long_Minus0_5f, { "0",                "0"                } },
	{ WWMathKey::Float_To_Long_2_7f,      { "3",                "3"                } },
	{ WWMathKey::Float_To_Long_100_5f,    { "100",              "100"              } },
	{ WWMathKey::Float_To_Long_Minus0_5,  { "0",                "0"                } },
	{ WWMathKey::Float_To_Long_2_7,       { "3",                "3"                } },
	{ WWMathKey::Float_To_Long_100_5,     { "100",              "100"              } },

	{ WWMathKey::Div_Safe_1_0f_3_0f,      { "3EAAAAAB",         "3EAAAAAB"         } },
	{ WWMathKey::Div_Safe_1_0f_0_0f,      { "461C4000",         "7F800000"         } },
	{ WWMathKey::Div_Safe_1_0_3_0,        { "3FD5555555555555", "3FD5555555555555" } },
	{ WWMathKey::Div_Safe_1_0_0_0,        { "40C3880008472404", "7FF0000000000000" } },
};

static_assert(hasEveryKeyInOrder(wwmathDumps), "wwmathDumps must have one row per WWMathKey, in WWMathKey order");
