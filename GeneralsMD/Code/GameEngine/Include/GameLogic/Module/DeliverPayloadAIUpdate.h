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

// DeliverPayloadAIUpdate.h ////////////
// Author: Graham Smallwood, March 2002
// Desc:   State machine that controls the approach and deployment of airborne cargo

#pragma once

#include "Common/StateMachine.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameClient/RadiusDecal.h"

class DeliverPayloadData;

//-------------------------------------------------------------------------------------------------
class DeliverPayloadStateMachine : public StateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DeliverPayloadStateMachine, "DeliverPayloadStateMachine" );
public:
	DeliverPayloadStateMachine( Object *owner );

	static Bool isOffMap( State *thisState, void* userData );

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};

//-------------------------------------------------------------------------------------------------
class ApproachState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ApproachState, "ApproachState")
	//Approaching the drop zone
public:
	ApproachState( StateMachine *machine ) :State( machine, "ApproachState" ) {}
	virtual StateReturnType update();
	virtual StateReturnType onEnter();
protected:
	// snapshot interface	STUBBED - no member vars to save. jba.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(ApproachState)

//-------------------------------------------------------------------------------------------------
class DeliveringState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(DeliveringState, "DeliveringState")
	// Kickin' stuff out the door
public:
	DeliveringState( StateMachine *machine ) :State( machine, "DeliveringState" )
	{
		m_dropDelayLeft = 0;
		m_didOpen = false;
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
	UnsignedInt m_dropDelayLeft;
	Bool m_didOpen;
};
EMPTY_DTOR(DeliveringState)

//-------------------------------------------------------------------------------------------------
class ConsiderNewApproachState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ConsiderNewApproachState, "ConsiderNewApproachState")
	//Should I try again?  Has own data to keep track.
public:
	ConsiderNewApproachState( StateMachine *machine ) : State( machine, "ConsiderNewApproachState" ), m_numberEntriesToState(0) { }
	virtual StateReturnType update();
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	Int m_numberEntriesToState;
};
EMPTY_DTOR(ConsiderNewApproachState)

//-------------------------------------------------------------------------------------------------
class RecoverFromOffMapState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(RecoverFromOffMapState, "RecoverFromOffMapState")
public:
	RecoverFromOffMapState( StateMachine *machine ) : State( machine, "RecoverFromOffMapState" ), m_reEntryFrame(0) { }
	virtual StateReturnType update();
	virtual StateReturnType onEnter();
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	UnsignedInt m_reEntryFrame;
};
EMPTY_DTOR(RecoverFromOffMapState)

//-------------------------------------------------------------------------------------------------
class HeadOffMapState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(HeadOffMapState, "HeadOffMapState")
	//I'm outta here
public:
  HeadOffMapState( StateMachine *machine ) :State( machine, "HeadOffMapState" ) { facingDirectionUponDelivery.zero(); }
	virtual StateReturnType update();
	virtual StateReturnType onEnter();
protected:
	// snapshot interface	STUBBED - no member vars to save. jba.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};

  Coord3D facingDirectionUponDelivery;
};
EMPTY_DTOR(HeadOffMapState)

//-------------------------------------------------------------------------------------------------
class CleanUpState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(CleanUpState, "CleanUpState")
	//Made it off map, delete ourselves
public:
	CleanUpState( StateMachine *machine ) :State( machine, "CleanUpState" ) {}
	virtual StateReturnType update(){return STATE_CONTINUE;}
	virtual StateReturnType onEnter();
protected:
	// snapshot interface	STUBBED - no member vars to save. jba.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(CleanUpState)


//-------------------------------------------------------------------------------------------------
enum
{
	APPROACH,								///< Flying towards target
	DELIVERING,							///< Delivering the payload to the target
	CONSIDER_NEW_APPROACH,	///< Deciding if I should reapproach to deliver more payload or go home
	RECOVER_FROM_OFF_MAP,		///< oops, went off the map, special recovery needed
	HEAD_OFF_MAP,						///< We're all done here, take off into the sunset
	CLEAN_UP,								///< Made it to the sunset. Delete peacefully, don't kill
};

