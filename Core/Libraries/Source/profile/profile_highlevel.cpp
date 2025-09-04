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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/profile_highlevel.cpp $
// $Author: mhoffe $
// $Revision: #2 $
// $DateTime: 2003/08/14 13:43:29 $
//
// (c) 2003 Electronic Arts
//
// High level profiling
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include "stringex.h"
#include <new>
#include <Utility/stdio_adapter.h>

// our own fast critical section
static ProfileFastCS cs;

//////////////////////////////////////////////////////////////////////////////
// ProfileHighLevel::Id

void ProfileHighLevel::Id::Increment(double add)
{
  if (m_idPtr)
    m_idPtr->Increment(add);
}

void ProfileHighLevel::Id::SetMax(double max)
{
  if (m_idPtr)
    m_idPtr->Maximum(max);
}

const char *ProfileHighLevel::Id::GetName(void) const
{
  return m_idPtr?m_idPtr->GetName():NULL;
}

const char *ProfileHighLevel::Id::GetDescr(void) const
{
  return m_idPtr?m_idPtr->GetDescr():NULL;
}

const char *ProfileHighLevel::Id::GetUnit(void) const
{
  return m_idPtr?m_idPtr->GetUnit():NULL;
}

const char *ProfileHighLevel::Id::GetCurrentValue(void) const
{
  return m_idPtr?m_idPtr->AsString(m_idPtr->GetCurrentValue()):NULL;
}

const char *ProfileHighLevel::Id::GetValue(unsigned frame) const
{
  double v;
  if (!m_idPtr||!m_idPtr->GetFrameValue(frame,v))
    return NULL;
  return m_idPtr->AsString(v);
}

const char *ProfileHighLevel::Id::GetTotalValue(void) const
{
  return m_idPtr?m_idPtr->AsString(m_idPtr->GetTotalValue()):NULL;
}

//////////////////////////////////////////////////////////////////////////////
// ProfileHighLevel::Block

ProfileHighLevel::Block::Block(const char *name)
{
  DFAIL_IF(!name) return;

  m_idTime=AddProfile(name,NULL,"msec",6,-4);

  char help[256];
  strlcpy(help, name, sizeof(help) - 2);
  strcat(help,".c");
  AddProfile(help,NULL,"calls",6,0).Increment();

  ProfileGetTime(m_start);
}

ProfileHighLevel::Block::~Block()
{
  _int64 end;
  ProfileGetTime(end);
  end-=m_start;

  m_idTime.Increment(double(end)/(double)Profile::GetClockCyclesPerSecond());
}

//////////////////////////////////////////////////////////////////////////////
// ProfileId

// profile ID stuff
ProfileId *ProfileId::first;
int ProfileId::curFrame;
unsigned ProfileId::frameRecordMask;
char ProfileId::stringBuf[ProfileId::STRING_BUFFER_SIZE];
unsigned ProfileId::stringBufUnused;

ProfileId::ProfileId(const char *name, const char *descr, const char *unit, int precision, int exp10)
{
  m_next=first; first=this;
  m_name=(char *)ProfileAllocMemory(strlen(name)+1);
  strcpy(m_name,name);
  if (descr)
  {
    m_descr=(char *)ProfileAllocMemory(strlen(descr)+1);
    strcpy(m_descr,descr);
  }
  else
    m_descr=NULL;
  if (unit)
  {
    m_unit=(char *)ProfileAllocMemory(strlen(unit)+1);
    strcpy(m_unit,unit);
  }
  else
    m_unit=NULL;
  m_precision=precision;
  m_exp10=exp10;
  m_curVal=m_totalVal=0.;
  m_recFrameVal=NULL;
  m_firstFrame=curFrame;
  m_valueMode=Unknown;
}

void ProfileId::Increment(double add)
{
  DFAIL_IF(m_valueMode!=Unknown&&m_valueMode!=ModeIncrement)
    return;

  m_valueMode=ModeIncrement;
  m_curVal+=add;
  m_totalVal+=add;
  if (frameRecordMask)
  {
    unsigned mask=frameRecordMask;
    for (unsigned i=0;i<MAX_FRAME_RECORDS;i++)
    {
      if (mask&1)
        m_frameVal[i]+=add;
      if (!(mask>>=1))
        break;
    }
  }
}

void ProfileId::Maximum(double max)
{
  DFAIL_IF(m_valueMode!=Unknown&&m_valueMode!=ModeMaximum)
    return;

  m_valueMode=ModeMaximum;
  if (max>m_curVal)
    m_curVal=max;
  if (max>m_totalVal)
    m_totalVal=max;
  if (frameRecordMask)
  {
    unsigned mask=frameRecordMask;
    for (unsigned i=0;i<MAX_FRAME_RECORDS;i++)
    {
      if (mask&1)
      {
        if (max>m_frameVal[i])
          m_frameVal[i]=max;
      }
      if (!(mask>>=1))
        break;
    }
  }
}

