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

// CUndoable.cpp
// Class to handle undo/redo.
// Author: John Ahlquist, April 2001

#include "StdAfx.h" 
#include "resource.h"

#include "CUndoable.h"
#include "PointerTool.h"
#include "RoadTool.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/SidesList.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "Common/Debug.h"
#include "mapobjectprops.h"
#include "ObjectOptions.h"
#include "BuildList.h"
#include "wbview3d.h"
#include "LayersList.h"
#include "Common/WellKnownKeys.h"
#include "WorldBuilder.h"	// for MAX_OBJECTS_IN_MAP 
#include "Common/UnicodeString.h"


// base mostly virtual class.

//
/// Undoable - destructor.
//
Undoable::~Undoable(void)
{
	REF_PTR_RELEASE(mNext);
}


//
/// Create a new undoable.
//
Undoable::Undoable(void):
	mNext(NULL)
{
}

//
/// Link another undoable to this.
//
void Undoable::LinkNext(Undoable *pNext)
{
	REF_PTR_SET(mNext, pNext);
}

//
/// Redo defaults to Do().
//
void Undoable::Redo(void)
{
	Do();  
}


/*************************************************************************
**                             WBDocUndoable
 An undoable that actually undoes something.  Saves the entire height map 
 state, and restores it.
***************************************************************************/
//
/// destructor.
//
WBDocUndoable::~WBDocUndoable(void)
{
	REF_PTR_RELEASE(mPNewHeightMapData);
	REF_PTR_RELEASE(mPOldHeightMapData);
	mPDoc = NULL;  // not ref counted.
}


//
/// Create a new undoable.
//
WBDocUndoable::WBDocUndoable(CWorldBuilderDoc *pDoc, WorldHeightMapEdit *pNewHtMap, Coord3D *pObjOffset):
	mPNewHeightMapData(NULL),
	mPOldHeightMapData(NULL),
	mPDoc(NULL)
{
	if (pObjOffset) {
		m_offsetObjects = true;
		m_objOffset = *pObjOffset;
	} else {
		m_offsetObjects = false;
		m_objOffset.x = m_objOffset.y = m_objOffset.z = 0;
	}
	REF_PTR_SET(mPNewHeightMapData, pNewHtMap);
	REF_PTR_SET(mPOldHeightMapData, pDoc->GetHeightMap());
	mPDoc = pDoc; // not ref counted.
}

//
/// Set the new height map.
//
void WBDocUndoable::Do(void)
{
	mPDoc->SetHeightMap(mPNewHeightMapData, false);		// SetHeightMap but don't inval.
	if (m_offsetObjects) {
		MapObject *pCur = MapObject::getFirstMapObject();
		while (pCur) {
			Coord3D loc = *pCur->getLocation();
			loc.x += m_objOffset.x;
			loc.y += m_objOffset.y;
			loc.z += m_objOffset.z;
			pCur->setLocation(&loc);
			pCur = pCur->getNext();
		}
		Int i;
		// Update build lists.
		for (i=0; i<TheSidesList->getNumSides(); i++) {
			SidesInfo *pSide = TheSidesList->getSideInfo(i); 
			for (BuildListInfo *pBuild = pSide->getBuildList(); pBuild; pBuild = pBuild->getNext()) {
				// Update.
				Coord3D loc = *pBuild->getLocation();
				loc.x += m_objOffset.x;
				loc.y += m_objOffset.y;
				loc.z += m_objOffset.z;
				pBuild->setLocation(loc);
			}
		}
		mPDoc->invalObject(NULL); // Inval all objects.	
	}
	mPNewHeightMapData->dbgVerifyAfterUndo();
}

//
/// Set the new height map.
//
void WBDocUndoable::Redo(void)
{
	// Cause the terrain texture to be regenerated.
	mPNewHeightMapData->resetResources();
	mPNewHeightMapData->getTerrainTexture();

	mPDoc->SetHeightMap(mPNewHeightMapData, true);		// SetHeightMap and inval the world.
	if (m_offsetObjects) {
		MapObject *pCur = MapObject::getFirstMapObject();
		while (pCur) {
			Coord3D loc = *pCur->getLocation();
			loc.x += m_objOffset.x;
			loc.y += m_objOffset.y;
			loc.z += m_objOffset.z;
			pCur->setLocation(&loc);
			pCur = pCur->getNext();
		}
		mPDoc->invalObject(NULL); // Inval all objects.	
	}
	mPNewHeightMapData->dbgVerifyAfterUndo();
}

//
/// Restore the old height map.
//
void WBDocUndoable::Undo(void)
{
	// Cause the terrain texture to be regenerated.
	mPOldHeightMapData->resetResources();
	mPOldHeightMapData->getTerrainTexture();

	mPDoc->SetHeightMap(mPOldHeightMapData, true);		// SetHeightMap and inval the world.
	if (m_offsetObjects) {
		MapObject *pCur = MapObject::getFirstMapObject();
		while (pCur) {
			Coord3D loc = *pCur->getLocation();
			loc.x -= m_objOffset.x;
			loc.y -= m_objOffset.y;
			loc.z -= m_objOffset.z;
			pCur->setLocation(&loc);
			pCur = pCur->getNext();
		}
		mPDoc->invalObject(NULL); // Inval all objects.	
	}
	mPOldHeightMapData->dbgVerifyAfterUndo();
}

