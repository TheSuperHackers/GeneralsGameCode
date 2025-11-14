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

// CUndoable.h
// Class to handle undo/redo.
// Author: John Ahlquist, April 2001

#pragma once

#include "Lib/BaseType.h"
#include "../../GameEngine/Include/Common/MapObject.h"
#include "../../GameEngine/Include/Common/GameCommon.h"
#include "../../GameEngine/Include/GameLogic/SidesList.h"
#include <vector>

class PolygonTrigger;
class BuildListInfo;
/*************************************************************************
**                             Undoable
***************************************************************************/
/// Base command class for all undoable commands.  Just the virtual shell.
class Undoable : public RefCountClass
{
protected:
	Undoable *mNext;

public:
		Undoable(void);

		~Undoable(void);

public:
	virtual void Do(void)=0; ///< pure virtual.
	virtual void Undo(void)=0;///< pure virtual.
	virtual void Redo(void);

	void LinkNext(Undoable *pNext);
	Undoable *GetNext(void) {return mNext;};

};

class CWorldBuilderDoc;
class WorldHeightMapEdit;
class MapObject;
/*************************************************************************
**                             CWBDocUndoable
***************************************************************************/
/// Command that saves & restores entire height map.
/** An undoable that actually undoes something.  Saves and restores the
entire height map. */
class WBDocUndoable : public Undoable
{
protected:
	CWorldBuilderDoc			*mPDoc;								///< Not ref counted.  This undoable should be in a list attached to the doc anyway.
	WorldHeightMapEdit		*mPNewHeightMapData;  ///< ref counted.
	WorldHeightMapEdit		*mPOldHeightMapData;  ///< ref counted.
	Bool									m_offsetObjects;			///< If true, apply m_objOffset.
	Coord3D								m_objOffset;					///< Offset to adjust all objects.


public:
		WBDocUndoable(CWorldBuilderDoc *pDoc, WorldHeightMapEdit *pNewHtMap, Coord3D *pObjOffset = NULL);

		// destructor.
		~WBDocUndoable(void);
		virtual void Do(void);
		virtual void Undo(void);
		virtual void Redo(void);

};

///                            AddObjectUndoable
/** An undoable that actually undoes something.  Adds an object
to the height map.  If it is a linked list, adds all objects. */
class AddObjectUndoable : public Undoable
{
protected:
	CWorldBuilderDoc *m_pDoc;  ///< Not ref counted.  This undoable should be in a list attached to the doc anyway.
	MapObject				 *m_objectToAdd;
	Int							  m_numObjects;
	Bool							m_addedToList;

public:
		AddObjectUndoable(CWorldBuilderDoc *pDoc, MapObject *pObjectToAdd);

		// destructor.
		~AddObjectUndoable(void);
		virtual void Do(void);
		virtual void Undo(void);
};


///                            ModifyObjectUndoable
/** An undoable that actually undoes something.  Modifies an object's
location and angle. */
// Helper class
class MoveInfo
{
public:
	MoveInfo(MapObject *pObjToMove);
	~MoveInfo();
	void DoMove(CWorldBuilderDoc *pDoc);
	void UndoMove(CWorldBuilderDoc *pDoc);

	void SetOffset(CWorldBuilderDoc *pDoc, Real x, Real y);
	void SetZOffset(CWorldBuilderDoc *pDoc, Real z);
	void RotateTo(CWorldBuilderDoc *pDoc, Real angle);
	void SetThingTemplate(CWorldBuilderDoc *pDoc, const ThingTemplate* thing);
	void SetName(CWorldBuilderDoc *pDoc, AsciiString name);

public:
	MapObject				 *m_objectToModify;
	MoveInfo				 *m_next;
	Real							m_newAngle;
	Real							m_oldAngle;
	Coord3D						m_newLocation;
	Coord3D						m_oldLocation;
	const ThingTemplate*		m_oldThing;
	const ThingTemplate*		m_newThing;
	AsciiString					m_oldName;
	AsciiString					m_newName;
};

