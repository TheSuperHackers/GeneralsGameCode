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

// FILE: Smudge.h /////////////////////////////////////////////////////////

#pragma once

#include <Utility/hash_map_adapter.h>
#include "WWMath/vector2.h"
#include "WWMath/vector3.h"
#include <deque>

#define SET_SMUDGE_PARAMETERS(smudge,pos,offset,size,opacity) (smudge->m_pos=pos;smudge->m_offset=offset;smudge->m_size=size;smudge->m_opacity=opacity;)

struct Smudge
{
	typedef void *Identifier;

	W3DMPO_CODE(Smudge)

	Identifier m_identifier;	//a number or pointer to identify this smudge
	Vector3 m_pos;	//position of smudge center
	Vector2 m_offset; // difference in position between "texture" extraction and re-insertion for center vertex
	Real m_size;		//size of smudge in world space.
	Real m_opacity;	//alpha of center vertex, corners are assumed at 0
	Bool m_draw;	//whether this smudge needs to be drawn

	struct smudgeVertex
	{
		Vector3 pos;	//world-space position of vertex
		Vector2 uv;	//uv coordinates of vertex
	};
	smudgeVertex m_verts[5];	//5 vertices of this smudge (in counter-clockwise order, starting at top-left, ending in center.)
};

typedef std::deque<Smudge*> SmudgeQueue;

#ifdef USING_STLPORT
namespace std
{
	template<> struct hash<Smudge::Identifier>
	{
		size_t operator()(Smudge::Identifier id) const { return reinterpret_cast<size_t>(id); }
	};
}
#endif // USING_STLPORT

struct SmudgeSet
{
	friend class SmudgeManager;

	W3DMPO_CODE(SmudgeSet)

	SmudgeSet();
	~SmudgeSet();

	void reset();
	void resetDraw();

	Smudge *addSmudgeToSet(Smudge::Identifier identifier); ///< add and return a smudge to the set with the given identifier
	Smudge *findSmudge(Smudge::Identifier identifier); ///< find the smudge that belongs to this identifier

	SmudgeQueue& getUsedSmudgeList() { return m_usedSmudgeList; }
	UnsignedInt getUsedSmudgeCount() const { return m_usedSmudgeList.size(); }	///<active smudges that need rendering.

private:
	typedef std::hash_map<Smudge::Identifier, Smudge *> SmudgeIdToPtrMap;

	SmudgeQueue m_usedSmudgeList;	///<list of smudges in this set.
	SmudgeIdToPtrMap m_usedSmudgeMap;
	static SmudgeQueue m_freeSmudgeList;	///<list of unused smudges for use by SmudgeSets.
};

typedef std::deque<SmudgeSet*> SmudgeSetQueue;

class SmudgeManager
{
public:
	SmudgeManager();
	virtual ~SmudgeManager();

	virtual void init();
	virtual void reset ();
	virtual void ReleaseResources() {}
	virtual void ReAcquireResources() {}

	void resetDraw(); ///< reset whether all smudges need to be drawn

	SmudgeSet *addSmudgeSet(); ///< add and return a new smudge set
	Smudge *findSmudge(Smudge::Identifier identifier); ///< find the smudge from any smudge set
	Int getSmudgeCountLastFrame() {return m_smudgeCountLastFrame;} ///<return number of smudges submitted last frame.
	Bool getHardwareSupport() { return m_hardwareSupportStatus != SMUDGE_SUPPORT_NO;}

protected:

	enum HardwareSmudgeSupport {SMUDGE_SUPPORT_UNKNOWN,SMUDGE_SUPPORT_NO,SMUDGE_SUPPORT_YES};

	HardwareSmudgeSupport m_hardwareSupportStatus;///< flag whether we verified that the effect is supported by hardware.

	SmudgeSetQueue m_usedSmudgeSetList;	///<used SmudgeSets
	SmudgeSetQueue m_freeSmudgeSetList;	///<unused SmudgeSets ready for re-use.
	Int m_smudgeCountLastFrame;	//number of total smudges in manager last frame.
};

extern SmudgeManager *TheSmudgeManager;	///<singleton
