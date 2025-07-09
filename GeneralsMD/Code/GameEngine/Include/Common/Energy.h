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

// FILE: Energy.h ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					         
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  Energy.h
//
// Created:    Steven Johnson, October 2001
//
// Desc:			 @todo
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef _ENERGY_H_
#define _ENERGY_H_

// INLCUDES /////////////////////////////////////////////////////////////////////////////////////
#include "Common/Snapshot.h"

// ----------------------------------------------------------------------------------------------

class Player;
class Object;

// ----------------------------------------------------------------------------------------------
/**
	This class is used to encapsulate the Player's energy use and production.
	for consistent nomenclature, we'll arbitrarily call energy units "kilowatts"
	(though that may have no bearing on reality).
*/
class Energy : public Snapshot
{

public:
	
	Energy();

	// reset energy information to base values.
	void init( Player *owner)
	{
		m_energyProduction = 0;
		m_energyConsumption = 0;
		m_powerSabotagedTillFrame = 0;
		m_owner = owner;
	}

	/// return current energy production in kilowatts
	Int getProduction() const;

	/// return current energy consumption in kilowatts
	Int getConsumption() const { return m_energyConsumption; }

	Bool hasSufficientPower(void) const;
	
	// If adding is false, we're supposed to be removing this.
	void adjustPower(Int powerDelta, Bool adding);

	/// new 'obj' will now add/subtract from this energy construct
	void objectEnteringInfluence( Object *obj );

	/// 'obj' will now no longer add/subtrack from this energy construct
	void objectLeavingInfluence( Object *obj );

	/** Adds an energy bonus to the player's pool if the power bonus status bit is set */
	void addPowerBonus( Object *obj );
	void removePowerBonus( Object *obj );

	void setPowerSabotagedTillFrame( UnsignedInt frame ) { m_powerSabotagedTillFrame = frame; }
	UnsignedInt getPowerSabotagedTillFrame() const { return m_powerSabotagedTillFrame; }

	/**
		return the percentage of energy needed that we actually produce, as a 0.0 ... 1.0 fraction.
	*/
	Real getEnergySupplyRatio() const;

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	void addProduction(Int amt);
	void addConsumption(Int amt);

private:

	Int		m_energyProduction;		///< level of energy production, in kw
	Int		m_energyConsumption;	///< level of energy consumption, in kw
	UnsignedInt m_powerSabotagedTillFrame; ///< If power is sabotaged, the frame will be greater than now.
	Player *m_owner;						///< Tight pointer to the Player I am intrinsic to.
};

#endif // _ENERGY_H_