class ModifyObjectUndoable : public Undoable
{
protected:
	CWorldBuilderDoc*	m_pDoc;  ///< Not ref counted.  This undoable should be in a list attached to the doc anyway.
	MoveInfo*					m_moveList;
	Bool							m_inval;

public:
		ModifyObjectUndoable(CWorldBuilderDoc *pDoc);
		// destructor.
		~ModifyObjectUndoable(void);

		virtual void Do(void);
		virtual void Undo(void);
		virtual void Redo(void);

		void SetOffset(Real x, Real y);
		void SetZOffset(Real z);
		void RotateTo(Real angle);
		void SetThingTemplate(const ThingTemplate* thing);
		void SetName(AsciiString name);
};

///                            ModifyFlagsUndoable
/** An undoable that actually undoes something.  Modifies an object's
flags. */
// Helper class
class FlagsInfo
{
public:
	FlagsInfo(MapObject *pObjToMove, Int flagMask, Int flagValue );
	~FlagsInfo();
	void DoFlags(CWorldBuilderDoc *pDoc);
	void UndoFlags(CWorldBuilderDoc *pDoc);

public:
	MapObject				 *m_objectToModify;
	FlagsInfo				 *m_next;
	Int								m_flagMask;
	Int								m_newValue;
	Int								m_oldValue;
};

class ModifyFlagsUndoable : public Undoable
{
protected:
	CWorldBuilderDoc *m_pDoc;  ///< Not ref counted.  This undoable should be in a list attached to the doc anyway.
	FlagsInfo *m_flagsList;

public:
		ModifyFlagsUndoable(CWorldBuilderDoc *pDoc, Int flagMask, Int flagValue);
		// destructor.
		~ModifyFlagsUndoable(void);

		virtual void Do(void);
		virtual void Undo(void);
		virtual void Redo(void);

};

class SidesListUndoable : public Undoable
{
protected:
	SidesList		m_old, m_new;
	CWorldBuilderDoc *m_pDoc;  ///< Not ref counted.  This undoable should be in a list attached to the doc anyway.

public:

	SidesListUndoable(const SidesList& newSL, CWorldBuilderDoc *pDoc);
	~SidesListUndoable(void);

	virtual void Do(void);
	virtual void Undo(void);

};

class DictItemUndoable : public Undoable
{
protected:
	Int m_numDictsToModify;
	std::vector<Dict*> m_dictToModify;
	std::vector<Dict> m_oldDictData;
	Dict m_newDictData;
	CWorldBuilderDoc *m_pDoc;
	Bool m_inval;
	NameKeyType m_key;

public:

	static Dict buildSingleItemDict(AsciiString k, Dict::DataType t, AsciiString v);

	// if you want to just add/modify/remove a single dict item, pass the item's key.
	// if you want to substitute the entire contents of the new dict, pass NAMEKEY_INVALID.
	DictItemUndoable(Dict **d, Dict data, NameKeyType key, Int dictsToModify = 1, CWorldBuilderDoc *pDoc = NULL, Bool inval = false);
	// destructor.
	~DictItemUndoable(void);

	virtual void Do(void);
	virtual void Undo(void);

};


///                            DeleteObjectUndoable
/** An undoable that actually undoes something.  Deletes an object. */
// Helper class
class DeleteInfo
{
public:
	DeleteInfo(MapObject *pObjToDelete);
	~DeleteInfo(void);
	void DoDelete(WorldHeightMapEdit *pMap);
	void UndoDelete(WorldHeightMapEdit *pMap);

public:
	Bool						 m_didDelete;
	MapObject				 *m_objectToDelete;
	MapObject				 *m_priorObject;
	DeleteInfo			 *m_next;
};


class DeleteObjectUndoable : public Undoable
{
protected:
	CWorldBuilderDoc *m_pDoc;  ///< Not ref counted.  This undoable should be in a list attached to the doc anyway.
	DeleteInfo *m_deleteList;
public:
		DeleteObjectUndoable(CWorldBuilderDoc *pDoc);

		// destructor.
		~DeleteObjectUndoable(void);
		virtual void Do(void);
		virtual void Undo(void);
};

