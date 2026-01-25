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

// FILE: Module.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2001
// Desc:	 Object and drawable modules and actions.  These are simply just class
//				 instances that we can assign to objects, drawables, and things to contain
//				 data and code for specific events, or just to hold data
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "Common/GameMemory.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Snapshot.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
enum TimeOfDay CPP_11(: Int);
enum StaticGameLODLevel CPP_11(: Int);
class Drawable;
class Object;
class Player;
class Thing;
class W3DModelDrawModuleData;	// ugh, hack (srj)
struct FieldParse;

// TYPES //////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
enum ModuleType CPP_11(: Int)
{
	MODULETYPE_BEHAVIOR = 0,

	//
	// drawable module types - you should *NOT* remove drawable module types, we write
	// modules into save game files in buckets of module types ... if you do remove one
	// here you will have to update the xfer code for a drawable.
	//
	// ALSO note that new drawable module types should go at the end of the existing drawable modules
	//
	MODULETYPE_DRAW = 1,
	MODULETYPE_CLIENT_UPDATE = 2,
	// put new drawable module types here

	NUM_MODULE_TYPES,

	FIRST_DRAWABLE_MODULE_TYPE = MODULETYPE_DRAW,
	LAST_DRAWABLE_MODULE_TYPE = MODULETYPE_CLIENT_UPDATE,
	NUM_DRAWABLE_MODULE_TYPES = (LAST_DRAWABLE_MODULE_TYPE - FIRST_DRAWABLE_MODULE_TYPE + 1)

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
enum ModuleInterfaceType CPP_11(: Int)
{
	MODULEINTERFACE_UPDATE					= 0x00000001,
	MODULEINTERFACE_DIE							= 0x00000002,
	MODULEINTERFACE_DAMAGE					= 0x00000004,
	MODULEINTERFACE_CREATE					= 0x00000008,
	MODULEINTERFACE_COLLIDE					= 0x00000010,
	MODULEINTERFACE_BODY						= 0x00000020,
	MODULEINTERFACE_CONTAIN					= 0x00000040,
	MODULEINTERFACE_UPGRADE					= 0x00000080,
	MODULEINTERFACE_SPECIAL_POWER		= 0x00000100,
	MODULEINTERFACE_DESTROY					= 0x00000200,
	MODULEINTERFACE_DRAW						= 0x00000400,
	MODULEINTERFACE_CLIENT_UPDATE		= 0x00000800
};

//-------------------------------------------------------------------------------------------------
/** Base class for data-read-from-INI for modules. */
//-------------------------------------------------------------------------------------------------
/// @todo srj -- make ModuleData be MemoryPool based
class ModuleData : public Snapshot
{
public:
	ModuleData() { }
	virtual ~ModuleData() { }

	void setModuleTagNameKey( NameKeyType key ) { m_moduleTagNameKey = key; }
	NameKeyType getModuleTagNameKey() const { return m_moduleTagNameKey; }

	virtual Bool isAiModuleData() const { return false; }

	// ugh, hack
	virtual const W3DModelDrawModuleData* getAsW3DModelDrawModuleData() const { return nullptr; }
	virtual StaticGameLODLevel getMinimumRequiredGameLOD() const { return (StaticGameLODLevel)0;}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
		// nothing
	}

public:
	virtual void crc( Xfer *xfer ) {}
	virtual void xfer( Xfer *xfer ) {}
	virtual void loadPostProcess( void ) {}

private:
	NameKeyType m_moduleTagNameKey;		///< module tag key, unique among all modules for an object instance
};

