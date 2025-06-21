/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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
#include "Lib/BaseType.h"

namespace rts
{

// TheSuperHackers @feature Adds support for launching multiple game clients and keeping track of their instance id.

class ClientInstance
{
public:
	// Can be called N times, but is initialized just once.
	static bool initialize();

	static bool isInitialized();

	static bool isMultiInstance();

	// Change multi instance on runtime. Must be called before initialize.
	static void setMultiInstance(bool v);

	// Skips using the primary instance. Must be called before initialize.
	// Useful when the new process is not meant to collide with another normal Generals process.
	static void skipPrimaryInstance();

	// Returns the instance index of this game client. Starts at 0.
	static UnsignedInt getInstanceIndex();

	// Returns the instance id of this game client. Starts at 1.
	static UnsignedInt getInstanceId();

	// Returns the instance name of the first game client.
	static const char* getFirstInstanceName();

private:
	static HANDLE s_mutexHandle;
	static UnsignedInt s_instanceIndex;
	static Bool s_isMultiInstance;
};

} // namespace rts