const char *ProfileId::AsString(double v) const
{
  char help1[10],help[40];
  wsprintf(help1,"%%%i.lf",m_precision);

  double mul=1.0;
  int k;
  for (k=m_exp10;k<0;k++) mul*=10.0;
  for (;k>0;k--) mul/=10.0;

  unsigned len=snprintf(help,sizeof(help),help1,v*mul)+1;

  ProfileFastCS::Lock lock(cs);
  if (stringBufUnused+len>STRING_BUFFER_SIZE)
    stringBufUnused=0;
  char *ret=stringBuf+stringBufUnused;
  memcpy(ret,help,len);
  stringBufUnused+=len;
  return ret;
}

int ProfileId::FrameStart(void)
{
  ProfileFastCS::Lock lock(cs);

  unsigned i=0;
  for (;i<MAX_FRAME_RECORDS;i++)
    if (!(frameRecordMask&(1<<i)))
      break;
  if (i==MAX_FRAME_RECORDS)
    return -1;

  for (ProfileId *p=first;p;p=p->m_next)
    p->m_frameVal[i]=0.;

  frameRecordMask|=1<<i;
  return i;
}

void ProfileId::FrameEnd(int which, int mixIndex)
{
  DFAIL_IF(which<0||which>=MAX_FRAME_RECORDS)
    return;
  DFAIL_IF(!(frameRecordMask&(1<<which)))
    return;
  DFAIL_IF(mixIndex>=curFrame)
    return;

  ProfileFastCS::Lock lock(cs);

  frameRecordMask^=1<<which;
  if (mixIndex<0)
  {
    // new frame
    curFrame++;
    for (ProfileId *p=first;p;p=p->m_next)
    {
      p->m_recFrameVal=(double *)ProfileReAllocMemory(p->m_recFrameVal,sizeof(double)*(curFrame-p->m_firstFrame));
      p->m_recFrameVal[curFrame-p->m_firstFrame-1]=p->m_frameVal[which];
    }
  }
  else
  {
    // append data
    for (ProfileId *p=first;p;p=p->m_next)
    {
      if (p->m_firstFrame>mixIndex)
        continue;

      double &val=p->m_recFrameVal[mixIndex-p->m_firstFrame];
      switch(p->m_valueMode)
      {
        case ProfileId::Unknown:
          break;
        case ProfileId::ModeIncrement:
          val+=p->m_frameVal[which];
          break;
        case ProfileId::ModeMaximum:
          if (p->m_frameVal[which]>val)
            val=p->m_frameVal[which];
          break;
        default:
          DFAIL();
      }
    }
  }
}

void ProfileId::Shutdown(void)
{
  if (frameRecordMask)
  {
    for (unsigned i=0;i<MAX_FRAME_RECORDS;i++)
      if (frameRecordMask&(1<<i))
        FrameEnd(i,-1);
  }
}

//////////////////////////////////////////////////////////////////////////////
// ProfileHighLevel

ProfileHighLevel::Id ProfileHighLevel::AddProfile(const char *name, const char *descr, const char *unit, int precision, int exp10)
{
  // check if there is already an ID with the given name...
  Id id;
  if (FindProfile(name,id))
    return id;

  // checks...
  DFAIL_IF(!name) return id;

  // no, allocate one
  ProfileFastCS::Lock lock(cs);
  id.m_idPtr=new (ProfileAllocMemory(sizeof(ProfileId))) ProfileId(name,descr,unit,precision,exp10);
  return id;
}

bool ProfileHighLevel::EnumProfile(unsigned index, Id &id)
{
  ProfileFastCS::Lock lock(cs);
  ProfileId *cur=ProfileId::GetFirst();
  for (;cur&&index--;cur=cur->GetNext());
  id.m_idPtr=cur;
  return cur!=NULL;
}

bool ProfileHighLevel::FindProfile(const char *name, Id &id)
{
  DFAIL_IF(!name) return false;

  ProfileFastCS::Lock lock(cs);
  for (ProfileId *cur=ProfileId::GetFirst();cur;cur=cur->GetNext())
    if (!strcmp(name,cur->GetName()))
    {
      id.m_idPtr=cur;
      return true;
    }

  id.m_idPtr=NULL;
  return false;
}

ProfileHighLevel::ProfileHighLevel(void)
{
}

ProfileHighLevel ProfileHighLevel::Instance;
