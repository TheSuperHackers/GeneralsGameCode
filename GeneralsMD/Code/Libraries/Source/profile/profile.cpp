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

/////////////////////////////////////////////////////////////////////////EA-V1
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/profile.cpp $
// $Author: mhoffe $
// $Revision: #6 $
// $DateTime: 2003/08/14 13:43:29 $
//
// (c) 2003 Electronic Arts
//
// Profile module main code
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include <new>
#include "mmsystem.h"

// yuk, I'm doing this so weird because the destructor
// of cmd must never be called...
static ProfileCmdInterface &cmd=*(ProfileCmdInterface *)new 
                  (ProfileAllocMemory(sizeof(ProfileCmdInterface))) ProfileCmdInterface();

// we have this here so that our command interface will always
// be linked in as well...
static bool __RegisterDebugCmdGroup_Profile=Debug::AddCommands("profile",&cmd);

void *ProfileAllocMemory(unsigned numBytes)
{
  HGLOBAL h=GlobalAlloc(GMEM_FIXED,numBytes);
  if (!h)
    DCRASH_RELEASE("Debug mem alloc failed");
  return (void *)h;
}

void *ProfileReAllocMemory(void *oldPtr, unsigned newSize)
{
  // Windows doesn't like ReAlloc with NULL handle/ptr...
  if (!oldPtr)
    return newSize?ProfileAllocMemory(newSize):0;

  // Shrinking to 0 size is basically freeing memory
  if (!newSize)
  {
    GlobalFree((HGLOBAL)oldPtr);
    return 0;
  }

  // now try GlobalReAlloc first
  HGLOBAL h=GlobalReAlloc((HGLOBAL)oldPtr,newSize,0);
  if (!h)
  {
    // this failed (Windows doesn't like ReAlloc'ing larger
    // fixed memory blocks) - go with Alloc/Free instead
    h=GlobalAlloc(GMEM_FIXED,newSize);
    if (!h)
      DCRASH_RELEASE("Debug mem realloc failed");
    unsigned oldSize=GlobalSize((HGLOBAL)oldPtr);
    memcpy((void *)h,oldPtr,oldSize<newSize?oldSize:newSize);
    GlobalFree((HGLOBAL)oldPtr);
  }

  return (void *)h;
}

void ProfileFreeMemory(void *ptr)
{
  if (ptr)
    GlobalFree((HGLOBAL)ptr);
}

//////////////////////////////////////////////////////////////////////////////

static _int64 GetClockCyclesFast(void)
{
  // this is where we're adding our internal result functions
  Profile::AddResultFunction(ProfileResultFileCSV::Create,
                              "file_csv",
                              "");
  Profile::AddResultFunction(ProfileResultFileCSV::Create,
                              "file_dot",
                              "[ file [ frame_name [ fold_threshold ] ] ]");
  
  // this must not take a very huge CPU hit...
  
  // measure clock cycles 3 times for 20 msec each
  // then take the 2 counts that are closest, average
  _int64 n[3];
  for (int k=0;k<3;k++)
  {
    // wait for end of current tick
    unsigned timeEnd=timeGetTime()+2;
    while (timeGetTime()<timeEnd);

    // get cycles
    _int64 start,startQPC,endQPC;
    QueryPerformanceCounter((LARGE_INTEGER *)&startQPC);
    ProfileGetTime(start);
    timeEnd+=20;
    while (timeGetTime()<timeEnd);
    ProfileGetTime(n[k]);
    n[k]-=start;

    // convert to 1 second
    if (QueryPerformanceCounter((LARGE_INTEGER *)&endQPC))
    {
      _int64 freq;
      QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
      n[k]=(n[k]*freq)/(endQPC-startQPC);
    }
    else
    {
      n[k]=(n[k]*1000)/20;
    }
  }

  // find two closest values
  _int64 d01=n[1]-n[0],d02=n[2]-n[0],d12=n[2]-n[1];
  if (d01<0) d01=-d01;
  if (d02<0) d02=-d02;
  if (d12<0) d12=-d12;
  _int64 avg;
  if (d01<d02)
  {
    avg=d01<d12?n[0]+n[1]:n[1]+n[2];
  }
  else
  {
    avg=d02<d12?n[0]+n[2]:n[1]+n[2];
  }

  // return result
  // (rounded to the next MHz)
  return ((avg/2+500000)/1000000)*1000000;
}

unsigned Profile::m_rec;
char **Profile::m_recNames;
unsigned Profile::m_names;
Profile::FrameName *Profile::m_frameNames;
_int64 Profile::m_clockCycles=GetClockCyclesFast();
Profile::PatternListEntry *Profile::firstPatternEntry;
Profile::PatternListEntry *Profile::lastPatternEntry;