/*************************************************************************
**                             AddObjectUndoable
***************************************************************************/
//
// AddObjectUndoable - destructor.
//
AddObjectUndoable::~AddObjectUndoable(void)
{
	m_pDoc = NULL;  // not ref counted.
	if (m_objectToAdd && !m_addedToList) {
		deleteInstance(m_objectToAdd);
		m_objectToAdd=NULL;
	}
}


//
// AddObjectUndoable - create a new undoable.
//
AddObjectUndoable::AddObjectUndoable(CWorldBuilderDoc *pDoc, MapObject *pObjectToAdd):
	m_pDoc(NULL),
	m_numObjects(0),
	m_addedToList(false),
	m_objectToAdd(NULL)
{
	m_pDoc = pDoc; // not ref counted.
	m_objectToAdd = pObjectToAdd;
}

//
/// Set the new mipping values, and force an inval of the windows.
//
void AddObjectUndoable::Do(void)
{
//	WorldHeightMapEdit *pMap = m_pDoc->GetHeightMap();
	MapObject *pCur = m_objectToAdd;
	MapObject *pLast = NULL;

	// Clear selection.
	PointerTool::clearSelection();
	m_numObjects = 0;
	while (pCur) { 
		if (!pCur->getFlag(FLAG_ROAD_FLAGS) && !pCur->getFlag(FLAG_BRIDGE_FLAGS) && !pCur->isWaypoint()) 
		{
			pCur->setSelected(true);
		}
		pLast = pCur;
		m_numObjects++;
		pCur = pCur->getNext();
	}
	if (pLast==NULL) {
		return;
	}
	pLast->setNextMap(MapObject::getFirstMapObject());
	
	MapObject::TheMapObjectListPtr	= m_objectToAdd;
	m_addedToList = true;

	pCur = m_objectToAdd;
	for (int i = 0; i < m_numObjects; i++) {
		// validate the object, giving it a valid ID and team.	
		pCur->validate();
		Dict *props = pCur->getProperties();
		Bool dontCare;	
		TheLayersList->addMapObjectToLayersList(pCur, props->getAsciiString(TheKey_objectLayer, &dontCare));

		m_pDoc->invalObject(pCur);
		pCur = pCur->getNext();
	}

	pCur = MapObject::getFirstMapObject();

	const Int flagsWeDontWant = FLAG_ROAD_FLAGS | FLAG_ROAD_CORNER_ANGLED | FLAG_BRIDGE_FLAGS | FLAG_ROAD_CORNER_TIGHT | FLAG_ROAD_JOIN;
	
	Int numObjects = 0;
	while (pCur) {
		Int flags = pCur->getFlags();
		if (!(pCur->isWaypoint() || (flags & flagsWeDontWant) != 0))
			++numObjects;
		
		pCur = pCur->getNext();
	}

	if (numObjects >= MAX_OBJECTS_IN_MAP) {
		CString str, loadStr;
		loadStr.Format(IDS_MAX_OBJECTS, MAX_OBJECTS_IN_MAP);
		AfxMessageBox(loadStr, MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
	}
}

//
// Restore the old mipping values, and inval.
//
void AddObjectUndoable::Undo(void)
{
//	WorldHeightMapEdit *pMap = m_pDoc->GetHeightMap();
	DEBUG_ASSERTCRASH(m_addedToList,("oops"));
	DEBUG_ASSERTCRASH(m_objectToAdd == MapObject::TheMapObjectListPtr,("oops"));
	if (!m_addedToList || m_objectToAdd!=MapObject::TheMapObjectListPtr) {
		return;
	}
	MapObject *pCur = MapObject::TheMapObjectListPtr;
	while (pCur && (m_numObjects > 1)) {
		m_pDoc->invalObject(pCur);	
		pCur = pCur->getNext();
		m_numObjects--;
	}
	if ((m_numObjects == 1) && pCur) {
		MapObject::TheMapObjectListPtr = pCur->getNext();
		pCur->setNextMap(NULL);
		m_addedToList = false;
	}
	pCur = m_objectToAdd;
	while (pCur) {
		m_pDoc->invalObject(pCur);	
		// Note : This undoable can add multiple objects, and removes multiple objects.
		TheLayersList->removeMapObjectFromLayersList(pCur);
		pCur = pCur->getNext();
	}

}

/*************************************************************************
**                             MoveInfo
***************************************************************************/
MoveInfo::~MoveInfo(void)
{
	m_objectToModify=NULL; // The map info list owns these, don't delete.
	MoveInfo *pCur = m_next;
	MoveInfo *tmp;
	while (pCur) {
		tmp = pCur;
		pCur = tmp->m_next;
		tmp->m_next = NULL;
		delete tmp;
	}
}

MoveInfo::MoveInfo( MapObject *pObjToMove):
	m_next(NULL)
{
	m_objectToModify = pObjToMove;	// Not copied.
	m_newAngle = m_objectToModify->getAngle();
	m_newLocation = *m_objectToModify->getLocation();
	m_oldAngle = m_objectToModify->getAngle();
	m_oldLocation = *m_objectToModify->getLocation();
	m_newThing = m_objectToModify->getThingTemplate();
	m_oldThing = m_objectToModify->getThingTemplate();
}

//
/// Sets the offset for all the objects in the move list, and moves them there.
//
void MoveInfo::SetOffset(CWorldBuilderDoc *pDoc, Real x, Real y)
{
	m_newLocation = m_oldLocation;
	m_newLocation.x += x;
	m_newLocation.y += y;
	if (m_objectToModify->getFlag(FLAG_ROAD_POINT1) || m_objectToModify->getFlag(FLAG_ROAD_POINT2)) {
		RoadTool::snap(&m_newLocation, false);
	}
	DoMove(pDoc);
}

void MoveInfo::SetZOffset(CWorldBuilderDoc *pDoc, Real z)
{
	m_newLocation = m_oldLocation;
	m_newLocation.z = z;
	if (m_objectToModify->getFlag(FLAG_ROAD_POINT1) || m_objectToModify->getFlag(FLAG_ROAD_POINT2)) {
		RoadTool::snap(&m_newLocation, false);
	}
	DoMove(pDoc);
}

void MoveInfo::SetThingTemplate(CWorldBuilderDoc *pDoc, const ThingTemplate* thing)
{
	m_newThing = thing;
	DoMove(pDoc);
}

void MoveInfo::SetName(CWorldBuilderDoc *pDoc, AsciiString name)
{
	m_newName = name;
	DoMove(pDoc);
}

//
/// Set the angle for all objects in the move list.
//
void MoveInfo::RotateTo(CWorldBuilderDoc *pDoc, Real angle)
{
	m_newAngle = angle;
	DoMove(pDoc);
}

//
/// Move the object.
//
void MoveInfo::DoMove(CWorldBuilderDoc *pDoc)
{
	pDoc->invalObject(m_objectToModify);
	m_objectToModify->setAngle(m_newAngle);
	m_objectToModify->setLocation(&m_newLocation);
	pDoc->invalObject(m_objectToModify);

	if (m_newThing != m_oldThing) {
		m_objectToModify->setThingTemplate(m_newThing);
	}
	if (m_newName != m_oldName) {
		m_objectToModify->setName(m_newName);
	}
}

void MoveInfo::UndoMove(CWorldBuilderDoc *pDoc)
{
	pDoc->invalObject(m_objectToModify);
	m_objectToModify->setAngle(m_oldAngle);
	m_objectToModify->setLocation(&m_oldLocation);
	pDoc->invalObject(m_objectToModify);

	if (m_newThing != m_oldThing) {
		m_objectToModify->setThingTemplate(m_oldThing);
	}
	if (m_newName != m_oldName) {
		m_objectToModify->setName(m_oldName);
	}
}

/*************************************************************************
**                             ModifyObjectUndoable
***************************************************************************/
//
// ModifyObjectUndoable - destructor.
//
ModifyObjectUndoable::~ModifyObjectUndoable(void)
{
	m_pDoc = NULL;  // not ref counted.
	if (m_moveList) {
		delete m_moveList;
	}
	m_moveList = NULL;
}

//
// ModifyObjectUndoable - create a new undoable.
//
ModifyObjectUndoable::ModifyObjectUndoable(CWorldBuilderDoc *pDoc):
	m_pDoc(NULL),
	m_moveList(NULL),
	m_inval(false)
{
	m_pDoc = pDoc; // not ref counted.
	MapObject *curMapObj = MapObject::getFirstMapObject();
	MoveInfo *pCurInfo = NULL;
	while (curMapObj) {
		if (curMapObj->isSelected()) {
			MoveInfo *pNew = new MoveInfo(curMapObj);
			if (pCurInfo == NULL) {
				m_moveList = pNew;
			} else {
				pCurInfo->m_next = pNew;
			}
			pCurInfo = pNew;
		}
		curMapObj = curMapObj->getNext();
	}
}

//
/// Sets the offset for all the objects in the move list, and moves them there.
//
void ModifyObjectUndoable::SetOffset(Real x, Real y)
{			
	MoveInfo *pCur = m_moveList;
	while (pCur) {
		pCur->SetOffset(m_pDoc, x, y);
		pCur = pCur->m_next;
	}
//	m_pDoc->updateAllViews();
}

void ModifyObjectUndoable::SetZOffset(Real z)
{
	MoveInfo *pCur = m_moveList;
	while (pCur) {
		pCur->SetZOffset(m_pDoc, z);
		pCur = pCur->m_next;
	}
}

//
/// Set the angle for all objects in the move list.
//
void ModifyObjectUndoable::RotateTo(Real angle)
{
	MoveInfo *pCur = m_moveList;
	while (pCur) {
		pCur->RotateTo(m_pDoc, angle);
		pCur = pCur->m_next;
	}
//	m_pDoc->updateAllViews();
}

void ModifyObjectUndoable::SetThingTemplate(const ThingTemplate* thing)
{
	MoveInfo *pCur = m_moveList;
	while (pCur) {
		pCur->SetThingTemplate(m_pDoc, thing);
		pCur = pCur->m_next;
	}
	m_inval = true;
	if (m_inval) 
	{
		WbView3d *p3View = m_pDoc->GetActive3DView();
		p3View->resetRenderObjects();
		p3View->invalObjectInView(NULL);
	}
}

void ModifyObjectUndoable::SetName(AsciiString name)
{
	MoveInfo *pCur = m_moveList;
	while (pCur) {
		pCur->SetName(m_pDoc, name);
		pCur = pCur->m_next;
	}
	m_inval = true;
	if (m_inval) 
	{
		WbView3d *p3View = m_pDoc->GetActive3DView();
		p3View->resetRenderObjects();
		p3View->invalObjectInView(NULL);
	}
}

//
/// Set the new values, and force an inval of the windows.
//
void ModifyObjectUndoable::Do(void)
{
	// Already done.
}

//
/// Set the new values, and force an inval of the windows.
//
void ModifyObjectUndoable::Redo(void)
{
	MoveInfo *pCur = m_moveList;
	while (pCur) {
		pCur->DoMove(m_pDoc);
		pCur = pCur->m_next;
	}
	if (m_inval) 
	{
		WbView3d *p3View = m_pDoc->GetActive3DView();
		p3View->resetRenderObjects();
		p3View->invalObjectInView(NULL);
	}
}

//
// Restore the old mipping values, and inval.
//
void ModifyObjectUndoable::Undo(void)
{
	MoveInfo *pCur = m_moveList;
	while (pCur) {
		pCur->UndoMove(m_pDoc);
		pCur = pCur->m_next;
	}
	if (m_inval) 
	{
		WbView3d *p3View = m_pDoc->GetActive3DView();
		p3View->resetRenderObjects();
		p3View->invalObjectInView(NULL);
	}
}

/*************************************************************************
**                             FlagsInfo
***************************************************************************/
FlagsInfo::~FlagsInfo(void)
{
	m_objectToModify=NULL; // The map info list owns these, don't delete.
	FlagsInfo *pCur = m_next;
	FlagsInfo *tmp;
	while (pCur) {
		tmp = pCur;
		pCur = tmp->m_next;
		tmp->m_next = NULL;
		delete tmp;
	}
}

FlagsInfo::FlagsInfo( MapObject *pObjToMove, Int flagMask, Int flagValue):
	m_next(NULL)
{
	m_objectToModify = pObjToMove;	// Not copied.
	m_flagMask = flagMask;
	m_newValue = flagValue&flagMask;
	m_oldValue = m_objectToModify->getFlags()&flagMask;
}


//
/// Move the object.
//
void FlagsInfo::DoFlags(CWorldBuilderDoc *pDoc)
{
	Bool found = false;
	MapObject *pMapObj; 
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj == m_objectToModify) {
			found = true;
			break;
		}
	}
	if (!found) return;
	pDoc->invalObject(m_objectToModify);
	m_objectToModify->clearFlag(m_flagMask);
	m_objectToModify->setFlag(m_newValue);
	pDoc->invalObject(m_objectToModify);
}

