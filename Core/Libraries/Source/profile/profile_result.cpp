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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/profile_result.cpp $
// $Author: mhoffe $
// $Revision: #2 $
// $DateTime: 2003/08/12 15:05:00 $
//
// (c) 2003 Electronic Arts
//
// Result function interface and result functions
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"
#include <new>
#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////
// ProfileResultFileCSV

ProfileResultInterface *ProfileResultFileCSV::Create(int, const char * const *)
{
  return new (ProfileAllocMemory(sizeof(ProfileResultFileCSV))) ProfileResultFileCSV();
}

void ProfileResultFileCSV::WriteThread(ProfileFuncLevel::Thread &thread)
{
  char help[40];

  sprintf(help,"prof%08x-all.csv",thread.GetId());
  FILE *f=fopen(help,"wt");

  // CSV file header
  fprintf(f,"Function\tFile\tCall count\tPTT (all)\tGTT (all)\tPT/C (all)\tGT/C (all)\tCaller (all)");
  unsigned k=0;
  for (;k<Profile::GetFrameCount();k++)
  {
    const char *s=Profile::GetFrameName(k);
    fprintf(f,"\tCall (%s)\tPTT (%s)\tGTT (%s)\tPT/C (%s)\tGT/C (%s)\tCaller (%s)",s,s,s,s,s,s);
  }
  fprintf(f,"\n");

  // now show all profile IDs (functions)
  ProfileFuncLevel::Id id;
  for (k=0;thread.EnumProfile(k,id);k++)
  {
    fprintf(f,"%s[%08x]\t%s, %i",id.GetFunction(),id.GetAddress(),
                                   id.GetSource(),id.GetLine());

    for (unsigned i=ProfileFuncLevel::Id::Total;i!=Profile::GetFrameCount();i++)
    {
      if (!id.GetCalls(i))
      {
        // early skip...
        fprintf(f,"\t\t\t\t\t\t");
        continue;
      }

      // call count
      fprintf(f,"\t%I64i",id.GetCalls(i));

      // pure total time
      fprintf(f,"\t%I64i",id.GetFunctionTime(i));

      // global total time
      fprintf(f,"\t%I64i",id.GetTime(i));

      // pure time per call
      fprintf(f,"\t%I64i",id.GetFunctionTime(i)/id.GetCalls(i));

      // global time per call
      fprintf(f,"\t%I64i",id.GetTime(i)/id.GetCalls(i));

      // list of callers
      ProfileFuncLevel::IdList idlist=id.GetCaller(i);
      fprintf(f,"\t");
      ProfileFuncLevel::Id callid;
      unsigned count;
      for (unsigned j=0;idlist.Enum(j,callid,&count);j++)
        fprintf(f," %s[%08x](%i)",callid.GetFunction(),callid.GetAddress(),count);
    }
    fprintf(f,"\n");
  }

  fclose(f);
}

void ProfileResultFileCSV::WriteResults(void)
{
  ProfileFuncLevel::Thread t;
  unsigned k=0;
  for (;ProfileFuncLevel::EnumThreads(k,t);k++)
    WriteThread(t);

  FILE *f=fopen("profile-high.csv","wt");

  // CSV file header
  fprintf(f,"Profile\tUnit\ttotal");
  for (k=0;k<Profile::GetFrameCount();k++)
    fprintf(f,"\t%s",Profile::GetFrameName(k));
  fprintf(f,"\n");

  // now show all high level profile IDs
  ProfileHighLevel::Id id;
  for (k=0;ProfileHighLevel::EnumProfile(k,id);k++)
  {
    fprintf(f,"%s\t%s\t%s",id.GetName(),id.GetUnit(),id.GetTotalValue());
    for (unsigned i=0;i<Profile::GetFrameCount();i++)
    {
      const char *p=id.GetValue(i);
      fprintf(f,"\t%s",p?p:"");
    }
    fprintf(f,"\n");
  }

  fclose(f);
}

void ProfileResultFileCSV::Delete(void)
{
  this->~ProfileResultFileCSV();
  ProfileFreeMemory(this);
}

//////////////////////////////////////////////////////////////////////////////
// ProfileResultFileDOT

ProfileResultInterface *ProfileResultFileDOT::Create(int argn, const char * const *argv)
{
  return new (ProfileAllocMemory(sizeof(ProfileResultFileDOT)))
    ProfileResultFileDOT(argn>0?argv[0]:NULL,
                         argn>1?argv[1]:NULL,
                         argn>2?atoi(argv[2]):NULL);
}

