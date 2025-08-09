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

/******************************************************************************\
sem4.cpp                        Neal Kettler

Simple Posix semaphore class
This is useful because the constructor will automatically call sem_init
  and you don't have to worry about it.  It also allows for other semaphore
  libraries if you don't have posix.
\******************************************************************************/

#include "sem4.h"

#ifdef _REENTRANT

Sem4::Sem4()
{
#ifndef _WINDOWS
  sem_init(&sem,1,1);
#else
  sem = CreateSemaphore(NULL, 1, 1, NULL);
#endif
}

Sem4::Sem4(uint32 value)
{
#ifndef _WINDOWS
  sem_init(&sem,1,value);
#else
  sem = CreateSemaphore(NULL, value, value, NULL);
#endif
}

Sem4::~Sem4()
{
#ifndef _WINDOWS
  sem_destroy(&sem);
#else
  if (sem) CloseHandle(sem);
#endif
}

sint32  Sem4::Wait(void) const
{
#ifndef _WINDOWS
	return(sem_wait((sem_t *)&sem));
#else
	if (!sem)
		return -1; // no semaphore!

	DWORD dwWaitResult = WaitForSingleObject(sem, INFINITE);
	switch (dwWaitResult) {
	case WAIT_OBJECT_0: // The semaphore object was signaled.
		return 0;
		break;
	case WAIT_TIMEOUT: // Should not happen ;)
		return -1;
		break;
	}
	return -1;
#endif
}

sint32 Sem4::Post(void) const
{
#ifndef _WINDOWS
  return(sem_post((sem_t *)&sem));
#else
  if (!sem)
	  return -1;
  if (!ReleaseSemaphore(sem, 1 ,NULL))
	  return -1;
  return 0;
#endif
}

sint32 Sem4::TryWait(void) const
{
#ifndef _WINDOWS
  return(sem_trywait((sem_t *)&sem));
#else
	if (!sem)
		return -1;
	DWORD dwWaitResult = WaitForSingleObject(sem, 0L);
	switch (dwWaitResult) {
	case WAIT_OBJECT_0: // The semaphore object was signaled.
		return 0;
		break;
	case WAIT_TIMEOUT:
		return -1;
		break;
	}
	return -1;
#endif
}

sint32 Sem4::GetValue(int *sval) const
{
#ifndef _WINDOWS
  return(sem_getvalue((sem_t *)&sem,sval));
#else
	if (!sem)
		return -1;
	long prev;
	if (!ReleaseSemaphore(sem, 0, &prev))
		return -1;
	if (sval)
		*sval = prev;
	return 0;
#endif
}

sint32 Sem4::Destroy(void)
{
#ifndef _WINDOWS
  return(sem_destroy(&sem));
#else
  return CloseHandle(sem);
#endif
}

#else

/****************************************************************************
non threaded versions that do nothing
*****************************************************************************/

Sem4::Sem4()
{
}

Sem4::Sem4(uint32)
{
}

Sem4::~Sem4()
{
}

sint32  Sem4::Wait(void) const
{
  return(0);
}

sint32 Sem4::Post(void) const
{
  return(0);
}

sint32 Sem4::TryWait(void) const
{
  return(0);
}

sint32 Sem4::GetValue(int *) const
{
  return(0);
}

sint32 Sem4::Destroy(void)
{
  return(0);
}

#endif