void FlagsInfo::UndoFlags(CWorldBuilderDoc *pDoc)
{
	Bool found = false;
	MapObject *pMapObj; 
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj == m_objectToModify) {
			found = true;
			break;
		}
	}
	if (!found) return;
	pDoc->invalObject(m_objectToModify);
	m_objectToModify->clearFlag(m_flagMask);
	m_objectToModify->setFlag(m_oldValue);
	pDoc->invalObject(m_objectToModify);
}

/*************************************************************************
**                             ModifyFlagsUndoable
***************************************************************************/
//
// ModifyFlagsUndoable - destructor.
//
ModifyFlagsUndoable::~ModifyFlagsUndoable(void)
{
	m_pDoc = NULL;  // not ref counted.
	if (m_flagsList) {
		delete m_flagsList;
	}
	m_flagsList = NULL;
}

//
// ModifyFlagsUndoable - create a new undoable.
//
ModifyFlagsUndoable::ModifyFlagsUndoable(CWorldBuilderDoc *pDoc, Int flagMask, Int flagValue):
	m_pDoc(NULL),
	m_flagsList(NULL)
{
	m_pDoc = pDoc; // not ref counted.
	MapObject *curMapObj = MapObject::getFirstMapObject();
	FlagsInfo *pCurInfo = NULL;
	while (curMapObj) {
		if (curMapObj->isSelected()) {
			FlagsInfo *pNew = new FlagsInfo(curMapObj, flagMask, flagValue);
			if (pCurInfo == NULL) {
				m_flagsList = pNew;
			} else {
				pCurInfo->m_next = pNew;
			}
			pCurInfo = pNew;
		}
		curMapObj = curMapObj->getNext();
	}
}


