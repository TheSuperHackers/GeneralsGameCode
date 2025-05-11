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

// FILE: ExperienceTracker.cpp //////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, February 2002
// Desc:   Keeps track of experience points so Veterance levels can be gained
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"


#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------------------------
ExperienceTracker::ExperienceTracker(Object *parent) :
	m_parent(parent),
	m_currentLevel(LEVEL_REGULAR),
	m_experienceSink(INVALID_ID),
	m_experienceScalar( 1.0f ),
	m_currentExperience(0) // Added By Sadullah Nader
{
}

//-------------------------------------------------------------------------------------------------
ExperienceTracker::~ExperienceTracker()
{
}

//-------------------------------------------------------------------------------------------------
Int ExperienceTracker::getExperienceValue( const Object* killer ) const
{
	// No experience for killing an ally, cheater.
	if( killer->getRelationship( m_parent ) == ALLIES )
		return 0;

	return m_parent->getTemplate()->getExperienceValue(m_currentLevel);
}

//-------------------------------------------------------------------------------------------------
Bool ExperienceTracker::isTrainable() const
{
	return m_parent->getTemplate()->isTrainable();
}

//-------------------------------------------------------------------------------------------------
Bool ExperienceTracker::isAcceptingExperiencePoints() const
{
	return isTrainable() || (m_experienceSink != INVALID_ID);
}

//-------------------------------------------------------------------------------------------------
void ExperienceTracker::setExperienceSink( ObjectID sink )
{
	m_experienceSink = sink;
}

//-------------------------------------------------------------------------------------------------
// Set Level to AT LEAST this... if we are already >= this level, do nothing.
void ExperienceTracker::setMinVeterancyLevel( VeterancyLevel newLevel )
{
	// This does not check for IsTrainable, because this function is for explicit setting,
	// so the setter is assumed to know what they are doing.  The game function
	// of addExperiencePoints cares about Trainability.
	if (m_currentLevel < newLevel)
	{
		VeterancyLevel oldLevel = m_currentLevel;
		m_currentLevel = newLevel;
		m_currentExperience = m_parent->getTemplate()->getExperienceRequired(m_currentLevel); //Minimum for this level
		if (m_parent)
			m_parent->onVeterancyLevelChanged( oldLevel, newLevel );
	}
}

//-------------------------------------------------------------------------------------------------
void ExperienceTracker::setVeterancyLevel( VeterancyLevel newLevel, Bool provideFeedback )
{
	// This does not check for IsTrainable, because this function is for explicit setting,
	// so the setter is assumed to know what they are doing.  The game function
	// of addExperiencePoints cares about Trainability, if flagged thus.
	if (m_currentLevel != newLevel)
	{
		VeterancyLevel oldLevel = m_currentLevel;
		m_currentLevel = newLevel;
		m_currentExperience = m_parent->getTemplate()->getExperienceRequired(m_currentLevel); //Minimum for this level
		if (m_parent)
			m_parent->onVeterancyLevelChanged( oldLevel, newLevel, provideFeedback );
	}
}

