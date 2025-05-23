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
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/profile/profile_cmd.cpp $
// $Author: mhoffe $
// $Revision: #2 $
// $DateTime: 2003/08/12 15:05:00 $
//
// (c) 2003 Electronic Arts
//
// Profile module command interface
//////////////////////////////////////////////////////////////////////////////
#include "_pch.h"

unsigned ProfileCmdInterface::numResIf;
ProfileCmdInterface::Factory *ProfileCmdInterface::resIf;

void ProfileCmdInterface::AddResultFunction(ProfileResultInterface* (*func)(int, const char * const *),
                                            const char *name, const char *arg)
{
  DFAIL_IF(!func) return;
  DFAIL_IF(!name) return;
  for (unsigned k=0;k<numResIf;k++)
    if (!strcmp(resIf[k].name,name))
      return;
  ++numResIf;
  resIf=(Factory *)ProfileReAllocMemory(resIf,numResIf*sizeof(Factory));
  resIf[numResIf-1].func=func;
  resIf[numResIf-1].name=name;
  resIf[numResIf-1].arg=arg;
}

void ProfileCmdInterface::RunResultFunctions(void)
{
  // no result functions registered?
  if (!numResFunc)
    Debug::Command("profile.result file_csv");

  // process result interfaces
  for (unsigned k=0;k<numResFunc;k++)
  {
    resFunc[k]->WriteResults();
    resFunc[k]->Delete();
  }
}

bool ProfileCmdInterface::Execute(class Debug& dbg, const char *cmd, CommandMode cmdmode,
                                  unsigned argn, const char * const * argv)
{
  // just for convenience...
  bool normalMode=cmdmode==CommandMode::Normal;

  if (!strcmp(cmd,"help"))
  {
    if (!normalMode)
      return true;
    
    if (!argn)
    {
      dbg << "profile group help:\n"
             "  result, caller, clear, add, view\n";
      return true;
    }
    else if (!strcmp(argv[0],"result"))
    {
      dbg << "result\n\n"
             "Shows the list of available result functions and their\n"
             "optional parameters.\n"
             "\n"
             "result <res_func_name> [ <arg1> .. <argN> ]\n\n"
             "Adds the given result function to be executed on program\n"
             "exit.\n";
    }
    else if (!strcmp(argv[0],"caller"))
    {
      dbg << "caller [ (+|-) ]\n\n"
             "Enables/disables recording of caller information while\n"
             "performing function level profiling. Turned off by default\n"
             "since CPU hit is non-zero.\n";
    }
    else if (!strcmp(argv[0],"clear"))
    {
      dbg << "clear\n\n"
             "Clears the profile inclusion/exclusion list.\n";
      return true;
    }
    else if (!strcmp(argv[0],"add"))
    {
      dbg << "add (+|-) <pattern>\n"
             "\n"
             "Adds a pattern to the profile list. By default all\n"
             "profile ranges are disabled. Each new range is then checked\n"
             "against all pattern in this list. If a match is found the\n"
             "active/inactive state is modified accordingly (+ for active,\n"
             "- for inactive). The final state is always the last match.";
      return true;
    }
    else if (!strcmp(argv[0],"view"))
    {
      dbg << "view\n\n"
             "Shows the active pattern list.\n";
      return true;
    }
    return false;
  }

  // command: result
  if (!strcmp(cmd,"result"))
  {
    if (!argn)
    {
      unsigned k=0;
      for (;k<numResIf;k++)
      {
        dbg << resIf[k].name;
        if ((resIf[k].arg&&*resIf[k].arg)||!normalMode)
          dbg << "\n  " << resIf[k].arg;
        dbg << "\n";
      }
    }
    else
    {
      unsigned k=0;
      for (;k<numResIf;k++)
        if (!strcmp(argv[0],resIf[k].name))
          break;
      if (k==numResIf)
      {
        dbg << "Unknown result function\n";
        return true;
      }

      ProfileResultInterface *newIf=resIf[k].func(argn-1,argv+1);
      if (!newIf)
      {
        dbg << "Could not add result function\n";
        return true;
      }
      
      ++numResFunc;
      resFunc=(ProfileResultInterface **)ProfileReAllocMemory(resFunc,numResFunc*sizeof(ProfileResultInterface *));
      resFunc[numResFunc-1]=newIf;
      if (normalMode)
        dbg << "Result function " << argv[0] << " added\n";
    }
    
    return true;
  }

  // command: caller
  if (!strcmp(cmd,"caller"))
  {
#ifdef HAS_PROFILE
    if (argn)
    {
      if (*argv[0]=='+')
        ProfileFuncLevelTracer::recordCaller=true;
      if (*argv[0]=='-')
        ProfileFuncLevelTracer::recordCaller=false;
    }
    if (normalMode)
      dbg << "Record caller: " << (ProfileFuncLevelTracer::recordCaller?"on":"off");
    else
      dbg << (ProfileFuncLevelTracer::recordCaller?"1":"0");
#endif

    return true;
  }

  // command: clear
  if (!strcmp(cmd,"clear"))
  {
    // remove some (or all) pattern
    const char *pattern=argn<1?"*":argv[0];
    for (Profile::PatternListEntry **entryPtr=&Profile::firstPatternEntry;*entryPtr;)
    {
      if (Profile::SimpleMatch((*entryPtr)->pattern,pattern))
      {
        // remove this entry
        Profile::PatternListEntry *cur=*entryPtr;
        *entryPtr=cur->next;
        ProfileFreeMemory(cur->pattern);
        ProfileFreeMemory(cur);
      }
      else
        entryPtr=&((*entryPtr)->next);
    }

    // must fixup lastPatternEntry now
    if (Profile::firstPatternEntry)
    {
      Profile::PatternListEntry *cur=Profile::firstPatternEntry;
      for (;cur->next;cur=cur->next);
      Profile::lastPatternEntry=cur;
    }
    else
      Profile::lastPatternEntry=NULL;
    return true;
  }

  // command: add
  if (!strcmp(cmd,"add"))
  {
    // add a pattern
    if (argn<2)
      dbg << "Please specify mode and pattern";
    else
    {
      // alloc new pattern entry
      Profile::PatternListEntry *cur=(Profile::PatternListEntry *)
          ProfileAllocMemory(sizeof(Profile::PatternListEntry));

      // init
      cur->next=NULL;
      cur->isActive=*argv[0]=='+';
      cur->pattern=(char *)ProfileAllocMemory(strlen(argv[1])+1);
      strcpy(cur->pattern,argv[1]);

      // add to list
      if (Profile::lastPatternEntry)
        Profile::lastPatternEntry->next=cur;
      else
        Profile::firstPatternEntry=cur;
      Profile::lastPatternEntry=cur;
    }
    return true;
  }

  // command: view
  if (!strcmp(cmd,"view"))
  {
    // show list of defined patterns
    for (Profile::PatternListEntry *cur=Profile::firstPatternEntry;cur;cur=cur->next)
      dbg << (cur->isActive?"+ ":"- ") << cur->pattern << "\n";
    return true;
  }

  // unknown command
  return false;
}