//
/// Set the new values, and force an inval of the objects.
//
void ModifyFlagsUndoable::Do(void)
{
	FlagsInfo *pCur = m_flagsList;
	while (pCur) {
		pCur->DoFlags(m_pDoc);
		pCur = pCur->m_next;
	}
}

//
/// Set the new values, and force an inval of the objects.
//
void ModifyFlagsUndoable::Redo(void)
{
	Do();
}

//
// Restore the old mipping values, and inval.
//
void ModifyFlagsUndoable::Undo(void)
{
	FlagsInfo *pCur = m_flagsList;
	while (pCur) {
		pCur->UndoFlags(m_pDoc);
		pCur = pCur->m_next;
	}
}

/*************************************************************************
**                             SidesListUndoable
***************************************************************************/

SidesListUndoable::SidesListUndoable(const SidesList& newSL, CWorldBuilderDoc *pDoc):
m_pDoc(pDoc)
{
	m_old = *TheSidesList;
	m_new = newSL;
	// ensure the new setup is valid. (don't mess with the old one.)
	Bool modified = m_new.validateSides();
	(void)modified;
	DEBUG_ASSERTLOG(!modified,("*** had to clean up sides in SidesListUndoable! (caller should do this)"));
}

SidesListUndoable::~SidesListUndoable()
{
}

