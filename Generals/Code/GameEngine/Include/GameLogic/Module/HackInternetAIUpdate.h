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

// HackInternetAIUpdate.h ////////////
// Author: Kris Morness, June 2002
// Desc:   State machine that allows hacking cash from the internet (free money).

#pragma once

#include "Common/StateMachine.h"
#include "GameLogic/Module/AIUpdate.h"

//-------------------------------------------------------------------------------------------------
class HackInternetStateMachine : public AIStateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( HackInternetStateMachine, "HackInternetStateMachine" );
public:
	HackInternetStateMachine( Object *owner, AsciiString name );
};

//-------------------------------------------------------------------------------------------------
class HackInternetState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(HackInternetState, "HackInternetState")
public:
	HackInternetState( StateMachine *machine ) :State( machine, "HackInternetState" )
	{
		//Added By Sadullah Nader
		//Initializations missing and needed
		m_framesRemaining = 0;
		//
	}
	virtual StateReturnType update();
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	UnsignedInt m_framesRemaining; //frames till next cash update
};
EMPTY_DTOR(HackInternetState)

//-------------------------------------------------------------------------------------------------
class PackingState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(PackingState, "PackingState")
public:
	PackingState( StateMachine *machine ) : State( machine, "PackingState" )
	{
		//Added By Sadullah Nader
		//Initializations inserted
		m_framesRemaining = 0;
		//
	}
	virtual StateReturnType update();
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	UnsignedInt m_framesRemaining; //frames till packing animation complete
};
EMPTY_DTOR(PackingState)

//-------------------------------------------------------------------------------------------------
class UnpackingState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(UnpackingState, "UnpackingState")
public:
	UnpackingState( StateMachine *machine ) :State( machine, "UnpackingState" )
	{
		m_framesRemaining = 0;
	}
	virtual StateReturnType update();
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	UnsignedInt m_framesRemaining; //frames till unpacking animation complete
};
EMPTY_DTOR(UnpackingState)


//-------------------------------------------------------------------------------------------------
enum
{
	UNPACKING = 1000,						///< Kneel down and set up internet satellite link
	HACK_INTERNET,							///< Once set up, start hacking for cash.
	PACKING,						///< Pack up before moving.
};

//-------------------------------------------------------------------------------------------------
class HackInternetAIUpdateModuleData : public AIUpdateModuleData
{
public:
	UnsignedInt		m_unpackTime;
	UnsignedInt		m_packTime;
	UnsignedInt		m_cashUpdateDelay;
	UnsignedInt		m_regularCashAmount;
	UnsignedInt		m_veteranCashAmount;
	UnsignedInt		m_eliteCashAmount;
	UnsignedInt		m_heroicCashAmount;
	UnsignedInt		m_xpPerCashUpdate;
	Real					m_packUnpackVariationFactor;

	HackInternetAIUpdateModuleData()
	{
		m_unpackTime = 0;
		m_packTime = 0;
		m_cashUpdateDelay = 0;
		m_regularCashAmount = 0;
		m_veteranCashAmount = 0;
		m_eliteCashAmount = 0;
		m_heroicCashAmount = 0;
		m_xpPerCashUpdate = 0;
		m_packUnpackVariationFactor = 0.0f;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    AIUpdateModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "UnpackTime",					INI::parseDurationUnsignedInt,	NULL, offsetof( HackInternetAIUpdateModuleData, m_unpackTime ) },
			{ "PackTime",						INI::parseDurationUnsignedInt,	NULL, offsetof( HackInternetAIUpdateModuleData, m_packTime ) },
			{ "PackUnpackVariationFactor", INI::parseReal,					NULL, offsetof( HackInternetAIUpdateModuleData, m_packUnpackVariationFactor ) },
			{ "CashUpdateDelay",		INI::parseDurationUnsignedInt,	NULL, offsetof( HackInternetAIUpdateModuleData, m_cashUpdateDelay ) },
			{ "RegularCashAmount",	INI::parseUnsignedInt,	NULL, offsetof( HackInternetAIUpdateModuleData, m_regularCashAmount ) },
			{ "VeteranCashAmount",	INI::parseUnsignedInt,	NULL, offsetof( HackInternetAIUpdateModuleData, m_veteranCashAmount ) },
			{ "EliteCashAmount",		INI::parseUnsignedInt,	NULL, offsetof( HackInternetAIUpdateModuleData, m_eliteCashAmount ) },
			{ "HeroicCashAmount",		INI::parseUnsignedInt,	NULL, offsetof( HackInternetAIUpdateModuleData, m_heroicCashAmount ) },
			{ "XpPerCashUpdate",		INI::parseUnsignedInt,	NULL, offsetof( HackInternetAIUpdateModuleData, m_xpPerCashUpdate ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);

	}
};

class HackInternetAIInterface
{
public:
	virtual Bool isHacking() const = 0;
	virtual Bool isHackingPackingOrUnpacking() const = 0;
};

//-------------------------------------------------------------------------------------------------
class HackInternetAIUpdate : public AIUpdateInterface, public HackInternetAIInterface
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( HackInternetAIUpdate, "HackInternetAIUpdate"  )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( HackInternetAIUpdate, HackInternetAIUpdateModuleData )

private:

public:
	HackInternetAIUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

 	virtual void aiDoCommand(const AICommandParms* parms);

	Real getPackUnpackVariationFactor() const { return getHackInternetAIUpdateModuleData()->m_packUnpackVariationFactor; }
	UnsignedInt getUnpackTime()					const;
	UnsignedInt getPackTime()						const;
	UnsignedInt getCashUpdateDelay()		const;
	UnsignedInt getRegularCashAmount()	const { return getHackInternetAIUpdateModuleData()->m_regularCashAmount; }
	UnsignedInt getVeteranCashAmount()	const { return getHackInternetAIUpdateModuleData()->m_veteranCashAmount; }
	UnsignedInt getEliteCashAmount()		const { return getHackInternetAIUpdateModuleData()->m_eliteCashAmount; }
	UnsignedInt getHeroicCashAmount()		const { return getHackInternetAIUpdateModuleData()->m_heroicCashAmount; }
	UnsignedInt getXpPerCashUpdate()		const { return getHackInternetAIUpdateModuleData()->m_xpPerCashUpdate; }

	void hackInternet();
	virtual UpdateSleepTime update();

	virtual Bool isIdle() const;

	virtual HackInternetAIInterface* getHackInternetAIInterface() { return this; }
	virtual const HackInternetAIInterface* getHackInternetAIInterface() const { return this; }

	virtual Bool isHacking() const;
	virtual Bool isHackingPackingOrUnpacking() const;

protected:

	virtual AIStateMachine* makeStateMachine();

	AICommandParmsStorage		m_pendingCommand;
	Bool m_hasPendingCommand;
};
