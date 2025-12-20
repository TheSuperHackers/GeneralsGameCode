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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: BuffSystem.h /////////////////////////////////////////////////////////////////////////////
// Author: Andi W, October 25
// Desc:   Buff/Debuff effects
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _BuffSystem_H_
#define _BuffSystem_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameMemory.h"
#include "GameLogic/Object.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class BuffEffectNugget;
class BuffTemplate;
class BuffTemplateStore;

struct BuffEffectTracker;

//class Object;


class BuffEffectNugget : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_ABC(BuffEffectNugget)

public:

	BuffEffectNugget() { }
	//virtual ~BuffEffectNugget() { }

	virtual void apply(Object* targetObj, const Object* sourceObj, BuffEffectTracker* buffTracker) const = 0;

	virtual void remove(Object* targetObj, BuffEffectTracker* buffTracker) const = 0;


};
EMPTY_DTOR(BuffEffectNugget)

// -----------------------------------------------
// -----------------------------------------------

class BuffTemplate : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(BuffTemplate, "BuffTemplate")

public:
	// for lookup
	AsciiString getName(void) const { return m_name; }
	void friend_setName(const AsciiString& n) { m_name = n; }

	inline UnsignedInt getMaxStackSize() const { return m_maxStackSize; }
	inline Bool isStackPerSource() const { return m_stackPerSource; }
	inline const std::vector<AsciiString>& getPriorityTemplates() const { return m_priorityTemplates; }

	//inline WeaponBonusConditionType getWeaponBonusType() const { return m_bonusType; }
	//inline WeaponBonusConditionType getWeaponBonusTypeAgainst() const { return m_bonusTypeAgainst; }
	//inline WeaponSetType getWeaponSetFlag() const { return m_weaponSetFlag; }
	//inline ArmorSetType getArmorSetFlag() const { return m_armorSetFlag; }
	//inline ObjectStatusMaskType getStatusToSet() const { return m_statusToSet; }

	BuffTemplate();

	/**
		Toss the contents.
	*/
	void clear();

	void addBuffEffectNugget(BuffEffectNugget* nugget);

	UnsignedInt getNextTickFrame(UnsignedInt startFrame, UnsignedInt endFrame) const;

	void applyEffects(Object* targetObj, Object* sourceObj, BuffEffectTracker* buffTracker) const;
	void removeEffects(Object* targetObj, BuffEffectTracker* buffTracker, Bool ignoreFlags = FALSE) const;

	Bool hasPriorityOver(AsciiString templateName) const;

	void reApplyFlags(Object* targetObj, const BuffTemplate* other) const;

	const FieldParse* getFieldParse(void) const { return TheBuffTemplateFieldParse; }


protected:
	AsciiString			m_name;
	UnsignedInt     m_maxStackSize;
	Bool						m_stackPerSource;   ///< Each sourceObj has its own stack of buffs on the target.

	std::vector<AsciiString>  m_priorityTemplates;   ///< list of buffTemplates this template has priority over.

	// Flags are stored directly in BuffTemplate
	WeaponBonusConditionType	m_bonusType;         ///< weapon bonus granted to the object
	WeaponBonusConditionType	m_bonusTypeAgainst;  ///< weapon bonus granted when attacking the object
	WeaponSetType m_weaponSetFlag;                 ///< the weaponset flag to set
	ArmorSetType m_armorSetFlag;                   ///< the armorset flag to set
	ObjectStatusMaskType m_statusToSet;           ///< the status to set (allows multiple)

	// --
	static const FieldParse TheBuffTemplateFieldParse[];
	//static const FieldParse flagModifierFieldParse[];

	static void parseFlagModifier(INI* ini, void* instance, void* store, const void* userData);

private:

	// note, this list doesn't own the nuggets; all nuggets are owned by the Store.
	typedef std::vector<BuffEffectNugget*> BuffEffectNuggetVector;
	BuffEffectNuggetVector m_nuggets;

};
EMPTY_DTOR(BuffTemplate)

//-------------------------------------------------------------------------------------------------
/**
	The "store" used to hold all the Buffs in existence.
*/
class BuffTemplateStore : public SubsystemInterface
{

public:

	BuffTemplateStore();
	~BuffTemplateStore();

	void init() {}
	void reset() {}
	void update() {}

	/**
		return the BuffTemplate with the given namekey.
		return NULL if no such BuffTemplate exists.
	*/
	const BuffTemplate* findBuffTemplate(const char* name) const;
	const BuffTemplate* findBuffTemplate(const AsciiString& name) const { return findBuffTemplate(name.str()); }

	BuffTemplate* findBuffTemplate(const char* name);
	BuffTemplate* findBuffTemplate(AsciiString& name) { return findBuffTemplate(name.str()); }

	//BuffTemplate* newBuffTemplate(AsciiString& name);

	static void parseBuffTemplateDefinition(INI* ini);

	void addBuffEffectNugget(BuffEffectNugget* nugget);

private:

	typedef std::map< NameKeyType, BuffTemplate*, std::less<NameKeyType> > BuffTemplateMap;
	BuffTemplateMap m_buffTemplates;

	// note, this list doesn't own the nuggets; all nuggets are owned by the Store.
	typedef std::vector<BuffEffectNugget*> BuffEffectNuggetVector;
	BuffEffectNuggetVector m_nuggets;

};

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern BuffTemplateStore* TheBuffTemplateStore;

#endif // _Buff_H_

