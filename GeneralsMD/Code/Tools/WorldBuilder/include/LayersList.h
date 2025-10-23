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

// FILE: LayersList.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  LayersList.h                                                  */
/* Created:    John K. McDonald, Jr., 5/10/2002                              */
/* Desc:       Layers List Declarations                                      */
/* Revision History:                                                         */
/*		5/10/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////
#include "Common/AsciiString.h"
#include <list>
#include <string>

// DEFINES ////////////////////////////////////////////////////////////////////

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////
class MapObject;
class PolygonTrigger;

// TYPE DEFINES ///////////////////////////////////////////////////////////////
typedef std::list<MapObject*> ListMapObjectPtr;
typedef ListMapObjectPtr::iterator ListMapObjectPtrIt;

typedef std::list<PolygonTrigger*> ListPolygonTriggerPtr;
typedef ListPolygonTriggerPtr::iterator ListPolygonTriggerPtrIt;

struct Layer
{
	AsciiString layerName;
	ListMapObjectPtr objectsInLayer;
	ListPolygonTriggerPtr polygonTriggersInLayer;
	Bool show;
};

typedef std::list<Layer> ListLayer;
typedef ListLayer::iterator ListLayerIt;

// Class Definition for overridden Tree control that supports Right-click context sensitive menu.
class CLLTreeCtrl : public CTreeCtrl
{
	public:
		AsciiString getLastClickedLayer(void) {	return mLastClickedLayer; }
		AsciiString getLastClickedObject(void) {	return mLastClickedObject; }

	protected:
		AsciiString mLastClickedLayer;
		AsciiString mLastClickedObject;
		void buildMoveMenu(CMenu* pPopup, UINT firstID);
		virtual void OnRButtonDown(UINT nFlags, CPoint point);
		DECLARE_MESSAGE_MAP()

};


// Class Definition

class LayersList : public CDialog
{

	public:
		enum { IDD = IDD_LAYERSLIST };
		LayersList(UINT nIDTemplate = LayersList::IDD, CWnd *parentWnd = NULL);
		virtual ~LayersList();

		void resetLayers();
		void addMapObjectToLayersList(IN MapObject *objToAdd, AsciiString layerToAddTo = AsciiString(TheDefaultLayerName.c_str()));
		void addPolygonTriggerToLayersList(IN PolygonTrigger *triggerToAdd, AsciiString layerToAddTo = AsciiString(TheDefaultLayerName.c_str()));
		AsciiString removeMapObjectFromLayersList(IN MapObject *objToRemove);
		AsciiString removePolygonTriggerFromLayersList(IN PolygonTrigger *triggerToRemove);

		void changeMapObjectLayer(IN MapObject *objToChange, AsciiString layerToPlaceOn);
		void changePolygonTriggerLayer(IN PolygonTrigger *triggerToChange, AsciiString layerToPlaceOn);

		void addLayerNamed(IN AsciiString layerToAdd);
		void removeLayerNamed(IN AsciiString layerToRemove);
		void changeLayerName(IN AsciiString oldLayerName, IN AsciiString newLayerName);
		void mergeLayerInto(IN ListLayerIt src, IN ListLayerIt dst);

		Bool isLayerHidden(IN AsciiString layerToTest);

		void updateUIFromList(void);

		void disableUpdates() { m_performUpdates = false; }
		void enableUpdates() { m_performUpdates = true; updateUIFromList(); }

		const ListLayer& GetAllLayers(void) const { return mLayers; }

		static MapObject *findObjectByUID(AsciiString objectIDToFind);
		static PolygonTrigger *findPolygonTriggerByUID(AsciiString objectIDToFind);
		static Bool findAndSelectMapObject(AsciiString selectedItemAsciiString);
		static Bool findAndSelectPolygonTrigger(AsciiString selectedItemAsciiString);
		static void unselectAllMapObjects(void);
		static void unselectAllPolygonTriggers(void);

	public:
		// This is a string because making it an AsciiString makes us barf on construction. :-(
		static std::string TheDefaultLayerName;
		static std::string ThePolygonTriggerLayerName;
		static std::string TheDefaultNewLayerName;
		static std::string TheActiveLayerName;
		static const std::string TheUnmutableDefaultLayerName;

	protected:
		AsciiString mCurrentlyEditingLabel;

		ListLayer mLayers;
		CLLTreeCtrl *mTree;
		CImageList mImageList;
		Bool m_performUpdates;
		Bool m_activatedLayer;


		HTREEITEM findTreeLayerNamed(const AsciiString& nameToFind);
		HTREEITEM findTreeObjectNamed(const AsciiString& objectToFind, HTREEITEM layerItem);

		// This function takes an MapObject, and does one of the following:
		// 1) Return true if the MapObject can be found, and
		//    layerIt points to a valid layer iterator in which the MapObject was found
		//    MapObjectIt points to a valid MapObject iterator on the layerIts MapObjectsInLayer member
		// 2) Returns false if the MapObject cannot be found.
		Bool findMapObjectAndList(IN MapObject *MapObjectToFind, OUT ListLayerIt *layerIt = NULL, OUT ListMapObjectPtrIt *MapObjectIt = NULL);
		Bool findPolygonTriggerAndList(IN PolygonTrigger *PolygonTriggerToFind, OUT ListLayerIt *layerIt = NULL, OUT ListPolygonTriggerPtrIt *PolygonTriggerIt = NULL);

		// This function takes a layer name, and does one of the following:
		// 1) Return true if the layer can be found, and
		//		layerIt points to a valid layer iterator named layerName
		// 2) Returns false if the layer cannot be found.
		Bool findLayerNamed(IN AsciiString layerName, OUT ListLayerIt *layerIt = NULL);

		void addMapObjectToLayer(IN MapObject *objToAdd, IN ListLayerIt *layerIt);
		void addPolygonTriggerToLayer(IN PolygonTrigger *objToAdd, IN ListLayerIt *layerIt);
		void removeMapObjectFromLayer(IN MapObject *objToRemove, IN ListLayerIt *layerIt = NULL, IN ListMapObjectPtrIt *MapObjectIt = NULL);
		void removePolygonTriggerFromLayer(IN PolygonTrigger *triggerToRemove, IN ListLayerIt *layerIt = NULL, IN ListPolygonTriggerPtrIt *PolygonTriggerIt = NULL);
		void updateObjectRenderFlags(IN ListLayerIt *layerIt);
		void updateTreeImages();

	protected:
		virtual void OnOK();
		virtual void OnCancel();
		virtual BOOL OnInitDialog();

		afx_msg void OnBeginEditLabel(NMHDR *pNotifyStruct, LRESULT* pResult);
		afx_msg void OnEndEditLabel(NMHDR *pNotifyStruct, LRESULT* pResult);

		afx_msg void OnMergeLayer(UINT commandID);
		afx_msg void OnMergeObject(UINT commandID);
		afx_msg void OnMergeViewSelection(UINT commandID);
		afx_msg void OnSelectActiveLayer();


		afx_msg void OnSelectLayerObject();
		afx_msg void OnNewLayer();
		afx_msg void OnDeleteLayer();
		afx_msg void OnHideShowLayer();
		DECLARE_MESSAGE_MAP()
};

extern LayersList *TheLayersList;