///                            AddPolygonUndoable
/** An undoable that actually undoes something.  Adds a polygon. */
class AddPolygonUndoable : public Undoable
{
protected:
	PolygonTrigger *m_trigger;
	Bool					 m_isTriggerInList;
public:
		AddPolygonUndoable( PolygonTrigger *pTrig);
		// destructor.
		~AddPolygonUndoable(void);
		virtual void Do(void);
		virtual void Undo(void);
};

///                            AddPolygonPointUndoable
/** An undoable that actually undoes something.  Adds a polygon. */
class AddPolygonPointUndoable : public Undoable
{
protected:
	PolygonTrigger *m_trigger;
	ICoord3D				m_point;
public:
		AddPolygonPointUndoable(PolygonTrigger *pTrig, ICoord3D pt);
		// destructor.
		~AddPolygonPointUndoable(void);
		virtual void Do(void);
		virtual void Undo(void);
};

///                            ModifyPolygonPointUndoable
/** An undoable that actually undoes something.  Modifys a polygon. */
class ModifyPolygonPointUndoable : public Undoable
{
protected:
	PolygonTrigger *m_trigger;
	Int							m_pointIndex;
	ICoord3D				m_point;
	ICoord3D				m_savPoint;
public:
		ModifyPolygonPointUndoable(PolygonTrigger *pTrig, Int ndx);
		// destructor.
		~ModifyPolygonPointUndoable(void);
		virtual void Do(void);
		virtual void Undo(void);
};

///                            MovePolygonUndoable
/** An undoable that actually undoes something.  Moves a polygon. */
class MovePolygonUndoable : public Undoable
{
protected:
	PolygonTrigger *m_trigger;
	ICoord3D				m_point;
	ICoord3D				m_offset;
public:
		MovePolygonUndoable(PolygonTrigger *pTrig);
		// destructor.
		~MovePolygonUndoable(void);
		virtual void Do(void);
		virtual void Undo(void);

		void SetOffset(const ICoord3D &offset);
		PolygonTrigger *getTrigger(void) {return m_trigger;}
};

///                            InsertPolygonPointUndoable
/** An undoable that actually undoes something.  Inserts a polygon point. */
class InsertPolygonPointUndoable : public Undoable
{
protected:
	PolygonTrigger *m_trigger;
	Int							m_pointIndex;
	ICoord3D				m_point;
public:
		InsertPolygonPointUndoable(PolygonTrigger *pTrig, ICoord3D pt, Int ndx);
		// destructor.
		~InsertPolygonPointUndoable(void);
		virtual void Do(void);
		virtual void Undo(void);
};

///                            DeletePolygonPointUndoable
/** An undoable that actually undoes something.  Deletes a polygon point. */
class DeletePolygonPointUndoable : public Undoable
{
protected:
	PolygonTrigger *m_trigger;
	Int							m_pointIndex;
	ICoord3D				m_point;
public:
		DeletePolygonPointUndoable(PolygonTrigger *pTrig, Int ndx);
		// destructor.
		~DeletePolygonPointUndoable(void);
		virtual void Do(void);
		virtual void Undo(void);
};

///                            DeletePolygonUndoable
/** An undoable that actually undoes something.  Deletes a polygon. */
class DeletePolygonUndoable : public Undoable
{
protected:
	PolygonTrigger *m_trigger;
	Bool					 m_isTriggerInList;
public:
		DeletePolygonUndoable(PolygonTrigger *pTrig);
		// destructor.
		~DeletePolygonUndoable(void);
		virtual void Do(void);
		virtual void Undo(void);
};

///                            MultipleUndoable
/**
  * An undoable that doesn't do anything; it just consolidates a number of other
  * Undoables in a single logical undo step.
  */
class MultipleUndoable : public Undoable
{
protected:
  Undoable * m_undoableList; //< The head of the list of undoables, in the order they should be done. Reverse order for undoes
public:
		MultipleUndoable();
    // destructor.
    ~MultipleUndoable(void);

    /** Add other undoables in the order you would want them UNdone; e.g. in the reverse order you want them done
      * The MultipleUndoable object will then own the pointers.
      */
    void addUndoable( Undoable * undoable );

    virtual void Do(void);
    virtual void Undo(void);
    virtual void Redo(void);
};
