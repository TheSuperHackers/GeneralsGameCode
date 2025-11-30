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

////////////////////////////////////////////////////////////////////////////////
// User class copy and comparisons

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameNetwork/User.h"

/**
 * Constructor.  Sets up the member variables with the appropriate values.
 */
User::User(UnicodeString name, UnsignedInt addr, UnsignedInt port) {
	m_name.set(name);
	m_ipaddr = addr;
	m_port = port;
}

/**
 * The assignment operator.
 */
User & User::operator= (const User *other)
{
	m_name = other->m_name;
	m_ipaddr = other->m_ipaddr;
	m_port = other->m_port;

	return *this;
}

/**
 * The equality operator.
 */
Bool User::operator== (const User *other)
{
	return (m_name.compare(other->m_name) == 0);
}

/**
 * The inequality operator.
 */
Bool User::operator!= (const User *other)
{
	return (m_name.compare(other->m_name) != 0);
}

/**
 * Set the name of this user.
 */
void User::setName(UnicodeString name) {
	m_name.set(name);
}
