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

// FILE: OverlordContain.h ////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, September, 2002
// Desc:   Contain module that acts as transport normally, but when full it redirects queries to the first passenger
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/TransportContain.h"
#include "GameLogic/GameLogic.h"


typedef std::vector<AsciiString> TemplateNameList;
typedef std::vector<AsciiString>::const_iterator TemplateNameIterator;

//-------------------------------------------------------------------------------------------------
class OverlordContainModuleData : public TransportContainModuleData
{
public:

	OverlordContainModuleData();

	TemplateNameList m_payloadTemplateNameData;
	Bool m_experienceSinkForRider;


	static void buildFieldParse(MultiIniFieldParse& p);
	static void parseInitialPayload( INI* ini, void *instance, void *store, const void* /*userData*/ );
};

//-------------------------------------------------------------------------------------------------
class OverlordContain : public TransportContain
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( OverlordContain, "OverlordContain" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( OverlordContain, OverlordContainModuleData )

	virtual void onBodyDamageStateChange( const DamageInfo* damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState);  ///< state change callback
public:

	OverlordContain( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual OpenContain *asOpenContain() { return this; }  ///< treat as open container
	virtual Bool isGarrisonable() const;	///< can this unit be Garrisoned? (ick)
  virtual Bool isBustable() { return false;};	///< can this container get busted by bunkerbuster? (ick)
	virtual Bool isHealContain() const { return false; } ///< true when container only contains units while healing (not a transport!)
	virtual Bool isTunnelContain() const { return FALSE; }
	virtual Bool isImmuneToClearBuildingAttacks() const { return true; }
  virtual Bool isSpecialOverlordStyleContainer() const {return TRUE;}
	virtual Bool isPassengerAllowedToFire( ObjectID id = INVALID_ID ) const;	///< Hey, can I shoot out of this container?


	virtual void onDie( const DamageInfo *damageInfo );  ///< the die callback
	virtual void onDelete( void );	///< Last possible moment cleanup
	virtual void onCapture( Player *oldOwner, Player *newOwner ); // Our main guy goes with us, but our redirected contain needs to do his thing too
	virtual void onObjectCreated();

	// Contain stuff we need to override to redirect on a condition
	virtual void onContaining( Object *obj, Bool wasSelected );		///< object now contains 'obj'
	virtual void onRemoving( Object *obj );			///< object no longer contains 'obj'

	virtual Bool isValidContainerFor(const Object* obj, Bool checkCapacity) const;
	virtual void addToContain( Object *obj );				///< add 'obj' to contain list
	virtual void addToContainList( Object *obj );		///< The part of AddToContain that inheritors can override (Can't do whole thing because of all the private stuff involved)
	virtual void removeFromContain( Object *obj, Bool exposeStealthUnits = FALSE );	///< remove 'obj' from contain list
	virtual void removeAllContained( Bool exposeStealthUnits = FALSE );				///< remove all objects on contain list
	virtual Bool isEnclosingContainerFor( const Object *obj ) const;	///< Does this type of Contain Visibly enclose its contents?
	virtual Bool isDisplayedOnControlBar() const ;///< Does this container display its contents on the ControlBar?
	virtual Bool isKickOutOnCapture();// The bunker may want to, but we certainly don't
	virtual void killAllContained( void );				///< kill all objects inside.  For us, this does not mean our rider

	// contain list access
	virtual void iterateContained( ContainIterateFunc func, void *userData, Bool reverse );
	virtual UnsignedInt getContainCount() const;
	virtual Int getContainMax( void ) const;
	virtual const ContainedItemsList* getContainedItemsList() const;

	// Friend for our Draw module only.
	virtual const Object *friend_getRider() const; ///< Damn.  The draw order dependency bug for riders means that our draw module needs to cheat to get around it.

	///< if my object gets selected, then my visible passengers should, too
	///< this gets called from
	virtual void clientVisibleContainedFlashAsSelected();

	virtual Bool getContainerPipsToShow(Int& numTotal, Int& numFull);
	virtual void createPayload();

private:
	/**< An empty overlord is a container, but a full one redirects calls to its passengers.  If this returns null,
	we are either empty or carrying a non container.
	*/
	ContainModuleInterface *getRedirectedContain() const; ///< And this gets what are redirecting to.
	void activateRedirectedContain();///< I need to shut this off since I can talk directly to my bunker, but he can never directly see me
	void deactivateRedirectedContain();
  void parseInitialPayload( INI* ini, void *instance, void *store, const void* /*userData*/ );

	Bool m_redirectionActivated;

};