//-------------------------------------------------------------------------------------------------
class DeliverPayloadAIUpdateModuleData : public AIUpdateModuleData
{
public:
	UnsignedInt		m_doorDelay;
	Real					m_maxDistanceToTarget;				///< How far away from target I can unload, plus how far after target I need to turn around at
	Int						m_maxNumberAttempts;					///< How many times I can re-approach
	UnsignedInt		m_dropDelay;									///< How long to wait after entering Deliver state (to allow for doors opening)
	Coord3D				m_dropOffset;									///< where to disgorge the guys, relative to me
	Coord3D				m_dropVariance;								///< variance in dropping position among guys that I am dropping
	AsciiString		m_putInContainerName;
	RadiusDecalTemplate	m_deliveryDecalTemplate;
	Real					m_deliveryDecalRadius;

	DeliverPayloadAIUpdateModuleData()
	{
		m_doorDelay = 0;
		m_maxDistanceToTarget = 0.0f;
		m_maxNumberAttempts = 0;
		m_dropDelay = 0;
		m_dropOffset.zero();
		m_dropVariance.zero();
		m_deliveryDecalRadius = 0;
		m_putInContainerName.clear();
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    AIUpdateModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			//These values represent script only reinforcements using deliverPayloadViaModuleData()!
			//***********************************************************************************
			//DO NOT ADD DATA HERE UNLESS YOU ARE SUPPORTING SCRIPTED TEAM REINFORCEMENT DELIVERY
			//THESE DATA VALUES ARE SPECIFIED ONLY BY FACTIONUNIT.INI
			//***********************************************************************************
			{ "DoorDelay",								INI::parseDurationUnsignedInt,	NULL, offsetof( DeliverPayloadAIUpdateModuleData, m_doorDelay ) },
			{ "PutInContainer",						INI::parseAsciiString,					NULL, offsetof( DeliverPayloadAIUpdateModuleData, m_putInContainerName ) },
			{ "DeliveryDistance",					INI::parseReal,									NULL, offsetof( DeliverPayloadAIUpdateModuleData, m_maxDistanceToTarget ) },
			{ "MaxAttempts",							INI::parseInt,									NULL, offsetof( DeliverPayloadAIUpdateModuleData, m_maxNumberAttempts ) },
			{ "DropDelay",								INI::parseDurationUnsignedInt,	NULL, offsetof( DeliverPayloadAIUpdateModuleData, m_dropDelay ) },
			{ "DropOffset",								INI::parseCoord3D,							NULL, offsetof( DeliverPayloadAIUpdateModuleData, m_dropOffset ) },
			{ "DropVariance",							INI::parseCoord3D,							NULL, offsetof( DeliverPayloadAIUpdateModuleData, m_dropVariance ) },
			{ "DeliveryDecal",						RadiusDecalTemplate::parseRadiusDecalTemplate,	NULL, offsetof( DeliverPayloadAIUpdateModuleData, m_deliveryDecalTemplate ) },
			{ "DeliveryDecalRadius",			INI::parseReal,									NULL,	offsetof( DeliverPayloadAIUpdateModuleData, m_deliveryDecalRadius ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);

	}
};

//This data is set by DeliverPayloadNugget in ObjectCreationList. This struct contains
//all the necessary information for DeliverPayloadAIUpdate to perform it's necessary
//functions. If you add something here, make sure you add the parsing function inside
//DeliverPayloadNugget.
//***********************************************************************************
//THESE DATA VALUES ARE SPECIFIED ONLY BY OBJECTCREATIONLIST.INI
//***********************************************************************************
class DeliverPayloadData
{
public:
	AsciiString						m_visibleDropBoneName;				///< Where the payload is created (offset by current bone number 01-xx)
	AsciiString						m_visibleSubObjectName;				///< Visible subobject to show or hide (offset by current drop number 01-xx)
	AsciiString						m_visiblePayloadTemplateName;
  Real                  m_distToTarget;
	Real									m_preOpenDistance;
	Int										m_maxAttempts;
	Coord3D								m_dropOffset;
	Coord3D								m_dropVariance;
	UnsignedInt						m_dropDelay;
	Bool									m_fireWeapon;
	Bool									m_selfDestructObject;
	Int										m_visibleNumBones;						///< The number of visible bones to process.
	Real									m_diveStartDistance;
	Real									m_diveEndDistance;
	WeaponSlotType				m_strafingWeaponSlot;
	Int										m_visibleItemsDroppedPerInterval;
	Bool									m_inheritTransportVelocity;
	Bool									m_isParachuteDirectly;		///< Instead of parachuting to below the point of exit, go ahead and bunch up on the target
	Real									m_exitPitchRate;
	const FXList					*m_strafeFX;
	Real									m_strafeLength;
	const WeaponTemplate	*m_visiblePayloadWeaponTemplate;
	RadiusDecalTemplate		m_deliveryDecalTemplate;
	Real									m_deliveryDecalRadius;

