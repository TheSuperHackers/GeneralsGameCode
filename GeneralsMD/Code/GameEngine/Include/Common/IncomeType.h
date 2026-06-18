/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

// FILE: IncomeType.h ///////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
// Categories used to attribute a player's income to a source, for the
// per-source money breakdown reported to the stats backend. Threaded through
// Money::deposit() and accumulated per-player in ScoreKeeper. INCOME_OTHER is
// 0 so a zeroed bucket array / the default deposit() argument both land there.
//
//-----------------------------------------------------------------------------

#pragma once

enum IncomeType
{
	INCOME_OTHER = 0,		///< uncategorized (scripted/cheat money, map start money, etc.)
	INCOME_SUPPLY,			///< supply collection (trucks/workers -> supply center)
	INCOME_HACKER,			///< internet hacker passive income
	INCOME_BLACK_MARKET,	///< GLA black market passive income
	INCOME_SUPPLY_DROP,		///< USA supply drop zone passive income
	INCOME_OIL_DERRICK,		///< captured neutral tech building (oil derrick, etc.)
	INCOME_BOUNTY,			///< cash bounty from kills / POW prisoner delivery
	INCOME_SALVAGE,			///< salvage crates dropped by destroyed units
	INCOME_CRATE,			///< money crates
	INCOME_THEFT,			///< cash hack / sabotage / special-ability steal

	INCOME_COUNT			///< keep last
};