//-------------------------------------------------------------------------------------------------
// This macro is to assist in the creation of new modules, contains the common
// things that all module definitions must have in order to work with
// the module creation factory
//-------------------------------------------------------------------------------------------------
#define MAKE_STANDARD_MODULE_MACRO( cls ) \
public: \
	static Module* friend_newModuleInstance( Thing *thing, const ModuleData* moduleData ) { return newInstance( cls )( thing, moduleData ); } \
	virtual NameKeyType getModuleNameKey() const { static NameKeyType nk = NAMEKEY(#cls); return nk; } \
protected: \
	virtual void crc( Xfer *xfer ); \
	virtual void xfer( Xfer *xfer ); \
	virtual void loadPostProcess( void );

// ------------------------------------------------------------------------------------------------
// For the creation of abstract module classes
// ------------------------------------------------------------------------------------------------
#define MAKE_STANDARD_MODULE_MACRO_ABC( cls ) \
protected: \
	virtual void crc( Xfer *xfer ); \
	virtual void xfer( Xfer *xfer ); \
	virtual void loadPostProcess( void );

//-------------------------------------------------------------------------------------------------
// only use this macro for an ABC. for a real class, use MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA.
#define MAKE_STANDARD_MODULE_DATA_MACRO_ABC( cls, clsmd ) \
private: \
	const clsmd* get##clsmd() const { return (clsmd*)getModuleData(); } \
public: \
	static ModuleData* friend_newModuleData(INI* ini) \
	{ \
		clsmd* data = MSGNEW( "AllModuleData" ) clsmd; \
		if (ini) ini->initFromINIMultiProc(data, clsmd::buildFieldParse); \
		return data; \
	}

//-------------------------------------------------------------------------------------------------
#define MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( cls, clsmd ) \
	MAKE_STANDARD_MODULE_MACRO(cls) \
	MAKE_STANDARD_MODULE_DATA_MACRO_ABC(cls, clsmd)

//-------------------------------------------------------------------------------------------------
/** Common interface for thing modules, we want a single common base class
	* for all the modules (either object or drawable) so that we can use
	* a single module factory to handle instancing them ... it's just
	* convenient this way */
//-------------------------------------------------------------------------------------------------
class Module : public MemoryPoolObject,
							 public Snapshot
{

	MEMORY_POOL_GLUE_ABC( Module )						///< this abstract class needs memory pool hooks

public:

	Module(const ModuleData* moduleData) : m_moduleData(moduleData) { }
	// virtual destructor prototype defined by MemoryPoolObject

	// this method should NEVER be overridden by user code, only via the MAKE_STANDARD_MODULE_xxx macros!
	// it should also NEVER be called directly; it's only for use by ModuleFactory!
	static ModuleData* friend_newModuleData(INI* ini);

	virtual NameKeyType getModuleNameKey() const = 0;

	NameKeyType getModuleTagNameKey() const { return getModuleData()->getModuleTagNameKey(); }

	/** this is called after all the Modules for a given Thing are created; it
		allows Modules to resolve any inter-Module dependencies.
	*/
	virtual void onObjectCreated() { }

	/**
		this is called whenever a drawable is bound to the object.
		drawable is NOT guaranteed to be non-null.
	*/
	virtual void onDrawableBoundToObject() { }

	/// preload any assets we might have for this time of day
	virtual void preloadAssets( TimeOfDay timeOfDay ) { }

	/** onDelete() will be called on all modules contained by an object or drawable before
	the actual deletion of each of those modules happens */
	virtual void onDelete( void ) { }

protected:

	const ModuleData* getModuleData() const { return m_moduleData; }

	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

private:
	const ModuleData* m_moduleData;

};
//-------------------------------------------------------------------------------------------------


//=================================================================================================
//														OBJECT Module interface and modules
//=================================================================================================


//-------------------------------------------------------------------------------------------------
/** Module interface specific for Objects, this is really just to make a clear distinction
	* between modules intended for use in objects and modules intended for use
	* in drawables */
//-------------------------------------------------------------------------------------------------
class ObjectModule : public Module
{

	MEMORY_POOL_GLUE_ABC( ObjectModule )			///< this abstract class needs memory pool hooks

public:

	ObjectModule( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	virtual void onCapture( Player *oldOwner, Player *newOwner ) { }

protected:

	Object *getObject() { return m_object; }
	const Object *getObject() const { return m_object; }

	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

private:

	// it shouldn't be legal for subclasses to ever modify this, only to look at it;
	// so, we'll enforce this by making it private and providing a protected access method.
	Object *m_object;													///< the object this module is a part of

};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------






//=================================================================================================
//													DRAWABLE module interface and modules
//=================================================================================================

//-------------------------------------------------------------------------------------------------
/** Module interface specific for Drawables, this is really just to make a clear distinction
	* between modules intended for use in objects and modules intended for use
	* in drawables */
//-------------------------------------------------------------------------------------------------
class DrawableModule : public Module
{

	MEMORY_POOL_GLUE_ABC( DrawableModule )		///< this abstract class needs memory pool hooks

public:

	DrawableModule( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

protected:

	Drawable *getDrawable() { return m_drawable; }
	const Drawable *getDrawable() const { return m_drawable; }

	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

private:

	// it shouldn't be legal for subclasses to ever modify this, only to look at it;
	// so, we'll enforce this by making it private and providing a protected access method.
	Drawable *m_drawable;											///< the drawble this module is a part of

};
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
/** VARIOUS MODULE INTERFACES */
//-------------------------------------------------------------------------------------------------