ProfileResultFileDOT::ProfileResultFileDOT(const char *fileName, const char *frameName, int foldThreshold)
{
  if (!fileName)
    fileName="profile.dot";
  m_fileName=(char *)ProfileAllocMemory(strlen(fileName)+1);
  strcpy(m_fileName,fileName);
  if (frameName)
  {
    m_frameName=(char *)ProfileAllocMemory(strlen(frameName)+1);
    strcpy(m_frameName,frameName);
  }
  else
    m_frameName=NULL;
  m_foldThreshold=foldThreshold;
}

void ProfileResultFileDOT::WriteResults(void)
{
  // search "main" thread
  ProfileFuncLevel::Thread t,tMax;
  if (!ProfileFuncLevel::EnumThreads(0,tMax))
    return;

  unsigned curMax=0;
  unsigned k=1;
  for (;ProfileFuncLevel::EnumThreads(k,t);k++)
  {
    for (;curMax++;)
    {
      ProfileFuncLevel::Id help;
      if (!tMax.EnumProfile(curMax,help))
      {
        tMax=t;
        break;
      }
      if (!t.EnumProfile(curMax,help))
        break;
      curMax++;
    }
  }

  // search frame
  unsigned frame=ProfileFuncLevel::Id::Total;
  if (m_frameName)
  {
    for (unsigned k=0;k<Profile::GetFrameCount();k++)
      if (!strcmp(Profile::GetFrameName(k),m_frameName))
      {
        frame=k;
        break;
      }
  }

  // determine number of active functions
  unsigned active=0;
  ProfileFuncLevel::Id id;
  for (k=0;tMax.EnumProfile(k,id);k++)
    if (id.GetCalls(frame))
      active++;

  FILE *f=fopen(m_fileName,"wt");
  if (!f)
    return;

  // DOT header
  fprintf(f,"digraph G { rankdir=\"LR\";\n");
  fprintf(f,"node [shape=box, fontname=Arial]\n");
  fprintf(f,"edge [arrowhead=%s, labelfontname=Arial, labelfontsize=10, labelangle=0, labelfontcolor=blue]\n",
    active>m_foldThreshold?"closed":"none");

  // fold or not?
  if (active>m_foldThreshold)
  {
    // folding version

    // build source code clusters first
    FoldHelper *fold=NULL;
    for (k=0;tMax.EnumProfile(k,id);k++)
    {
      const char *source=id.GetSource();
      FoldHelper *cur=fold;
      for (;cur;cur=cur->next)
        if (!strcmp(source,cur->source))
        {
          if (cur->numId<MAX_FUNCTIONS_PER_FILE)
            cur->id[cur->numId++]=id;
          break;
        }
      if (!cur)
      {
        cur=(FoldHelper *)ProfileAllocMemory(sizeof(FoldHelper));
        cur->next=fold;
        fold=cur;
        cur->source=source;
        cur->numId=1;
        cur->id[0]=id;
      }
    }

    // now write data
    for (FoldHelper *cur=fold;cur;cur=cur->next)
    {
      FoldHelper *cur2=fold;
      for (;cur2;cur2=cur2->next)
        cur2->mark=false;

      for (k=0;k<cur->numId;k++)
      {
        ProfileFuncLevel::IdList idlist=id.GetCaller(frame);
        ProfileFuncLevel::Id caller;
        for (unsigned i=0;idlist.Enum(i,caller);i++)
        {
          const char *s=caller.GetSource();
          for (FoldHelper *cur2=fold;cur2;cur2=cur2->next)
            if (!strcmp(cur2->source,s))
              break;
          if (!cur2||cur2->mark)
            continue;
          cur2->mark=true;

          fprintf(f,"\"%s\" -> \"%s\"\n",s,cur->source);
        }
      }
    }

    // cleanup
    while (fold)
    {
      FoldHelper *next=fold->next;
      ProfileFreeMemory(fold);
      fold=next;
    }
  }
  else
  {
    // non-folding version
    for (k=0;tMax.EnumProfile(k,id);k++)
      if (id.GetCalls(frame))
        fprintf(f,"f%08x [label=\"%s\"]\n",id.GetAddress(),id.GetFunction());
    for (k=0;tMax.EnumProfile(k,id);k++)
    {
      ProfileFuncLevel::IdList idlist=id.GetCaller(frame);
      ProfileFuncLevel::Id caller;
      unsigned count;
      for (unsigned i=0;idlist.Enum(i,caller,&count);i++)
        fprintf(f,"f%08x -> f%08x [headlabel=\"%i\"];\n",caller.GetAddress(),id.GetAddress(),count);
    }
  }

  fprintf(f,"}\n");
  fclose(f);
}

void ProfileResultFileDOT::Delete(void)
{
  this->~ProfileResultFileDOT();
  ProfileFreeMemory(this);
}
