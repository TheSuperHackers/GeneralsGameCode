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

// FILE: DynamicAudioEventInfo.cpp /////////////////////////////////////////////////////////////////////////
// Derivation of AudioEventInfo structure, for customized sounds
// Author: Ian Barkley-Yeung, June 2003

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/DynamicAudioEventInfo.h"
#include "Common/Xfer.h"

/** Default constructor */
DynamicAudioEventInfo::DynamicAudioEventInfo()
{
}

/** Initialize AudioEventInfo portion of DynamicAudioEventInfo as copy; leave remainder uninitialized */
DynamicAudioEventInfo::DynamicAudioEventInfo( const AudioEventInfo & baseInfo )
: AudioEventInfo( baseInfo )
{
}

DynamicAudioEventInfo::~DynamicAudioEventInfo()
{
}

/** Override; dynamic audio events are used only for level-specific stuff at the moment */
Bool DynamicAudioEventInfo::isLevelSpecific() const
{
  return true;
}

/** Override; cheap dynamic casting */
DynamicAudioEventInfo * DynamicAudioEventInfo::getDynamicAudioEventInfo()
{
  return this;
}

/** Override; cheap dynamic casting */
const DynamicAudioEventInfo * DynamicAudioEventInfo::getDynamicAudioEventInfo() const
{
  return this;
}


/** Override; change the name of this audio event*/
void DynamicAudioEventInfo::overrideAudioName( const AsciiString & newName )
{
  // Record new name. Needed later for load & save
  m_originalName = m_audioName;

  m_overriddenFields.set( OVERRIDE_NAME );

  m_audioName = newName;
}

/** Override; change the looping property of this audio event */
void DynamicAudioEventInfo::overrideLoopFlag( Bool newLoopFlag )
{
  m_overriddenFields.set( OVERRIDE_LOOP_FLAG );

  if ( newLoopFlag)
    BitSet( m_control, AC_LOOP );
  else
    BitClear( m_control, AC_LOOP );
}

/** Override; change the looping properties of this audio event*/
void DynamicAudioEventInfo::overrideLoopCount( Int newLoopCount )
{
  m_overriddenFields.set( OVERRIDE_LOOP_COUNT );

  m_loopCount = newLoopCount;
}

/** Override; change the volume of this audio event*/
void DynamicAudioEventInfo::overrideVolume( Real newVolume )
{
  m_overriddenFields.set( OVERRIDE_VOLUME );

  m_volume = newVolume;
}

/** Override; change the minimum volume of this audio event*/
void DynamicAudioEventInfo::overrideMinVolume( Real newMinVolume )
{
  m_overriddenFields.set( OVERRIDE_MIN_VOLUME );

  m_minVolume = newMinVolume;
}

/** Override; change the name of this audio event*/
void DynamicAudioEventInfo::overrideMinRange( Real newMinRange )
{
  m_overriddenFields.set( OVERRIDE_MIN_RANGE );

  m_minDistance = newMinRange;
}

/** Override; change the name of this audio event*/
void DynamicAudioEventInfo::overrideMaxRange( Real newMaxRange )
{
  m_overriddenFields.set( OVERRIDE_MAX_RANGE );

  m_maxDistance = newMaxRange;
}

/** Override; change the name of this audio event*/
void DynamicAudioEventInfo::overridePriority( AudioPriority newPriority )
{
  m_overriddenFields.set( OVERRIDE_PRIORITY );

  m_priority = newPriority;
}

/** Get the name of the INI entry this event info was derived from */
const AsciiString & DynamicAudioEventInfo::getOriginalName() const
{
  if ( wasAudioNameOverriden() )
  {
    return m_originalName;
  }
  else
  {
    return m_audioName;
  }
}

/** Transfer all overridden fields except the customized name */
void DynamicAudioEventInfo::xferNoName( Xfer * xfer )
{
  const XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

  // The default BitFlags xfer function is really strange. It also requires us to name our
  // bits, which I don't want to do, since it forces anyone who has the same number of bits
  // as us to use our names. Instead, just transfer directly and count on our local version
  // to keep things straight, If you change the list, be sure to change this code too...
  if ( xfer->getXferMode() == XFER_LOAD )
  {
    UnsignedByte overriddenFlags;
    DEBUG_ASSERTCRASH( OVERRIDE_COUNT <= sizeof( overriddenFlags ) * 8, ("Save/load code assumes override flags can fit into UnsignedByte, but it doesn't work anymore! Move up to larger integer type") );
    xfer->xferUnsignedByte( &overriddenFlags );
    Int field;
    for ( field = 0; field < OVERRIDE_COUNT; field++ )
    {
      m_overriddenFields.set( field, BitIsSet( overriddenFlags, 1 << field ) );
    }
  }
  else
  {
    UnsignedByte overriddenFlags = 0;
    DEBUG_ASSERTCRASH( OVERRIDE_COUNT <= sizeof( overriddenFlags ) * 8, ("Save/load code assumes override flags can fit into UnsignedByte, but it doesn't work anymore! Move up to larger integer type") );
    Int field;
    for ( field = 0; field < OVERRIDE_COUNT; field++ )
    {
      if ( m_overriddenFields.test( field ) )
      {
        BitSet( overriddenFlags, 1 << field );
      }
    }
    xfer->xferUnsignedByte( &overriddenFlags );
  }

  if ( wasLoopFlagOverriden() )
  {
    Bool loopFlag = BitIsSet( m_control, AC_LOOP );
    xfer->xferBool( &loopFlag );
    if ( loopFlag )
    {
      BitSet( m_control, AC_LOOP );
    }
    else
    {
      BitClear( m_control, AC_LOOP );
    }
  }

  if ( wasLoopCountOverriden() )
  {
    xfer->xferInt( &m_loopCount );
  }

  if ( wasVolumeOverriden() )
  {
    xfer->xferReal( &m_volume );
  }

  if ( wasMinVolumeOverriden() )
  {
    xfer->xferReal( &m_minVolume );
  }

  if ( wasMinRangeOverriden() )
  {
    xfer->xferReal( &m_minDistance );
  }

  if ( wasMaxRangeOverriden() )
  {
    xfer->xferReal( &m_maxDistance );
  }

  if ( wasPriorityOverriden() )
  {
    UnsignedByte priority = (UnsignedByte)m_priority;
    xfer->xferUnsignedByte( &priority );
    m_priority = (AudioPriority)priority;
  }
}
