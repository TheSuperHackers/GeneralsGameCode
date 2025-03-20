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

// FILE: W3DGhostObject.cpp /////////////////////////////////////////////////////
//
// Placeholder for objects that have been deleted but need to be maintained because
// a player can see them fogged.
//
// Author: Mark Wilczynski, August 2002
///////////////////////////////////////////////////////////////////////////////

#include "Common/Debug.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameLogic/W3DGhostObject.h"
#include "WW3D2/rendobj.h"
#include "WW3D2/hlod.h"
#include "WW3D2/scene.h"
#include "WW3D2/matinfo.h"


#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

/**This class will hold all information about a W3D RenderObject needed to
reconstruct it if necessary*/
class W3DRenderObjectSnapshot : public Snapshot
{
	friend W3DGhostObject;

	W3DRenderObjectSnapshot(RenderObjClass *m_parentRobj, DrawableInfo *drawInfo, Bool cloneParentRobj = TRUE);
	~W3DRenderObjectSnapshot() {REF_PTR_RELEASE(m_robj);}
	inline void update(RenderObjClass *robj, DrawableInfo *drawInfo, Bool cloneParentRobj=TRUE);	///<refresh the current snapshot with latest state
	inline void addToScene(void);	///< add this fogged renderobject to the scene.
protected:

	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

#ifdef DEBUG_FOG_MEMORY
	const char *m_robjName;		///<debug pointer so we know what this is a snapshot of.
#endif
	RenderObjClass *m_robj;		///<render object representing state at time of snapshot
	W3DRenderObjectSnapshot *m_next;	///<snapshot of next render object belonging to same drawable
};

//Dummy material override which we assign to all ghost objects to disable their
//texture animation.
static RenderObjClass::Material_Override animationDisableOverride;

