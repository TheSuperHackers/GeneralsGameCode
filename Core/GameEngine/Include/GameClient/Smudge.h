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

#include "WW3D2/dllist.h"
#include "WWMath/vector2.h"
#include "WWMath/vector3.h"

#define SET_SMUDGE_PARAMETERS(smudge,pos,offset,size,opacity) (smudge->m_pos=pos;smudge->m_offset=offset;smudge->m_size=size;smudge->m_opacity=opacity;)

struct Smudge : public DLNodeClass<Smudge>
{
	W3DMPO_GLUE(Smudge)

	Vector3 m_pos;	//position of smudge center
	Vector2 m_offset; // difference in position between "texture" extraction and re-insertion for center vertex
	Real m_size;		//size of smudge in world space.
	Real m_opacity;	//alpha of center vertex, corners are assumed at 0

	struct smudgeVertex
	{	Vector3 pos;	//world-space position of vertex
		Vector2 uv;	//uv coordinates of vertex
	};
	smudgeVertex m_verts[5];	//5 vertices of this smudge (in counter-clockwise order, starting at top-left, ending in center.)
};

struct SmudgeSet : public DLNodeClass<SmudgeSet>
{
	W3DMPO_GLUE(SmudgeSet)

public:

	friend class SmudgeManager;

	SmudgeSet( void );
	virtual ~SmudgeSet();
	void reset(void);

	Smudge *addSmudgeToSet(void);
	void removeSmudgeFromSet ( Smudge &mySmudge);
	DLListClass<Smudge> &getUsedSmudgeList( void ) { return m_usedSmudgeList;}
	Int getUsedSmudgeCount(void) { return m_usedSmudgeCount; }	///<active smudges that need rendering.

private:
	DLListClass<Smudge> m_usedSmudgeList;	///<list of smudges in this set.
	static DLListClass<Smudge> m_freeSmudgeList;	///<list of unused smudges for use by SmudgeSets.
	Int m_usedSmudgeCount;
};

class SmudgeManager
{
public:
	SmudgeManager( void );
	virtual ~SmudgeManager();

	virtual void init(void);
	virtual void reset (void);
	virtual void ReleaseResources(void) {}
	virtual void ReAcquireResources(void) {}

	SmudgeSet *addSmudgeSet(void);
	void removeSmudgeSet(SmudgeSet &mySmudge);
	inline Int getSmudgeCountLastFrame(void) {return m_smudgeCountLastFrame;} ///<return number of smudges submitted last frame.
	inline void setSmudgeCountLastFrame(Int count) { m_smudgeCountLastFrame = count;}
	inline Bool getHardwareSupport(void) { return m_hardwareSupportStatus != SMUDGE_SUPPORT_NO;}

protected:

	enum HardwareSmudgeSupport {SMUDGE_SUPPORT_UNKNOWN,SMUDGE_SUPPORT_NO,SMUDGE_SUPPORT_YES};

	HardwareSmudgeSupport m_hardwareSupportStatus;///< flag whether we verified that the effect is supported by hardware.

	DLListClass<SmudgeSet> m_usedSmudgeSetList;	///<used SmudgeSets
	DLListClass<SmudgeSet> m_freeSmudgeSetList;	///<unused SmudgeSets ready for re-use.
	Int m_smudgeCountLastFrame;	//number of total smudges in manager last frame.
};

extern SmudgeManager *TheSmudgeManager;	///<singleton