	DeliverPayloadData()
	{
		m_distToTarget = 0.0f;
		m_preOpenDistance = 0.0f;
		m_maxAttempts = 1;
		m_dropOffset.zero();
		m_dropVariance.zero();
		m_dropDelay = 0;
		m_fireWeapon = false;
		m_visibleNumBones = 0;
		m_diveStartDistance = 0.0f;
		m_diveEndDistance = 0.0f;
		m_strafingWeaponSlot = (WeaponSlotType)-1;
		m_visibleItemsDroppedPerInterval = 0;
		m_inheritTransportVelocity = false;
		m_isParachuteDirectly = FALSE;
		m_exitPitchRate = 0.0f;
		m_strafeFX = NULL;
		m_strafeLength = 0.0f;
		m_visiblePayloadWeaponTemplate = NULL;
		m_selfDestructObject = FALSE;
		m_deliveryDecalRadius = 0;
		m_visibleDropBoneName.clear();
		m_visiblePayloadTemplateName.clear();
		m_visibleSubObjectName.clear();
	}

	static const FieldParse* getFieldParse();
};

//-------------------------------------------------------------------------------------------------
class DeliverPayloadAIUpdate : public AIUpdateInterface
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DeliverPayloadAIUpdate, "DeliverPayloadAIUpdate"  )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( DeliverPayloadAIUpdate, DeliverPayloadAIUpdateModuleData )

private:

public:
	DeliverPayloadAIUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual AIFreeToExitType getAiFreeToExit(const Object* exiter) const;

	const Coord3D* getTargetPos() const { return &m_targetPos; }
	const Coord3D* getMoveToPos() const { return &m_moveToPos; }
	UnsignedInt getDoorDelay() const { return getDeliverPayloadAIUpdateModuleData()->m_doorDelay; }
	Bool isDeliveringPayload() const { return m_deliverPayloadStateMachine != NULL; }
	const ThingTemplate* getPutInContainerTemplateViaModuleData() const;

	Real getAllowedDistanceToTarget() const { return m_data.m_distToTarget; }
	Real getPreOpenDistance() const { return m_data.m_preOpenDistance; }
	Int getMaxNumberAttempts() const { return m_data.m_maxAttempts; }
	UnsignedInt getDropDelay() const { return m_data.m_dropDelay; }
	const Coord3D& getDropOffset() const { return m_data.m_dropOffset; }
	const Coord3D& getDropVariance() const { return m_data.m_dropVariance; }
	Bool isFireWeapon() const { return m_data.m_fireWeapon; }
	Int getVisibleItemsDelivered() const { return m_visibleItemsDelivered; }
	void setVisibleItemsDelivered( Int num ) { m_visibleItemsDelivered = num; }

	Bool isCloseEnoughToTarget();
	Bool isOffMap() const;
	Real calcMinTurnRadius(Real* timeToTravelThatDist) const;

	void deliverPayload( const Coord3D *moveToPos, const Coord3D *targetPos, const DeliverPayloadData *data );
	void deliverPayloadViaModuleData( const Coord3D *moveToPos );

	const DeliverPayloadData* getData() { return &m_data; }

	virtual UpdateSleepTime update();

	void killDeliveryDecal();

	void friend_setFreeToExit(Bool f) { m_freeToExit = f; }
	void friend_setAcceptingCommands(Bool f) { m_acceptingCommands = f; }

protected:

	virtual AIStateMachine* makeStateMachine();
	virtual Bool isAllowedToRespondToAiCommands(const AICommandParms* parms) const;

	DeliverPayloadStateMachine*		m_deliverPayloadStateMachine;	///< Controls my special logic
	Coord3D												m_targetPos;									///< Where I plan to deliver my little friends, if obj is null
	Coord3D												m_moveToPos;									///< Where I am moving to.
	DeliverPayloadData						m_data;
	Int														m_visibleItemsDelivered;
	RadiusDecal										m_deliveryDecal;
	Real													m_previousDistanceSqr;
	Bool													m_freeToExit;
	Bool													m_acceptingCommands;

	enum DiveState	// Stored in save file as int, don't renumber!  jba.
	{
		DIVESTATE_PREDIVE=0,
		DIVESTATE_DIVING=1,
		DIVESTATE_POSTDIVE=2,
	};
	DiveState											m_diveState;



};
