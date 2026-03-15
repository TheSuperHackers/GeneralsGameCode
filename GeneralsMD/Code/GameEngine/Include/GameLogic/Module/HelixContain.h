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

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  FILE: HelixContain.h ////////////////////////////////////////////////////////////////////////
//  Author: Mark Lorenzen, April, 2003
//
//  Desc:   Contain module that acts as transport normally, but
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/TransportContain.h"

typedef std::vector<AsciiString> TemplateNameList;
typedef std::vector<AsciiString>::const_iterator TemplateNameIterator;

//-------------------------------------------------------------------------------------------------
class HelixContainModuleData : public TransportContainModuleData
{
public:

	HelixContainModuleData();

	TemplateNameList m_payloadTemplateNameData;
  Bool m_drawPips;


	static void buildFieldParse(MultiIniFieldParse& p);
	static void parseInitialPayload( INI* ini, void *instance, void *store, const void* /*userData*/ );
};

//-------------------------------------------------------------------------------------------------
class HelixContain : public TransportContain
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( HelixContain, "HelixContain" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( HelixContain, HelixContainModuleData )

	virtual void onBodyDamageStateChange( const DamageInfo* damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState) override;  ///< state change callback
public:

	HelixContain( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual OpenContain *asOpenContain() override { return this; }  ///< treat as open container
	virtual Bool isHealContain() const override { return false; } ///< true when container only contains units while healing (not a transport!)
	virtual Bool isTunnelContain() const override { return FALSE; }
	virtual Bool isImmuneToClearBuildingAttacks() const override { return true; }
  virtual Bool isSpecialOverlordStyleContainer() const override {return TRUE;}

	virtual void onDie( const DamageInfo *damageInfo ) override;  ///< the die callback
	virtual void onDelete() override;	///< Last possible moment cleanup
	virtual void onCapture( Player *oldOwner, Player *newOwner ) override;
	virtual void onObjectCreated() override;
  virtual void onContaining( Object *obj, Bool wasSelected  ) override;
  virtual void onRemoving( Object *obj ) override;


  virtual UpdateSleepTime update() override;							///< called once per frame


 // virtual void onContaining( Object *obj, Bool wasSelected );		///< object now contains 'obj'
//	virtual void onRemoving( Object *obj );			///< object no longer contains 'obj'

	virtual Bool isValidContainerFor(const Object* obj, Bool checkCapacity) const override;
	virtual void addToContain( Object *obj ) override;				///< add 'obj' to contain list
	virtual void addToContainList( Object *obj );		///< The part of AddToContain that inheritors can override (Can't do whole thing because of all the private stuff involved)
	virtual void removeFromContain( Object *obj, Bool exposeStealthUnits = FALSE ) override;	///< remove 'obj' from contain list
	//virtual void removeAllContained( Bool exposeStealthUnits = FALSE );				///< remove all objects on contain list
	virtual Bool isEnclosingContainerFor( const Object *obj ) const override;	///< Does this type of Contain Visibly enclose its contents?
	virtual Bool isPassengerAllowedToFire(  ObjectID id = INVALID_ID  ) const override;	///< Hey, can I shoot out of this container?

	// Friend for our Draw module only.
	virtual const Object *friend_getRider() const override; ///< Damn.  The draw order dependency bug for riders means that our draw module needs to cheat to get around it.

	///< if my object gets selected, then my visible passengers should, too
	///< this gets called from
	virtual void clientVisibleContainedFlashAsSelected() override;

  virtual void redeployOccupants() override;

	virtual Bool getContainerPipsToShow(Int& numTotal, Int& numFull) override
  {
    if ( getHelixContainModuleData()->m_drawPips == FALSE )
    {
      numTotal = 0;
      numFull = 0;
      return FALSE;
    }

    return ContainModuleInterface::getContainerPipsToShow( numTotal, numFull );
  }

	virtual void createPayload() override;

private:
  void parseInitialPayload( INI* ini, void *instance, void *store, const void* /*userData*/ );
  Object *getPortableStructure();
  ObjectID  m_portableStructureID;

};
