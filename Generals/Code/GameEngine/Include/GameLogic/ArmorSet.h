/*
**	Command & Conquer Generals(tm)
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// ArmorSet.h

#pragma once

#ifndef _ArmorSet_H_
#define _ArmorSet_H_

#include "Lib/BaseType.h"
#include "Common/GameType.h"
#include "Common/SparseMatchFinder.h"
#include "Common/SparseMatchFinder.h"

//-------------------------------------------------------------------------------------------------
class ArmorTemplate;
class DamageFX;
class INI;

//-------------------------------------------------------------------------------------------------
// IMPORTANT NOTE: you should endeavor to set up states such that the most "normal"
// state is defined by the bit being off. That is, the typical "normal" condition
// has all condition flags set to zero.
enum ArmorSetType CPP_11(: Int)
{
	// The access and use of this enum has the bit shifting built in, so this is a 0,1,2,3,4,5 enum
	ARMORSET_VETERAN		= 0,
	ARMORSET_ELITE			= 1,
	ARMORSET_HERO				= 2,
	ARMORSET_PLAYER_UPGRADE = 3,
	ARMORSET_WEAK_VERSUS_BASEDEFENSES = 4,

	ARMORSET_COUNT			///< keep last, please
};

//-------------------------------------------------------------------------------------------------
typedef BitFlags<ARMORSET_COUNT> ArmorSetFlags;

//-------------------------------------------------------------------------------------------------
class ArmorTemplateSet
{
private:
	ArmorSetFlags m_types;
	const ArmorTemplate* m_template;
	const DamageFX* m_fx;

public:
	inline ArmorTemplateSet()
	{
		clear();
	}

	inline void clear()
	{
		m_types.clear();
		m_template = NULL;
		m_fx = NULL;
	}

	inline const ArmorTemplate* getArmorTemplate() const { return m_template; } 
	inline const DamageFX* getDamageFX() const { return m_fx; } 

	inline Int getConditionsYesCount() const { return 1; }
	inline const ArmorSetFlags& getNthConditionsYes(Int i) const { return m_types; }
#if defined(RTS_DEBUG) || defined(_INTERNAL)
	inline AsciiString getDescription() const { return AsciiString("ArmorTemplateSet"); }
#endif

	void parseArmorTemplateSet( INI* ini );
};

//-------------------------------------------------------------------------------------------------
typedef std::vector<ArmorTemplateSet> ArmorTemplateSetVector;

//-------------------------------------------------------------------------------------------------
typedef SparseMatchFinder<ArmorTemplateSet, ArmorSetFlags> ArmorTemplateSetFinder;

#endif	// _ArmorSet_H_
