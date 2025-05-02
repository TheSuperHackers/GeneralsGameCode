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
#include "PreRTS.h"
#include "GameClient/ClientInstance.h"

#define GENERALS_GUID "685EAFF2-3216-4265-B047-251C5F4B82F3"

namespace rts
{
HANDLE ClientInstance::s_mutexHandle = NULL;
UnsignedInt ClientInstance::s_instanceIndex = 0;

bool ClientInstance::initialize()
{
	if (isInitialized())
	{
		return true;
	}

	// Create a mutex with a unique name to Generals in order to determine if our app is already running.
	// WARNING: DO NOT use this number for any other application except Generals.
	while (true)
	{
#ifdef RTS_MULTI_INSTANCE
		std::string guidStr = getFirstInstanceName();
		if (s_instanceIndex > 0u)
		{
			char idStr[33];
			itoa(s_instanceIndex, idStr, 10);
			guidStr.push_back('-');
			guidStr.append(idStr);
		}
		s_mutexHandle = CreateMutex(NULL, FALSE, guidStr.c_str());
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			if (s_mutexHandle != NULL)
			{
				CloseHandle(s_mutexHandle);
				s_mutexHandle = NULL;
			}
			// Try again with a new instance.
			++s_instanceIndex;
			continue;
		}
#else
		s_mutexHandle = CreateMutex(NULL, FALSE, getFirstInstanceName());
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			if (s_mutexHandle != NULL)
			{
				CloseHandle(s_mutexHandle);
				s_mutexHandle = NULL;
			}
			return false;
		}
#endif
		break;
	}

	return true;
}

bool ClientInstance::isInitialized()
{
	return s_mutexHandle != NULL;
}

UnsignedInt ClientInstance::getInstanceIndex()
{
	DEBUG_ASSERTLOG(!isInitialized(), ("ClientInstance::isInitialized() failed"));
	return s_instanceIndex;
}

UnsignedInt ClientInstance::getInstanceId()
{
	return getInstanceIndex() + 1;
}

const char* ClientInstance::getFirstInstanceName()
{
	return GENERALS_GUID;
}

} // namespace rts