void SidesListUndoable::Do(void)
{
	*TheSidesList = m_new;
	MapObjectProps::update();	// ugh, hack to update panel
	ObjectOptions::update();
	// Sides list contains build list, so inval the build list.
	BuildList::update();
	WbView3d *p3View = m_pDoc->GetActive3DView();
	p3View->resetRenderObjects();
	p3View->invalObjectInView(NULL);
}

void SidesListUndoable::Undo(void)
{
	*TheSidesList = m_old;
	MapObjectProps::update();	// ugh, hack to update panel
	ObjectOptions::update();
	// Sides list contains build list, so inval the build list.
	BuildList::update();
	WbView3d *p3View = m_pDoc->GetActive3DView();
	p3View->resetRenderObjects();
	p3View->invalObjectInView(NULL);
}

/*************************************************************************
**                             DictItemUndoable
***************************************************************************/

DictItemUndoable::DictItemUndoable(Dict **d, Dict data, NameKeyType key, Int dictsToModify, CWorldBuilderDoc *pDoc, Bool inval) :
	m_newDictData(data),
	m_key(key),
	m_numDictsToModify(dictsToModify),
	m_pDoc(pDoc),
	m_inval(inval)
{
	m_dictToModify.resize(m_numDictsToModify);
	m_oldDictData.resize(m_numDictsToModify);

	for (int i = 0; i < m_numDictsToModify; ++i) {
		m_dictToModify[i] = d[i];
		if (m_key == NAMEKEY_INVALID)
			m_oldDictData[i] = *d[i];
		else
		{
			DEBUG_ASSERTCRASH(data.getPairCount() <= 1, ("hmm"));
			m_oldDictData[i].copyPairFrom(*d[i], m_key);
		}
	}
}

DictItemUndoable::~DictItemUndoable()
{
}


void DictItemUndoable::Do(void)
{
	for (int i = 0; i < m_numDictsToModify; ++i) {
		if (m_key == NAMEKEY_INVALID)
			*m_dictToModify[i] = m_newDictData;
		else
			m_dictToModify[i]->copyPairFrom(m_newDictData, m_key);
		MapObjectProps::update();	// ugh, hack to update panel
		ObjectOptions::update();	// ditto
	}
	if (m_inval && m_pDoc) {
		WbView3d *p3View = m_pDoc->GetActive3DView();
		p3View->resetRenderObjects();
		p3View->invalObjectInView(NULL);
	}
}

void DictItemUndoable::Undo(void)
{
	for (int i = 0; i < m_numDictsToModify; ++i) {
		if (m_key == NAMEKEY_INVALID)
			*m_dictToModify[i] = m_oldDictData[i];
		else
			m_dictToModify[i]->copyPairFrom(m_oldDictData[i], m_key);
		MapObjectProps::update();		// ugh, hack to update panel
		ObjectOptions::update();	// ditto
	}
	if (m_inval && m_pDoc) {
		WbView3d *p3View = m_pDoc->GetActive3DView();
		p3View->resetRenderObjects();
		p3View->invalObjectInView(NULL);
	}
}

