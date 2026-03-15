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

// FILE: DynamicAudioEventInfo.h /////////////////////////////////////////////////////////////////////////
// Derivation of AudioEventInfo structure, for customized sounds
// Author: Ian Barkley-Yeung, June 2003

#pragma once

#include "Common/AudioEventInfo.h"
#include "Common/BitFlags.h"

class AsciiString;
class Xfer;

/*****************************************************************************
 * Derivation of AudioEventInfo structure, for customized sounds
 *****************************************************************************/
class DynamicAudioEventInfo : public AudioEventInfo
{
    // NOTE: This implementation would be a lot cleaner & safer if AudioEventInfo was better
    // written. Ideally, AudioEventInfo would be a class, not a struct, and provide only
    // "get" functions, not "set" functions except for the INI parsing. Then we could
    // force people to go through our override...() functions.

    MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DynamicAudioEventInfo, "DynamicAudioEventInfo" )

  public:
    DynamicAudioEventInfo();
    explicit DynamicAudioEventInfo( const AudioEventInfo & baseInfo );

    // DynamicAudioEventInfo interfacing function overrides
    virtual Bool isLevelSpecific() const;
    virtual DynamicAudioEventInfo * getDynamicAudioEventInfo();
    virtual const DynamicAudioEventInfo * getDynamicAudioEventInfo() const;

    // Change various fields from their default (INI) values
    void overrideAudioName( const AsciiString & newName );
    void overrideLoopFlag( Bool newLoopFlag );
    void overrideLoopCount( Int newLoopCount );
    void overrideVolume( Real newVolume );
    void overrideMinVolume( Real newMinVolume );
    void overrideMinRange( Real newMinRange );
    void overrideMaxRange( Real newMaxRange );
    void overridePriority( AudioPriority newPriority );

    // Query fields to see if they have been changed from their INI values
    Bool wasAudioNameOverriden() const;
    Bool wasLoopFlagOverriden() const;
    Bool wasLoopCountOverriden() const;
    Bool wasVolumeOverriden() const;
    Bool wasMinVolumeOverriden() const;
    Bool wasMinRangeOverriden() const;
    Bool wasMaxRangeOverriden() const;
    Bool wasPriorityOverriden() const;

    // Get the name of the audio event which this was based off of
    const AsciiString & getOriginalName() const;

    // Transfer all overridden fields except the customized name
    void xferNoName( Xfer * xfer );

  private:
    // List of fields we can override
    enum OverriddenFields CPP_11(: Int)
    {
      OVERRIDE_NAME = 0,
      OVERRIDE_LOOP_FLAG,
      OVERRIDE_LOOP_COUNT,
      OVERRIDE_VOLUME,
      OVERRIDE_MIN_VOLUME,
      OVERRIDE_MIN_RANGE,
      OVERRIDE_MAX_RANGE,
      OVERRIDE_PRIORITY,

      OVERRIDE_COUNT  // Keep list
    };
    // Warning: update xferNoName if you modify the enum list!

    BitFlags< OVERRIDE_COUNT > m_overriddenFields;

    // Retain the original name so we can look it up later
    AsciiString m_originalName;
};

/** Query: was overrideAudioName called? */
inline Bool DynamicAudioEventInfo::wasAudioNameOverriden() const
{
  return m_overriddenFields.test( OVERRIDE_NAME );
}

/** Query: was overrideLoopFlag called? */
inline Bool DynamicAudioEventInfo::wasLoopFlagOverriden() const
{
  return m_overriddenFields.test( OVERRIDE_LOOP_FLAG );
}

/** Query: was overrideLoopCount called? */
inline Bool DynamicAudioEventInfo::wasLoopCountOverriden() const
{
  return m_overriddenFields.test( OVERRIDE_LOOP_COUNT );
}

/** Query: was overrideVolume called? */
inline Bool DynamicAudioEventInfo::wasVolumeOverriden() const
{
  return m_overriddenFields.test( OVERRIDE_VOLUME );
}

/** Query: was overrideMinVolume called? */
inline Bool DynamicAudioEventInfo::wasMinVolumeOverriden() const
{
  return m_overriddenFields.test( OVERRIDE_MIN_VOLUME );
}

/** Query: was overrideMinRange called? */
inline Bool DynamicAudioEventInfo::wasMinRangeOverriden() const
{
  return m_overriddenFields.test( OVERRIDE_MIN_RANGE );
}

/** Query: was overrideMaxRange called? */
inline Bool DynamicAudioEventInfo::wasMaxRangeOverriden() const
{
  return m_overriddenFields.test( OVERRIDE_MAX_RANGE );
}

/** Query: was overridePriority called? */
inline Bool DynamicAudioEventInfo::wasPriorityOverriden() const
{
  return m_overriddenFields.test( OVERRIDE_PRIORITY );
}
