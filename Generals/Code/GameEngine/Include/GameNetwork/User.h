/*
**	Command & Conquer Generals(tm)
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

/**
 * User class used by the network.
 */

#pragma once

#ifndef __USER_H
#define __USER_H

#include "GameNetwork/NetworkDefs.h"
#include "Common/UnicodeString.h"

class User : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(User, "User")
public:
	User() {}
	User(UnicodeString name, UnsignedInt addr, UnsignedInt port);
	User &operator= (const User *other);
	Bool operator== (const User *other);
	Bool operator!= (const User *other);

	inline UnicodeString GetName() { return m_name; }
	void setName(UnicodeString name);
	inline UnsignedShort GetPort() { return m_port; }
	inline UnsignedInt GetIPAddr() { return m_ipaddr; }
	inline void SetPort(UnsignedShort port) { m_port = port; }
	inline void SetIPAddr(UnsignedInt ipaddr) { m_ipaddr = ipaddr; }


private:
	UnicodeString m_name;
	UnsignedShort m_port;
	UnsignedInt m_ipaddr;
};
EMPTY_DTOR(User)

#endif