void Profile::StartRange(const char *range)
{
  // set default
  if (!range)
    range="frame";

  // known name?
  unsigned k=0;
  for (;k<m_names;++k)
    if (!strcmp(range,m_frameNames[k].name))
      break;
  if (k==m_names)
  {
    // no, must add to list
    m_frameNames=(FrameName *)ProfileReAllocMemory(m_frameNames,(++m_names)*sizeof(FrameName));
    m_frameNames[k].name=(char *)ProfileAllocMemory(strlen(range)+1);
    strcpy(m_frameNames[k].name,range);
    m_frameNames[k].frames=0;
    m_frameNames[k].isRecording=false;
    m_frameNames[k].doAppend=false;
    m_frameNames[k].lastGlobalIndex=-1;
  }

  // stop old recording?
  if (m_frameNames[k].isRecording)
    StopRange(range);

  // start new recording
  m_frameNames[k].isRecording=true;
  m_frameNames[k].doAppend=false;

  // but check first: is recording enabled?
  bool active=false;
  for (PatternListEntry *cur=firstPatternEntry;cur;cur=cur->next)
  {
    if (SimpleMatch(range,cur->pattern))
      active=cur->isActive;
  }

  if (active)
  {
#ifdef RTS_PROFILE
    m_frameNames[k].funcIndex=ProfileFuncLevelTracer::FrameStart();
    DASSERT(m_frameNames[k].funcIndex>=0);
#endif
    m_frameNames[k].highIndex=ProfileId::FrameStart();
    DASSERT(m_frameNames[k].highIndex>=0);
  }
  else
  {
    m_frameNames[k].funcIndex=-1;
    m_frameNames[k].highIndex=-1;
  }
}

void Profile::AppendRange(const char *range)
{
  // set default
  if (!range)
    range="frame";

  // known name?
  unsigned k=0;
  for (;k<m_names;++k)
    if (!strcmp(range,m_frameNames[k].name))
      break;
  if (k==m_names)
  {
    // no, so StartRange will do the job for us
    StartRange(range);
    return;
  }

  // still recording?
  if (m_frameNames[k].isRecording)
    // don't do anything
    return;

  // start new recording
  m_frameNames[k].isRecording=true;
  m_frameNames[k].doAppend=true;

  // but check first: is recording enabled?
  bool active=false;
  for (PatternListEntry *cur=firstPatternEntry;cur;cur=cur->next)
  {
    if (SimpleMatch(range,cur->pattern))
      active=cur->isActive;
  }

  if (active)
  {
#ifdef RTS_PROFILE
    m_frameNames[k].funcIndex=ProfileFuncLevelTracer::FrameStart();
    DASSERT(m_frameNames[k].funcIndex>=0);
#endif
    m_frameNames[k].highIndex=ProfileId::FrameStart();
    DASSERT(m_frameNames[k].highIndex>=0);
  }
  else
  {
    m_frameNames[k].funcIndex=-1;
    m_frameNames[k].highIndex=-1;
  }
}

void Profile::StopRange(const char *range)
{
  // set default
  if (!range)
    range="frame";

  // known name?
  unsigned k=0;
  for (;k<m_names;++k)
    if (!strcmp(range,m_frameNames[k].name))
      break;
  DFAIL_IF(k==m_names) return;
  DFAIL_IF(!m_frameNames[k].isRecording) return;

  // stop recording
  m_frameNames[k].isRecording=false;
  if (
#ifdef RTS_PROFILE
    m_frameNames[k].funcIndex>=0 ||
#endif
    m_frameNames[k].highIndex>=0
    )
  {
    // add to list of known frames?
    int atIndex;
    if (!m_frameNames[k].doAppend||
        m_frameNames[k].lastGlobalIndex<0)
    {
      atIndex=-1;
      m_frameNames[k].lastGlobalIndex=m_rec;
      m_recNames=(char **)ProfileReAllocMemory(m_recNames,(m_rec+1)*sizeof(char *));
      m_recNames[m_rec]=(char *)ProfileAllocMemory(strlen(range)+1+6);
      wsprintf(m_recNames[m_rec++],"%s:%i",range,++m_frameNames[k].frames);
    }
    else
      atIndex=m_frameNames[k].lastGlobalIndex;
#ifdef RTS_PROFILE
    if (m_frameNames[k].funcIndex>=0)
      ProfileFuncLevelTracer::FrameEnd(m_frameNames[k].funcIndex,atIndex);
    if (m_frameNames[k].highIndex>=0)
#endif
      ProfileId::FrameEnd(m_frameNames[k].highIndex,atIndex);
  }
}

bool Profile::IsEnabled(void)
{
  for (unsigned k=0;k<m_names;++k)
    if (m_frameNames[k].isRecording)
      return true;
  return false;
}

unsigned Profile::GetFrameCount(void)
{
  return m_rec;
}

const char *Profile::GetFrameName(unsigned frame)
{
  return frame>=m_rec?NULL:m_recNames[frame];
}

void Profile::ClearTotals(void)
{
#ifdef RTS_PROFILE
  ProfileFuncLevelTracer::ClearTotals();
#endif
  ProfileId::ClearTotals();
}

_int64 Profile::GetClockCyclesPerSecond(void)
{
  return m_clockCycles;
}

void Profile::AddResultFunction(ProfileResultInterface* (*func)(int, const char * const *),
                                const char *name, const char *arg)
{
  ProfileCmdInterface::AddResultFunction(func,name,arg);
}

bool Profile::SimpleMatch(const char *str, const char *pattern)
{
  DASSERT(str);
  DASSERT(pattern);
  while (*str&&*pattern)
  {
    if (*pattern=='*')
    {
      pattern++;
      while (*str)
        if (SimpleMatch(str++,pattern))
          return true;
      return *str==*pattern;
    }
    else 
    {
      if (*str++!=*pattern++)
        return false;
    }
  }

  return *str==*pattern;
}

static void ProfileShutdown(void)
{
#ifdef RTS_PROFILE
  ProfileFuncLevelTracer::Shutdown();
#endif
  ProfileId::Shutdown();

  DLOG("CPU speed is " << unsigned(Profile::GetClockCyclesPerSecond()) << " Hz.\n");

  cmd.RunResultFunctions();
}

int profileTracerInit=atexit(ProfileShutdown);
