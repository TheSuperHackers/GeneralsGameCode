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

// FILE: ExperienceTracker.h //////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, February 2002
// Desc:   Keeps track of experience points so Veterance levels can be gained
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/GameCommon.h"
#include "Common/GameType.h"
#include "Common/GameMemory.h"
#include "Common/Snapshot.h"

class Object;

class ExperienceTracker : public MemoryPoolObject, public Snapshot
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ExperienceTracker, "ExperienceTrackerPool" )
public:
	ExperienceTracker(Object *parent);

	VeterancyLevel getVeterancyLevel() const { return m_currentLevel; }			///< What level am I?
	Int getExperienceValue( const Object* killer ) const;										///< How much do give for being killed
	Int getCurrentExperience( void ) const { return m_currentExperience; };	///< How much experience do I have at the moment?
	Bool isTrainable() const;																						///< Can I gain experience?
	Bool isAcceptingExperiencePoints() const;														///< Either I am trainable, or I have a Sink set up

	void setVeterancyLevel( VeterancyLevel newLevel, Bool provideFeedback = TRUE );						///< Set Level to this
	void setMinVeterancyLevel( VeterancyLevel newLevel, Bool provideFeedback = TRUE );					///< Set Level to AT LEAST this... if we are already >= this level, do nothing.
	void addExperiencePoints( Int experienceGain, Bool canScaleForBonus = TRUE );	///< Gain this many exp.
	Bool gainExpForLevel(Int levelsToGain, Bool canScaleForBonus = TRUE );			  ///< Gain enough exp to gain a level. return false if can't gain a level.
	Bool canGainExpForLevel(Int levelsToGain) const;															///< return same value as gainExpForLevel, but don't change anything
	void setExperienceAndLevel(Int experienceIn, Bool provideFeedback = TRUE );
	void setExperienceSink( ObjectID sink );											///< My experience actually goes to this person (loose couple)
	ObjectID getExperienceSink() const;

	Real getExperienceScalar() const { return m_experienceScalar; }
	void setExperienceScalar( Real scalar ) { m_experienceScalar = scalar; }

	// --------------- inherited from Snapshot interface --------------
	void crc( Xfer *xfer );
	void xfer( Xfer *xfer );
	void loadPostProcess( void );

private:
	Object*						m_parent;														///< Object I am owned by
	VeterancyLevel		m_currentLevel;											///< Level of experience
	Int								m_currentExperience;								///< Number of experience points
	ObjectID					m_experienceSink;										///< ID of object I have pledged my experience point gains to
	Real							m_experienceScalar;									///< Scales any experience gained by this multiplier.
};