//-------------------------------------------------------------------------------------------------
Bool ExperienceTracker::gainExpForLevel(Int levelsToGain, Bool canScaleForBonus)
{
	Int newLevel = (Int)m_currentLevel + levelsToGain;
	if (newLevel > LEVEL_LAST)
		newLevel = LEVEL_LAST;
	// gain what levels we can, even if we can't use 'em all
	if (newLevel > m_currentLevel)
	{
		Int experienceNeeded = m_parent->getTemplate()->getExperienceRequired(newLevel) - m_currentExperience;
		addExperiencePoints( experienceNeeded, canScaleForBonus );
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
Bool ExperienceTracker::canGainExpForLevel(Int levelsToGain) const
{
	Int newLevel = (Int)m_currentLevel + levelsToGain;
	// return true if we can gain levels, even if we can't gain ALL the levels requested
	if (newLevel > LEVEL_LAST)
		newLevel = LEVEL_LAST;
	return (newLevel > m_currentLevel);
}

//-------------------------------------------------------------------------------------------------
void ExperienceTracker::addExperiencePoints( Int experienceGain, Bool canScaleForBonus)
{
	if( m_experienceSink != INVALID_ID )
	{
		// I have been set up to give my experience to someone else
		Object *sinkPointer = TheGameLogic->findObjectByID( m_experienceSink );
		if( sinkPointer )
		{
			// Not a fatal failure if not valid, he died when I was in the air.
			sinkPointer->getExperienceTracker()->addExperiencePoints( experienceGain * m_experienceScalar, canScaleForBonus );
			return;
		}
	}

	if( !isTrainable() )
		return; //safety

	VeterancyLevel oldLevel = m_currentLevel;

	Int amountToGain = experienceGain;
	if ( canScaleForBonus )
		amountToGain *= m_experienceScalar;


	m_currentExperience += amountToGain;

	Int levelIndex = 0;
	while( ( (levelIndex + 1) < LEVEL_COUNT) 
		&&  m_currentExperience >= m_parent->getTemplate()->getExperienceRequired(levelIndex + 1) 
		)
	{
		// If there is a higher level to qualify for, and I qualify for it, advance the index
		levelIndex++;
	}

	m_currentLevel = (VeterancyLevel)levelIndex;

	if( oldLevel != m_currentLevel )
	{
		// Edge trigger special level gain effects.
		m_parent->onVeterancyLevelChanged( oldLevel, m_currentLevel );
	}

}
//-------------------------------------------------------------------------------------------------
void ExperienceTracker::setExperienceAndLevel( Int experienceIn, Bool provideFeedback )
{
	if( m_experienceSink != INVALID_ID )
	{
		// I have been set up to give my experience to someone else
		Object *sinkPointer = TheGameLogic->findObjectByID( m_experienceSink );
		if( sinkPointer )
		{
			// Not a fatal failure if not valid, he died when I was in the air.
			sinkPointer->getExperienceTracker()->setExperienceAndLevel( experienceIn, provideFeedback );
			return;
		}
	}

	if( !isTrainable() )
		return; //safety

	VeterancyLevel oldLevel = m_currentLevel;

	m_currentExperience = experienceIn;

	Int levelIndex = 0;
	while( ( (levelIndex + 1) < LEVEL_COUNT) 
		&&  m_currentExperience >= m_parent->getTemplate()->getExperienceRequired(levelIndex + 1)
		)
	{
		// If there is a level to qualify for, and I qualify for it, advance the index
		levelIndex++;
	}

	m_currentLevel = (VeterancyLevel)levelIndex;

	if( oldLevel != m_currentLevel )
	{
		// Edge trigger special level gain effects.
		m_parent->onVeterancyLevelChanged( oldLevel, m_currentLevel, provideFeedback ); //<<== paradox! this may be a level lost!
	}

}

//-----------------------------------------------------------------------------
void ExperienceTracker::crc( Xfer *xfer )
{
	xfer->xferInt( &m_currentExperience );
	xfer->xferUser( &m_currentLevel, sizeof( VeterancyLevel ) );
}  // end crc

//-----------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version 
	*/
// ----------------------------------------------------------------------------
void ExperienceTracker::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// no need to save the m_parent pointer, it is connected on allocation time
	// m_parent

	// current level
	xfer->xferUser( &m_currentLevel, sizeof( VeterancyLevel ) );

	// current experience
	xfer->xferInt( &m_currentExperience );

	// experience sink
	xfer->xferObjectID( &m_experienceSink );

	// experience scalar
	xfer->xferReal( &m_experienceScalar );

}  // end xfer

//-----------------------------------------------------------------------------
void ExperienceTracker::loadPostProcess( void )
{

}  // end loadPostProcess

