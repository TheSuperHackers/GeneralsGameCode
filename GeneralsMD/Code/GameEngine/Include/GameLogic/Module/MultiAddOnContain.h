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

// FILE: MultiAddOnContain.h ////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, September, 2002
// Desc:   Contain module that acts as transport normally, but when full it redirects queries to the first passenger
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __MULTIADDON_CONTAIN_H_
#define __MULTIADDON_CONTAIN_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/TransportContain.h"
#include "GameLogic/GameLogic.h"


typedef std::vector<AsciiString> TemplateNameList;
typedef std::vector<AsciiString>::const_iterator TemplateNameIterator;

typedef std::vector<ObjectID> VecObjectID;
typedef VecObjectID::iterator VecObjectIDIt;

typedef std::vector<Object*> VecObjectPtr;
typedef VecObjectPtr::iterator VecObjectPtrIt;

typedef std::map< NameKeyType, AsciiString> AddOnEntryMap;

//-------------------------------------------------------------------------------------------------
class MultiAddOnContainModuleData : public TransportContainModuleData
{
public:

	MultiAddOnContainModuleData();

	TemplateNameList m_payloadTemplateNameData;
	Bool m_drawPips; // TODO: Move this to generic transportcontain

	AddOnEntryMap m_addOnEntries;  ///< allowed passengers and corresponding turrets

	AsciiString m_addOnBoneName;  ///< positions to place the turrets
	AsciiString m_emptySlotSubObjName;   ///< sub objects to hide/show when a slot is unmanned
	AsciiString m_occupiedSlotSubObjName;   ///< sub objects to hide/show when a slot is occupied


	static void buildFieldParse(MultiIniFieldParse& p);
	static void parseAddOnEntry(INI* ini, void* instance, void* store, const void* /*userData*/);
};

//-------------------------------------------------------------------------------------------------
class MultiAddOnContain : public TransportContain
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(MultiAddOnContain, "MultiAddOnContain")
		MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(MultiAddOnContain, MultiAddOnContainModuleData)

		virtual void onBodyDamageStateChange( const DamageInfo* damageInfo,
																					BodyDamageType oldState,
																					BodyDamageType newState);  ///< state change callback
public:

	MultiAddOnContain(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	virtual OpenContain* asOpenContain() { return this; }  ///< treat as open container
	virtual Bool isHealContain() const { return false; } ///< true when container only contains units while healing (not a transport!)
	virtual Bool isTunnelContain() const { return FALSE; }
	virtual Bool isImmuneToClearBuildingAttacks() const { return true; }
	virtual Bool isSpecialOverlordStyleContainer() const { return TRUE; }

	virtual void onDie(const DamageInfo* damageInfo);  ///< the die callback
	virtual void onDelete(void);	///< Last possible moment cleanup
	virtual void onCapture(Player* oldOwner, Player* newOwner);
	virtual void onObjectCreated();
	virtual void onContaining(Object* obj, Bool wasSelected);
	virtual void onRemoving(Object* obj);
	//virtual UpdateSleepTime update();							///< called once per frame

	virtual void onSelling();

	virtual Bool isValidContainerFor(const Object* obj, Bool checkCapacity) const;
	//virtual void addToContain(Object* obj);				///< add 'obj' to contain list
	//virtual void addToContainList(Object* obj);		///< The part of AddToContain that inheritors can override (Can't do whole thing because of all the private stuff involved)
	//virtual void removeFromContain(Object* obj, Bool exposeStealthUnits = FALSE);	///< remove 'obj' from contain list
	//virtual void removeAllContained( Bool exposeStealthUnits = FALSE );				///< remove all objects on contain list
	virtual Bool isEnclosingContainerFor(const Object* obj) const;	///< Does this type of Contain Visibly enclose its contents?
	virtual Bool isPassengerAllowedToFire(ObjectID id = INVALID_ID) const;	///< Hey, can I shoot out of this container?

	// Friend for our Draw module only.
	//virtual const Object* friend_getRider() const; ///< Damn.  The draw order dependency bug for riders means that our draw module needs to cheat to get around it.

	///< if my object gets selected, then my visible passengers should, too
	///< this gets called from
	virtual void clientVisibleContainedFlashAsSelected();

	virtual void redeployOccupants();

	virtual Bool getContainerPipsToShow(Int& numTotal, Int& numFull)
	{
		if (getMultiAddOnContainModuleData()->m_drawPips == FALSE)
		{
			return FALSE;
		}

		return ContainModuleInterface::getContainerPipsToShow(numTotal, numFull);
	}

	virtual void createPayload();

	virtual short getRiderSlot(ObjectID riderID) const;
	virtual short getPortableSlot(ObjectID riderID) const;
	virtual const ContainedItemsList* getAddOnList() const { return &m_addOnList; }

private:
	void parseAddOnEntry(INI* ini, void* instance, void* store, const void* /*userData*/);

	//VecObjectPtr& getPortableStructures(void) const;

	struct AddOnSlotData
	{
		ObjectID  occupantID;
		ObjectID  portableID;
		UnsignedShort slot;
	};

	std::vector<AddOnSlotData> m_addOnSlots;
	ContainedItemsList m_addOnList;
	//bool m_addOnList_valid;

	//std::vector<Matrix3D> m_addOnPoints;
	//Bool m_noAddOnPointsInArt;

	bool createAddOnForRider(Object* obj);
	bool removeAddOnForRider(Object* obj);

	void redeployAddOns();
	void putObjAtSlot(Object* obj, short slot);

	void updateSubObjForSlot(short slot, bool isNowOccupied);

};

#endif