/*static*/ Dict DictItemUndoable::buildSingleItemDict(AsciiString k, Dict::DataType t, AsciiString v)
{
	Dict d;
	NameKeyType key = TheNameKeyGenerator->nameToKey(k);
	switch(t)
	{
		case Dict::DICT_BOOL:
			d.setBool(key, strcmp(v.str(), "true") == 0);
			break;
		case Dict::DICT_INT:
			d.setInt(key, atoi(v.str()));
			break;
		case Dict::DICT_REAL:
			d.setReal(key, atof(v.str()));
			break;
		case Dict::DICT_ASCIISTRING:
			d.setAsciiString(key, v);
			break;
		case Dict::DICT_UNICODESTRING:
			{
				UnicodeString tmp;
				tmp.translate(v);
				d.setUnicodeString(key, tmp);
			}
			break;
		default:
			DEBUG_CRASH(("bug"));
			break;
	}
	return d;
}

/*************************************************************************
**                             DeleteInfo
***************************************************************************/
DeleteInfo::~DeleteInfo(void)
{
	if (m_didDelete && m_objectToDelete) {
		deleteInstance(m_objectToDelete);
	}
	DeleteInfo *pCur = m_next;
	DeleteInfo *tmp;
	while (pCur) {
		tmp = pCur;
		pCur = tmp->m_next;
		tmp->m_next = NULL;
		delete tmp;
	}
	m_objectToDelete=NULL;
	m_priorObject=NULL;
}

DeleteInfo::DeleteInfo( MapObject *pObjectToDelete):
	m_objectToDelete(NULL),
	m_didDelete(false),
	m_next(NULL),
	m_priorObject(NULL)
{
	m_objectToDelete = pObjectToDelete;	// Not copied.
}

//
/// Delete the object.
//
void DeleteInfo::DoDelete(WorldHeightMapEdit *pMap)
{
	DEBUG_ASSERTCRASH(!m_didDelete,("oops"));
	m_priorObject = NULL;
	MapObject *curMapObj = MapObject::getFirstMapObject();
	Bool found = false;
	while (curMapObj) {
		if (curMapObj == m_objectToDelete) {
			found = true;
			break;
		}
		m_priorObject = curMapObj;
		curMapObj = curMapObj->getNext();
	}
	DEBUG_ASSERTCRASH(found,("not found"));
	if (!found) {
		m_objectToDelete = NULL;
		return;
	}
	if (m_priorObject) {
		m_priorObject->setNextMap(m_objectToDelete->getNext());
	} else {
		DEBUG_ASSERTCRASH(MapObject::TheMapObjectListPtr == m_objectToDelete,("oops"));
		MapObject::TheMapObjectListPtr = MapObject::TheMapObjectListPtr->getNext();
	}
	m_objectToDelete->setNextMap(NULL);
	m_didDelete = true;
}

void DeleteInfo::UndoDelete(WorldHeightMapEdit *pMap)
{
	DEBUG_ASSERTCRASH(m_didDelete,("oops"));
	if (!m_didDelete) return;
	if (m_priorObject) {
		m_objectToDelete->setNextMap(m_priorObject->getNext());
		m_priorObject->setNextMap(m_objectToDelete);
		m_priorObject = NULL;
	} else {
		m_objectToDelete->setNextMap(MapObject::TheMapObjectListPtr);
		MapObject::TheMapObjectListPtr = m_objectToDelete;
	}
	m_didDelete = false;
}

/*************************************************************************
**                             DeleteObjectUndoable
***************************************************************************/
//
// DeleteObjectUndoable - destructor.
//
DeleteObjectUndoable::~DeleteObjectUndoable(void)
{
	m_pDoc = NULL;  // not ref counted.
	if (m_deleteList) {
		delete m_deleteList;
	}
	m_deleteList=NULL;
}


//
// DeleteObjectUndoable - create a new undoable.	Deletes all selected objects.
//
DeleteObjectUndoable::DeleteObjectUndoable(CWorldBuilderDoc *pDoc):
	m_pDoc(NULL),
	m_deleteList(NULL)
{
	// Note - you can't delete just one end of a map segment.  So delete both.
	MapObject *pMapObj;
	MapObject *pMapObj2;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->getFlag(FLAG_ROAD_POINT1)) {
			pMapObj2 = pMapObj->getNext();
			DEBUG_ASSERTCRASH(pMapObj2 && pMapObj2->getFlag(FLAG_ROAD_POINT2), ("oops"));
			if (pMapObj2==NULL) break;
			if (!pMapObj2->getFlag(FLAG_ROAD_POINT2)) continue;
			// If one end of a road segment is selected, both are.
			if (pMapObj->isSelected() || pMapObj2->isSelected()) {
				pMapObj->setSelected(true);
				pMapObj2->setSelected(true);
			}
		}
	}


	m_pDoc = pDoc; // not ref counted.
	MapObject *curMapObj = MapObject::getFirstMapObject();
	DeleteInfo *pCurInfo = NULL;
	while (curMapObj) {
		if (curMapObj->isSelected()) {
			DeleteInfo *pNew = new DeleteInfo(curMapObj);
			if (pCurInfo == NULL) {
				m_deleteList = pNew;
			} else {
				pCurInfo->m_next = pNew;
			}
			pCurInfo = pNew;
		}
		curMapObj = curMapObj->getNext();
	}
}