//Helper function used to disable all UV mapper animations on a given model.
//Also use this pass to disable muzzle effects.
void disableUVAnimations(RenderObjClass *robj)
{
	if (robj && robj->Class_ID() == RenderObjClass::CLASSID_HLOD)
	{
		//Also disable any animations that may be playing using mappers (texture scrolling)
		for (Int i=0; i < robj->Get_Num_Sub_Objects(); i++)
		{
			RenderObjClass *subObj=robj->Get_Sub_Object(i);
			if (subObj && subObj->Class_ID() == RenderObjClass::CLASSID_MESH)
			{	//check if sub-object has the correct material to do texture scrolling.
				MaterialInfoClass *mat=subObj->Get_Material_Info();
				if (mat)
				{	for (Int j=0; j<mat->Vertex_Material_Count(); j++)
					{
						VertexMaterialClass *vmaterial=mat->Peek_Vertex_Material(j);
						LinearOffsetTextureMapperClass *mapper=(LinearOffsetTextureMapperClass *)vmaterial->Peek_Mapper();
						if (mapper && mapper->Mapper_ID() == TextureMapperClass::MAPPER_ID_LINEAR_OFFSET)
						{	
							subObj->Set_User_Data(&animationDisableOverride);	//tell W3D about custom material settings
						}
					}
					REF_PTR_RELEASE(mat);
				}
				//We don't want muzzle flashes visible inside fog, so turn them off.
				if (subObj->Get_Name() && strstr(subObj->Get_Name(),"MUZZLEFX"))
					subObj->Set_Hidden(true);
			}
			REF_PTR_RELEASE(subObj);
		}
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DRenderObjectSnapshot::update(RenderObjClass *robj, DrawableInfo *drawInfo,
																		 Bool cloneParentRobj)
{
	REF_PTR_RELEASE(m_robj);

	if( cloneParentRobj == TRUE )
	{

		m_robj=robj->Clone();
		m_robj->Set_ObjectColor(robj->Get_ObjectColor());
#ifdef DEBUG_FOG_MEMORY
		m_robjName = m_robj->Get_Name();
#endif
		//Set cloned object to same state as original object.
		m_robj->Set_Transform(robj->Get_Transform());
		if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD)
		{	
			float frame,mult;
			int mode,numFrames;

			HAnimClass *hanim=((HLodClass *)robj)->Peek_Animation_And_Info(frame,numFrames,mode,mult);
			m_robj->Set_Animation(hanim,frame);
			disableUVAnimations(m_robj);
		}	//HLOD
	}
	else
		m_robj=robj;

	m_robj->Set_User_Data(drawInfo);

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DRenderObjectSnapshot::addToScene(void)
{
	((SimpleSceneClass *)W3DDisplay::m_3DScene)->Add_Render_Object(m_robj);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
W3DRenderObjectSnapshot::W3DRenderObjectSnapshot(RenderObjClass *robj, DrawableInfo *drawInfo,
																								 Bool cloneParentRobj)
{
	m_robj=NULL;
	m_next=NULL;
	update(robj, drawInfo, cloneParentRobj);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DRenderObjectSnapshot::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DRenderObjectSnapshot::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// sanity
	DEBUG_ASSERTCRASH( m_robj, ("W3DRenderObjectSnapshot::xfer - invalid m_robj\n") );

	// transform on the main render object
	Matrix3D transform;
	transform = m_robj->Get_Transform();
	xfer->xferUser( &transform, sizeof( Matrix3D ) );
	if( xfer->getXferMode() == XFER_LOAD )
		m_robj->Set_Transform( transform );

	// how many sub objects of data will follow
	Int subObjectCount = m_robj->Get_Num_Sub_Objects();
	xfer->xferInt( &subObjectCount );

	Bool visible;
	RenderObjClass *subObject;
	AsciiString subObjectName;
	for( Int i = 0; i < subObjectCount; ++i )
	{

		//
		// when saving we get sub objects by index and xfer their name, when loading
		// we read the name and find that sub object
		//
		if( xfer->getXferMode() == XFER_SAVE )
		{

			// get sub object
			subObject = m_robj->Get_Sub_Object( i );

			// xfer sub object name which is unique among those in this render object
			subObjectName.set( subObject->Get_Name() );
			xfer->xferAsciiString( &subObjectName );

		}  // end if, save
		else
		{

			// read sub object name
			xfer->xferAsciiString( &subObjectName );

			// find this sub object on the object
			subObject = m_robj->Get_Sub_Object_By_Name( subObjectName.str() );

		}  // end else load

		//
		// NOTE that the remainder of this xfer code works on a sub object only *if*
		// it is present.  It is possible that in future patches we change the artwork
		// for some objects which could remove sub objects for which we have data saved
		// for in the save game file.  If we encounter data in the save file for 
		// sub objects that are no longer in the artwork we just read the data and
		// throw it away
		//

		// visible/hidden status of this sub object
		if( subObject )
			visible = subObject->Is_Not_Hidden_At_All();
		xfer->xferBool( &visible );
		if( subObject && xfer->getXferMode() == XFER_LOAD )
			subObject->Set_Hidden( !visible );

		// transform of this sub object
		if( subObject )
			transform = subObject->Get_Transform();
		xfer->xferUser( &transform, sizeof( Matrix3D ) );
		if( subObject && xfer->getXferMode() == XFER_LOAD )
			subObject->Set_Transform( transform );

		// need to tell W3D that this sub object transforms are ok
		if( subObject )
		{

			// need to cast to HLod if we can to validate the hierarchy
			if( subObject->Class_ID() == RenderObjClass::CLASSID_HLOD )
				((HLodClass *)subObject)->Friend_Set_Hierarchy_Valid( TRUE );

		}  // end if

		// release reference to sub object
		if( subObject )
			REF_PTR_RELEASE( subObject );

	}  // end for, i

	// tell W3D that the transforms for our sub objects are all OK cause we've done them ourselves
	m_robj->Set_Sub_Object_Transforms_Dirty( FALSE );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DRenderObjectSnapshot::loadPostProcess( void )
{

}  // end loadPostProcess

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
W3DGhostObject::W3DGhostObject()
{

	for (Int i=0; i< MAX_PLAYER_COUNT; i++) 
		m_parentSnapshots[i]=NULL;

	m_drawableInfo.m_drawable = NULL;
	m_drawableInfo.m_flags = 0;
	m_drawableInfo.m_ghostObject = NULL;
	m_drawableInfo.m_shroudStatusObjectID = INVALID_ID;

	m_nextSystem = NULL;
	m_prevSystem = NULL;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
W3DGhostObject::~W3DGhostObject()
{
#ifdef DEBUG_FOG_MEMORY
	for (Int i=0; i<MAX_PLAYER_COUNT; i++)
	{
		DEBUG_ASSERTCRASH(m_parentSnapshots[i] == NULL, ("Delete of non-empty GhostObject"));
	}
#else
	DEBUG_ASSERTCRASH(m_parentSnapshots[TheGhostObjectManager->getLocalPlayerIndex()] == NULL, ("Delete of non-empty GhostObject"));
#endif
}

// ------------------------------------------------------------------------------------------------
/** Record the current state of the renderobjects used by this parent object
so we can display cached state when player is looking at fogged object.
Should only be called when object enters the fogged state.*/
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::snapShot(int playerIndex)
{
#ifndef DEBUG_FOG_MEMORY
	if (playerIndex != TheGhostObjectManager->getLocalPlayerIndex())
		return;	//we only snapshot things for the initial local player because local player can't change in non-debug game.
#endif

	Drawable *draw=m_parentObject->getDrawable();
	if (draw->isDrawableEffectivelyHidden())
		return;	//don't bother to snapshot things which nobody can see.
	
	W3DRenderObjectSnapshot *snap=m_parentSnapshots[playerIndex],*prevSnap=NULL;

	//walk through all W3D render objects used by this object
	for (DrawModule ** dm = draw->getDrawModules(); *dm; ++dm)
	{
		const ObjectDrawInterface* di = (*dm)->getObjectDrawInterface();
		if (di)
		{
			W3DModelDraw *w3dDraw= (W3DModelDraw *)di;
			RenderObjClass *robj=NULL;

			robj=w3dDraw->getRenderObject();
			//robj may be null for modules which have no render objects such
			//as for build-ups that are currently disabled.
			if (robj)
			{
				if (snap == NULL)
				{	
					snap = NEW W3DRenderObjectSnapshot(robj, &m_drawableInfo);	// poolify
					if (prevSnap)
						prevSnap->m_next=snap;
					else
						m_parentSnapshots[playerIndex]=snap;
				}
				else
					m_parentSnapshots[playerIndex]->update(robj, &m_drawableInfo);

				//Adding and removing render objects to the scene is expensive
				//so only do it for the real player watching the screen.  There is
				//also no point in displaying the other player's ghost objects to
				//the current player.
				if (playerIndex == TheGhostObjectManager->getLocalPlayerIndex())
				{
					robj->Remove();	//remove normal object from scene
					snap->addToScene();
				}

				prevSnap=snap;
				snap = snap->m_next;
			}
		}
	}

	//Check if we captured at least one snapshot
	if (snap != m_parentSnapshots[playerIndex])
	{	//save off other info we may need in case the parent object is destroyed.
		///@todo: We're going to ignore the case where each player index could be
		//looking at a different geometry info/orientation because ghostobjects
		//are supposed to be used on immobile buildings.
		m_parentGeometryType=m_parentObject->getGeometryInfo().getGeomType();
		m_parentGeometryIsSmall=m_parentObject->getGeometryInfo().getIsSmall();
		m_parentGeometryMajorRadius=m_parentObject->getGeometryInfo().getMajorRadius();
		m_parentGeometryminorRadius=m_parentObject->getGeometryInfo().getMinorRadius();
		m_parentPosition=*m_parentObject->getPosition();
		m_parentAngle=m_parentObject->getOrientation();
	}
}

// ------------------------------------------------------------------------------------------------
/** Remove the original object from our 3D scene*/
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::removeParentObject(void)
{

	// sanity
	if( m_parentObject == NULL )
		return;

	Drawable *draw=m_parentObject->getDrawable();

	//After we remove the unfogged object, we also disable
	//anything that should be hidden inside fog - shadow, particles, etc.
	draw->setFullyObscuredByShroud(true);

	//walk through all W3D render objects used by this object
	for (DrawModule ** dm = draw->getDrawModules(); *dm; ++dm)
	{
		const ObjectDrawInterface* di = (*dm)->getObjectDrawInterface();
		if (di)
		{
			W3DModelDraw *w3dDraw= (W3DModelDraw *)di;
			RenderObjClass *robj=NULL;

			robj=w3dDraw->getRenderObject();
			if (robj)
			{
				DEBUG_ASSERTCRASH(robj->Peek_Scene() != NULL, ("Removing GhostObject parent not in scene "));
				robj->Remove();
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** Reinsert the original object into our 3D scene*/
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::restoreParentObject(void)
{
	Drawable *draw=m_parentObject->getDrawable();
	if (!draw)
		return;

	//Notify drawable that it's okay to render its render objects again.
	draw->setFullyObscuredByShroud(false);

	//walk through all W3D render objects used by this object
	for (DrawModule ** dm = draw->getDrawModules(); *dm; ++dm)
	{
		const ObjectDrawInterface* di = (*dm)->getObjectDrawInterface();
		if (di)
		{
			W3DModelDraw *w3dDraw= (W3DModelDraw *)di;
			RenderObjClass *robj=NULL;

			robj=w3dDraw->getRenderObject();
			//robj may be null for modules which have no render objects such
			//as for build-ups that are currently disabled.
			if (robj)
			{	//if we have a render object that's not in the scene, it must have been
				//removed by the ghost object manager, so restore it.  If we have a render
				//object that is in the scene, then it was probably added because the model
				//changed while the object was ghosted (for damage states, garrison, etc.).
				if (robj->Peek_Scene() == NULL)
					((SimpleSceneClass *)W3DDisplay::m_3DScene)->Add_Render_Object(robj);
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::freeAllSnapShots(void)
{
	Int playerIndex;

#ifdef DEBUG_FOG_MEMORY
	for (playerIndex=0; playerIndex<MAX_PLAYER_COUNT; playerIndex++)
#else
	playerIndex = TheGhostObjectManager->getLocalPlayerIndex();
#endif
		if (m_parentSnapshots[playerIndex])
		{	//if we have a snapshot for this object, remove it from
			//scene.
			removeFromScene(playerIndex);

			//Restore actual objects assuming they are still alive.
			if (m_parentObject)
				restoreParentObject();

			W3DRenderObjectSnapshot *snap=m_parentSnapshots[playerIndex];
			W3DRenderObjectSnapshot *nextSnap;
			while (snap)
			{	nextSnap = snap->m_next;
				delete snap;
				snap = nextSnap;
			}
			m_parentSnapshots[playerIndex]=NULL;
		}
}

// ------------------------------------------------------------------------------------------------
/** Player has unfogged the object so he no longer needs the snapshot*/
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::freeSnapShot(int playerIndex)
{
#ifndef DEBUG_FOG_MEMORY
	if (playerIndex != TheGhostObjectManager->getLocalPlayerIndex())
		return;	//we only snapshot things for the local player
#endif

	if (m_parentSnapshots[playerIndex])
	{	//if we have a snapshot for this object, remove it from
		//scene and put back the original object if it still exists.
		if (playerIndex == TheGhostObjectManager->getLocalPlayerIndex())
		{
			//Adding and removing render objects to the scene is expensive
			//so only do it for the real player watching the screen.  There is
			//also no point in displaying the other player's objects to
			//the current player.
			removeFromScene(playerIndex);

			//Restore actual objects assuming they are still alive.
			if (m_parentObject)
				restoreParentObject();
		}

		W3DRenderObjectSnapshot *snap=m_parentSnapshots[playerIndex];
		W3DRenderObjectSnapshot *nextSnap;
		while (snap)
		{	nextSnap = snap->m_next;
			delete snap;
			snap = nextSnap;
		}
		m_parentSnapshots[playerIndex]=NULL;
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::updateParentObject(Object *object, PartitionData *mod)
{
	m_parentObject = object;
	m_partitionData = mod;
}

// ------------------------------------------------------------------------------------------------
/**Remove the dummy render objects from scene that belong to given player*/
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::removeFromScene(int playerIndex)
{
	W3DRenderObjectSnapshot *snap=m_parentSnapshots[playerIndex];

	while (snap)
	{
		snap->m_robj->Remove();
		snap=snap->m_next;
	}
}

// ------------------------------------------------------------------------------------------------
/**Add the dummy render objects to scene so player sees the correct version within the fog*/
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::addToScene(int playerIndex)
{
	W3DRenderObjectSnapshot *snap=m_parentSnapshots[playerIndex];

	while (snap)
	{
		((SimpleSceneClass *)W3DDisplay::m_3DScene)->Add_Render_Object(snap->m_robj);
		snap=snap->m_next;
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::release(void)
{
	W3DRenderObjectSnapshot *snap,*nextSnap;

	for (Int i=0; i<MAX_PLAYER_COUNT; i++)
	{	snap = m_parentSnapshots[i];
		while (snap)
		{
			nextSnap=snap->m_next;
			delete snap;
			snap=nextSnap;
		}
		m_parentSnapshots[i]=NULL;
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::getShroudStatus(int playerIndex)
{
	m_partitionData->getShroudedStatus(playerIndex); 
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::crc( Xfer *xfer )
{

	// extend base class
	GhostObject::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	GhostObject::xfer( xfer );
	
	// xfer the drawable info object id
	xfer->xferObjectID( &m_drawableInfo.m_shroudStatusObjectID );

	// drawable info flags
	xfer->xferInt( &m_drawableInfo.m_flags );

	// drawable info drawable pointer
	DrawableID drawableID = m_drawableInfo.m_drawable ? m_drawableInfo.m_drawable->getID() : INVALID_DRAWABLE_ID;
	xfer->xferDrawableID( &drawableID );
	if( xfer->getXferMode() == XFER_LOAD )
	{

		// reconnect the drawable pointer
		m_drawableInfo.m_drawable = TheGameClient->findDrawableByID( drawableID );

		// sanity
		if( drawableID != INVALID_DRAWABLE_ID && m_drawableInfo.m_drawable == NULL )
			DEBUG_CRASH(( "W3DGhostObject::xfer - Unable to find drawable for ghost object\n" ));

	}  // end if

	//
	// no need to mess with this "circular" back into itself pointer to the ghost object
	// because it is already valid and assigned upon creation of this ghost object
	//
	// m_drawableInfo.m_ghostObject <--- no need to mess with this
	
	// xfer snapshot array
	UnsignedByte snapshotCount;
	for( Int i = 0; i < MAX_PLAYER_COUNT; ++i )
	{

		// count the snapshots at this index
		snapshotCount = 0;
		W3DRenderObjectSnapshot *objectSnapshot = m_parentSnapshots[ i ];
		while( objectSnapshot )
		{
		
			// increment count
			snapshotCount++;

			// on to the next snapshot
			objectSnapshot = objectSnapshot->m_next;

		}  // end while

		// xfer the snapshot count at this index
		xfer->xferUnsignedByte( &snapshotCount );

		//
		// sanity, this catches when we read from the file a count of zero, but our data
		// structure already has something allocated in this snapshot index
		//
		if( snapshotCount == 0 && m_parentSnapshots[ i ] != NULL )
		{

			DEBUG_CRASH(( "W3DGhostObject::xfer - m_parentShapshots[ %d ] has data present but the count from the xfer stream is empty\n" ));
			throw INI_INVALID_DATA;

		}  // end if

		// xfer each of the snapshots at this index
		Real scale;
		UnsignedInt color;
		AsciiString name;
		if( xfer->getXferMode() == XFER_SAVE )
		{

			// iterate through list
			objectSnapshot = m_parentSnapshots[ i ];
			while( objectSnapshot )
			{

				// write name from render object
				name.set( objectSnapshot->m_robj->Get_Name() );
				xfer->xferAsciiString( &name );

				// write scale from render object
				scale = objectSnapshot->m_robj->Get_ObjectScale();
				xfer->xferReal( &scale );

				// write color from render object
				color = objectSnapshot->m_robj->Get_ObjectColor();
				xfer->xferUnsignedInt( &color );

				// xfer data
				xfer->xferSnapshot( objectSnapshot );

				// onto the next
				objectSnapshot = objectSnapshot->m_next;

			}  // end while

		}  // end if, save
		else
		{
			RenderObjClass *renderObject;
			W3DRenderObjectSnapshot *prevObjectSnapshot = NULL;

			for( UnsignedByte j = 0; j < snapshotCount; ++j )
			{

				// read render object name
				xfer->xferAsciiString( &name );

				// read scale
				xfer->xferReal( &scale );

				// read color
				xfer->xferUnsignedInt( &color );

				// create the render object
				renderObject = W3DDisplay::m_assetManager->Create_Render_Obj( name.str(), scale, color );
				disableUVAnimations(renderObject);

				// we're loading, allocate new snapshot
				objectSnapshot = NEW W3DRenderObjectSnapshot( renderObject, &m_drawableInfo, FALSE );

				// attach to list
				if( prevObjectSnapshot )
					prevObjectSnapshot->m_next = objectSnapshot;
				else
					m_parentSnapshots[ i ] = objectSnapshot;
				prevObjectSnapshot = objectSnapshot;

				// xfer data
				xfer->xferSnapshot( objectSnapshot );

				// add snapshot to the scene
				objectSnapshot->addToScene();
										
			}  // end for, j	

		}  // end else, load

	}  // end for, i

	//
	// since there is a snapshot for this object, there cannot be a regular object/drawable
	// in the world, we need to remove it
	//
	if( m_parentObject && 
			m_parentSnapshots[ ThePlayerList->getLocalPlayer()->getPlayerIndex() ] != NULL &&
			xfer->getXferMode() == XFER_LOAD )
		removeParentObject();

	// count of partition shroudedness info to follow
	UnsignedByte shroudednessCount = 0;
	UnsignedByte playerIndex;
	for( playerIndex = 0; playerIndex < MAX_PLAYER_COUNT; ++playerIndex )
		if( m_parentSnapshots[ playerIndex ] )
			shroudednessCount++;
	xfer->xferUnsignedByte( &shroudednessCount );

	// shroudedness info
	ObjectShroudStatus status;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// iterate through all array points
		for( playerIndex = 0; playerIndex < MAX_PLAYER_COUNT; ++playerIndex )
		{

			// is this a location with any info
			if( m_parentSnapshots[ playerIndex ] )
			{

				// write this index
				xfer->xferUnsignedByte( &playerIndex );

				// write shroudedness
//				status = m_partitionData->getShroudedStatus( playerIndex );
//				xfer->xferUser( &status, sizeof( ObjectShroudStatus ) );

				// write previous shroudedness
				status = m_partitionData->friend_getShroudednessPrevious( playerIndex );
				xfer->xferUser( &status, sizeof( ObjectShroudStatus ) );

			}  // end if, snapshot list here exists

		}  // end for, i

	}  // end if, save
	else
	{
		UnsignedByte i;

		// how much data is here
		for( i = 0; i < shroudednessCount; ++i )
		{

			// which player index is this data for
			xfer->xferUnsignedByte( &playerIndex );

			// read shrouded status and set
//			xfer->xferUser( &status, sizeof( ObjectShroudStatus ) );
//			m_partitionData->friend_setShroudedness( playerIndex, status );

			// read shroudedness previous and set
			xfer->xferUser( &status, sizeof( ObjectShroudStatus ) );
			m_partitionData->friend_setShroudednessPrevious( playerIndex, status );

		}  // end for, i

	}  // end else load
		
}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DGhostObject::loadPostProcess( void )
{

	// extend base class
	GhostObject::loadPostProcess();

}  // end loadPostProcess

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
W3DGhostObjectManager::W3DGhostObjectManager(void)
{

	m_freeModules = NULL;
	m_usedModules = NULL;
	
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
W3DGhostObjectManager::~W3DGhostObjectManager()
{
	reset();	//make sure it's empty
	
	W3DGhostObject *mod = m_freeModules;
	W3DGhostObject *nextmod;

	while (mod)
	{
		nextmod=mod->m_nextSystem;
		delete mod;
		mod=nextmod;
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DGhostObjectManager::reset(void)
{
	W3DGhostObject *mod = m_usedModules;
	W3DGhostObject *nextmod;

	//Remove any orphaned modules that were not deleted with their parent object because player had fogged memory of them.
	while (mod)
	{
		nextmod=mod->m_nextSystem;
		if (!mod->m_parentObject)	//make sure it has no parent object
		{
			ThePartitionManager->unRegisterGhostObject(mod);
			removeGhostObject(mod);
		}
		mod=nextmod;
	}

	DEBUG_ASSERTCRASH(m_usedModules == NULL, ("Reset of Non-Empty GhostObjectManager"));

	//Delete any remaining modules (should be none)
	mod=m_usedModules;
	while (mod)
	{
		nextmod=mod->m_nextSystem;
		removeGhostObject(mod);
		mod=nextmod;
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DGhostObjectManager::removeGhostObject(GhostObject *object)
{
	if (!object)
		return;

	W3DGhostObject *mod = (W3DGhostObject *)object;

	mod->freeAllSnapShots();

	// remove module from used list
	if( mod->m_nextSystem )
		mod->m_nextSystem->m_prevSystem = mod->m_prevSystem;
	if( mod->m_prevSystem )
		mod->m_prevSystem->m_nextSystem = mod->m_nextSystem;
	else
		m_usedModules = mod->m_nextSystem;

	// add module to free list
	mod->m_prevSystem = NULL;
	mod->m_nextSystem = m_freeModules;
	if( m_freeModules )
		m_freeModules->m_prevSystem = mod;
	m_freeModules = mod;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GhostObject *W3DGhostObjectManager::addGhostObject(Object *object, PartitionData *pd)
{

	// we disabled adding new ghost objects - used during map border resizing and loading
	if (m_lockGhostObjects || m_saveLockGhostObjects )
		return NULL;

#ifdef DEBUG_FOG_MEMORY
	// sanity
	if( object != NULL )
	{

		W3DGhostObject *sanity = m_usedModules;
		while( sanity )
		{

			DEBUG_ASSERTCRASH( sanity->m_parentObject != object,	
												 ("W3DGhostObjectManager::addGhostObject - Duplicate ghost object detected\n") );
			sanity = sanity->m_nextSystem;

		}  // end while

	}  // end if
#endif

	W3DGhostObject *mod = m_freeModules;
	if( mod )
	{
		// take module off the free list
		if( mod->m_nextSystem )
			mod->m_nextSystem->m_prevSystem = mod->m_prevSystem;
		if( mod->m_prevSystem )
			mod->m_prevSystem->m_nextSystem = mod->m_nextSystem;
		else
			m_freeModules = mod->m_nextSystem;
	}
	else
	{
		mod = NEW W3DGhostObject;	// poolify
	}

	mod->m_prevSystem = NULL;
	mod->m_nextSystem = m_usedModules;
	if( m_usedModules )
		m_usedModules->m_prevSystem = mod;
	m_usedModules = mod;

	//Copy settings from parent object
	mod->m_parentObject=object;
	mod->m_drawableInfo.m_drawable=NULL;	//these dummy render objects don't have drawables.
	mod->m_drawableInfo.m_ghostObject = mod;
	mod->m_partitionData=pd;

	return mod;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void W3DGhostObjectManager::setLocalPlayerIndex(int index)
{
	//Whenever we switch local players, we need to remove all ghost objects belonging
	//to another player from the map.  We then insert the current local player's
	//ghost objects into the map.

	W3DGhostObject *mod = m_usedModules;

	while (mod)
	{
		mod->removeFromScene(m_localPlayer);
		if (mod->m_parentSnapshots[index])
		{	//new player has his own snapshot
			if (!mod->m_parentSnapshots[m_localPlayer] && mod->m_parentObject)
			{	//previous player didn't have a snapshot so real object must
				//have been in the scene.  Replace it with our snapshot.
				mod->removeParentObject();
			}
			mod->addToScene(index);
		}
		//new player doesn't have a snapshot which means restore original object
		//if it was replaced by a snapshot by the previous player.
		else
		if (mod->m_parentSnapshots[m_localPlayer] && mod->m_parentObject)
			mod->restoreParentObject();
	
		mod=mod->m_nextSystem;
	}

	m_localPlayer = index;
}

// ------------------------------------------------------------------------------------------------
/** When a game object/drawable dies, it is removed from the rest of the engine.  It leaves behind
a GhostObject in case any players didn't see the death and have a fogged view of the pre-death object.
We need to manually determine if these orphaned GhostObjects ever become visible and are no longer
needed*/
// ------------------------------------------------------------------------------------------------
void W3DGhostObjectManager::updateOrphanedObjects(int *playerIndexList, int numNonLocalPlayers)
{

	W3DGhostObject *mod = m_usedModules, *nextmod;
	int numStoredSnapshots;

	while (mod)
	{
		//updating the shroud status of this ghostobject could remove
		//it from the scene if it becomes visible but parent object is gone.
		nextmod=mod->m_nextSystem;
		if (!mod->m_parentObject)
		{	
			numStoredSnapshots=0;
#ifdef DEBUG_FOG_MEMORY
			for (int i=0; i<numNonLocalPlayers; i++, playerIndexList++)
			{
				if (mod->m_parentSnapshots[*playerIndexList])
					mod->getShroudStatus(*playerIndexList);
				if (mod->m_parentSnapshots[*playerIndexList])
					numStoredSnapshots++;
			}
#endif
			mod->getShroudStatus(m_localPlayer);
			if (mod->m_parentSnapshots[m_localPlayer])
					numStoredSnapshots++;
			if (!numStoredSnapshots)
			{	ThePartitionManager->unRegisterGhostObject(mod);
				mod->m_partitionData=NULL;
				removeGhostObject(mod);
			}
		}

		mod=nextmod;
	}
}

// ------------------------------------------------------------------------------------------------
/*When a map border changes (via script) we reset the partition manager.  Since ghost objects are
stored inside the partition manager, we need to save and restore them.  This function will save
enough data to restore the state of the partition manager.*/
// ------------------------------------------------------------------------------------------------
void W3DGhostObjectManager::releasePartitionData(void)
{
	W3DGhostObject *mod = m_usedModules;
	W3DGhostObject *nextmod;

	while (mod)
	{
		nextmod=mod->m_nextSystem;
		//if module has no parent object, then its holding a ghost object which
		//needs to have it's own partition data.
		if (!mod->m_parentObject)
		{
			ThePartitionManager->unRegisterGhostObject(mod);
			mod->m_partitionData = NULL;
		}
		else
		{	//The parent object will handle unregistering so just tell to break the
			//ghost object link.
			mod->friend_getPartitionData()->friend_setGhostObject(NULL);
			mod->m_partitionData = NULL;
		}
		mod=nextmod;
	}
}

// ------------------------------------------------------------------------------------------------
/*Insert ghost objects back into the partition manager*/
// ------------------------------------------------------------------------------------------------
void W3DGhostObjectManager::restorePartitionData(void)
{
	W3DGhostObject *mod = m_usedModules;
	W3DGhostObject *nextmod;

	while (mod)
	{
		nextmod=mod->m_nextSystem;
		//if module has no parent object, then its holding a ghost object which
		//needs to have it's own partition data.
		if (mod->m_parentObject)
		{
			//restore into parent's partition data
			mod->m_parentObject->friend_getPartitionData()->friend_setGhostObject(mod);
			mod->m_partitionData=mod->m_parentObject->friend_getPartitionData();
		}
		else
		{	
			//restore into our own partition data
			ThePartitionManager->registerGhostObject(mod);
		}

		//set partition data to reflect that we've seen a fogged version
		//of this object if one exists.
		for (Int i=0; i<MAX_PLAYER_COUNT; i++)
		{	if (mod->m_parentSnapshots[i])
				mod->m_partitionData->friend_setShroudednessPrevious(i,OBJECTSHROUD_FOGGED);
		}

		mod=nextmod;
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DGhostObjectManager::crc( Xfer *xfer )
{

	// extend base class
	GhostObjectManager::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DGhostObjectManager::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	GhostObjectManager::xfer( xfer );

	// count the number of used modules we have
	UnsignedShort count = 0;
	W3DGhostObject *w3dGhostObject;
	for( w3dGhostObject = m_usedModules; w3dGhostObject; w3dGhostObject = w3dGhostObject->m_nextSystem )
		count++;

	// xfer count
	xfer->xferUnsignedShort( &count );

	// ghost object themselves
	ObjectID objectID;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// iterate all ghost objects
		for( w3dGhostObject = m_usedModules; w3dGhostObject; w3dGhostObject = w3dGhostObject->m_nextSystem )
		{

			// write out object ID
			if( w3dGhostObject->m_parentObject )
				objectID = w3dGhostObject->m_parentObject->getID();
			else
				objectID = INVALID_ID;
			xfer->xferObjectID( &objectID );

			// write out ghost object data
			xfer->xferSnapshot( w3dGhostObject );

		}  // end for, ghostObject

	}  // end if, saving
	else
	{

		// sanity, there should be no ghost objects loaded at this time
		DEBUG_ASSERTCRASH( m_usedModules == NULL, 
											 ("W3DGhostObjectManager::xfer - The used module list is not NULL upon load, but should be!\n") );

		// now it's time to unlock the ghost objects for loading
		DEBUG_ASSERTCRASH( m_saveLockGhostObjects == TRUE,
											 ("W3DGhostObjectManager::xfer - Ghost object manager is not save locked, but should be\n") );
		TheGhostObjectManager->saveLockGhostObjects( FALSE );

		// read all ghost objects
		GhostObject *ghostObject;
		Object *object;
		for( UnsignedShort i = 0; i < count; ++i )
		{

			// read object id
			xfer->xferObjectID( &objectID );
					
			// get object from id
			object = TheGameLogic->findObjectByID( objectID );

			// create ghost object data
			if( object )
			{
				
				// create ghost object
				ghostObject = addGhostObject( object, object->friend_getPartitionData() );

				// sanity
				DEBUG_ASSERTCRASH( ghostObject != NULL, 
													 ("W3DGhostObjectManager::xfer - Could not create ghost object for object '%s'\n", 
													 object->getTemplate()->getName().str()) );

				// link the ghost object and logical object togehter through partition/ghostObject dat
				DEBUG_ASSERTCRASH( object->friend_getPartitionData()->getGhostObject() == NULL,
													 ("W3DGhostObjectManager::xfer - Ghost object already on object '%s'\n",
													  object->getTemplate()->getName().str()) );
				object->friend_getPartitionData()->friend_setGhostObject( ghostObject );

			}  // end if
			else
			{

				// create object with no object or partition data
				ghostObject = addGhostObject( NULL, NULL );

				// register ghost object object with partition system and fill out partition data
				ThePartitionManager->registerGhostObject( ghostObject );

			}  // end else

			// read ghost object data
			xfer->xferSnapshot( ghostObject );

		}  // end for, i

	}  // end else, loading

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DGhostObjectManager::loadPostProcess( void )
{

	// extend base class
	GhostObjectManager::loadPostProcess();

}  // end loadPostProcess