//
/// Delete the objects.
//
void DeleteObjectUndoable::Do(void)
{
	WorldHeightMapEdit *pMap = m_pDoc->GetHeightMap();
	DeleteInfo *pCur = m_deleteList;
	DeleteInfo *pInvertedList = NULL;
	while (pCur) {
		// first, remove it from the Layers list. 
		TheLayersList->removeMapObjectFromLayersList(pCur->m_objectToDelete);

		DeleteInfo *tmp;
		pCur->DoDelete(pMap);
		tmp = pCur;
		pCur = pCur->m_next;
		tmp->m_next = pInvertedList;
		pInvertedList = tmp;
	}
	WbView3d *p3View = m_pDoc->GetActive3DView();
	if (p3View) { // Shouldn't ever be null, but just in case... jba.
		p3View->resetRenderObjects();
		p3View->invalObjectInView(NULL);
	}
	m_deleteList = pInvertedList;
}

//
// Restore the old mipping values, and inval.
//
void DeleteObjectUndoable::Undo(void)
{
	WorldHeightMapEdit *pMap = m_pDoc->GetHeightMap();
	DeleteInfo *pCur = m_deleteList;
	DeleteInfo *pInvertedList=NULL;
	while (pCur) {
		// Re-Add it to the layers list
		Dict* objDict = pCur->m_objectToDelete->getProperties();
		Bool exists;
		TheLayersList->addMapObjectToLayersList(pCur->m_objectToDelete, objDict->getAsciiString(TheKey_objectLayer, &exists));

		DeleteInfo *tmp;
		pCur->UndoDelete(pMap);
		tmp = pCur;
		pCur = pCur->m_next;
		tmp->m_next = pInvertedList;
		pInvertedList = tmp;
		m_pDoc->invalObject(tmp->m_objectToDelete);
	}
	m_deleteList = pInvertedList;
}


/*************************************************************************
**                             AddPolygonUndoable
***************************************************************************/
//
// AddPolygonUndoable - destructor.
//
AddPolygonUndoable::~AddPolygonUndoable(void)
{
	if (m_trigger && !m_isTriggerInList) {
		DEBUG_ASSERTCRASH(m_trigger->getNext()==NULL, ("Logic error."));
		deleteInstance(m_trigger);
	}
	m_trigger=NULL;
}

//
// AddPolygonUndoable - create a new undoable.	Adds a polygon trigger.
//
AddPolygonUndoable::AddPolygonUndoable(PolygonTrigger *pTrig):
	m_trigger(pTrig),
	m_isTriggerInList(false)
{
}

//
/// Add the trigger.
//
void AddPolygonUndoable::Do(void)
{
	PolygonTrigger::addPolygonTrigger(m_trigger);
	m_isTriggerInList = true;
}

//
// Remove the trigger.
//
void AddPolygonUndoable::Undo(void)
{
	PolygonTrigger::removePolygonTrigger(m_trigger);
	m_isTriggerInList = false;
}


/*************************************************************************
**                             AddPolygonPointUndoable
***************************************************************************/
//
// AddPolygonUndoable - destructor.
//
AddPolygonPointUndoable::~AddPolygonPointUndoable(void)
{
	m_trigger=NULL;
}

//
// AddPolygonUndoable - create a new undoable.	Adds a polygon point.
//
AddPolygonPointUndoable::AddPolygonPointUndoable(PolygonTrigger *pTrig,
																			 ICoord3D pt):
	m_trigger(pTrig),
	m_point(pt)
{
}

//
/// Add the trigger.
//
void AddPolygonPointUndoable::Do(void)
{
	m_trigger->addPoint(m_point);
}

//
// Remove the trigger.
//
void AddPolygonPointUndoable::Undo(void)
{
	m_point = *m_trigger->getPoint(m_trigger->getNumPoints()-1);
	m_trigger->deletePoint(m_trigger->getNumPoints()-1);
}


/*************************************************************************
**                             ModifyPolygonPointUndoable
***************************************************************************/
//
// ModifyPolygonPointUndoable - destructor.
//
ModifyPolygonPointUndoable::~ModifyPolygonPointUndoable(void)
{
	m_trigger=NULL;
}

//
// ModifyPolygonPointUndoable - create a new undoable.	Edit a polygon point.
//
ModifyPolygonPointUndoable::ModifyPolygonPointUndoable(PolygonTrigger *pTrig, Int ndx):
	m_trigger(pTrig),
	m_pointIndex(ndx)
{
	m_point = *pTrig->getPoint(ndx);
}

//
/// Add the trigger.
//
void ModifyPolygonPointUndoable::Do(void)
{
	m_savPoint = *m_trigger->getPoint(m_pointIndex);
	m_trigger->setPoint(m_point, m_pointIndex);
}

//
// Remove the trigger.
//
void ModifyPolygonPointUndoable::Undo(void)
{
	m_point = *m_trigger->getPoint(m_pointIndex);
	m_trigger->setPoint(m_savPoint, m_pointIndex);
}

/*************************************************************************
**                             MovePolygonUndoable
***************************************************************************/
//
// MovePolygonUndoable - destructor.
//
MovePolygonUndoable::~MovePolygonUndoable(void)
{
	m_trigger=NULL;
}

//
// MovePolygonUndoable - create a new undoable.	Edit a polygon point.
//
MovePolygonUndoable::MovePolygonUndoable(PolygonTrigger *pTrig):
	m_trigger(pTrig)
{
	m_offset.x = 0; 
	m_offset.y = 0;
	m_offset.z = 0;
}

void MovePolygonUndoable::SetOffset(const ICoord3D &offset)
{
	Int deltaX = offset.x - m_offset.x;
	Int deltaY = offset.y - m_offset.y;
	Int deltaZ = offset.z - m_offset.z;
	if (!m_trigger->isWaterArea()) {
		deltaZ = 0;
	}
	m_offset = offset;
	Int i;
	for (i=0; i<m_trigger->getNumPoints(); i++) {
		ICoord3D iLoc = *m_trigger->getPoint(i);
		iLoc.x += deltaX;
		iLoc.y += deltaY;
		iLoc.z += deltaZ;
		m_trigger->setPoint(iLoc, i);
	}
}


//
/// Offset the trigger.
//
void MovePolygonUndoable::Do(void)
{
	Int i;
	for (i=0; i<m_trigger->getNumPoints(); i++) {
		ICoord3D iLoc = *m_trigger->getPoint(i);
		iLoc.x += m_offset.x;
		iLoc.y += m_offset.y;
		iLoc.z += m_offset.z;
		m_trigger->setPoint(iLoc, i);
	}
}

//
// Unoffset the trigger.
//
void MovePolygonUndoable::Undo(void)
{
	Int i;
	for (i=0; i<m_trigger->getNumPoints(); i++) {
		ICoord3D iLoc = *m_trigger->getPoint(i);
		iLoc.x -= m_offset.x;
		iLoc.y -= m_offset.y;
		iLoc.z -= m_offset.z;
		m_trigger->setPoint(iLoc, i);
	}
}


/*************************************************************************
**                             InsertPolygonPointUndoable
***************************************************************************/
//
// InsertPolygonPointUndoable - destructor.
//
InsertPolygonPointUndoable::~InsertPolygonPointUndoable(void)
{
	m_trigger=NULL;
}

//
// InsertPolygonPointUndoable - create a new undoable.	Edit a polygon point.
//
InsertPolygonPointUndoable::InsertPolygonPointUndoable(PolygonTrigger *pTrig, ICoord3D pt, Int ndx):
	m_trigger(pTrig),
	m_point(pt),
	m_pointIndex(ndx)
{
}

//
/// Insert the point.
//
void InsertPolygonPointUndoable::Do(void)
{
	m_trigger->insertPoint(m_point, m_pointIndex);
}

//
// Remove the point.
//
void InsertPolygonPointUndoable::Undo(void)
{
	m_trigger->deletePoint(m_pointIndex);
}

/*************************************************************************
**                             DeletePolygonPointUndoable
***************************************************************************/
//
// DeletePolygonPointUndoable - destructor.
//
DeletePolygonPointUndoable::~DeletePolygonPointUndoable(void)
{
	m_trigger=NULL;
}

//
// DeletePolygonPointUndoable - create a new undoable.	Edit a polygon point.
//
DeletePolygonPointUndoable::DeletePolygonPointUndoable(PolygonTrigger *pTrig, Int ndx):
	m_trigger(pTrig),
	m_pointIndex(ndx)
{
	m_point = *pTrig->getPoint(ndx);
}

//
/// Delete the point.
//
void DeletePolygonPointUndoable::Do(void)
{
	m_trigger->deletePoint(m_pointIndex);
}

//
// Remove the point.
//
void DeletePolygonPointUndoable::Undo(void)
{
	m_trigger->insertPoint(m_point, m_pointIndex);
}

/*************************************************************************
**                             DeletePolygonUndoable
***************************************************************************/
//
// DeletePolygonUndoable - destructor.
//
DeletePolygonUndoable::~DeletePolygonUndoable(void)
{
	if (m_trigger && !m_isTriggerInList) {
		DEBUG_ASSERTCRASH(m_trigger->getNext()==NULL, ("Logic error."));
		deleteInstance(m_trigger);
	}
	m_trigger=NULL;
}

//
// DeletePolygonUndoable - create a new undoable.	Edit a polygon point.
//
DeletePolygonUndoable::DeletePolygonUndoable(PolygonTrigger *pTrig):
	m_trigger(pTrig),
	m_isTriggerInList(true)
{
}

//
/// Delete the point.
//
void DeletePolygonUndoable::Do(void)
{
	PolygonTrigger::removePolygonTrigger(m_trigger);
	m_isTriggerInList = false;
}

//
// Remove the point.
//
void DeletePolygonUndoable::Undo(void)
{
	PolygonTrigger::addPolygonTrigger(m_trigger);
	m_isTriggerInList = true;
}



